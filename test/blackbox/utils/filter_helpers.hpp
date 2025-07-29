// Copyright 2019, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/dds/core/Entity.hpp>
#include <fastrtps/rtps/common/Types.h>

#include "../types/core/core_typesPubSubTypes.h"

namespace eprosima {
namespace fastdds {
namespace helpers {

inline rtps::core::AckNackSubmessage cdr_parse_acknack_submsg(
        char* serialized_buffer,
        size_t length)
{
    eprosima::fastdds::rtps::core::AckNackSubmessage acknack_submsg;
    eprosima::fastcdr::FastBuffer buffer(serialized_buffer, length);
    eprosima::fastcdr::Cdr cdr(buffer,
            eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1);
    cdr >> acknack_submsg;
    return acknack_submsg;
}

inline uint16_t cdr_parse_u16(
        char* serialized_buffer)
{
    uint16_t u16;
    eprosima::fastcdr::FastBuffer buffer(serialized_buffer, 2);
    eprosima::fastcdr::Cdr cdr(buffer,
            eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1);
    cdr >> u16;
    return u16;
}

inline uint32_t cdr_parse_u32(
        char* serialized_buffer)
{
    uint32_t u32;
    eprosima::fastcdr::FastBuffer buffer(serialized_buffer, 4);
    eprosima::fastcdr::Cdr cdr(buffer,
            eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1);
    cdr >> u32;
    return u32;
}

} // namespace helpers
} // namespace fastdds
} // namespace eprosima
