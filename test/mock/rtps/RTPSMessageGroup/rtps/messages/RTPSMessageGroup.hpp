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
 * @file RTPSMessageGroup.h
 *
 */

#ifndef FASTDDS_RTPS_MESSAGES__RTPSMESSAGEGROUP_H
#define FASTDDS_RTPS_MESSAGES__RTPSMESSAGEGROUP_H

#include <chrono>

#include <gmock/gmock.h>

#include <rtps/messages/IRTPSMessageGroupLimitation.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class Endpoint;
class RTPSMessageSenderInterface;

class RTPSMessageGroup
{
public:

    class timeout : public std::runtime_error
    {
    public:

        timeout()
            : std::runtime_error("timeout")
        {
        }

        virtual ~timeout() = default;
    };

    RTPSMessageGroup(
            RTPSParticipantImpl*,
            bool)
    {
    }

    RTPSMessageGroup(
            RTPSParticipantImpl*,
            Endpoint*,
            const RTPSMessageSenderInterface*,
            std::chrono::steady_clock::time_point)
    {
    }

    MOCK_METHOD0(flush_and_reset, void());

    void sender(
            Endpoint*,
            RTPSMessageSenderInterface* sender)
    {
        sender_ = sender;
    }

    void set_limitation(
            IRTPSMessageGroupLimitation* limitation)
    {
        limitation_ = limitation;
    }

    IRTPSMessageGroupLimitation* limitation_ {nullptr};
    RTPSMessageSenderInterface* sender_ {nullptr};
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_MESSAGES__RTPSMESSAGEGROUP_H
