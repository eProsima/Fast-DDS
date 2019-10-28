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

#ifndef OMG_DDS_CORE_XTYPES_STRUCT_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_STRUCT_TYPE_HPP_

#include <dds/core/xtypes/AggregationType.hpp>

#include <string>
#include <map>
#include <vector>

namespace dds {
namespace core {
namespace xtypes {

class StructType : public AggregationType
{
public:
    StructType(
            const std::string& name)
        : AggregationType(TypeKind::STRUCTURE_TYPE, name)
        , memory_size_(0)
    {}

    StructType(const StructType& other) = default;
    StructType(StructType&& other) = default;

    bool has_parent() const { return parent_.get() != nullptr; }
    const DynamicType& parent() const { return *parent_; }

    StructType& add_member(const Member& member)
    {
        Member& inner = insert_member(member);
        inner.offset_ = memory_size_;
        memory_size_ += inner.type().memory_size();
        return *this;
    }

    StructType& add_member(
            const std::string& name,
            const DynamicType& type)
    {
        return add_member(Member(name, type));
    }

    template<typename DynamicTypeImpl>
    StructType& add_member(
            const std::string& name,
            const DynamicTypeImpl&& type)
    {
        return add_member(Member(name, type));
    }

    virtual size_t memory_size() const override
    {
        return memory_size_;
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        for(auto&& member: members())
        {
            member.type().construct_instance(instance + member.offset());
        }
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        for(auto&& member: members())
        {
            member.type().copy_instance(target + member.offset(), source + member.offset());
        }
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        assert(other.kind() == TypeKind::STRUCTURE_TYPE);
        const StructType& other_struct = static_cast<const StructType&>(other);

        auto other_member = other_struct.members().begin();
        for(auto&& member: members())
        {
            if(other_member != other_struct.members().end())
            {
                member.type().copy_instance_from_type(
                        target + member.offset(),
                        source + other_member->offset(),
                        other_member->type());
            }
            else
            {
                member.type().construct_instance(target + member.offset());
            }
            other_member++;
        }
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source) const override
    {
        for(auto&& member: members())
        {
            member.type().move_instance(target + member.offset(), source + member.offset());
        }
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        for (auto&& member = members().rbegin(); member != members().rend(); ++member)
        {
            member->type().destroy_instance(instance + member->offset());
        }
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        for(auto&& member: members())
        {
            if(!member.type().compare_instance(instance + member.offset(), other_instance + member.offset()))
            {
                return false;
            }
        }
        return true;
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if(other.kind() != TypeKind::STRUCTURE_TYPE)
        {
            return TypeConsistency::NONE;
        }

        const StructType& other_struct = static_cast<const StructType&>(other);

        TypeConsistency consistency = TypeConsistency::EQUALS;
        auto other_member = other_struct.members().begin();
        for(auto&& member: members())
        {
            if(other_member != other_struct.members().end())
            {
                TypeConsistency internal_consistency = member.type().is_compatible(other_member->type());
                if(internal_consistency == TypeConsistency::NONE)
                {
                    return TypeConsistency::NONE;
                }

                if(member.name() != other_member->name())
                {
                    consistency |= TypeConsistency::IGNORE_MEMBER_NAMES;
                }
                consistency |= internal_consistency;
            }
            else
            {
                return TypeConsistency::IGNORE_MEMBERS;
            }
            other_member++;
        }
        if(other_member != other_struct.members().end())
        {
            consistency |= TypeConsistency::IGNORE_MEMBERS;
        }

        return consistency;
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        visitor(node);
        for(size_t i = 0; i < members().size(); i++)
        {
            const Member& member = members()[i];
            InstanceNode child(node, member.type(), node.instance + member.offset(), i, &member);
            member.type().for_each_instance(child, visitor);
        }
    }

protected:
    virtual DynamicType* clone() const override
    {
        return new StructType(*this);
    }

private:
    DynamicType::Ptr parent_;
    size_t memory_size_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_STRUCT_TYPE_HPP_
