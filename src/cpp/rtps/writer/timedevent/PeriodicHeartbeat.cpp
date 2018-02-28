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
 * @file PeriodicHeartbeat.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <fastrtps/log/Log.h>

#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps{


PeriodicHeartbeat::~PeriodicHeartbeat()
{
    logInfo(RTPS_WRITER,"Destroying PeriodicHB");
    destroy();
}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* p_SFW, double interval):
    TimedEvent(p_SFW->getRTPSParticipant()->getEventResource().getIOService(),
            p_SFW->getRTPSParticipant()->getEventResource().getThread(), interval),
    m_cdrmessages(p_SFW->getRTPSParticipant()->getMaxMessageSize(),
            p_SFW->getRTPSParticipant()->getGuid().guidPrefix), mp_SFW(p_SFW)
{

}

void PeriodicHeartbeat::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        SequenceNumber_t firstSeq, lastSeq;
        Count_t heartbeatCount = 0;
        LocatorList_t locList;
        bool unacked_changes = false;
        std::vector<GUID_t> remote_readers;

        {//BEGIN PROTECTION
            std::lock_guard<std::recursive_mutex> guardW(*mp_SFW->getMutex());
            for(std::vector<ReaderProxy*>::iterator it = mp_SFW->matchedReadersBegin();
                    it != mp_SFW->matchedReadersEnd(); ++it)
            {
                if(!unacked_changes)
                {
                    if((*it)->thereIsUnacknowledged())
                    {
                        unacked_changes= true;
                    }
                }
                locList.push_back((*it)->m_att.endpoint.unicastLocatorList);
                locList.push_back((*it)->m_att.endpoint.multicastLocatorList);
                remote_readers.push_back((*it)->m_att.guid);
            }

            if (unacked_changes)
            {
                firstSeq = mp_SFW->get_seq_num_min();
                lastSeq = mp_SFW->get_seq_num_max();

                if (firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
                {
                    return;
                }
                else
                {
                    (void)firstSeq;
                    assert(firstSeq <= lastSeq);
                }

                mp_SFW->incrementHBCount();
                heartbeatCount = mp_SFW->getHeartbeatCount();

                // TODO(Ricardo) Use StatefulWriter::send_heartbeat_to_nts.
            }
        }

        if (unacked_changes)
        {
            RTPSMessageGroup group(mp_SFW->getRTPSParticipant(), mp_SFW, RTPSMessageGroup::WRITER, m_cdrmessages);

            // FinalFlag is always false because this class is used only by StatefulWriter in Reliable.
            group.add_heartbeat(remote_readers,
                    firstSeq, lastSeq, heartbeatCount, false, false, locList);
            logInfo(RTPS_WRITER,mp_SFW->getGuid().entityId << " Sending Heartbeat ("<<firstSeq<< " - " << lastSeq<<")" );

            //Reset TIMER
            this->restart_timer();
        }

    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_WRITER,"Aborted");
    }
    else
    {
        logInfo(RTPS_WRITER,"Event message: " << msg);
    }
}

}
}
} /* namespace eprosima */
