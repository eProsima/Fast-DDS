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
 * @file HandshakeMessageTokenResent.h
 *
*/

#ifndef _RTPS_SECURITY_TIMEDEVENT_HANDSHAKEMESSAGETOKENRESENT_H_
#define _RTPS_SECURITY_TIMEDEVENT_HANDSHAKEMESSAGETOKENRESENT_H_

#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/rtps/common/Guid.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class SecurityManager;

class HandshakeMessageTokenResent : public TimedEvent
{
    public:

        HandshakeMessageTokenResent(SecurityManager& security_manager, const GUID_t& remote_participant_key, double interval);

        virtual ~HandshakeMessageTokenResent();

        void event(EventCode code, const char* msg = nullptr);

    private:

        HandshakeMessageTokenResent& operator=(const HandshakeMessageTokenResent&) = delete;

        SecurityManager& security_manager_;

        GUID_t remote_participant_key_;
};

} // namespace security
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_SECURITY_TIMEDEVENT_HANDSHAKEMESSAGETOKENRESENT_H_
