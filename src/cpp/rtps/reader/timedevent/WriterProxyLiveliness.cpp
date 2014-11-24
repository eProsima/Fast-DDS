/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxyLiveliness.cpp
 *
 */

#include "fastrtps/reader/timedevent/WriterProxyLiveliness.h"
#include "fastrtps/reader/StatefulReader.h"
#include "fastrtps/reader/WriterProxy.h"
#include "fastrtps/reader/WriterProxyData.h"
#include "fastrtps/resources/ResourceEvent.h"
#include "fastrtps/utils/RTPSLog.h"

#include "fastrtps/pubsub/SubscriberListener.h"



namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "WriterProxyLiveliness";

WriterProxyLiveliness::WriterProxyLiveliness(WriterProxy* wp,boost::posix_time::milliseconds interval):
				TimedEvent(&wp->mp_SFR->mp_event_thr->io_service,interval),
				mp_WP(wp)
{

}

WriterProxyLiveliness::~WriterProxyLiveliness()
{
	stop_timer();
	delete(timer);
}

void WriterProxyLiveliness::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
	
		logInfo(RTPS_LIVELINESS,"Checking Writer: "<<mp_WP->m_data->m_guid,RTPS_MAGENTA);
		if(!mp_WP->m_data->m_isAlive)
		{
			logWarning(RTPS_LIVELINESS,"Liveliness failed, leaseDuration was "<< this->getIntervalMsec().total_milliseconds()<< " ms",RTPS_MAGENTA);
			if(mp_WP->mp_SFR->matched_writer_remove(mp_WP->m_data))
			{
				if(mp_WP->mp_SFR->getListener()!=NULL)
				{
					MatchingInfo info(REMOVED_MATCHING,mp_WP->m_data->m_guid);
					mp_WP->mp_SFR->getListener()->onSubscriptionMatched(info);
				}
			}
			return;
		}
		this->restart_timer();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_LIVELINESS,"Aborted");
		this->mp_stopSemaphore->post();
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"boost message: " <<ec.message());
	}
}



} /* namespace rtps */
} /* namespace eprosima */
