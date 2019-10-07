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
 *
*/

#ifndef OMG_DDS_CORE_XTYPES_SEQUENCE_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_SEQUENCE_TYPE_HPP_

#include <dds/core/xtypes/MutableCollectionType.hpp>

namespace dds {
namespace core {
namespace xtypes {

class SequenceType : public MutableCollectionType
{
public:
    SequenceType(
            const DynamicType& content,
            uint32_t bounds = 0)
        : MutableCollectionType(TypeKind::SEQUENCE_TYPE, "sequence_" + std::to_string(bounds), DynamicType::Ptr(content), bounds)
    {}

    template<typename DynamicTypeImpl>
    SequenceType(
            const DynamicTypeImpl&& content,
            uint32_t bounds = 0)
        : MutableCollectionType(TypeKind::SEQUENCE_TYPE, "sequence_" + std::to_string(bounds), DynamicType::Ptr(std::move(content)), bounds)
    {}

    SequenceType(const SequenceType& other) = default;
    SequenceType(SequenceType&& other) = default;

    virtual size_t memory_size() const
    {
        return 0; //TODO
    }

    virtual void init_instance(uint8_t* /*instance*/) const
    {
        //TODO
    }

    virtual void copy_instance(uint8_t* /*target*/, const uint8_t* /*source*/) const
    {
        //TODO
    }

    virtual void destroy_instance(uint8_t* /*instance*/) const
    {
        //TODO
    }

protected:
    virtual DynamicType* clone() const
    {
        return new SequenceType(*this);
    }
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_SEQUENCE_TYPE_HPP_
