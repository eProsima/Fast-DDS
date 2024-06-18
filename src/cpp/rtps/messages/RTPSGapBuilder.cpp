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
 * @file RTPSGapBuilder.cpp
 *
 */

#include "RTPSGapBuilder.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

RTPSGapBuilder::~RTPSGapBuilder()
{
    flush();
}

bool RTPSGapBuilder::add(
        const SequenceNumber_t& gap_sequence)
{
    // Check if it is the first gap being added
    if (!is_gap_pending_)
    {
        is_gap_pending_ = true;
        initial_sequence_ = gap_sequence;
        gap_bitmap_.base(gap_sequence + 1);
        return true;
    }

    // Check for contiguous from initial_sequence_
    SequenceNumber_t base = gap_bitmap_.base();
    if (gap_sequence == base)
    {
        gap_bitmap_.base(gap_sequence + 1);
        return true;
    }

    // Check if past last in bitmap
    if (gap_bitmap_.add(gap_sequence))
    {
        return true;
    }

    // Did not fit inside bitmap. Difference between gap_sequence and base is greater than 255.
    // Send GAP with current info and prepare info for next GAP.
    bool ret_val = flush();
    is_gap_pending_ = true;
    initial_sequence_ = gap_sequence;
    gap_bitmap_.base(gap_sequence + 1);

    return ret_val;
}

bool RTPSGapBuilder::flush()
{
    if (is_gap_pending_)
    {
        bool ok = with_specific_destination_ ?
                group_.add_gap(initial_sequence_, gap_bitmap_, reader_guid_) :
                group_.add_gap(initial_sequence_, gap_bitmap_);
        if (!ok)
        {
            return false;
        }
    }

    is_gap_pending_ = false;
    return true;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
