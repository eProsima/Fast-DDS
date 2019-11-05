/*
 * Copyright 2010, Object Management Group, Inc.
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

#include <type_traits>

namespace dds {
namespace core {
namespace xtypes {

/// \brief Enumeration that describe the kind of a DynamicType.
/// This enumeration is treated as a bitset.
enum class TypeKind
{
    NO_TYPE          = 0, ///< No valid type
    PRIMITIVE_TYPE   = 0x4000, ///< Represents a PrimitiveType
    CONSTRUCTED_TYPE = 0x8000, ///< Represents a non PrimitiveType
    COLLECTION_TYPE  = 0x0200, ///< Represents a CollectionType
    AGGREGATION_TYPE = 0x0100, ///< Represents an AggregationType

    UNSIGNED_TYPE   = 0x80, ///< Represents an unsigned value

    BOOLEAN_TYPE     = PRIMITIVE_TYPE | 0x0001, ///< bool
    UINT_8_TYPE      = PRIMITIVE_TYPE | 0x0002 | UNSIGNED_TYPE, ///< uint8_t
    INT_16_TYPE      = PRIMITIVE_TYPE | 0x0003, ///< int16_t
    UINT_16_TYPE     = PRIMITIVE_TYPE | 0x0004 | UNSIGNED_TYPE, ///< uint16_t
    INT_32_TYPE      = PRIMITIVE_TYPE | 0x0005, ///< int32_t
    UINT_32_TYPE     = PRIMITIVE_TYPE | 0x0006 | UNSIGNED_TYPE, ///< uint32_t
    INT_64_TYPE      = PRIMITIVE_TYPE | 0x0007, ///< int64_t
    UINT_64_TYPE     = PRIMITIVE_TYPE | 0x0008 | UNSIGNED_TYPE, ///< uint64_t
    FLOAT_32_TYPE    = PRIMITIVE_TYPE | 0x0009, ///< float
    FLOAT_64_TYPE    = PRIMITIVE_TYPE | 0x000A, ///< double
    FLOAT_128_TYPE   = PRIMITIVE_TYPE | 0x000B, ///< long double
    CHAR_8_TYPE      = PRIMITIVE_TYPE | 0x000C, ///< char
    CHAR_16_TYPE     = PRIMITIVE_TYPE | 0x000D, ///< wchar

    ENUMERATION_TYPE = CONSTRUCTED_TYPE | 0x0001, ///< Not supported
    BITSET_TYPE      = CONSTRUCTED_TYPE | 0x0002, ///< Not supported
    ALIAS_TYPE       = CONSTRUCTED_TYPE | 0x0003, ///< Not supported

    ARRAY_TYPE       = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0004, ///< Reprensets an ArrayType
    SEQUENCE_TYPE    = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0005, ///< Reprensets a SequenceType
    STRING_TYPE      = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0006, ///< Represents a StringType
    WSTRING_TYPE     = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0007, ///< Represents a WStringType
    MAP_TYPE         = CONSTRUCTED_TYPE | COLLECTION_TYPE | 0x0008, ///< Not supported

    UNION_TYPE                = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x0009, ///< Not supported
    STRUCTURE_TYPE            = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x000A, ///< Represents a StructType
    UNION_FWD_DECL_TYPE       = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x000B, ///< Not supported
    STRUCTURE_FWD_DECL_TYPE   = CONSTRUCTED_TYPE | AGGREGATION_TYPE | 0x000C ///< Not supported
};

/// \brief OR operator
inline TypeKind operator | (TypeKind lhs, TypeKind rhs)
{
    using T = std::underlying_type<TypeKind>::type;
    return static_cast<TypeKind>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

/// \brief OR operator
inline TypeKind& operator |= (TypeKind& lhs, TypeKind rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

/// \brief AND operator
inline TypeKind operator & (TypeKind lhs, TypeKind rhs)
{
    using T = std::underlying_type<TypeKind>::type;
    return static_cast<TypeKind>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

/// \brief AND operator
inline TypeKind& operator &= (TypeKind& lhs, TypeKind rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_XTYPES_TYPE_KIND_HPP_

