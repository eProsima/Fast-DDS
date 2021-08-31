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
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <utils/collections/FixedSizeQueue.hpp>

#include <memory>

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
        , free_history_size_(0)
        , writer_(nullptr)
    {
    }

    ~WriterPool()
    {
        logInfo(DATASHARING_PAYLOADPOOL, "DataSharingPayloadPool::WriterPool destructor");

        // We cannot destroy the objects in the SHM, as the Reader may still be using them.
        // We just remove the segment, and when the Reader closes it, it will be removed from the system.
        if (segment_)
        {
            segment_->remove();
        }
    }

    bool get_payload(
            uint32_t /*size*/,
            CacheChange_t& cache_change) override
    {
        if (free_payloads_.empty())
        {
            return false;
        }

        PayloadNode* payload = free_payloads_.front();
        free_payloads_.pop_front();
        // Reset all the metadata to signal the reader that the payload is dirty
        payload->reset();

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

    bool release_payload(
            CacheChange_t& cache_change) override
    {
        assert(cache_change.payload_owner() == this);

        // Payloads are reset on the `get` operation, the `release` leaves the data to give more chances to the reader
        PayloadNode* payload = PayloadNode::get_from_data(cache_change.serializedPayload.data);
        if (payload->has_been_removed())
        {
            advance_till_first_non_removed();
        }
        else
        {
            free_payloads_.push_back(payload);
        }
        logInfo(DATASHARING_PAYLOADPOOL, "Change released with SN " << cache_change.sequenceNumber);

        return DataSharingPayloadPool::release_payload(cache_change);
    }

    template <typename T>
    bool init_shared_segment(
            const RTPSWriter* writer,
            const std::string& shared_dir)
    {
        writer_ = writer;
        segment_id_ = writer_->getGuid();
        segment_name_ = generate_segment_name(shared_dir, segment_id_);

        // We need to reserve the whole segment at once, and the underlying classes use uint32_t as size type.
        // In order to avoid overflows, we will calculate using uint64 and check the casting
        bool overflow = false;
        size_t per_allocation_extra_size = T::compute_per_allocation_extra_size(
            alignof(PayloadNode), DataSharingPayloadPool::domain_name());
        size_t payload_size = DataSharingPayloadPool::node_size(max_data_size_);

        uint64_t estimated_size_for_payloads_pool = pool_size_ * payload_size;
        overflow |= (estimated_size_for_payloads_pool != static_cast<uint32_t>(estimated_size_for_payloads_pool));
        uint32_t size_for_payloads_pool = static_cast<uint32_t>(estimated_size_for_payloads_pool);

        //Reserve one extra to avoid pointer overlapping
        uint64_t estimated_size_for_history = (pool_size_ + 1) * sizeof(Segment::Offset);
        overflow |= (estimated_size_for_history != static_cast<uint32_t>(estimated_size_for_history));
        uint32_t size_for_history = static_cast<uint32_t>(estimated_size_for_history);

        uint32_t descriptor_size = static_cast<uint32_t>(sizeof(PoolDescriptor));
        uint64_t estimated_segment_size = size_for_payloads_pool + per_allocation_extra_size +
                size_for_history + per_allocation_extra_size +
                descriptor_size + per_allocation_extra_size;
        overflow |= (estimated_segment_size != static_cast<uint32_t>(estimated_segment_size));
        uint32_t segment_size = static_cast<uint32_t>(estimated_segment_size);

        if (overflow)
        {
            logError(DATASHARING_PAYLOADPOOL, "Failed to create segment " << segment_name_
                                                                          << ": Segment size is too large: " << estimated_size_for_payloads_pool
                                                                          << " (max is " << std::numeric_limits<uint32_t>::max() << ")."
                                                                          << " Please reduce the maximum size of the history");
            return false;
        }

        //Open the segment
        T::remove(segment_name_);
        std::unique_ptr<T> local_segment;
        try
        {
            local_segment = std::unique_ptr<T>(
                new T(boost::interprocess::create_only,
                segment_name_,
                segment_size + T::EXTRA_SEGMENT_SIZE));
        }
        catch (const std::exception& e)
        {
            logError(DATASHARING_PAYLOADPOOL, "Failed to create segment " << segment_name_
                                                                          << ": " << e.what());
            return false;
        }

        try
        {
            // Alloc the memory for the pool
            // Cannot use 'construct' because we need to reserve extra space for the data,
            // which is not considered in sizeof(PayloadNode).
            payloads_pool_ = static_cast<octet*>(local_segment->get().allocate(size_for_payloads_pool));

            // Initialize each node in the pool
            free_payloads_.init(pool_size_);
            octet* payload = payloads_pool_;
            for (uint32_t i = 0; i < pool_size_; ++i)
            {
                new (payload) PayloadNode();

                // All payloads are free
                free_payloads_.push_back(reinterpret_cast<PayloadNode*>(payload));

                payload += (ptrdiff_t)payload_size;
            }

            //Alloc the memory for the history
            history_ = local_segment->get().template construct<Segment::Offset>(history_chunk_name())[pool_size_ + 1]();

            //Alloc the memory for the descriptor
            descriptor_ = local_segment->get().template construct<PoolDescriptor>(descriptor_chunk_name())();

            // Initialize the data in the descriptor
            descriptor_->history_size = pool_size_ + 1;
            descriptor_->notified_begin = 0u;
            descriptor_->notified_end = 0u;
            descriptor_->liveliness_sequence = 0u;

            free_history_size_ = pool_size_;
        }
        catch (std::exception& e)
        {
            T::remove(segment_name_);

            logError(DATASHARING_PAYLOADPOOL, "Failed to initialize segment " << segment_name_
                                                                              << ": " << e.what());
            return false;
        }

        segment_ = std::move(local_segment);
        is_initialized_ = true;
        return true;
    }

    bool init_shared_memory(
            const RTPSWriter* writer,
            const std::string& shared_dir) override
    {
        if (shared_dir.empty())
        {
            return init_shared_segment<fastdds::rtps::SharedMemSegment>(writer, shared_dir);
        }
        else
        {
            return init_shared_segment<fastdds::rtps::SharedFileSegment>(writer, shared_dir);
        }
    }

    /**
     * Fills the metadata of the shared payload from the cache change information
     * and adds the payload's offset to the shared history
     */
    void add_to_shared_history(
            const CacheChange_t* cache_change)
    {
        assert(cache_change);
        assert(cache_change->serializedPayload.data);
        assert(cache_change->payload_owner() == this);
        assert(free_history_size_ > 0);

        // Fill the payload metadata with the change info
        PayloadNode* node = PayloadNode::get_from_data(cache_change->serializedPayload.data);
        node->status(ALIVE);
        node->data_length(cache_change->serializedPayload.length);
        node->source_timestamp(cache_change->sourceTimestamp);
        node->writer_GUID(cache_change->writerGUID);
        node->instance_handle(cache_change->instanceHandle);
        if (cache_change->write_params.related_sample_identity() != SampleIdentity::unknown())
        {
            node->related_sample_identity(cache_change->write_params.related_sample_identity());
        }

        // Set the sequence number last, it signals the data is ready
        node->sequence_number(cache_change->sequenceNumber);

        // Add it to the history
        history_[static_cast<uint32_t>(descriptor_->notified_end)] = segment_->get_offset_from_address(node);
        logInfo(DATASHARING_PAYLOADPOOL, "Change added to shared history"
                << " with SN " << cache_change->sequenceNumber);
        advance(descriptor_->notified_end);
        --free_history_size_;
    }

    /**
     * Removes the payload's offset from the shared history
     *
     * Payloads don't need to be removed from the history in the same order
     * they where added, but a payload will not be available through @ref get_payload until all
     * payloads preceding it have been removed from the shared history.
     */
    void remove_from_shared_history(
            const CacheChange_t* cache_change)
    {
        assert(cache_change);
        assert(cache_change->serializedPayload.data);
        assert(cache_change->payload_owner() == this);
        assert(descriptor_->notified_end != descriptor_->notified_begin);
        assert(free_history_size_ < descriptor_->history_size);

        logInfo(DATASHARING_PAYLOADPOOL, "Change removed from shared history"
                << " with SN " << cache_change->sequenceNumber);

        PayloadNode* payload = PayloadNode::get_from_data(cache_change->serializedPayload.data);
        payload->has_been_removed(true);
    }

    void advance_till_first_non_removed()
    {
        while (descriptor_->notified_begin != descriptor_->notified_end)
        {
            auto offset = history_[static_cast<uint32_t>(descriptor_->notified_begin)];
            auto payload = static_cast<PayloadNode*>(segment_->get_address_from_offset(offset));
            if (!payload->has_been_removed())
            {
                break;
            }

            payload->has_been_removed(false);
            free_payloads_.push_back(payload);
            advance(descriptor_->notified_begin);
            ++free_history_size_;
        }
    }

    void assert_liveliness()
    {
        ++descriptor_->liveliness_sequence;
    }

    bool is_initialized() const
    {
        return is_initialized_;
    }

private:

    octet* payloads_pool_;          //< Shared pool of payloads

    uint32_t max_data_size_;        //< Maximum size of the serialized payload data
    uint32_t pool_size_;            //< Number of payloads in the pool
    uint32_t free_history_size_;    //< Number of elements currently unused in the shared history

    FixedSizeQueue<PayloadNode*> free_payloads_;    //< Pointers to the free payloads in the pool

    const RTPSWriter* writer_;      //< Writer that is owner of the pool

    bool is_initialized_ = false;   //< Whether the pool has been initialized on shared memory

};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_DATASHARING_WRITERPOOL_HPP
