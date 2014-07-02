/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HeartbeatResponseDelay.cpp
 *
 */

#include "eprosimartps/reader/timedevent/HeartbeatResponseDelay.h"
#include "eprosimartps/reader/WriterProxy.h"
#include "eprosimartps/reader/StatefulReader.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/RTPSMessageCreator.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

HeartbeatResponseDelay::~HeartbeatResponseDelay()
{
	timer->cancel();
	delete(timer);
}

HeartbeatResponseDelay::HeartbeatResponseDelay(WriterProxy* p_WP,boost::posix_time::milliseconds interval):
		TimedEvent(&p_WP->mp_SFR->mp_event_thr->io_service,interval),
		mp_WP(p_WP)
{

}

void HeartbeatResponseDelay::event(const boost::system::error_code& ec)
{
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("HeartbeatResponse:event:"<<endl;);
		std::vector<ChangeFromWriter_t*> ch_vec;
		{
			boost::lock_guard<WriterProxy> guard(*mp_WP);
			mp_WP->missing_changes(&ch_vec);
		}
		if(!ch_vec.empty() || !mp_WP->m_heartbeatFinalFlag)
		{
			SequenceNumberSet_t sns;
			if(!mp_WP->available_changes_max(&sns.base)) //if no changes are available
			{
				pError("HeartbeatResponse: event: no available changes max"<<endl;);
			}
			sns.base++;
			std::vector<ChangeFromWriter_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				if(!sns.add((*cit)->seqNum))
				{
					pWarning("HBResponse:event:error adding seqNum "<<(*cit)->seqNum.to64long()<< " with SeqNumSet Base: "<< sns.base.to64long()<< endl;);
				}
			}
			mp_WP->m_acknackCount++;
			pDebugInfo("Sending ACKNACK: "<< sns <<endl;);
			CDRMessage::initCDRMsg(&m_heartbeat_response_msg);
			RTPSMessageCreator::addHeader(&m_heartbeat_response_msg,mp_WP->mp_SFR->getGuid().guidPrefix);
			RTPSMessageCreator::addSubmessageInfoDST(&m_heartbeat_response_msg,mp_WP->param.remoteWriterGuid.guidPrefix);
			RTPSMessageCreator::addSubmessageAcknack(&m_heartbeat_response_msg,
												mp_WP->mp_SFR->getGuid().entityId,
												mp_WP->param.remoteWriterGuid.entityId,
												sns,
												mp_WP->m_acknackCount,
												false);

			std::vector<Locator_t>::iterator lit;

			for(lit = mp_WP->param.unicastLocatorList.begin();lit!=mp_WP->param.unicastLocatorList.end();++lit)
				mp_WP->mp_SFR->mp_send_thr->sendSync(&m_heartbeat_response_msg,(*lit));

			for(lit = mp_WP->param.multicastLocatorList.begin();lit!=mp_WP->param.multicastLocatorList.end();++lit)
				mp_WP->mp_SFR->mp_send_thr->sendSync(&m_heartbeat_response_msg,(*lit));

		}

	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pInfo("HB response aborted"<<endl);
	}
	else
	{
		pInfo("HB response boost message: " <<ec.message()<<endl);
	}
}




} /* namespace rtps */
} /* namespace eprosima */
