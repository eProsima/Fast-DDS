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

#ifndef FASTDDS_RTPS_DATASHARING__DATASHARINGPAYLOADPOOL_HPP
#define FASTDDS_RTPS_DATASHARING__DATASHARINGPAYLOADPOOL_HPP

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class DataSharingPayloadPool : public IPayloadPool
{

public:

    DataSharingPayloadPool() = default;

    ~DataSharingPayloadPool() = default;

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override
    {
        payload.data = new octet[size];
        payload.max_size = size;
        payload.length = size;
        payload.payload_owner = this;
        return true;
    }

    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override
    {
        payload.data = data.data;
        payload.max_size = data.max_size;
        payload.length = data.length;
        payload.payload_owner = this;
        return true;
    }

    bool release_payload(
            SerializedPayload_t& payload) override
    {
        delete[] payload.data;
        payload.max_size = 0;
        payload.length = 0;
        payload.payload_owner = nullptr;
        return true;
    }

    bool init_shared_memory(
            const GUID_t& writer_guid,
            const std::string& /*shared_dir*/)
    {
        writer_guid_ = writer_guid;
        return true;
    }

    virtual bool init_shared_memory(
            const RTPSWriter* /*writer*/,
            const std::string& /*shared_dir*/)
    {
        // Default implementation is NOP
        // will be overriden by children if needed
        return false;
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

    void fill_metadata(
            const CacheChange_t* /*cache_change*/)
    {
    }

    bool get_next_unread_payload(
            CacheChange_t& cache_change)
    {
        return get_payload(1, cache_change.serializedPayload);
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

    bool is_sample_valid(
            const CacheChange_t& /*change*/) const
    {
        return true;
    }

protected:

    GUID_t writer_guid_;

};


}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_DATASHARING__DATASHARINGPAYLOADPOOL_HPP
