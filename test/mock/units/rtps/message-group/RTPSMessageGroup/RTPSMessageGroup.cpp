// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of RTPSMessageGroup: replaces src/cpp/rtps/messages/RTPSMessageGroup.cpp. Submessages are
// accepted but never serialized, and nothing is sent.

#include <rtps/messages/RTPSMessageGroup.hpp>

#include <rtps/messages/RTPSMessageGroup_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

RTPSMessageGroup::RTPSMessageGroup(
        RTPSParticipantImpl* participant,
        bool internal_buffer,
        std::chrono::steady_clock::time_point max_blocking_time_point)
    : participant_(participant)
    , max_blocking_time_point_(max_blocking_time_point)
    , internal_buffer_(internal_buffer)
{
}

RTPSMessageGroup::RTPSMessageGroup(
        RTPSParticipantImpl* participant,
        Endpoint* endpoint,
        RTPSMessageSenderInterface* msg_sender,
        std::chrono::steady_clock::time_point max_blocking_time_point)
    : sender_(msg_sender)
    , endpoint_(endpoint)
    , participant_(participant)
    , max_blocking_time_point_(max_blocking_time_point)
{
}

RTPSMessageGroup::~RTPSMessageGroup() noexcept(false)
{
}

bool RTPSMessageGroup::add_data(
        CacheChange_t&,
        bool)
{
    return true;
}

bool RTPSMessageGroup::add_data_frag(
        CacheChange_t&,
        const uint32_t,
        bool)
{
    return true;
}

bool RTPSMessageGroup::add_heartbeat(
        const SequenceNumber_t&,
        const SequenceNumber_t&,
        Count_t,
        bool,
        bool)
{
    return true;
}

bool RTPSMessageGroup::add_gap(
        std::set<SequenceNumber_t>&)
{
    return true;
}

bool RTPSMessageGroup::add_gap(
        const SequenceNumber_t&,
        const SequenceNumberSet_t&)
{
    return true;
}

bool RTPSMessageGroup::add_gap(
        const SequenceNumber_t&,
        const SequenceNumberSet_t&,
        const GUID_t&)
{
    return true;
}

bool RTPSMessageGroup::add_acknack(
        const SequenceNumberSet_t&,
        int32_t,
        bool)
{
    return true;
}

bool RTPSMessageGroup::add_nackfrag(
        const SequenceNumber_t&,
        FragmentNumberSet_t,
        int32_t)
{
    return true;
}

void RTPSMessageGroup::flush_and_reset()
{
}

void RTPSMessageGroup::sender(
        Endpoint* endpoint,
        RTPSMessageSenderInterface* msg_sender)
{
    endpoint_ = endpoint;
    sender_ = msg_sender;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
