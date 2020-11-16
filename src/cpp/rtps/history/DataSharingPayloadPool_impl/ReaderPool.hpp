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
 * @file ReaderPool.hpp
 */

#ifndef RTPS_HISTORY_DATASHARINGPAYLOADPOOLIMPL_READERPOOL_HPP
#define RTPS_HISTORY_DATASHARINGPAYLOADPOOLIMPL_READERPOOL_HPP

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

class ReaderPool : public DataSharingPayloadPool
{

public:

    ReaderPool (
            bool is_volatile)
        : is_volatile_(is_volatile)
    {
    }

    bool get_payload(
            uint32_t /*size*/,
            CacheChange_t& /*cache_change*/) override
    {
        // Only WriterPool can return new payloads
        return false;
    }

    bool get_payload(
            SerializedPayload_t& data,
            IPayloadPool*& data_owner,
            CacheChange_t& cache_change) override
    {
        assert(cache_change.writerGUID != GUID_t::unknown());
        assert(cache_change.sequenceNumber != SequenceNumber_t::unknown());
        assert(data_owner == this || data_owner == nullptr);
        assert(cache_change.writerGUID == segment_id_);

        if (data_owner == this)
        {
            PayloadNode::reference(data.data);

            cache_change.serializedPayload.data = data.data;
            cache_change.serializedPayload.length = data.length;
            cache_change.serializedPayload.max_size = data.length;
            cache_change.payload_owner(this);
            return true;
        }

        if (cache_change.writerGUID != segment_id_)
        {
            // Can only point to payloads allocated by someone else
            return false;
        }

        // `data.data` must be received from shared memory
        PayloadNode::reference(data.data);
        cache_change.serializedPayload = data;
        cache_change.payload_owner(this);
        data_owner = this;
        return true;
    }

    bool release_payload(
            CacheChange_t& cache_change)
    {
        assert(cache_change.payload_owner() == this);

        PayloadNode::dereference(cache_change.serializedPayload.data);

        return DataSharingPayloadPool::release_payload(cache_change);
    }

    bool init_shared_memory(
            const GUID_t& writer_guid,
            const std::string& shared_dir) override
    {
        segment_id_ = writer_guid;
        segment_name_ = generate_segment_name(shared_dir, writer_guid);

        //Open the segment
        try
        {
            segment_ = std::unique_ptr<fastdds::rtps::SharedMemSegment>(
                new fastdds::rtps::SharedMemSegment(boost::interprocess::open_only,
                    segment_name_.c_str()));
        }
        catch (const std::exception& e)
        {
            logError(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to open segment " << segment_name_
                                                                                << ": " << e.what());
            return false;
        }

        // Get the pool description
        descriptor_ = segment_->get().find<PoolDescriptor>("descriptor").first;
        if (!descriptor_)
        {
            segment_.reset();

            logError(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to open payload pool descriptor " << segment_name_);
            return false;
        }

        // Get the pool buffer
        payloads_buffer_ = static_cast<octet*>(segment_->get_address_from_offset(descriptor_->payloads_base));

        // Set the reading pointer
        if (is_volatile_)
        {
            next_payload_ = end();
        }
        else
        {
            next_payload_ = begin();
        }

        return true;
    }

bool get_next_unread_payload(
        CacheChange_t& cache_change) override
{
    // TODO [ILG] access to next_payload is protected?
    if (next_payload_ != end())
    {
        PayloadNode* payload = static_cast<PayloadNode*>(segment_->get_address_from_offset(next_payload_));
        payload->reference();

        // TODO [ILG]:We must handle the endian here
        cache_change.serializedPayload.data = payload->data();
        cache_change.serializedPayload.max_size = payload->data_length();
        cache_change.serializedPayload.length = payload->data_length();
        cache_change.payload_owner(this);

        cache_change.kind = static_cast<ChangeKind_t>(payload->status());
        cache_change.writerGUID = payload->writer_GUID();
        cache_change.instanceHandle = payload->instance_handle();
        cache_change.sequenceNumber = payload->sequence_number();
        cache_change.sourceTimestamp = payload->source_timestamp();
        cache_change.write_params.sample_identity(payload->related_sample_identity());

        next_payload_ = advance(next_payload_);
        return true;
    }

    return false;
}

private:

    bool is_volatile_;              //< Whether the reader is volatile or not
    Segment::Offset next_payload_;  //< Next payload to read
};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_DATASHARINGPAYLOADPOOLIMPL_READERPOOL_HPP
