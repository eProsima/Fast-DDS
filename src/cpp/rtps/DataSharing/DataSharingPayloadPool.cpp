// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataSharingPayloadPool.cpp
 */

#include <rtps/DataSharing/DataSharingPayloadPool.hpp>

#include "./ReaderPool.hpp"
#include "./WriterPool.hpp"

#include <memory>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool DataSharingPayloadPool::release_payload(
        SerializedPayload_t& payload)
{
    payload.length = 0;
    payload.pos = 0;
    payload.max_size = 0;
    payload.data = nullptr;
    payload.payload_owner = nullptr;
    return true;
}

void DataSharingPayloadPool::advance(
        uint64_t& index) const
{
    // lower part is the index, upper part is the loop counter
    if (static_cast<uint32_t>(index) + 1 <= descriptor_->history_size)
    {
        ++index;
    }
    if (static_cast<uint32_t>(index) % descriptor_->history_size == 0)
    {
        index = ((index >> 32) + 1) << 32;
    }
}

uint64_t DataSharingPayloadPool::begin() const
{
    return descriptor_->notified_begin;
}

uint64_t DataSharingPayloadPool::end() const
{
    return descriptor_->notified_end;
}

bool DataSharingPayloadPool::empty() const
{
    return descriptor_->notified_begin == descriptor_->notified_end;
}

const GUID_t& DataSharingPayloadPool::writer() const
{
    return segment_id_;
}

uint32_t DataSharingPayloadPool::last_liveliness_sequence() const
{
    return descriptor_->liveliness_sequence;
}

bool DataSharingPayloadPool::check_sequence_number(
        const octet* data,
        const SequenceNumber_t& sn)
{
    return (PayloadNode::get_from_data(data)->sequence_number() == sn);
}

bool DataSharingPayloadPool::is_sample_valid(
        const CacheChange_t& change) const
{
    return check_sequence_number(change.serializedPayload.data, change.sequenceNumber);
}

std::shared_ptr<DataSharingPayloadPool> DataSharingPayloadPool::get_reader_pool(
        bool is_reader_volatile)
{
    return std::make_shared<ReaderPool>(is_reader_volatile);
}

std::shared_ptr<DataSharingPayloadPool> DataSharingPayloadPool::get_writer_pool(
        const PoolConfig& config)
{
    assert (config.memory_policy == PREALLOCATED_MEMORY_MODE ||
            config.memory_policy == PREALLOCATED_WITH_REALLOC_MEMORY_MODE);

    return std::make_shared<WriterPool>(
        config.maximum_size,
        config.payload_initial_size);
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
