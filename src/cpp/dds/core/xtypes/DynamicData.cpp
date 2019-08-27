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

#include <dds/core/xtypes/DynamicData.hpp>

namespace dds {
namespace core {
namespace xtypes {

/**
 * This class is used to read/write data for DynamicTypes. It allows reading and
 * writing of samples in a type-safe manner but without any compile-time knowledge
 * of the type being read or written.
 */
template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
TDynamicData<DELEGATE, DELEGATE_TYPE>::TDynamicData(
            const TDynamicType<DELEGATE_TYPE>& type)
{
    (void) type;
}

template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
template<typename T>
void TDynamicData<DELEGATE, DELEGATE_TYPE>::value(
        uint32_t mid,
        const T& v) const
{
    (void) mid;
    (void) v;
}

template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
template<typename T>
T TDynamicData<DELEGATE, DELEGATE_TYPE>::value(
        const std::string& mid,
        const T& v) const
{
    (void) mid;
    (void) v;
}

template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
template<typename T>
T TDynamicData<DELEGATE, DELEGATE_TYPE>::value(
        uint32_t mid) const{
    (void) mid;
}

template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
template<typename T>
T TDynamicData<DELEGATE, DELEGATE_TYPE>::value(
        const std::string& mid) const{
    (void) mid;
}

template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
TDynamicType<DELEGATE_TYPE> TDynamicData<DELEGATE, DELEGATE_TYPE>::type() const
{
}

template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
MemberType TDynamicData<DELEGATE, DELEGATE_TYPE>::member_type(
        uint32_t id) const
{
    (void) id;
}

template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
MemberType TDynamicData<DELEGATE, DELEGATE_TYPE>::member_type(
        const std::string& name) const
{
    (void) name;
}


template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
uint32_t TDynamicData<DELEGATE, DELEGATE_TYPE>::member_id(
        const std::string& name) const
{
    (void) name;
}

} //namespace xtypes
} //namespace core
} //namespace dds
