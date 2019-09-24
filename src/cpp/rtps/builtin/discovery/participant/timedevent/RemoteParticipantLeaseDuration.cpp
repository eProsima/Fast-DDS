// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file RTPSParticipantLeaseDuration.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include "../../../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/utils/TimeConversion.h>

#include <fastrtps/log/Log.h>

#include <mutex>
#include <chrono>



namespace eprosima {
namespace fastrtps{
namespace rtps {


RemoteParticipantLeaseDuration::RemoteParticipantLeaseDuration(PDPSimple* p_SPDP,
        ParticipantProxyData* pdata,
        double interval)
    : TimedEvent(p_SPDP->getRTPSParticipant()->getEventResource().getIOService(),
            p_SPDP->getRTPSParticipant()->getEventResource().getThread(), interval)
    , mp_PDP(p_SPDP)
    , mp_participantProxyData(pdata)
    {
        lease_duration_ = std::chrono::microseconds(
                TimeConv::Duration_t2MicroSecondsInt64(mp_participantProxyData->m_leaseDuration));

    }

RemoteParticipantLeaseDuration::~RemoteParticipantLeaseDuration()
{
    destroy();
}

void RemoteParticipantLeaseDuration::event(EventCode code, const char* msg)
{
    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        // Check last received message's time_point plus lease duration time doesn't overcome now().
        // If overcame, remove participant.
        auto now = std::chrono::steady_clock::now();
        auto real_lease_tm = last_received_message_tm_ + lease_duration_;
        if (now > real_lease_tm)
        {
            logInfo(RTPS_LIVELINESS,"RTPSParticipant no longer ALIVE, trying to remove: "
                    << mp_participantProxyData->m_guid);

            // This assignment must be before removeRemoteParticipant because mp_participantProxyData is deleted there.
            ParticipantDiscoveryInfo info;
            info.status = ParticipantDiscoveryInfo::DROPPED_PARTICIPANT;
            info.info.copy(*mp_participantProxyData);

            // Set pointer to null because this call will be delete itself.
            mp_participantProxyData->mp_leaseDurationTimer = nullptr;
            if(mp_PDP->removeRemoteParticipant(mp_participantProxyData->m_guid))
            {
                if(mp_PDP->getRTPSParticipant()->getListener()!=nullptr)
                {
                    mp_PDP->getRTPSParticipant()->getListener()->onParticipantDiscovery(
                            mp_PDP->getRTPSParticipant()->getUserRTPSParticipant(), std::move(info));
                }
            }
            mark_for_destruction();
            return;
        }

        // Calculate next trigger.
        auto next_trigger = real_lease_tm - now;
        update_interval_millisec((double)std::chrono::duration_cast<std::chrono::milliseconds>(next_trigger).count());
        restart_timer();
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_LIVELINESS," Stopped for "<<mp_participantProxyData->m_participantName
                << " with ID: "<< mp_participantProxyData->m_guid.guidPrefix);
    }
    else
    {
        logInfo(RTPS_LIVELINESS,"message: " <<msg);
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
