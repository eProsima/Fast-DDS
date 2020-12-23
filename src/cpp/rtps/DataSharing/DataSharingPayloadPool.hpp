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

#ifndef RTPS_DATASHARING_DATASHARINGPAYLOADPOOL_HPP
#define RTPS_DATASHARING_DATASHARINGPAYLOADPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/PoolConfig.h>
#include <utils/shared_memory/RobustExclusiveLock.hpp>
#include <utils/shared_memory/SharedMemSegment.hpp>

#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataSharingPayloadPool : public IPayloadPool
{

protected:

    class PayloadNode;

public:

    using Segment = fastdds::rtps::SharedMemSegment;
    using sharable_mutex = Segment::sharable_mutex;
    template <class M>
    using sharable_lock = Segment::sharable_lock<M>;

    DataSharingPayloadPool() = default;

    ~DataSharingPayloadPool() = default;

    virtual bool release_payload(
            CacheChange_t& cache_change) override;

    static std::shared_ptr<DataSharingPayloadPool> get_reader_pool(
            bool is_reader_volatile);

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

    constexpr static const char* descriptor_chunk_name()
    {
        return "descriptor";
    }

    constexpr static const char* history_chunk_name()
    {
        return "history";
    }

    /**
     * Advances an index to the history to the next position
     */
    void advance(
            uint64_t& index) const;

    /**
     * The index of the first valid position in the history
     */
    uint64_t begin() const;

    /**
     * The index of one past the last valid position in the history
     */
    uint64_t end() const;

    /**
     * Whether the history is empty or not
     */
    bool emtpy() const;

    const GUID_t& writer() const;

    uint32_t last_liveliness_sequence() const;

    static bool check_sequence_number(
            const octet* data,
            const SequenceNumber_t& sn);

    static sharable_mutex& shared_mutex(
            octet*);

protected:

#pragma warning(push)
#pragma warning(disable:4324)
    class alignas (8) PayloadNode
    {

        struct PayloadNodeMetaData
        {
        public:

            PayloadNodeMetaData()
                : status(fastrtps::rtps::ChangeKind_t::ALIVE)
                , data_length(0)
                , sequence_number(c_SequenceNumber_Unknown)
                , writer_GUID(c_Guid_Unknown)
                , instance_handle(c_InstanceHandle_Unknown)
            {
            }

            ~PayloadNodeMetaData() = default;

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

            // Mutex for shared read / exclusive write access to the payload
            sharable_mutex mutex;

        };

    public:

        // Payload data comes after the metadata
        static constexpr size_t data_offset = sizeof(PayloadNodeMetaData);


        PayloadNode() = default;

        ~PayloadNode() = default;

        void reset()
        {
            metadata_.sequence_number = c_SequenceNumber_Unknown;
            metadata_.status = fastrtps::rtps::ChangeKind_t::ALIVE;
            metadata_.data_length = 0;
            metadata_.writer_GUID = c_Guid_Unknown;
            metadata_.instance_handle = c_InstanceHandle_Unknown;
            metadata_.related_sample_identity = fastrtps::rtps::SampleIdentity();
        }

        static const PayloadNode* get_from_data(
                const octet* data)
        {
            return reinterpret_cast<const PayloadNode*>(data - data_offset);
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

        sharable_mutex& mutex()
        {
            return metadata_.mutex;
        }

    private:

        PayloadNodeMetaData metadata_;

    };

    struct alignas (8) PoolDescriptor
    {
        uint32_t history_size;          //< Number of payloads in the history
        uint64_t notified_begin;        //< The index of the oldest history entry already notified (ready to read)
        uint64_t notified_end;          //< The index of the history entry that will be notified next
        uint32_t liveliness_sequence;   //< The ID of the last liveliness assertion sent by the writer
    };
#pragma warning(pop)

    static std::string generate_segment_name(
            const std::string& /*shared_dir*/,
            const GUID_t& writer_guid)
    {
        std::stringstream ss;
        ss << DataSharingPayloadPool::domain_name() << "_" << writer_guid.guidPrefix << "_" << writer_guid.entityId;
        return ss.str();
    }

    static size_t node_size (
            size_t payload_size)
    {
        return (payload_size + PayloadNode::data_offset + alignof(PayloadNode) - 1)
               & ~(alignof(PayloadNode) - 1);
    }

    GUID_t segment_id_;         //< The ID of the segment
    std::string segment_name_;  //< Segment name

    std::unique_ptr<Segment> segment_;    //< Shared memory segment

    octet* payloads_pool_;          //< Shared pool of payloads
    Segment::Offset* history_;      //< Offsets of the payloads that are currently in the writer's history
    PoolDescriptor* descriptor_;    //< Shared descriptor of the pool

};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_DATASHARING_DATASHARINGPAYLOADPOOL_HPP
