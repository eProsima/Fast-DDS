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

#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_STRUCT_TYPE_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_STRUCT_TYPE_HPP_

namespace dds {
namespace core {
namespace xtypes {
namespace detail {

template<typename T>
class TStructType; //[EPROSIMA CHECK]: changes StructType by TStructType with a template

template<typename T>
bool is_final(
        const TStructType<T>& s)
{
    return false;
}

template<typename T>
bool is_extensible(
        const TStructType<T>& s)
{
    return false;
}

template<typename T>
bool is_mutable(
        const TStructType<T>& s)
{
    return false;
}

template<typename T>
bool is_nested(
const TStructType<T>& s)
{
    return false;
}

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_STRUCT_TYPE_HPP_
