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
#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/PoolConfig.h>
#include <utils/shared_memory/RobustExclusiveLock.hpp>
#include <utils/shared_memory/SharedMemSegment.hpp>

#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataSharingPayloadPool : public IPayloadPool
{

protected:
    class PayloadNode;

public:

    typedef fastdds::rtps::SharedMemSegment Segment;

    DataSharingPayloadPool() = default;

    ~DataSharingPayloadPool() = default;

    virtual bool get_next_unread_payload(
        CacheChange_t& cache_change) = 0;

    virtual bool release_payload(
            CacheChange_t& cache_change) override;

    static std::shared_ptr<DataSharingPayloadPool> get_reader_pool(
            const PoolConfig& config,
            bool is_volatile);

    static std::shared_ptr<DataSharingPayloadPool> get_writer_pool(
            const PoolConfig& config);

    static std::string get_default_directory()
    {
        std::string dir;
        fastdds::rtps::SharedDir::get_default_shared_dir(dir);
        return dir;
    }

    virtual bool init_shared_memory(
            const GUID_t& writer_guid,
            const std::string& shared_dir) = 0;

    constexpr static const char* domain_name()
    {
        return "fast_datasharing";
    }

    constexpr static const char* pool_chunck_name()
    {
        return "pool";
    }

    /**
     * Fills the metadata of the chared payload from the cache change information
     */
    void fill_metadata(const CacheChange_t* cache_change);

    /**
     * Advances a pointer in the pool to the next payload
     */
    uint32_t advance(
            uint32_t pointer) const;

    uint32_t begin() const;

    uint32_t end() const;

    bool emtpy() const;

    bool full() const;

    const GUID_t& writer() const;

protected:

    class alignas(8) PayloadNode
    {

        struct PayloadNodeMetaData
        {
        public:

            PayloadNodeMetaData()
                : reference_counter(0)
                , status(fastrtps::rtps::ChangeKind_t::ALIVE)
                , data_length(0)
                , sequence_number(c_SequenceNumber_Unknown)
                , writer_GUID(c_Guid_Unknown)
                , instance_handle(c_InstanceHandle_Unknown)
            {
            }

            ~PayloadNodeMetaData() = default;


            // Reference counter for this payload
            std::atomic<uint32_t> reference_counter;

            // writer/instance status
            alignas(2) uint8_t status;

            // Encapsulation of the data
            uint16_t encapsulation;

            // Actual data size of the payload. Must be less than the configured maximum
            uint32_t data_length;

            // Writer's timestamp 
            Time_t source_timestamp;

            // Sequence number of the payload inside the writer
            SequenceNumber_t sequence_number;

            // GUID of the writer that created the payload
            GUID_t writer_GUID;

            // Instance handel for the change
            InstanceHandle_t instance_handle;

            // Related sample identity for the change
            fastrtps::rtps::SampleIdentity related_sample_identity;

        };

    public:

        // Payload data comes after the metadata
        static constexpr size_t data_offset = sizeof(PayloadNodeMetaData);


        PayloadNode() = default;

        ~PayloadNode() = default;

        void reset()
        {
            metadata_.reference_counter.store(0);
            metadata_.status = fastrtps::rtps::ChangeKind_t::ALIVE;
            metadata_.data_length = 0;
            metadata_.sequence_number = c_SequenceNumber_Unknown;
            metadata_.writer_GUID = c_Guid_Unknown;
            metadata_.instance_handle = c_InstanceHandle_Unknown;
            metadata_.related_sample_identity = fastrtps::rtps::SampleIdentity();
        }

        static PayloadNode* get_from_data(
                octet* data)
        {
            return reinterpret_cast<PayloadNode*>(data - data_offset);
        }

        octet* data()
        {
            return reinterpret_cast<octet*>(this) + data_offset;
        }

        uint32_t data_length() const
        {
            return metadata_.data_length;
        }

        void data_length(
                uint32_t length)
        {
            metadata_.data_length = length;
        }

        uint16_t encapsulation() const
        {
            return metadata_.encapsulation;
        }

        void encapsulation(
                uint16_t encapsulation)
        {
            metadata_.encapsulation = encapsulation;
        }

        GUID_t writer_GUID() const
        {
            return metadata_.writer_GUID;
        }

        void writer_GUID(
                const GUID_t& guid)
        {
            metadata_.writer_GUID = guid;
        }

        SequenceNumber_t sequence_number() const
        {
            return metadata_.sequence_number;
        }

        void sequence_number(
                SequenceNumber_t sequence_number)
        {
            metadata_.sequence_number = sequence_number;
        }

        Time_t source_timestamp() const
        {
            return metadata_.source_timestamp;
        }

        void source_timestamp(
                Time_t timestamp)
        {
            metadata_.source_timestamp = timestamp;
        }

        InstanceHandle_t instance_handle() const
        {
            return metadata_.instance_handle;
        }

        void instance_handle(
                InstanceHandle_t handle)
        {
            metadata_.instance_handle = handle;
        }

        uint8_t status() const
        {
            return metadata_.status;
        }

        void status(
                uint8_t status)
        {
            metadata_.status = status;
        }

        fastrtps::rtps::SampleIdentity related_sample_identity() const
        {
            return metadata_.related_sample_identity;
        }

        void related_sample_identity(
                fastrtps::rtps::SampleIdentity identity)
        {
            metadata_.related_sample_identity = identity;
        }

        void reference()
        {
            metadata_.reference_counter.fetch_add(1, std::memory_order_relaxed);
        }

        bool dereference()
        {
            return (metadata_.reference_counter.fetch_sub(1, std::memory_order_acq_rel) == 1);
        }

        static void reference(
                octet* data)
        {
            PayloadNode::get_from_data(data)->reference();
        }

        static bool dereference(
                octet* data)
        {
            return PayloadNode::get_from_data(data)->dereference();
        }


    private:

        PayloadNodeMetaData metadata_;

    };

    struct alignas(8) PoolDescriptor
    {
        Segment::Offset payloads_base;      //< Base address for the payloads buffer
        Segment::Offset payloads_limit;     //< One beyond the reserved for the payloadas buffer
        Segment::Offset first_used_payload; //< The oldest payload in the pool
        Segment::Offset first_free_payload; //< The payload that will be written next
        uint32_t free_payloads;             //< Number of free payloads
        uint32_t aligned_payload_size;      //< The offset from a payload to the next in the buffer
    };

    static std::string generate_segment_name(
            const std::string& /*shared_dir*/,
            const GUID_t& writer_guid)
    {
        std::stringstream ss;
        ss << DataSharingPayloadPool::domain_name() << "_" << writer_guid.guidPrefix << "_" << writer_guid.entityId;
        return ss.str();
    }

    static size_t aligned_node_size (size_t payload_size)
    {
        return (payload_size + PayloadNode::data_offset + alignof(PayloadNode) - 1)
            & ~(alignof(PayloadNode) - 1);
    }

    constexpr static size_t aligned_descriptor_size()
    {
        return (sizeof(PoolDescriptor) + alignof(PoolDescriptor) - 1)
                & ~(alignof(PoolDescriptor) - 1);
    }

    GUID_t segment_id_;         //< The ID of the segment
    std::string segment_name_;  //< Segment name

    std::unique_ptr<Segment> segment_;    //< Shared memory segment
    //eprosima::fastdds::rtps::RobustExclusiveLock segment_name_lock_;        //< Segment access lock

    octet* payloads_buffer_;        //< Shared buffer of payloads
    PoolDescriptor* descriptor_;    //< Shared descriptor of the pool

};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_DATASHARINGPAYLOADPOOL_HPP
