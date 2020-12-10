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
 * @file WriterPool.hpp
 */

#ifndef RTPS_DATASHARING_WRITERPOOL_HPP
#define RTPS_DATASHARING_WRITERPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>

#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class WriterPool : public DataSharingPayloadPool
{

public:

    WriterPool(
        uint32_t pool_size,
        uint32_t payload_size)
        : max_data_size_(payload_size)
        , pool_size_(pool_size)
    {
    }

    ~WriterPool()
    {
        logInfo(HISTORY_DATASHARING_PAYLOADPOOL, "DataSharingPayloadPool::WriterPool destructor");

        // Destroy each node in the pool
        uint32_t aligned_size = static_cast<uint32_t>(DataSharingPayloadPool::node_size(max_data_size_));
        for (octet* payload = payloads_pool_;
                payload < payloads_pool_ + (pool_size_ * aligned_size);
                payload += aligned_size)
        {
            reinterpret_cast<PayloadNode*>(payload)->~PayloadNode();
        }

        // Free the pool
        segment_->get().deallocate(payloads_pool_);

        // Free the history
        segment_->get().destroy<Segment::Offset>(history_chunk_name());

        // Free the descriptor
        segment_->get().destroy<PoolDescriptor>(descriptor_chunk_name());

        // Destroy the shared segment.
        // The file will be deleted once the last reader has closed it.
        segment_->remove(segment_name_);
    }

    bool get_payload(
            uint32_t /*size*/,
            CacheChange_t& cache_change) override
    {
        if (free_payloads_.empty())
        {
            return false;
        }

        PayloadNode* payload = free_payloads_.back();
        free_payloads_.pop_back();

        cache_change.serializedPayload.data = payload->data();
        cache_change.serializedPayload.max_size = max_data_size_;
        cache_change.payload_owner(this);

        return true;
    }

    bool get_payload(
            SerializedPayload_t& data,
            IPayloadPool*& data_owner,
            CacheChange_t& cache_change) override
    {
        assert(cache_change.writerGUID != GUID_t::unknown());
        assert(cache_change.sequenceNumber != SequenceNumber_t::unknown());

        if (data_owner == this)
        {
            cache_change.serializedPayload.data = data.data;
            cache_change.serializedPayload.length = data.length;
            cache_change.serializedPayload.max_size = data.length;
            cache_change.payload_owner(this);
            return true;
        }
        else
        {
            if (get_payload(data.length, cache_change))
            {
                if (!cache_change.serializedPayload.copy(&data, true))
                {
                    release_payload(cache_change);
                    return false;
                }

                if (data_owner == nullptr)
                {
                    data_owner = this;
                    data.data = cache_change.serializedPayload.data;
                }

                return true;
            }
        }

        return false;
    }

    bool get_next_unread_payload(
            CacheChange_t& /*cache_change*/) override
    {
        // Only ReaderPool has a reading pointer
        return false;
    }

    bool release_payload(
            CacheChange_t& cache_change) override
    {
        assert(cache_change.payload_owner() == this);

        PayloadNode* payload = PayloadNode::get_from_data(cache_change.serializedPayload.data);
        payload->reset();
        free_payloads_.push_back(payload);
        logInfo(HISTORY_DATASHARING_PAYLOADPOOL, "Change released with SN " << cache_change.sequenceNumber);

        return DataSharingPayloadPool::release_payload(cache_change);
    }

    bool init_shared_memory(
            const GUID_t& writer_guid,
            const std::string& shared_dir) override
    {
        segment_id_ = writer_guid;
        segment_name_ = generate_segment_name(shared_dir, writer_guid);

        // Extra size for the internal allocator structures (512bytes estimated)
        uint32_t extra = 512;
        uint32_t per_allocation_extra_size = fastdds::rtps::SharedMemSegment::compute_per_allocation_extra_size(
                alignof(PayloadNode), DataSharingPayloadPool::domain_name());
        uint32_t payload_size = static_cast<uint32_t>(DataSharingPayloadPool::node_size(max_data_size_));
        uint32_t size_for_payloads_pool = pool_size_ * payload_size;
        //Reserve one extra to avoid pointer overlapping
        uint32_t size_for_history = (pool_size_ + 1) * static_cast<uint32_t>(sizeof(Segment::Offset));
        uint32_t descriptor_size = static_cast<uint32_t>(sizeof(PoolDescriptor));
        uint32_t segment_size = size_for_payloads_pool + per_allocation_extra_size +
                size_for_history + per_allocation_extra_size + 
                descriptor_size + per_allocation_extra_size;

        //Open the segment
        fastdds::rtps::SharedMemSegment::remove(segment_name_);
        try
        {
            segment_ = std::unique_ptr<Segment>(
                new Segment(boost::interprocess::create_only,
                    segment_name_,
                    segment_size + extra));
        }
        catch (const std::exception& e)
        {
            logError(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to create segment " << segment_name_
                                                                                  << ": " << e.what());
            return false;
        }

        try
        {
            // Memset the whole segment to zero in order to force physical map of the buffer
            auto tmp = segment_->get().allocate(segment_size - per_allocation_extra_size);
            memset(tmp, 0, segment_size - per_allocation_extra_size);
            segment_->get().deallocate(tmp);

            // Alloc the memory for the pool
            // Cannot use 'construct' because we need to reserve extra space for the data,
            // which is not considered in sizeof(PayloadNode).
            payloads_pool_ = static_cast<octet*>(segment_->get().allocate(size_for_payloads_pool));

            // Initialize each node in the pool
            free_payloads_.reserve(pool_size_);
            octet* payload = payloads_pool_;
            for (uint32_t i = 0; i < pool_size_; ++i)
            {
                new (payload) PayloadNode();

                // All payloads are free
                free_payloads_.push_back(reinterpret_cast<PayloadNode*>(payload));

                payload += (ptrdiff_t)payload_size;
            }

            //Alloc the memory for the history
            history_ = segment_->get().construct<Segment::Offset>(history_chunk_name())[pool_size_ + 1]();

            //Alloc the memory for the descriptor
            descriptor_ = segment_->get().construct<PoolDescriptor>(descriptor_chunk_name())();

            // Initialize the data in the descriptor
            descriptor_->history_size = pool_size_ + 1;
            descriptor_->notified_begin = 0u;
            descriptor_->notified_end = 0u;
            descriptor_->liveliness_sequence = 0u;
        }
        catch (std::exception& e)
        {
            Segment::remove(segment_name_);

            logError(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to initialize segment " << segment_name_
                                                                                      << ": " << e.what());
            return false;
        }

        return true;
    }

    void add_to_shared_history(const CacheChange_t* cache_change) override
    {
        assert(cache_change);
        assert(cache_change->serializedPayload.data);
        assert(cache_change->payload_owner() == this);
        assert(advance(descriptor_->notified_end) != descriptor_->notified_begin);

        // Fill the payload metadata with the change info
        PayloadNode* node = PayloadNode::get_from_data(cache_change->serializedPayload.data);
        node->sequence_number(cache_change->sequenceNumber);
        node->status(ALIVE);
        node->data_length(cache_change->serializedPayload.length);
        node->source_timestamp(cache_change->sourceTimestamp);
        node->writer_GUID(cache_change->writerGUID);
        node->instance_handle(cache_change->instanceHandle);
        node->related_sample_identity(cache_change->write_params.sample_identity());

        // Add it to the history
        history_[descriptor_->notified_end] = segment_->get_offset_from_address(node);
        logInfo(HISTORY_DATASHARING_PAYLOADPOOL, "Change added to shared history"
                << " with SN " << cache_change->sequenceNumber);
        descriptor_->notified_end = advance(descriptor_->notified_end);
    }

    void remove_from_shared_history(const CacheChange_t* cache_change) override
    {
        assert(cache_change);
        assert(cache_change->serializedPayload.data);
        assert(cache_change->payload_owner() == this);
        assert(descriptor_->notified_end != descriptor_->notified_begin);

        PayloadNode* payload = PayloadNode::get_from_data(cache_change->serializedPayload.data);
        assert(segment_->get_offset_from_address(payload) == history_[descriptor_->notified_begin]);
        payload->reset();
        logInfo(HISTORY_DATASHARING_PAYLOADPOOL, "Change removed from shared history"
                << " with SN " << cache_change->sequenceNumber);
        descriptor_->notified_begin = advance(descriptor_->notified_begin);
    }

    void assert_liveliness() override
    {
        ++descriptor_->liveliness_sequence;
    }


private:

    uint32_t max_data_size_;    //< Maximum size of the serialized payload data
    uint32_t pool_size_;        //< Number of payloads in the pool

    std::vector<PayloadNode*> free_payloads_;   //< Pointers to the free payloads in the pool
};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_DATASHARING_WRITERPOOL_HPP
