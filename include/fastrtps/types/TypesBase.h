// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_BASE_H
#define TYPES_BASE_H

#include <string>

#include <fastdds/dds/xtypes/Types.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {

//! A special value indicating an unlimited quantity
using eprosima::fastdds::dds::BOUND_UNLIMITED;

using eprosima::fastdds::dds::CONST_TRUE;
using eprosima::fastdds::dds::CONST_FALSE;

using eprosima::fastdds::dds::ANNOTATION_KEY_ID;
using eprosima::fastdds::dds::ANNOTATION_EPKEY_ID;
using eprosima::fastdds::dds::ANNOTATION_TOPIC_ID;
using eprosima::fastdds::dds::ANNOTATION_EXTENSIBILITY_ID;
using eprosima::fastdds::dds::ANNOTATION_FINAL_ID;
using eprosima::fastdds::dds::ANNOTATION_APPENDABLE_ID;
using eprosima::fastdds::dds::ANNOTATION_MUTABLE_ID;
using eprosima::fastdds::dds::ANNOTATION_NESTED_ID;
using eprosima::fastdds::dds::ANNOTATION_OPTIONAL_ID;
using eprosima::fastdds::dds::ANNOTATION_MUST_UNDERSTAND_ID;
using eprosima::fastdds::dds::ANNOTATION_NON_SERIALIZED_ID;
using eprosima::fastdds::dds::ANNOTATION_BIT_BOUND_ID;
using eprosima::fastdds::dds::ANNOTATION_DEFAULT_ID;
using eprosima::fastdds::dds::ANNOTATION_DEFAULT_LITERAL_ID;
using eprosima::fastdds::dds::ANNOTATION_VALUE_ID;
using eprosima::fastdds::dds::ANNOTATION_POSITION_ID;
using eprosima::fastdds::dds::ANNOTATION_EXTERNAL_ID;

using eprosima::fastdds::dds::EXTENSIBILITY_FINAL;
using eprosima::fastdds::dds::EXTENSIBILITY_APPENDABLE;
using eprosima::fastdds::dds::EXTENSIBILITY_MUTABLE;

using eprosima::fastdds::dds::TKNAME_BOOLEAN;
using eprosima::fastdds::dds::TKNAME_INT16;
using eprosima::fastdds::dds::TKNAME_UINT16;
using eprosima::fastdds::dds::TKNAME_INT32;
using eprosima::fastdds::dds::TKNAME_UINT32;
using eprosima::fastdds::dds::TKNAME_INT64;
using eprosima::fastdds::dds::TKNAME_UINT64;
using eprosima::fastdds::dds::TKNAME_CHAR8;
using eprosima::fastdds::dds::TKNAME_BYTE;
using eprosima::fastdds::dds::TKNAME_UINT8;
using eprosima::fastdds::dds::TKNAME_CHAR16;
using eprosima::fastdds::dds::TKNAME_CHAR16T;
using eprosima::fastdds::dds::TKNAME_FLOAT32;
using eprosima::fastdds::dds::TKNAME_FLOAT64;
using eprosima::fastdds::dds::TKNAME_FLOAT128;

using eprosima::fastdds::dds::TKNAME_STRING8;
using eprosima::fastdds::dds::TKNAME_STRING16;
using eprosima::fastdds::dds::TKNAME_ALIAS;
using eprosima::fastdds::dds::TKNAME_ENUM;
using eprosima::fastdds::dds::TKNAME_BITMASK;
using eprosima::fastdds::dds::TKNAME_ANNOTATION;
using eprosima::fastdds::dds::TKNAME_STRUCTURE;
using eprosima::fastdds::dds::TKNAME_UNION;
using eprosima::fastdds::dds::TKNAME_BITSET;
using eprosima::fastdds::dds::TKNAME_SEQUENCE;
using eprosima::fastdds::dds::TKNAME_ARRAY;
using eprosima::fastdds::dds::TKNAME_MAP;

using eprosima::fastdds::dds::MAX_BITMASK_LENGTH;
using eprosima::fastdds::dds::MAX_ELEMENTS_COUNT;
using eprosima::fastdds::dds::MAX_STRING_LENGTH;

using eprosima::fastdds::dds::MemberId;
using eprosima::fastdds::dds::MEMBER_ID_INVALID;

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_BASE_H
