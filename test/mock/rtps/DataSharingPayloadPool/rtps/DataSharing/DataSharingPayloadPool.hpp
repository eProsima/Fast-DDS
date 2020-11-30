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
 * @file DataSharingPayloadPool.hpp
 */

#ifndef RTPS_HISTORY_DATASHARINGPAYLOADPOOL_HPP
#define RTPS_HISTORY_DATASHARINGPAYLOADPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataSharingPayloadPool : public IPayloadPool
{

public:

    DataSharingPayloadPool() = default;

    ~DataSharingPayloadPool() = default;

    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        cache_change.serializedPayload.data = new octet[size];
        cache_change.serializedPayload.max_size = size;
        cache_change.serializedPayload.length = size;
        cache_change.payload_owner(this);
        return true;
    }

    bool get_payload(
            SerializedPayload_t& data,
            IPayloadPool*& data_owner,
            CacheChange_t& cache_change) override
    {
        cache_change.serializedPayload.data = data.data;
        cache_change.serializedPayload.max_size = data.max_size;
        cache_change.serializedPayload.length = data.length;
        cache_change.payload_owner(this);
        if (data_owner == nullptr)
        {
            data_owner = this;
        }
        return true;
    }

    bool release_payload(
            CacheChange_t& cache_change)
    {
        delete[] cache_change.serializedPayload.data;
        cache_change.serializedPayload.max_size = 0;
        cache_change.serializedPayload.length = 0;
        cache_change.payload_owner(nullptr);
        return true;
    }

    bool init_shared_memory(
            const GUID_t& writer_guid,
            const std::string& /*shared_dir*/)
    {
        writer_guid_ = writer_guid;
        return true;
    }

    static std::shared_ptr<DataSharingPayloadPool> get_reader_pool(
            const PoolConfig& /*config*/,
            bool /*is_volatile*/)
    {
        return std::make_shared<DataSharingPayloadPool>();
    }

    static std::shared_ptr<DataSharingPayloadPool> get_writer_pool(
            const PoolConfig& /*config*/)
    {
        return std::make_shared<DataSharingPayloadPool>();
    }

    static std::string get_default_directory()
    {
        return std::string();
    }

    constexpr static const char* domain_name()
    {
        return "fast_datasharing";
    }

    constexpr static const char* pool_chunck_name()
    {
        return "pool";
    }

    void fill_metadata(const CacheChange_t* /*cache_change*/)
    {
    }

    bool get_next_unread_payload(
        CacheChange_t& cache_change)
    {
        return get_payload(1, cache_change);
    }

    uint32_t advance(
            uint32_t /*pointer*/) const
    {
        return 0;
    }

    uint32_t begin() const
    {
        return end();
    }

    uint32_t end() const
    {
        return 0;
    }

    bool emtpy() const
    {
        return true;
    }

    bool full() const
    {
        return false;
    }

    const GUID_t& writer() const
    {
        return writer_guid_;
    }

    static bool check_sequence_number(
            octet* /*data*/,
            SequenceNumber_t /*sn*/)
    {
        return true;
    }

protected:

    GUID_t writer_guid_;

};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_DATASHARINGPAYLOADPOOL_HPP
