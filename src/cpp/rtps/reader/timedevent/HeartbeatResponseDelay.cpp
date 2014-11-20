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
#include "eprosimartps/reader/WriterProxyData.h"
#include "eprosimartps/reader/StatefulReader.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/RTPSMessageCreator.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "HeartbeatResponseDelay";

HeartbeatResponseDelay::~HeartbeatResponseDelay()
{
	stop_timer();
	delete(timer);
}

HeartbeatResponseDelay::HeartbeatResponseDelay(WriterProxy* p_WP,boost::posix_time::milliseconds interval):
		TimedEvent(&p_WP->mp_SFR->mp_event_thr->io_service,interval),
		mp_WP(p_WP)
{

}

void HeartbeatResponseDelay::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		logInfo(RTPS_READER,"");
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
				logError(RTPS_READER,"No available changes max"<<endl;);
			}
			sns.base++;
			std::vector<ChangeFromWriter_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				if(!sns.add((*cit)->seqNum))
				{
					logWarning(RTPS_READER,"Error adding seqNum "<<(*cit)->seqNum.to64long()
							<< " with SeqNumSet Base: "<< sns.base.to64long());
					break;
				}
			}
			mp_WP->m_acknackCount++;
			logInfo(RTPS_READER,"Sending ACKNACK: "<< sns;);

			bool final = false;
			if(sns.isSetEmpty())
				final = true;
			CDRMessage::initCDRMsg(&m_heartbeat_response_msg);
			RTPSMessageCreator::addMessageAcknack(&m_heartbeat_response_msg,
												mp_WP->mp_SFR->getGuid().guidPrefix,
												mp_WP->mp_SFR->getGuid().entityId,
												mp_WP->m_data->m_guid.entityId,
												sns,
												mp_WP->m_acknackCount,
												final);

			std::vector<Locator_t>::iterator lit;

			for(lit = mp_WP->m_data->m_unicastLocatorList.begin();lit!=mp_WP->m_data->m_unicastLocatorList.end();++lit)
				mp_WP->mp_SFR->mp_send_thr->sendSync(&m_heartbeat_response_msg,(*lit));

			for(lit = mp_WP->m_data->m_multicastLocatorList.begin();lit!=mp_WP->m_data->m_multicastLocatorList.end();++lit)
				mp_WP->mp_SFR->mp_send_thr->sendSync(&m_heartbeat_response_msg,(*lit));

		}

	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_READER,"Response aborted");
		this->mp_stopSemaphore->post();
	}
	else
	{
		logInfo(RTPS_READER,"Response boost message: " <<ec.message());
	}
}




} /* namespace rtps */
} /* namespace eprosima */
