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

#ifndef OMG_DDS_XTYPES_TYPE_CONSISTENCY_HPP_
#define OMG_DDS_XTYPES_TYPE_CONSISTENCY_HPP_

#include <type_traits>

namespace dds {
namespace core {
namespace xtypes {

/// \brief Differents type consistencies available for type compatibility.
/// When two types are checked by DynamicType::is_compatible
/// a set of consistencies is returned depending of the required modifications to allow the matching.
/// \remark TypeConssitency is an enum treats as a bitset.
enum class TypeConsistency
{
    /// \brief Unknow way to interpret the types as equivalents.
    NONE = ~0,

    /// \brief The types are totally equals.
    EQUALS = 0,

    /// \brief Ignore sign at PrimitiveTypes is required to interpret the types as equivalents.
    IGNORE_TYPE_SIGN = 1,

    /// \brief Ignore width at PrimitiveTypes is required to interpret the types as equivalents.
    IGNORE_TYPE_WIDTH = 2,

    /// \brief Ignore sequence bounds is required to interpret the types as equivalents.
    IGNORE_SEQUENCE_BOUNDS = 4,

    /// \brief Ignore array bounds is required to interpret the types as equivalents.
    IGNORE_ARRAY_BOUNDS = 8,

    /// \brief Ignore string bounds is required to interpret the types as equivalents.
    IGNORE_STRING_BOUNDS = 16,

    /// \brief Ignore member names is required to interpret the types as equivalents.
    IGNORE_MEMBER_NAMES = 32,

    /// \brief Ignore some members is required to interpret the types as equivalents.
    IGNORE_MEMBERS = 64,
};

/// \brief OR operator
inline TypeConsistency operator | (TypeConsistency lhs, TypeConsistency rhs)
{
    using T = std::underlying_type<TypeConsistency>::type;
    return static_cast<TypeConsistency>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

/// \brief OR operator
inline TypeConsistency& operator |= (TypeConsistency& lhs, TypeConsistency rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

/// \brief AND operator
inline TypeConsistency operator & (TypeConsistency lhs, TypeConsistency rhs)
{
    using T = std::underlying_type<TypeConsistency>::type;
    return static_cast<TypeConsistency>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

/// \brief AND operator
inline TypeConsistency& operator &= (TypeConsistency& lhs, TypeConsistency rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_XTYPES_TYPE_CONSISTENCY_HPP_

