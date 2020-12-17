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

#ifndef RTPS_DATASHARING_READERPOOL_HPP
#define RTPS_DATASHARING_READERPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>

#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class ReaderPool : public DataSharingPayloadPool
{

public:

    ReaderPool (
            bool is_volatile)
        : is_volatile_(is_volatile)
        , last_sn_(c_SequenceNumber_Unknown)
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
        assert(data_owner == this || data_owner == nullptr);

        assert(data_owner == this);
        if (data_owner == this)
        {
            cache_change.serializedPayload.data = data.data;
            cache_change.serializedPayload.length = data.length;
            cache_change.serializedPayload.max_size = data.length;
            cache_change.payload_owner(this);
            return true;
        }

        return false;
    }

    bool release_payload(
            CacheChange_t& cache_change) override
    {
        assert(cache_change.payload_owner() == this);

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
        descriptor_ = segment_->get().find<PoolDescriptor>(descriptor_chunk_name()).first;
        if (!descriptor_)
        {
            segment_.reset();

            logError(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to open payload pool descriptor " << segment_name_);
            return false;
        }

        // Get the history
        history_ = segment_->get().find<Segment::Offset>(history_chunk_name()).first;
        if (!history_)
        {
            segment_.reset();

            logError(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to open payload history " << segment_name_);
            return false;
        }

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

void get_next_unread_payload(
        CacheChange_t& cache_change,
        SequenceNumber_t& last_sequence_number)
{
    last_sequence_number = last_sn_;

    while (next_payload_ != end())
    {
        // history_[next_payload_] contains the offset to the payload
        PayloadNode* payload = static_cast<PayloadNode*>(segment_->get_address_from_offset(history_[next_payload_]));

        // The SN is the first thing to be invalidated on the writer
        cache_change.sequenceNumber = payload->sequence_number();
        if (cache_change.sequenceNumber == c_SequenceNumber_Unknown)
        {
            // Reset by the writer. Discard and continue
            next_payload_ = advance(next_payload_);
            if (last_sn_ != c_SequenceNumber_Unknown)
            {
                ++last_sn_;
            }
            logWarning(RTPS_READER, "Dirty data detected on datasharing writer " << writer());
            continue;
        }

        if (last_sn_ != c_SequenceNumber_Unknown && cache_change.sequenceNumber != last_sn_ + 1)
        {
            ++last_sn_;
            logWarning(RTPS_READER, "GAP for SN " << last_sn_ << " detected on datasharing writer " << writer());
        }
        else
        {
            last_sn_ = cache_change.sequenceNumber;
        }

        cache_change.serializedPayload.data = payload->data();
        cache_change.serializedPayload.max_size = payload->data_length();
        cache_change.serializedPayload.length = payload->data_length();

        cache_change.kind = static_cast<ChangeKind_t>(payload->status());
        cache_change.writerGUID = payload->writer_GUID();
        cache_change.instanceHandle = payload->instance_handle();
        cache_change.sourceTimestamp = payload->source_timestamp();
        cache_change.write_params.sample_identity(payload->related_sample_identity());

        cache_change.payload_owner(this);

        if (payload->sequence_number() != cache_change.sequenceNumber)
        {
            // Overriden while retrieving. Discard and continue
            next_payload_ = advance(next_payload_);
            ++last_sn_;
            logWarning(RTPS_READER, "Dirty data detected on datasharing writer " << writer());
            continue;
        }

        next_payload_ = advance(next_payload_);
        return;
    }

    cache_change.sequenceNumber = c_SequenceNumber_Unknown;
}

const SequenceNumber_t& get_last_read_sequence_number()
{
    return last_sn_;
}

private:

    bool is_volatile_;          //< Whether the reader is volatile or not
    uint32_t next_payload_;     //< Index of the next history position to read
    SequenceNumber_t last_sn_;  //< Sequence number of the last read payload
};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_DATASHARING_DATASHARINGPAYLOADPOOLIMPL_READERPOOL_HPP
