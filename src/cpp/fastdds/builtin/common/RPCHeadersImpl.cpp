// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RPCHeadersImpl.cpp
 *
 */

#include <fastdds/dds/builtin/common/ReplyHeader.hpp>
#include <fastdds/dds/builtin/common/RequestHeader.hpp>
#include <fastcdr/Cdr.h>

using namespace eprosima::fastdds::dds::rpc;

size_t ReplyHeader::getCdrSerializedSize(
        const ReplyHeader& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 16); // SampleIdentity.GUID_t
    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8); // SampleIdentity.SequenceNumber_t
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // RemoteException

    return current_alignment - initial_alignment;
}

void ReplyHeader::serialize(
        eprosima::fastcdr::Cdr &scdr) const
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        scdr << relatedRequestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        scdr << relatedRequestId.writer_guid().entityId.value[i];
    }

    scdr << relatedRequestId.sequence_number().high;
    scdr << relatedRequestId.sequence_number().low;

    scdr << static_cast<uint32_t>(remoteEx);
}

void ReplyHeader::deserialize(
        eprosima::fastcdr::Cdr &dcdr)
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        dcdr >> relatedRequestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        dcdr >> relatedRequestId.writer_guid().entityId.value[i];
    }
    dcdr >> relatedRequestId.sequence_number().high;
    dcdr >> relatedRequestId.sequence_number().low;

    uint32_t aux;
    dcdr >> aux;
    remoteEx = static_cast<RemoteExceptionCode_t>(aux);
}

size_t RequestHeader::getCdrSerializedSize(
        const RequestHeader& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 16); // SampleIdentity.GUID_t
    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8); // SampleIdentity.SequenceNumber_t
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.instanceName.size() + 1;

    return current_alignment - initial_alignment;
}

void RequestHeader::serialize(
        eprosima::fastcdr::Cdr &scdr) const
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        scdr << requestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        scdr << requestId.writer_guid().entityId.value[i];
    }
    scdr << requestId.sequence_number().high;
    scdr << requestId.sequence_number().low;

    scdr << instanceName.to_string();
}

void RequestHeader::deserialize(
        eprosima::fastcdr::Cdr &dcdr)
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        dcdr >> requestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        dcdr >> requestId.writer_guid().entityId.value[i];
    }
    dcdr >> requestId.sequence_number().high;
    dcdr >> requestId.sequence_number().low;

    std::string aux;
    dcdr >> aux;
    instanceName = aux;
}
