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

#include "HandshakeMessageTokenResent.h"
#include "../SecurityManager.h"
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::rtps::security;

HandshakeMessageTokenResent::HandshakeMessageTokenResent(SecurityManager& security_manager,
        const GUID_t& remote_participant_key, double interval) :
    TimedEvent(security_manager.participant()->getEventResource().getIOService(),
            security_manager.participant()->getEventResource().getThread(), interval),
    security_manager_(security_manager), remote_participant_key_(remote_participant_key)
{
}

HandshakeMessageTokenResent::~HandshakeMessageTokenResent()
{
    destroy();
}

void HandshakeMessageTokenResent::event(EventCode code, const char* msg)
{
    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        security_manager_.mutex_.lock();
        auto dp_it = security_manager_.discovered_participants_.find(remote_participant_key_);

        if(dp_it != security_manager_.discovered_participants_.end())
        {
            SecurityManager::DiscoveredParticipantInfo::AuthUniquePtr remote_participant_info = dp_it->second.get_auth();

            if(remote_participant_info)
            {
                if(remote_participant_info->change_sequence_number_ != SequenceNumber_t::unknown())
                {
                    CacheChange_t* p_change = security_manager_.participant_stateless_message_writer_history_->remove_change_and_reuse(
                            remote_participant_info->change_sequence_number_);
                    remote_participant_info->change_sequence_number_ = SequenceNumber_t::unknown();

                    if(p_change != nullptr)
                    {
                        logInfo(SECURITY, "Authentication handshake resent to participant " <<
                                remote_participant_key_);
                        if(security_manager_.participant_stateless_message_writer_history_->add_change(p_change))
                        {
                            remote_participant_info->change_sequence_number_ = p_change->sequenceNumber;
                        }
                        //TODO (Ricardo) What to do if not added?
                    }
                }

                dp_it->second.set_auth(remote_participant_info);
            }
        }

        security_manager_.mutex_.unlock();

        this->restart_timer();
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(SECURITY, "HandshakeMessageTokenResent aborted");
    }
    else
    {
        logInfo(SECURITY, "HandshakeMessageTokenResent event message: " <<msg);
    }
}
