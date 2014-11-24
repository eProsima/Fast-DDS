/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipantLeaseDuration.cpp
 *
 */

#include "fastrtps/builtin/discovery/RTPSParticipant/timedevent/RemoteRTPSParticipantLeaseDuration.h"
#include "fastrtps/resources/ResourceEvent.h"
#include "fastrtps/builtin/discovery/RTPSParticipant/PDPSimple.h"
#include "fastrtps/RTPSParticipantProxyData.h"
#include "fastrtps/utils/RTPSLog.h"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "RemoteRTPSParticipantLeaseDuration";

RemoteRTPSParticipantLeaseDuration::RemoteRTPSParticipantLeaseDuration(PDPSimple* pPDP,
		RTPSParticipantProxyData* pdata,
		ResourceEvent* pEvent,
		boost::posix_time::milliseconds interval):
				TimedEvent(&pEvent->io_service,interval),
				mp_PDP(pPDP),
				mp_RTPSParticipantProxyData(pdata)
{

}

RemoteRTPSParticipantLeaseDuration::~RemoteRTPSParticipantLeaseDuration()
{
	stop_timer();
	delete(timer);
}

void RemoteRTPSParticipantLeaseDuration::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		logInfo(RTPS_LIVELINESS,"Checking RTPSParticipant: "
				<< mp_RTPSParticipantProxyData->m_RTPSParticipantName << " with GUID: "
				<< mp_RTPSParticipantProxyData->m_guid,EPRO_MAGENTA);
		if(mp_RTPSParticipantProxyData->isAlive)
			mp_RTPSParticipantProxyData->isAlive = false;
		else
		{
			logInfo(RTPS_LIVELINESS,"RTPSParticipant no longer ALIVE, trying to remove: "
					<< mp_RTPSParticipantProxyData->m_guid,EPRO_MAGENTA);
			mp_PDP->removeRemoteRTPSParticipant(mp_RTPSParticipantProxyData->m_guid);
			return;
		}
		this->restart_timer();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_LIVELINESS,"Remote RTPSParticipant Lease Duration aborted",EPRO_MAGENTA);
		this->mp_stopSemaphore->post();
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"boost message: " <<ec.message(),EPRO_MAGENTA);
	}
}


} /* namespace rtps */
} /* namespace eprosima */
