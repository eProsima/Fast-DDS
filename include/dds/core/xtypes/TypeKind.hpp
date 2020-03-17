/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OMG_DDS_XTYPES_TYPE_KIND_HPP_
#define OMG_DDS_XTYPES_TYPE_KIND_HPP_

#include <dds/core/detail/conformance.hpp>
#include <dds/core/SafeEnumeration.hpp>

#include <bitset>

#if defined(OMG_DDS_X_TYPES_DYNAMIC_TYPE_SUPPORT)

namespace dds {
namespace core {
namespace xtypes {

enum class TypeKind
{
    NO_TYPE          = 0, // sentinel indicating "null" value
    PRIMITIVE_TYPE   = 0x4000,
    CONSTRUCTED_TYPE = 0x8000,
    COLLECTION_TYPE  = 0x0200,
    AGGREGATION_TYPE = 0x0100,
    ANNOTATION_TYPE  = 0x0080,

    BOOLEAN_TYPE     = PRIMITIVE_TYPE | 0x0001,
    UINT_8_TYPE      = PRIMITIVE_TYPE | 0x0002,
    INT_16_TYPE      = PRIMITIVE_TYPE | 0x0003,
    UINT_16_TYPE     = PRIMITIVE_TYPE | 0x0004,
    INT_32_TYPE      = PRIMITIVE_TYPE | 0x0005,
    UINT_32_TYPE     = PRIMITIVE_TYPE | 0x0006,
    INT_64_TYPE      = PRIMITIVE_TYPE | 0x0007,
    UINT_64_TYPE     = PRIMITIVE_TYPE | 0x0008,
    FLOAT_32_TYPE    = PRIMITIVE_TYPE | 0x0009,
    FLOAT_64_TYPE    = PRIMITIVE_TYPE | 0x000A,
    FLOAT_128_TYPE   = PRIMITIVE_TYPE | 0x000B,
    CHAR_8_TYPE      = PRIMITIVE_TYPE | 0x000C,
    CHAR_32_TYPE     = PRIMITIVE_TYPE | 0x000D,

    ENUMERATION_TYPE = CONSTRUCTED_TYPE | 0x0001,
    BITSET_TYPE      = CONSTRUCTED_TYPE | 0x0002,
    ALIAS_TYPE       = CONSTRUCTED_TYPE | 0x0003,

    ARRAY_TYPE       = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0004,
    SEQUENCE_TYPE    = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0005,
    STRING_TYPE      = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0006,
    MAP_TYPE         = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0007,

    UNION_TYPE       = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x0008,
    STRUCTURE_TYPE   = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x0009,
    UNION_FWD_DECL_TYPE       = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x000A,
    STRUCTURE_FWD_DECL_TYPE   = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x000B
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_X_TYPES_DYNAMIC_TYPE_SUPPORT

#endif //OMG_DDS_XTYPES_TYPE_KIND_HPP_

