/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipantLeaseDuration.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include "../../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/utils/RTPSLog.h>

#include <boost/thread/recursive_mutex.hpp>



namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "RemoteParticipantLeaseDuration";

RemoteParticipantLeaseDuration::RemoteParticipantLeaseDuration(PDPSimple* p_SPDP,
		ParticipantProxyData* pdata,
		double interval):
				TimedEvent(p_SPDP->getRTPSParticipant()->getEventResource().getIOService(),
                p_SPDP->getRTPSParticipant()->getEventResource().getThread(), interval, TimedEvent::ON_SUCCESS),
				mp_PDP(p_SPDP),
				mp_participantProxyData(pdata)
{

}

RemoteParticipantLeaseDuration::~RemoteParticipantLeaseDuration()
{
    destroy();
}

void RemoteParticipantLeaseDuration::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_LIVELINESS,"RTPSParticipant no longer ALIVE, trying to remove: "
                << mp_participantProxyData->m_guid,C_MAGENTA);
        // Set pointer to null because this call will be delete itself.
        mp_participantProxyData->mp_mutex->lock();
        mp_participantProxyData->mp_leaseDurationTimer = nullptr;
        mp_participantProxyData->mp_mutex->unlock();
        mp_PDP->removeRemoteParticipant(mp_participantProxyData->m_guid);
	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_LIVELINESS," Stopped for "<<mp_participantProxyData->m_participantName
				<< " with ID: "<< mp_participantProxyData->m_guid.guidPrefix,C_MAGENTA);
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"boost message: " <<msg,C_MAGENTA);
	}
}

}
} /* namespace rtps */
} /* namespace eprosima */
