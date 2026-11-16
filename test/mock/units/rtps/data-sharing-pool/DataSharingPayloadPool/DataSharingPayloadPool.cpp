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

// Link-seam implementation of DataSharingPayloadPool: replaces src/cpp/rtps/DataSharing/DataSharingPayloadPool.cpp.
// get_reader_pool() and get_writer_pool() return nullptr, so no pool exists and no shared memory is used. The other
// methods only provide the symbols the code under test references.

#include <rtps/DataSharing/DataSharingPayloadPool.hpp>

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
    payload.is_serialized_key = false;
    payload.payload_owner = nullptr;
    return true;
}

void DataSharingPayloadPool::advance(
        uint64_t&) const
{
}

uint64_t DataSharingPayloadPool::begin() const
{
    return 0;
}

uint64_t DataSharingPayloadPool::end() const
{
    return 0;
}

bool DataSharingPayloadPool::empty() const
{
    return true;
}

const GUID_t& DataSharingPayloadPool::writer() const
{
    return segment_id_;
}

uint32_t DataSharingPayloadPool::last_liveliness_sequence() const
{
    return 0;
}

bool DataSharingPayloadPool::check_sequence_number(
        const octet*,
        const SequenceNumber_t&)
{
    return true;
}

bool DataSharingPayloadPool::is_sample_valid(
        const CacheChange_t&) const
{
    return true;
}

std::shared_ptr<DataSharingPayloadPool> DataSharingPayloadPool::get_reader_pool(
        bool)
{
    return nullptr;
}

std::shared_ptr<DataSharingPayloadPool> DataSharingPayloadPool::get_writer_pool(
        const PoolConfig&)
{
    return nullptr;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
