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

#ifndef OMG_DDS_CORE_XTYPES_COLLECTION_TYPES_HPP_
#define OMG_DDS_CORE_XTYPES_COLLECTION_TYPES_HPP_

#include <dds/core/xtypes/detail/CollectionTypes.hpp>
#include <dds/core/xtypes/DynamicType.hpp>

namespace dds {
namespace core {
namespace xtypes {


template<typename DELEGATE>
class TCollectionType : public TDynamicType<DELEGATE>
{
public:
    constexpr static uint32_t UNBOUNDED = 0xFFFFFFFF;

    uint32_t bounds() const;

protected:
    TCollectionType(
            const std::string& name,
            TypeKind kind); 
};

template<
        typename DELEGATE,
        typename DELEGATE_K,
        typename DELEGATE_V>
class TMapType : public TCollectionType<DELEGATE>
{
public:
    TMapType(
            const TDynamicType<DELEGATE_K>& key_type,
            const TDynamicType<DELEGATE_V>& value_type);

    TMapType(
            const TDynamicType<DELEGATE_K>& key_type,
            const TDynamicType<DELEGATE_V>& value_type,
            uint32_t bounds);

    const TDynamicType<DELEGATE_K>& key_type();

    const TDynamicType<DELEGATE_V>& value_type();
};


template<
        typename DELEGATE,
        typename DELEGATE_T>
class TSequenceType : public TCollectionType<DELEGATE>
{
public:
    TSequenceType(
        const TDynamicType<DELEGATE_T>& type);

    TSequenceType(
        const TDynamicType<DELEGATE_T>& type,
        uint32_t bounds);
public:
    const TDynamicType<DELEGATE_T>& key_type() const;
};


template<typename DELEGATE>
class TStringType : public TCollectionType<DELEGATE>
{
public:
    TStringType(
            uint32_t bounds);
};

typedef TCollectionType<detail::CollectionType> CollectionType;
typedef TMapType<detail::MapType, detail::DynamicType, detail::DynamicType> TypeMap;
typedef TSequenceType<detail::SequenceType, detail::DynamicType> SequenceType;
typedef TStringType<detail::StringType> StringType;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_COLLECTION_TYPES_HPP_
