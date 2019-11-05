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

/**
 * @file FragmentedChangePitStop.h
 */
#ifndef _RTPS_READER_FRAGMENTEDCHANGEPITSTOP_H_
#define _RTPS_READER_FRAGMENTEDCHANGEPITSTOP_H_

#include <fastrtps/rtps/common/CacheChange.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/*!
 * @brief Manages not completed fragmented CacheChanges in reader side.
 * @remarks This class is non thread-safe.
 */
class FragmentedChangePitStop
{
public:

    /*!
     * @brief Add received fragments to change
     *
     * @param change Pointer to the change being reassembled
     * @param incoming_data Serialized payload received on the DATA_FRAG message
     * @param fragment_starting_num First fragment number (1-based) as received on the DATA_FRAG message
     * @param fragments_in_submessage Number of fragments as received on the DATA_FRAG message
     *
     * @return true if the change is fully reassembled after adding received fragments.
     */
    static bool add_fragments_to_change(
            CacheChange_t* change,
            const SerializedPayload_t& incoming_data,
            uint32_t fragment_starting_num,
            uint32_t fragments_in_submessage);

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_READER_FRAGMENTEDCHANGEPITSTOP_H_
