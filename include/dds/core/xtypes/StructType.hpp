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
#include <dds/core/xtypes/StructMember.hpp>

#include <string>
#include <map>
#include <vector>

namespace dds {
namespace core {
namespace xtypes {

class StructType : public AggregationType<StructMember>
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

    StructType&& add_member(StructMember&& member)
    {
        StructMember& inner = insert_member(member.name(), std::move(member));
        inner.offset_ = memory_size_;
        memory_size_ += inner.type().memory_size();
        return std::move(*this);
    }

    virtual bool is_subset_of(const DynamicType& other) const
    {
        if(other.kind() != TypeKind::STRUCTURE_TYPE)
        {
            return false;
        }

        const StructType& other_struct = static_cast<const StructType&>(other);
        bool comp = true;
        for(auto&& it: member_map())
        {
            if(!it.second.is_optional())
            {
                auto other_it = other_struct.member_map().find(it.first);
                if(other_it != other_struct.member_map().end())
                {
                    comp &= it.second.type().is_subset_of(other_it->second.type());
                }
                else
                {
                    return false;
                }
            }
        }
        return comp;
    }

    virtual size_t memory_size() const
    {
        return memory_size_;
    }

    virtual void construct_instance(
            uint8_t* instance) const
    {
        for(auto&& it: member_map())
        {
            it.second.type().construct_instance(instance + it.second.offset());
        }
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const
    {
        //TODO: implement optional
        for(auto&& it: member_map())
        {
            it.second.type().copy_instance(target + it.second.offset(), source + it.second.offset());
        }
    }

    virtual void destroy_instance(
            uint8_t* instance) const
    {
        for(auto&& it: member_map())
        {
            it.second.type().destroy_instance(instance + it.second.offset());
        }
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const
    {
        //TODO: implement optional
        bool comp = true;
        for(auto&& it: member_map())
        {
            comp &= it.second.type().compare_instance(instance + it.second.offset(), other_instance + it.second.offset());
        }
        return comp;
    }

    virtual void for_each_instance(const InstanceNode& node, InstanceVisitor visitor) const
    {
        visitor(node);
        for(auto&& it: member_map())
        {
            const StructMember& member = it.second;
            InstanceNode child(node, member.type(), node.instance + member.offset(), InstanceNode::Access(member));
            member.type().for_each_instance(child, visitor);
        }
    }

protected:
    virtual DynamicType* clone() const
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
