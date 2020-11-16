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

#ifndef RTPS_HISTORY_DATASHARINGPAYLOADPOOLIMPL_WRITERPOOL_HPP
#define RTPS_HISTORY_DATASHARINGPAYLOADPOOLIMPL_WRITERPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/DataSharingPayloadPool.hpp>

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

        // Destroy each node in the buffer
        size_t aligned_size = DataSharingPayloadPool::aligned_node_size(max_data_size_);
        for (octet* payload = payloads_buffer_;
                payload < payloads_buffer_ + (pool_size_ * aligned_size);
                payload += aligned_size)
        {
            reinterpret_cast<PayloadNode*>(payload)->~PayloadNode();
        }

        // Free the buffer
        segment_->get().deallocate(payloads_buffer_);

        // Free the descriptor
        segment_->get().destroy<PoolDescriptor>("descriptor");
    }

    bool get_payload(
            uint32_t /*size*/,
            CacheChange_t& cache_change) override
    {
        if (full())
        {
            return false;
        }

        PayloadNode* payload = static_cast<PayloadNode*>(
                segment_->get_address_from_offset(descriptor_->first_free_payload));
        descriptor_->first_free_payload = advance(descriptor_->first_free_payload);
        payload->reference();
        --descriptor_->free_payloads;

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
            PayloadNode::reference(data.data);

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
                    PayloadNode::reference(data.data);
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
        assert(segment_->get_offset_from_address(payload) == descriptor_->first_used_payload);

        // The writer should always be the last to release the payload
        bool is_released_by_all = payload->dereference();
        if (!is_released_by_all)
        {
            logError(HISTORY_DATASHARING_PAYLOADPOOL,
                "Writer releasing a payload with active references in segment " << segment_name_);
        }
        payload->reset();
        descriptor_->first_used_payload = advance(descriptor_->first_used_payload);
        ++descriptor_->free_payloads;
        return DataSharingPayloadPool::release_payload(cache_change);
    }


    bool init_shared_memory(
            const GUID_t& writer_guid,
            const std::string& shared_dir) override
    {
        segment_id_ = writer_guid;
        segment_name_ = generate_segment_name(shared_dir, writer_guid);

        // Extra size for the internal allocator structures (512bytes estimated)
        size_t extra = 512;
        size_t per_allocation_extra_size = fastdds::rtps::SharedMemSegment::compute_per_allocation_extra_size(
                alignof(PayloadNode), DataSharingPayloadPool::domain_name());
        size_t aligned_payload_size = DataSharingPayloadPool::aligned_node_size(max_data_size_);
        //Reserve one extra to avoid pointer overlapping
        size_t size_for_payloads_buffer = (pool_size_ + 1) * aligned_payload_size;
        size_t aligned_descriptor_size = DataSharingPayloadPool::aligned_descriptor_size();
        size_t segment_size = size_for_payloads_buffer + per_allocation_extra_size +
                aligned_descriptor_size + per_allocation_extra_size;

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
            // which is not in considered in sizeof(PayloadNode).
            payloads_buffer_ = static_cast<octet*>(segment_->get().allocate(size_for_payloads_buffer));

            // Initialize each node in the buffer
            octet* payload = static_cast<octet*>(payloads_buffer_);

            for (uint32_t i = 0; i <= pool_size_; ++i)
            {
                new (payload) PayloadNode();
                payload += (ptrdiff_t)aligned_payload_size;
            }

            //Alloc the memory for the descriptor
            descriptor_ = segment_->get().construct<PoolDescriptor>("descriptor")();

            // Initialize the data in the descriptor
            descriptor_->payloads_base = segment_->get_offset_from_address(payloads_buffer_);
            descriptor_->payloads_limit = segment_->get_offset_from_address(payloads_buffer_ + size_for_payloads_buffer);
            descriptor_->first_free_payload = descriptor_->payloads_base;
            descriptor_->first_used_payload = descriptor_->payloads_base;
            descriptor_->free_payloads = pool_size_;
            descriptor_->aligned_payload_size = aligned_payload_size;
        }
        catch (std::exception& e)
        {
            Segment::remove(segment_name_);

            logError(HISTORY_DATASHARING_LISTENER, "Failed to initialize segment " << segment_name_
                                                                                   << ": " << e.what());
            return false;
        }

        return true;
    }

private:

    uint32_t max_data_size_;    //< Maximum size of the serialized payload data
    size_t pool_size_;          //< Number of payloads in the pool
};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_DATASHARINGPAYLOADPOOLIMPL_WRITERPOOL_HPP
