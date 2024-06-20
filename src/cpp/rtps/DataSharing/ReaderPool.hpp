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

#ifndef FASTDDS_RTPS_DATASHARING__READERPOOL_HPP
#define FASTDDS_RTPS_DATASHARING__READERPOOL_HPP

#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>

#include <memory>

namespace eprosima {
namespace fastdds {
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
            SerializedPayload_t& /*payload*/) override
    {
        // Only WriterPool can return new payloads
        return false;
    }

    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override
    {
        if (data.payload_owner == this)
        {
            payload.data = data.data;
            payload.length = data.length;
            payload.max_size = data.length;
            payload.payload_owner = this;
            return true;
        }
        return false;
    }

    /**
     * Prepares and fills the change in the datasharing protocol.
     *
     * If the payload is not owned by this pool, it is assumed to be an intraprocess writer and the
     * change is direclty read from the shared history.
     *
     * @param data The serialized payload data to be used.
     * @param cache_change The cache change to be prepared.
     */
    void get_datasharing_change(
            SerializedPayload_t& data,
            CacheChange_t& cache_change)
    {
        if (!get_payload(data, cache_change.serializedPayload))
        {
            // If owner is not this, then it must be an intraprocess datasharing writer
            assert(nullptr != dynamic_cast<DataSharingPayloadPool*>(data.payload_owner));
            PayloadNode* payload = PayloadNode::get_from_data(data.data);

            // No need to check validity, on intraprocess there is no override of payloads
            read_from_shared_history(cache_change, payload);
        }
    }

    bool release_payload(
            SerializedPayload_t& payload) override
    {
        assert(payload.payload_owner == this);

        return DataSharingPayloadPool::release_payload(payload);
    }

    template <typename T>
    bool init_shared_segment(
            const GUID_t& writer_guid,
            const std::string& shared_dir)
    {
        segment_id_ = writer_guid;
        segment_name_ = generate_segment_name(shared_dir, writer_guid);

        std::unique_ptr<T> local_segment;
        // Open the segment
        try
        {
            local_segment = std::unique_ptr<T>(
                new T(boost::interprocess::open_read_only,
                segment_name_.c_str()));
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to open segment " << segment_name_
                                                                                          << ": " << e.what());
            return false;
        }

        // Get the pool description
        descriptor_ = local_segment->get().template find<PoolDescriptor>(descriptor_chunk_name()).first;
        if (!descriptor_)
        {
            local_segment.reset();

            EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_PAYLOADPOOL,
                    "Failed to open payload pool descriptor " << segment_name_);
            return false;
        }

        // Get the history
        history_ = local_segment->get().template find<Segment::Offset>(history_chunk_name()).first;
        if (!history_)
        {
            local_segment.reset();

            EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_PAYLOADPOOL, "Failed to open payload history " << segment_name_);
            return false;
        }

        // Set the reading pointer
        next_payload_ = begin();
        segment_ = std::move(local_segment);
        if (is_volatile_)
        {
            CacheChange_t ch;
            SequenceNumber_t last_sequence = c_SequenceNumber_Unknown;
            uint64_t current_end = end();
            get_next_unread_payload(ch, last_sequence, current_end);
            while (ch.sequenceNumber != SequenceNumber_t::unknown() || next_payload_ != current_end)
            {
                current_end = end();
                advance(next_payload_);
                get_next_unread_payload(ch, last_sequence, current_end);
            }
            assert(next_payload_ == current_end);
        }

        return true;
    }

    bool init_shared_memory(
            const GUID_t& writer_guid,
            const std::string& shared_dir) override
    {
        if (shared_dir.empty())
        {
            return init_shared_segment<fastdds::rtps::SharedMemSegment>(writer_guid, shared_dir);
        }
        else
        {
            return init_shared_segment<fastdds::rtps::SharedFileSegment>(writer_guid, shared_dir);
        }
    }

    void get_next_unread_payload(
            CacheChange_t& cache_change,
            SequenceNumber_t& last_sequence_number)
    {
        get_next_unread_payload(cache_change, last_sequence_number, end());
    }

    void get_next_unread_payload(
            CacheChange_t& cache_change,
            SequenceNumber_t& last_sequence_number,
            uint64_t until)
    {
        last_sequence_number = last_sn_;

        while (next_payload_ < until)
        {
            // First ensure we are not too far behind
            // This may move the next_payload_ past the until value
            if (!ensure_reading_reference_is_in_bounds() && next_payload_ >= until)
            {
                break;
            }

            // history_[next_payload_] contains the offset to the payload
            PayloadNode* payload = static_cast<PayloadNode*>(
                segment_->get_address_from_offset(history_[static_cast<uint32_t>(next_payload_)]));
            if (!read_from_shared_history(cache_change, payload))
            {
                // Overriden while retrieving. Discard and continue
                advance(next_payload_);
                EPROSIMA_LOG_WARNING(RTPS_READER, "Dirty data detected on datasharing writer " << writer());
                continue;
            }

            if (last_sn_ != c_SequenceNumber_Unknown && last_sn_ >= cache_change.sequenceNumber)
            {
                // Sequence number went backwards, it was most probably overriden.
                continue;
            }

            if (!ensure_reading_reference_is_in_bounds())
            {
                // We may have been taken over and read a payload that is too far forward. Discard and continue
                continue;
            }

            last_sn_ = cache_change.sequenceNumber;

            return;
        }

        // Reset the data (may cause errors later on)
        cache_change.sequenceNumber = c_SequenceNumber_Unknown;
        cache_change.serializedPayload.data = nullptr;
        cache_change.serializedPayload.payload_owner = nullptr;
    }

    bool read_from_shared_history(
            CacheChange_t& cache_change,
            PayloadNode* payload)
    {
        // The sequence number can be unknown already, but we defer the check to the end
        cache_change.sequenceNumber = payload->sequence_number();

        cache_change.serializedPayload.data = payload->data();
        cache_change.serializedPayload.max_size = payload->data_length();
        cache_change.serializedPayload.length = payload->data_length();

        cache_change.kind = static_cast<ChangeKind_t>(payload->status());
        cache_change.writerGUID = payload->writer_GUID();
        cache_change.instanceHandle = payload->instance_handle();
        cache_change.sourceTimestamp = payload->source_timestamp();
        cache_change.write_params.sample_identity(payload->related_sample_identity());

        SequenceNumber_t check = payload->sequence_number();
        if (check == c_SequenceNumber_Unknown || check != cache_change.sequenceNumber)
        {
            // data override while processing
            return false;
        }

        cache_change.serializedPayload.payload_owner = this;
        return true;
    }

    const SequenceNumber_t& get_last_read_sequence_number()
    {
        return last_sn_;
    }

    bool advance_to_next_payload()
    {
        if (next_payload_ < end())
        {
            advance(next_payload_);
            return true;
        }
        return false;
    }

protected:

    bool ensure_reading_reference_is_in_bounds()
    {
        auto notified_end = end();
        auto notified_end_high = notified_end >> 32;
        auto next_payload_high = next_payload_ >> 32;
        if (next_payload_high + 1 < notified_end_high ||
                (next_payload_high < notified_end_high &&
                static_cast<uint32_t>(next_payload_) <= static_cast<uint32_t>(notified_end)))
        {
            EPROSIMA_LOG_WARNING(RTPS_READER, "Writer " << writer() << " overtook reader in datasharing pool."
                                                        << " Some changes will be missing.");

            // lower part is the index, upper part is the loop counter
            next_payload_ = ((notified_end_high - 1) << 32) + static_cast<uint32_t>(notified_end);
            advance(next_payload_);
            return false;
        }
        return true;
    }

private:

    using DataSharingPayloadPool::init_shared_memory;

    bool is_volatile_;              //< Whether the reader is volatile or not
    uint64_t next_payload_;         //< Index of the next history position to read
    SequenceNumber_t last_sn_;      //< Sequence number of the last read payload
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_DATASHARING__READERPOOL_HPP
