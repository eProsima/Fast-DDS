/*
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
 *
 */

#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_PRIMITIVE_TYPES_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_PRIMITIVE_TYPES_HPP_

#include <dds/core/xtypes/TypeKind.hpp>

// This template specialization have the intent to provide guidelines
// implementors of this specification. Notice that this define only a subset
// of primitive types, thus more work for you to do...

namespace dds {
namespace core {
namespace xtypes {

// Notice that const char* const is used instead of std::string
// to limit initialization issues with ctors.

template<>
struct dynamic_type_traits<bool>
{
    static constexpr TypeKind TYPE_ID = TypeKind::BOOLEAN_TYPE;
    static constexpr const char* NAME = "bool";
};


template<>
struct dynamic_type_traits<uint8_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::UINT_8_TYPE;
    static constexpr const char* NAME = "uint8_t";
};

template<>
struct dynamic_type_traits<int16_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::INT_16_TYPE;
    static constexpr const char* NAME = "int16_t";
};

template<>
struct dynamic_type_traits<uint16_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::UINT_16_TYPE;
    static constexpr const char* NAME = "uint16_t";
};

template<>
struct dynamic_type_traits<int32_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::INT_32_TYPE;
    static constexpr const char* NAME = "int32_t";
};

template<>
struct dynamic_type_traits<uint32_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::UINT_32_TYPE;
    static constexpr const char* NAME = "uint32_t";
};

template<>
struct dynamic_type_traits<int64_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::INT_64_TYPE;
    static constexpr const char* NAME = "int64_t";
};

template<>
struct dynamic_type_traits<uint64_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::UINT_64_TYPE;
    static constexpr const char* NAME = "uint64_t";
};

template<>
struct dynamic_type_traits<float>
{
    static constexpr TypeKind TYPE_ID = TypeKind::FLOAT_32_TYPE;
    static constexpr const char* NAME = "float";
};

template<>
struct dynamic_type_traits<double>
{
    static constexpr TypeKind TYPE_ID = TypeKind::FLOAT_64_TYPE;
    static constexpr const char* NAME = "double";
};

template<>
struct dynamic_type_traits<long double>
{
    static constexpr TypeKind TYPE_ID = TypeKind::FLOAT_128_TYPE;
    static constexpr const char* NAME = "longdouble";
};

template<>
struct dynamic_type_traits<char>
{
    static constexpr TypeKind TYPE_ID = TypeKind::CHAR_8_TYPE;
    static constexpr const char* NAME = "char";
};

template<>
struct dynamic_type_traits<char32_t>
{
    static constexpr TypeKind TYPE_ID = TypeKind::CHAR_32_TYPE;
    static constexpr const char* NAME = "char32_t";
};


} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_PRIMITIVE_TYPES_HPP_
