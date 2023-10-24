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

/*!
 * @file TypesBase.cpp
 */

#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastcdr {
template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::MemberFlag&,
        size_t& current_alignment)
{
    size_t calculated_size {2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2)};
    current_alignment += calculated_size;
    return calculated_size;
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& cdr,
        const eprosima::fastrtps::types::MemberFlag& data)
{
#if FASTCDR_VERSION_MAJOR == 1
    uint16_t bits = static_cast<uint16_t>(data.bitset().to_ulong());
    cdr << bits;
#else
    cdr << data.bitset();
#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        eprosima::fastrtps::types::MemberFlag& data)
{
#if FASTCDR_VERSION_MAJOR == 1
    uint16_t bits;
    cdr >> bits;
    data.bitset(std::bitset<16>(bits));
#else
    std::bitset<16> bitset;
    cdr >> bitset;
    data.bitset(bitset);
#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::TypeFlag&,
        size_t& current_alignment)
{
    size_t calculated_size {2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2)};
    current_alignment += calculated_size;
    return calculated_size;
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& cdr,
        const eprosima::fastrtps::types::TypeFlag& data)
{
#if FASTCDR_VERSION_MAJOR == 1
    uint16_t bits = static_cast<uint16_t>(data.bitset().to_ulong());
    cdr << bits;
#else
    cdr << data.bitset();
#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        eprosima::fastrtps::types::TypeFlag& data)
{
#if FASTCDR_VERSION_MAJOR == 1
    uint16_t bits;
    cdr >> bits;
    data.bitset(std::bitset<16>(bits));
#else
    std::bitset<16> bitset;
    cdr >> bitset;
    data.bitset(bitset);
#endif // FASTCDR_VERSION_MAJOR == 1
}

} // namespace fastcdr
} // namespace eprosima
