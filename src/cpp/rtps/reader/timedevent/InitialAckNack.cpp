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
 * @file InitialAckNack.cpp
 *
 */

#include <fastrtps/rtps/reader/timedevent/InitialAckNack.h>
#include <mutex>

#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps{

// TODO(Ricardo) Maybe join initial heartbeat, ack and gap

InitialAckNack::~InitialAckNack()
{
    logInfo(RTPS_WRITER,"Destroying InitialAckNack");
    destroy();
}

InitialAckNack::InitialAckNack(
        WriterProxy* wp,
        double interval)
    : TimedEvent(wp->mp_SFR->getRTPSParticipant()->getEventResource().getIOService(),
            wp->mp_SFR->getRTPSParticipant()->getEventResource().getThread(), interval)
    , m_cdrmessages(wp->mp_SFR->getRTPSParticipant()->getMaxMessageSize(),
            wp->mp_SFR->getRTPSParticipant()->getGuid().guidPrefix)
    , wp_(wp)
    , m_destination_locators(wp->mp_SFR->getRTPSParticipant()->network_factory().
            ShrinkLocatorLists({wp->m_att.endpoint.unicastLocatorList}))
    , m_remote_endpoints(1, wp->m_att.guid)
{
    if(m_destination_locators.empty())
    {
        m_destination_locators.push_back(wp->m_att.endpoint.multicastLocatorList);
    }
}

void InitialAckNack::event(
        EventCode code,
        const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        Count_t acknackCount = 0;

        {//BEGIN PROTECTION
            std::lock_guard<std::recursive_timed_mutex> guard_reader(wp_->mp_SFR->getMutex());
            wp_->mp_SFR->m_acknackCount++;
            acknackCount = wp_->mp_SFR->m_acknackCount;
        }

        // Send initial NACK.
        SequenceNumberSet_t sns(SequenceNumber_t(0, 0));

        logInfo(RTPS_READER,"Sending ACKNACK: "<< sns);

        RTPSMessageGroup group(wp_->mp_SFR->getRTPSParticipant(), wp_->mp_SFR, RTPSMessageGroup::READER, m_cdrmessages,
            m_destination_locators, m_remote_endpoints);

        group.add_acknack(m_remote_endpoints, sns, acknackCount, false, m_destination_locators);
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_WRITER,"Aborted");
    }
    else
    {
        logInfo(RTPS_WRITER,"Event message: " <<msg);
    }
}

}
}
} /* namespace eprosima */
