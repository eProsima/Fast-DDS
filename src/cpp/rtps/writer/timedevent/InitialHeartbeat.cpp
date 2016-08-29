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
 * @file InitialHeartbeat.cpp
 *
 */

#include "InitialHeartbeat.h"
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <fastrtps/log/Log.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps{


InitialHeartbeat::~InitialHeartbeat()
{
	logInfo(RTPS_WRITER,"Destroying InitialHB");
    destroy();
}

InitialHeartbeat::InitialHeartbeat(RTPSParticipantImpl* participant, StatefulWriter* sfw,
        ReaderProxy& remote_reader, double interval):
    TimedEvent(participant->getEventResource().getIOService(),
            participant->getEventResource().getThread(), interval, TimedEvent::ALLWAYS),
    participant_(participant), sfw_guid_(sfw->getGuid()), remote_reader_guid_(remote_reader.m_att.guid),
    remote_reader_multicast_locators_(remote_reader.m_att.endpoint.multicastLocatorList),
    remote_reader_unicast_locators_(remote_reader.m_att.endpoint.unicastLocatorList)
{
}

void InitialHeartbeat::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
		SequenceNumber_t firstSeq, lastSeq;
		Count_t heartbeatCount = 0;
		LocatorList_t locList;
        StatefulWriter* sfw = nullptr;

		{//BEGIN PROTECTION
            boost::lock_guard<boost::recursive_mutex> guard_participant(*participant_->getParticipantMutex());

            for(auto sfwit : participant_->getAllWriters())
            {
                if(sfwit->getGuid() == sfw_guid_)
                {
                    sfw = static_cast<StatefulWriter*>(sfwit);
                    break;
                }
            }

            if(sfw != nullptr)
            {
                boost::lock_guard<boost::recursive_mutex> guard_writer(*sfw->getMutex());

				firstSeq = sfw->get_seq_num_min();
				lastSeq = sfw->get_seq_num_max();

                if(firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
                {
					firstSeq = sfw->next_sequence_number();
                    lastSeq = SequenceNumber_t(0, 0);
                }
                else
                {
                    (void)firstSeq;
                    assert(firstSeq <= lastSeq);
                }

				sfw->incrementHBCount();
				heartbeatCount = sfw->getHeartbeatCount();
            }
            else
                return;
		}

        CDRMessage::initCDRMsg(&initial_hb_msg_);
        // FinalFlag is always false because this is a StatefulWriter in Reliable.
        RTPSMessageCreator::addMessageHeartbeat(&initial_hb_msg_, sfw_guid_.guidPrefix, remote_reader_guid_.guidPrefix,
                remote_reader_guid_.entityId, sfw_guid_.entityId,
                firstSeq, lastSeq, heartbeatCount, false, false);
        logInfo(RTPS_WRITER, sfw_guid_.entityId << " Sending Heartbeat (" << firstSeq << " - " << lastSeq << ")");
        for (auto lit = remote_reader_multicast_locators_.begin(); lit != remote_reader_multicast_locators_.end(); ++lit)
            participant_->sendSync(&initial_hb_msg_, (Endpoint *)sfw, (*lit));
        for (auto lit = remote_reader_unicast_locators_.begin(); lit != remote_reader_unicast_locators_.end(); ++lit)
            participant_->sendSync(&initial_hb_msg_, (Endpoint *)sfw, (*lit));
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
