// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rtps/reader/FragmentedChangePitStop.h>
#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps::rtps;
using Log = eprosima::fastdds::dds::Log;

bool FragmentedChangePitStop::add_fragments_to_change(
        CacheChange_t* change,
        const SerializedPayload_t& incoming_data,
        uint32_t fragment_starting_num,
        uint32_t fragments_in_submessage)
{
    uint32_t fragment_size = change->getFragmentSize();
    uint32_t original_offset = (fragment_starting_num - 1) * fragment_size;
    uint32_t total_length = change->serializedPayload.length;
    uint32_t incoming_length = incoming_data.length;
    uint32_t total_fragments = change->getFragmentCount();
    uint32_t last_fragment_index = fragment_starting_num + fragments_in_submessage - 1;

    // Validate fragment indexes
    if (last_fragment_index > total_fragments)
    {
        logWarning(RTPS_MSG_IN, "Inconsistent fragment numbers " << last_fragment_index << " > " << total_fragments);
        return false;
    }

    // validate lengths
    if (original_offset + incoming_length > total_length)
    {
        logWarning(RTPS_MSG_IN, "Incoming fragment length would exceed sample length");
        return false;
    }

    if (last_fragment_index < total_fragments)
    {
        if (incoming_length % fragment_size != 0)
        {
            logWarning(RTPS_MSG_IN, "Incoming payload length not multiple of fragment size");
            return false;
        }
    }

    change->received_fragments(fragment_starting_num - 1, fragments_in_submessage);

    memcpy(
        &change->serializedPayload.data[original_offset],
        incoming_data.data, incoming_length);

    return change->is_fully_assembled();
}

CacheChange_t* FragmentedChangePitStop::process(
        CacheChange_t* incoming_change,
        uint32_t sample_size,
        FragmentNumber_t fragment_starting_num,
        uint16_t fragments_in_submessage)
{
    CacheChange_t* returnedValue = nullptr;

    // Search CacheChange_t with the sample sequence number.
    auto range = changes_.equal_range(ChangeInPit(incoming_change));

    auto original_change_cit = range.first;

    // If there is a range, search the CacheChange_t with the same writer GUID_t.
    if(original_change_cit != changes_.end())
    {
        for(; original_change_cit != range.second; ++original_change_cit)
        {
            if(original_change_cit->getChange()->writerGUID == incoming_change->writerGUID)
                break;
        }
    }
    else
        original_change_cit = range.second;

    // If not found an existing CacheChange_t, reserve one and insert.
    if(original_change_cit == range.second)
    {
        CacheChange_t* original_change = nullptr;

        if(!parent_->reserveCache(&original_change, sample_size))
            return nullptr;

        if(original_change->serializedPayload.max_size < sample_size)
        {
            parent_->releaseCache(original_change);
            return nullptr;
        }

        //Change comes preallocated (size sampleSize)
        original_change->copy_not_memcpy(incoming_change);
        // The length of the serialized payload has to be sample size.
        original_change->serializedPayload.length = sample_size;
        original_change->setFragmentSize(incoming_change->getFragmentSize(), true);

        // Insert
        original_change_cit = changes_.insert(ChangeInPit(original_change));
    }

    if(add_fragments_to_change(
        original_change_cit->getChange(),
        incoming_change->serializedPayload,
        fragment_starting_num,
        fragments_in_submessage))
    {
        returnedValue = original_change_cit->getChange();
        changes_.erase(original_change_cit);
    }

    return returnedValue;
}

CacheChange_t* FragmentedChangePitStop::find(const SequenceNumber_t& sequence_number, const GUID_t& writer_guid)
{
    CacheChange_t* returnedValue = nullptr;

    auto range = changes_.equal_range(ChangeInPit(sequence_number));

    auto cit = range.first;

    // If there is a range, search the CacheChange_t with the same writer GUID_t.
    if(cit != changes_.end())
    {
        for(; cit != range.second; ++cit)
        {
            if(cit->getChange()->writerGUID == writer_guid)
            {
                returnedValue = cit->getChange();
                break;
            }
        }
    }

    return returnedValue;
}

bool FragmentedChangePitStop::try_to_remove(const SequenceNumber_t& sequence_number, const GUID_t& writer_guid)
{
    bool returnedValue = false;

    auto range = changes_.equal_range(ChangeInPit(sequence_number));

    auto cit = range.first;

    // If there is a range, search the CacheChange_t with the same writer GUID_t.
    if(cit != changes_.end())
    {
        for(; cit != range.second; ++cit)
        {
            if(cit->getChange()->writerGUID == writer_guid)
            {
                // Destroy CacheChange_t.
                parent_->releaseCache(cit->getChange());
                changes_.erase(cit);
                returnedValue = true;
                break;
            }
        }
    }

    return returnedValue;
}

bool FragmentedChangePitStop::try_to_remove_until(const SequenceNumber_t& sequence_number, const GUID_t& writer_guid)
{
    bool returnedValue = false;

    auto cit = changes_.begin();
    while(cit != changes_.end())
    {
        if(cit->getChange()->sequenceNumber < sequence_number &&
                cit->getChange()->writerGUID == writer_guid)
        {
            // Destroy CacheChange_t.
            parent_->releaseCache(cit->getChange());
            cit = changes_.erase(cit);
            returnedValue = true;
        }
        else
        {
            ++cit;
        }
    }

    return returnedValue;
}
