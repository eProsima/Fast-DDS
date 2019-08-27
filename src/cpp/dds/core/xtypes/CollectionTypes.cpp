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

#include <dds/core/xtypes/CollectionTypes.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename DELEGATE>
uint32_t TCollectionType<DELEGATE>::bounds() const
{
}

template<typename DELEGATE>
TCollectionType<DELEGATE>::TCollectionType(
            const std::string& name,
            TypeKind kind)
{
    (void) name;
    (void) kind;
}

template<
        typename DELEGATE,
        typename DELEGATE_K,
        typename DELEGATE_V>
TMapType<DELEGATE, DELEGATE_K, DELEGATE_V>::TMapType(
            const TDynamicType<DELEGATE_K>& key_type,
            const TDynamicType<DELEGATE_V>& value_type)
{
    (void) key_type;
    (void) value_type;
}

template<
        typename DELEGATE,
        typename DELEGATE_K,
        typename DELEGATE_V>
TMapType<DELEGATE, DELEGATE_K, DELEGATE_V>::TMapType(
            const TDynamicType<DELEGATE_K>& key_type,
            const TDynamicType<DELEGATE_V>& value_type,
            uint32_t bounds)
{
    (void) key_type;
    (void) value_type;
    (void) bounds;
}

template<
        typename DELEGATE,
        typename DELEGATE_K,
        typename DELEGATE_V>
const TDynamicType<DELEGATE_K>& TMapType<DELEGATE, DELEGATE_K, DELEGATE_V>::key_type()
{
}

template<
        typename DELEGATE,
        typename DELEGATE_K,
        typename DELEGATE_V>
const TDynamicType<DELEGATE_V>& TMapType<DELEGATE, DELEGATE_K, DELEGATE_V>::value_type()
{
}

template<
        typename DELEGATE,
        typename DELEGATE_T>
TSequenceType<DELEGATE, DELEGATE_T>::TSequenceType(
        const TDynamicType<DELEGATE_T>& type)
{
    (void) type;
}

template<
        typename DELEGATE,
        typename DELEGATE_T>
TSequenceType<DELEGATE, DELEGATE_T>::TSequenceType(
        const TDynamicType<DELEGATE_T>& type,
        uint32_t bounds)
{
    (void) type;
    (void) bounds;
}

template<
        typename DELEGATE,
        typename DELEGATE_T>
const TDynamicType<DELEGATE_T>& TSequenceType<DELEGATE, DELEGATE_T>::key_type() const
{
}


template<typename DELEGATE>
TStringType<DELEGATE>::TStringType(
            uint32_t bounds)
{
    (void) bounds;
}

} //namespace xtypes
} //namespace core
} //namespace dds
