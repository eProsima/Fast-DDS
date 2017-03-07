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


InitialAckNack::~InitialAckNack()
{
	logInfo(RTPS_WRITER,"Destroying InitialAckNack");
    destroy();
}

InitialAckNack::InitialAckNack(WriterProxy* wp, double interval):
    TimedEvent(wp->mp_SFR->getRTPSParticipant()->getEventResource().getIOService(),
            wp->mp_SFR->getRTPSParticipant()->getEventResource().getThread(), interval),
    wp_(wp)
{
}

void InitialAckNack::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
		Count_t acknackCount = 0;

		{//BEGIN PROTECTION
            std::lock_guard<std::recursive_mutex> guard_reader(*wp_->getMutex());
            wp_->m_acknackCount++;
            acknackCount = wp_->m_acknackCount;
		}

        // Send initial NACK.
        SequenceNumberSet_t sns;
        sns.base = SequenceNumber_t(0, 0);

        logInfo(RTPS_READER,"Sending ACKNACK: "<< sns;);

        CDRMessage::initCDRMsg(&initial_acknack_msg_);
        RTPSMessageCreator::addMessageAcknack(&initial_acknack_msg_,
                wp_->mp_SFR->getGuid().guidPrefix,
                wp_->m_att.guid.guidPrefix,
                wp_->mp_SFR->getGuid().entityId,
                wp_->m_att.guid.entityId,
                sns,
                acknackCount,
                false);

        for(auto lit = wp_->m_att.endpoint.unicastLocatorList.begin();
                lit != wp_->m_att.endpoint.unicastLocatorList.end(); ++lit)
            wp_->mp_SFR->getRTPSParticipant()->sendSync(&initial_acknack_msg_, static_cast<Endpoint *>(wp_->mp_SFR), (*lit));

        for(auto lit = wp_->m_att.endpoint.multicastLocatorList.begin();
                lit != wp_->m_att.endpoint.multicastLocatorList.end(); ++lit)
            wp_->mp_SFR->getRTPSParticipant()->sendSync(&initial_acknack_msg_, static_cast<Endpoint *>(wp_->mp_SFR),(*lit));
	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_WRITER,"Aborted");
	}
	else
	{
		logInfo(RTPS_WRITER,"Message: " <<msg);
	}
}

}
}
} /* namespace eprosima */
