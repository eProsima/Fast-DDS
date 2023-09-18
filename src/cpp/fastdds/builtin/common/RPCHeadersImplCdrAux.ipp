// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_BUILTIN_COMMON_RPCHEADERSIMPLCDRAUX_IPP
#define FASTDDS_BUILTIN_COMMON_RPCHEADERSIMPLCDRAUX_IPP

namespace eprosima {
namespace fastcdr {
template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::rpc::ReplyHeader&,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 16); // SampleIdentity.GUID_t
    calculated_size += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8); // SampleIdentity.SequenceNumber_t
    calculated_size += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4); // RemoteException

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::rpc::ReplyHeader& data)
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        scdr << data.relatedRequestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        scdr << data.relatedRequestId.writer_guid().entityId.value[i];
    }

    scdr << data.relatedRequestId.sequence_number().high;
    scdr << data.relatedRequestId.sequence_number().low;

    scdr << static_cast<uint32_t>(data.remoteEx);
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::rpc::ReplyHeader& data)
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        dcdr >> data.relatedRequestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        dcdr >> data.relatedRequestId.writer_guid().entityId.value[i];
    }
    dcdr >> data.relatedRequestId.sequence_number().high;
    dcdr >> data.relatedRequestId.sequence_number().low;

    dcdr >> data.remoteEx;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::rpc::RequestHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 16); // SampleIdentity.GUID_t
    calculated_size += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8); // SampleIdentity.SequenceNumber_t
    calculated_size += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.instanceName.size() + 1;

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);


    return calculated_size;
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::rpc::RequestHeader& data)
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        scdr << data.requestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        scdr << data.requestId.writer_guid().entityId.value[i];
    }
    scdr << data.requestId.sequence_number().high;
    scdr << data.requestId.sequence_number().low;

    scdr << data.instanceName.to_string();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::rpc::RequestHeader& data)
{
    for (uint32_t i = 0; i < fastrtps::rtps::GuidPrefix_t::size; ++i)
    {
        dcdr >> data.requestId.writer_guid().guidPrefix.value[i];
    }
    for (uint32_t i = 0; i < fastrtps::rtps::EntityId_t::size; ++i)
    {
        dcdr >> data.requestId.writer_guid().entityId.value[i];
    }
    dcdr >> data.requestId.sequence_number().high;
    dcdr >> data.requestId.sequence_number().low;

    dcdr >> data.instanceName;
}

} // namespace fastcdr
} // namespace eprosima

#endif // FASTDDS_BUILTIN_COMMON_RPCHEADERSIMPLCDRAUX_IPP
