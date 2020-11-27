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

#include <rtps/history/DataSharingPayloadPool.hpp>

#include "./DataSharingPayloadPool_impl/ReaderPool.hpp"
#include "./DataSharingPayloadPool_impl/WriterPool.hpp"

#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool DataSharingPayloadPool::release_payload(
        CacheChange_t& cache_change)
{
    cache_change.serializedPayload.length = 0;
    cache_change.serializedPayload.pos = 0;
    cache_change.serializedPayload.max_size = 0;
    cache_change.serializedPayload.data = nullptr;
    cache_change.payload_owner(nullptr);
    return true;
}

DataSharingPayloadPool::Segment::Offset DataSharingPayloadPool::advance(
        Segment::Offset offset) const
{
    offset += descriptor_->aligned_payload_size;
    if (offset >= descriptor_->payloads_limit)
    {
        offset = descriptor_->payloads_base;
    }
    return offset;
}

DataSharingPayloadPool::Segment::Offset DataSharingPayloadPool::begin() const
{
    return descriptor_->notified_begin;
}

DataSharingPayloadPool::Segment::Offset DataSharingPayloadPool::end() const
{
    return descriptor_->notified_end;
}

bool DataSharingPayloadPool::emtpy() const
{
    return descriptor_->notified_begin == descriptor_->notified_end;
}

bool DataSharingPayloadPool::full() const
{
    return descriptor_->free_payloads == 0;
}

const GUID_t& DataSharingPayloadPool::writer() const
{
    return segment_id_;
}

bool DataSharingPayloadPool::check_sequence_number(
        octet* data, SequenceNumber_t sn)
{
    return (PayloadNode::get_from_data(data)->sequence_number() == sn);
}


std::shared_ptr<DataSharingPayloadPool> DataSharingPayloadPool::get_reader_pool(
        const PoolConfig& config,
        bool is_volatile)
{
    (void) config;
    assert (config.memory_policy == PREALLOCATED_MEMORY_MODE);
    
    return std::make_shared<ReaderPool>(is_volatile);
}

std::shared_ptr<DataSharingPayloadPool> DataSharingPayloadPool::get_writer_pool(
        const PoolConfig& config)
{
    assert (config.memory_policy == PREALLOCATED_MEMORY_MODE);

    return std::make_shared<WriterPool>(
        config.maximum_size,
        config.payload_initial_size);
}

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
