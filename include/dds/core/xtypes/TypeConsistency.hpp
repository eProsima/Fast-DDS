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

enum class TypeConsistency
{
    NONE = ~0,
    EQUALS = 0,
    IGNORE_TYPE_SIGN = 1,
    IGNORE_TYPE_WIDTH = 2,
    IGNORE_SEQUENCE_BOUNDS = 4,
    IGNORE_ARRAY_BOUNDS = 8,
    IGNORE_STRING_BOUNDS = 16,
    IGNORE_MEMBER_NAMES = 32,
    IGNORE_MEMBERS = 64,
};

inline TypeConsistency operator | (TypeConsistency lhs, TypeConsistency rhs)
{
    using T = std::underlying_type<TypeConsistency>::type;
    return static_cast<TypeConsistency>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline TypeConsistency& operator |= (TypeConsistency& lhs, TypeConsistency rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline TypeConsistency operator & (TypeConsistency lhs, TypeConsistency rhs)
{
    using T = std::underlying_type<TypeConsistency>::type;
    return static_cast<TypeConsistency>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline TypeConsistency& operator &= (TypeConsistency& lhs, TypeConsistency rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_XTYPES_TYPE_CONSISTENCY_HPP_

