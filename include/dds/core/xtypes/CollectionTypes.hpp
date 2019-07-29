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

namespace dds{
namespace core{
namespace xtypes{


template<typename DELEGATE>
class TCollectionType : public TDynamicType<DELEGATE>
{
public:
    const uint32_t UNBOUNDED = 0xFFFFFFFF;

    uint32_t bounds() const;

protected:
    TCollectionType(
            const std::string& name, TypeKind kind);
};


typedef TCollectionType<detail::CollectionType> CollectionType;

template<typename DELEGATE>
class TMapType : public TCollectionType<DELEGATE>
{
public:
    /**
     * Create an unbounded Map with the given key/value type.
     */
    TMapType(
            const DynamicType& key_type,
            const DynamicType& value_type);

    /**
     * Create an bounded Map with the given key/value type.
     */
    TMapType(
            const DynamicType& key_type,
            const DynamicType& value_type,
            uint32_t bounds);

    const DynamicType& key_type();

    const DynamicType& value_type();
};

template<typename DELEGATE>
class TSequenceType : public TCollectionType<DELEGATE>
{
public:
    /**
     * Create an unbounded sequence for the given type.
     */
    TSequenceType(
            const DynamicType& type);

    /**
     * Create a bounded sequence for the given type.
     */
    TSequenceType(
            const DynamicType& type,
            uint32_t bounds);

    const DynamicType& key_type() const;
};

template<
        typename CHAR_T,
        template <typename C> class DELEGATE>
class TStringType : public TCollectionType<detail::CollectionType>
{
public:
    TStringType(
            uint32_t bounds);
};

typedef TMapType<detail::MapType> MapType;

typedef TSequenceType<detail::SequenceType> SequenceType;

template<
        typename CHAR_T,
        template <typename C> class DELEGATE = detail::StringType>
class TStringType;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_COLLECTION_TYPES_HPP_
