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

#include "InitialAckNack.h"
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <fastrtps/log/Log.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps{


InitialAckNack::~InitialAckNack()
{
	logInfo(RTPS_WRITER,"Destroying InitialAckNack");
    destroy();
}

InitialAckNack::InitialAckNack(RTPSParticipantImpl* participant, StatefulReader* sfr,
        WriterProxy& remote_writer, double interval):
    TimedEvent(participant->getEventResource().getIOService(),
            participant->getEventResource().getThread(), interval, TimedEvent::ALLWAYS),
    participant_(participant), sfr_guid_(sfr->getGuid()), remote_writer_guid_(remote_writer.m_att.guid)
{
}

void InitialAckNack::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
		Count_t acknackCount = 0;
        StatefulReader* sfr = nullptr;
        WriterProxy* remote_writer = nullptr;

		{//BEGIN PROTECTION
            boost::lock_guard<boost::recursive_mutex> guard_participant(*participant_->getParticipantMutex());

            for(auto sfrit : participant_->getAllReaders())
            {
                if(sfrit->getGuid() == sfr_guid_)
                {
                    sfr = static_cast<StatefulReader*>(sfrit);
                    break;
                }
            }

            if(sfr != nullptr)
            {
                boost::lock_guard<boost::recursive_mutex> guard_reader(*sfr->getMutex());

                sfr-> matched_writer_lookup(remote_writer_guid_, &remote_writer);

                if(remote_writer != nullptr)
                {
                    remote_writer->m_acknackCount++;
                    acknackCount = remote_writer->m_acknackCount;
                }
                else
                    return;
            }
            else
                return;
		}

        // Send initial NACK.
        SequenceNumberSet_t sns;
        sns.base = SequenceNumber_t(0, 0);

        logInfo(RTPS_READER,"Sending ACKNACK: "<< sns;);

        CDRMessage::initCDRMsg(&initial_acknack_msg_);
        RTPSMessageCreator::addMessageAcknack(&initial_acknack_msg_,
                sfr_guid_.guidPrefix,
                remote_writer_guid_.guidPrefix,
                sfr_guid_.entityId,
                remote_writer_guid_.entityId,
                sns,
                acknackCount,
                false);

        for(auto lit = remote_writer->m_att.endpoint.unicastLocatorList.begin();
                lit != remote_writer->m_att.endpoint.unicastLocatorList.end(); ++lit)
            participant_->sendSync(&initial_acknack_msg_, static_cast<Endpoint *>(sfr), (*lit));

        for(auto lit = remote_writer->m_att.endpoint.multicastLocatorList.begin();
                lit != remote_writer->m_att.endpoint.multicastLocatorList.end(); ++lit)
            participant_->sendSync(&initial_acknack_msg_, static_cast<Endpoint *>(sfr),(*lit));
	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_WRITER,"Aborted");
	}
	else
	{
		logInfo(RTPS_WRITER,"Boost message: " <<msg);
	}
}

}
}
} /* namespace eprosima */
