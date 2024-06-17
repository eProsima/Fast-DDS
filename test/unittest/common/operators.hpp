// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <algorithm>
#include <cstdint>

#include <fastdds/rtps/common/SequenceNumber.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

// Make SequenceNumberSet_t compatible with GMock macros
bool operator ==(
        const SequenceNumberSet_t& a,
        const SequenceNumberSet_t& b)
{
    // remember that using SequenceNumberSet_t = BitmapRange<SequenceNumber_t, SequenceNumberDiff, 256>;
    // see test\unittest\utils\BitmapRangeTests.cpp method TestResult::Check

    if (a.empty() && b.empty())
    {
        return true;
    }

    if (a.base() == b.base())
    {
        uint32_t num_bits[2];
        uint32_t num_longs[2];
        SequenceNumberSet_t::bitmap_type bitmap[2];

        a.bitmap_get(num_bits[0], bitmap[0], num_longs[0]);
        b.bitmap_get(num_bits[1], bitmap[1], num_longs[1]);

        if (num_bits[0] != num_bits[1] || num_longs[0] != num_longs[1])
        {
            return false;
        }
        return std::equal(bitmap[0].cbegin(), bitmap[0].cbegin() + num_longs[0], bitmap[1].cbegin());
    }
    else
    {
        bool equal = true;

        a.for_each([&b, &equal](const SequenceNumber_t& e)
                {
                    equal &= b.is_set(e);
                });

        if (!equal)
        {
            return false;
        }

        b.for_each([&a, &equal](const SequenceNumber_t& e)
                {
                    equal &= a.is_set(e);
                });

        return equal;
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
