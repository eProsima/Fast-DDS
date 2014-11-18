/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantLeaseDuration.cpp
 *
 */

#include "eprosimartps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h"
#include "eprosimartps/resources/ResourceEvent.h"
#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"
#include "eprosimartps/ParticipantProxyData.h"
#include "eprosimartps/utils/RTPSLog.h"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "RemoteParticipantLeaseDuration";

RemoteParticipantLeaseDuration::RemoteParticipantLeaseDuration(PDPSimple* pPDP,
		ParticipantProxyData* pdata,
		ResourceEvent* pEvent,
		boost::posix_time::milliseconds interval):
				TimedEvent(&pEvent->io_service,interval),
				mp_PDP(pPDP),
				mp_participantProxyData(pdata)
{

}

RemoteParticipantLeaseDuration::~RemoteParticipantLeaseDuration()
{
	stop_timer();
	delete(timer);
}

void RemoteParticipantLeaseDuration::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		logInfo(RTPS_LIVELINESS,"Checking participant: "
				<< mp_participantProxyData->m_participantName << " with GUID: "
				<< mp_participantProxyData->m_guid,EPRO_MAGENTA);
		if(mp_participantProxyData->isAlive)
			mp_participantProxyData->isAlive = false;
		else
		{
			logInfo(RTPS_LIVELINESS,"Participant no longer ALIVE, trying to remove: "
					<< mp_participantProxyData->m_guid,EPRO_MAGENTA);
			mp_PDP->removeRemoteParticipant(mp_participantProxyData->m_guid);
			return;
		}
		this->restart_timer();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_LIVELINESS,"Remote Participant Lease Duration aborted",EPRO_MAGENTA);
		this->mp_stopSemaphore->post();
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"boost message: " <<ec.message(),EPRO_MAGENTA);
	}
}


} /* namespace rtps */
} /* namespace eprosima */
