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
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/timedevent/HeartbeatResponseDelay.h"
#include "eprosimartps/reader/WriterProxy.h"
#include "eprosimartps/reader/StatefulReader.h"

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
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("HeartbeatResponse:event: Sending ACKNACK"<<endl);
		std::vector<ChangeFromWriter_t*> ch_vec;
		{
			boost::lock_guard<WriterProxy> guard(*mp_WP);
			mp_WP->missing_changes(&ch_vec);
		}
//		cout << "Missing changes: " << ch_vec.size() << " changesformW " << mp_WP->m_changesFromW.size() << endl;
		if(!ch_vec.empty() || !mp_WP->m_heartbeatFinalFlag)
		{
			SequenceNumberSet_t sns;
			if(!mp_WP->available_changes_max(&sns.base)) //if no changes are available
			{
				pError("HeartbeatResponse: event: no available changes max"<<endl;);
			}
			if(sns.base < mp_WP->m_lastRemovedSeqNum)
				sns.base = mp_WP->m_lastRemovedSeqNum;
			sns.base++;
			std::vector<ChangeFromWriter_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				if(!sns.add((*cit)->change->sequenceNumber))
				{
					pWarning("HBResponse:event:error adding seqNum"<<endl;);
				}
			}
			mp_WP->m_acknackCount++;
			CDRMessage::initCDRMsg(&m_heartbeat_response_msg);
			RTPSMessageCreator::addMessageAcknack(&m_heartbeat_response_msg,
												mp_WP->mp_SFR->m_guid.guidPrefix,
												mp_WP->mp_SFR->m_guid.entityId,
												mp_WP->param.remoteWriterGuid.entityId,
												sns,
												mp_WP->m_acknackCount,
												false);

			std::vector<Locator_t>::iterator lit;

			for(lit = mp_WP->param.unicastLocatorList.begin();lit!=mp_WP->param.unicastLocatorList.end();++lit)
				mp_WP->mp_SFR->mp_send_thr->sendSync(&m_heartbeat_response_msg,&(*lit));

			for(lit = mp_WP->param.multicastLocatorList.begin();lit!=mp_WP->param.multicastLocatorList.end();++lit)
				mp_WP->mp_SFR->mp_send_thr->sendSync(&m_heartbeat_response_msg,&(*lit));

		}
		m_isWaiting = false;
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
