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
#include <dds/core/xtypes/SequenceInstance.hpp>

#include <vector>
#include <cassert>

namespace dds {
namespace core {
namespace xtypes {

class SequenceType : public MutableCollectionType
{
public:
    SequenceType(
            const DynamicType& content,
            uint32_t bounds = 0)
        : MutableCollectionType(
                TypeKind::SEQUENCE_TYPE,
                "sequence_" + ((bounds > 0) ? std::to_string(bounds) + "_" : "") + content.name(),
                DynamicType::Ptr(content),
                bounds)
    {}

    template<typename DynamicTypeImpl>
    SequenceType(
            const DynamicTypeImpl&& content,
            uint32_t bounds)
        : MutableCollectionType(
                TypeKind::SEQUENCE_TYPE,
                "sequence_" + ((bounds > 0) ? std::to_string(bounds) + "_" : "") + content.name(),
                DynamicType::Ptr(std::move(content)),
                bounds)
    {}

    SequenceType(const SequenceType& other) = default;
    SequenceType(SequenceType&& other) = default;

    virtual size_t memory_size() const override
    {
        return sizeof(SequenceInstance);
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        new (instance) SequenceInstance(content_type(), bounds());
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        new (target) SequenceInstance(*reinterpret_cast<const SequenceInstance*>(source));
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        assert(other.kind() == TypeKind::SEQUENCE_TYPE);
        new (target) SequenceInstance(*reinterpret_cast<const SequenceInstance*>(source), content_type(), bounds());
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source) const override
    {
        new (target) SequenceInstance(std::move(*reinterpret_cast<const SequenceInstance*>(source)));
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        reinterpret_cast<SequenceInstance*>(instance)->~SequenceInstance();
    }

    virtual uint8_t* get_instance_at(
            uint8_t* instance,
            size_t index) const override
    {
        return reinterpret_cast<SequenceInstance*>(instance)->operator[](uint32_t(index));
    }

    virtual size_t get_instance_size(
            const uint8_t* instance) const override
    {
        return reinterpret_cast<const SequenceInstance*>(instance)->size();
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return *reinterpret_cast<const SequenceInstance*>(instance)
            == *reinterpret_cast<const SequenceInstance*>(other_instance);
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if(other.kind() != TypeKind::SEQUENCE_TYPE)
        {
            return TypeConsistency::NONE;
        }

        const SequenceType& other_sequence = static_cast<const SequenceType&>(other);

        if(bounds() == other_sequence.bounds())
        {
            return TypeConsistency::EQUALS
                | content_type().is_compatible(other_sequence.content_type());
        }

        return TypeConsistency::IGNORE_SEQUENCE_BOUNDS
            | content_type().is_compatible(other_sequence.content_type());
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        const SequenceInstance& sequence = *reinterpret_cast<const SequenceInstance*>(node.instance);
        visitor(node);
        for(uint32_t i = 0; i < sequence.size(); i++)
        {
            InstanceNode child(node, content_type(), sequence[i], i, nullptr);
            content_type().for_each_instance(child, visitor);
        }
    }

    uint8_t* push_instance(
            uint8_t* instance,
            const uint8_t* value) const
    {
        if(get_instance_size(instance) < bounds() || bounds() == 0)
        {
            return reinterpret_cast<SequenceInstance*>(instance)->push(value);
        }
        return nullptr;
    }

    void resize_instance(
            uint8_t* instance,
            size_t size) const
    {
        reinterpret_cast<SequenceInstance*>(instance)->resize(size);
    }

protected:
    virtual DynamicType* clone() const override
    {
        return new SequenceType(*this);
    }
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_SEQUENCE_TYPE_HPP_
