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

#include "FragmentedChangePitStop.h"
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps::rtps;
using Log = eprosima::fastrtps::Log;

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
