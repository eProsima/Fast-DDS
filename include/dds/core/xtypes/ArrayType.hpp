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

#ifndef OMG_DDS_CORE_XTYPES_ARRAY_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_ARRAY_TYPE_HPP_

#include <dds/core/xtypes/CollectionType.hpp>

#include <vector>
#include <cstring>
#include <cassert>

namespace dds {
namespace core {
namespace xtypes {

class ArrayType : public CollectionType
{
public:
    ArrayType(
            const DynamicType& content,
            uint32_t dimension)
        : CollectionType(
                TypeKind::ARRAY_TYPE,
                "array_" + std::to_string(dimension) + "_" + content.name(),
                DynamicType::Ptr(content))
        , dimension_(dimension)
    {}

    template<typename DynamicTypeImpl>
    ArrayType(
            const DynamicTypeImpl&& content,
            uint32_t dimension)
        : CollectionType(
                TypeKind::ARRAY_TYPE,
                "array_" + std::to_string(dimension) + "_" + content.name(),
                DynamicType::Ptr(std::move(content)))
        , dimension_(dimension)
    {}

    ArrayType(const ArrayType& other) = default;
    ArrayType(ArrayType&& other) = default;

    uint32_t dimension() const { return dimension_; }

    virtual size_t memory_size() const override
    {
        return dimension_ * content_type().memory_size();
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        size_t block_size = content_type().memory_size();
        for(uint32_t i = 0; i < dimension_; i++)
        {
            content_type().construct_instance(instance + i * block_size);
        }
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        size_t block_size = content_type().memory_size();
        if(content_type().is_constructed_type())
        {
            for(uint32_t i = 0; i < dimension_; i++)
            {
                content_type().copy_instance(target + i * block_size, source + i * block_size);
            }
        }
        else //optimization when the type is primitive
        {
            std::memcpy(target, source, dimension_ * block_size);
        }
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        assert(other.kind() == TypeKind::ARRAY_TYPE);
        const ArrayType& other_array = static_cast<const ArrayType&>(other);
        size_t block_size = content_type().memory_size();
        size_t other_block_size = other_array.content_type().memory_size();
        size_t min_dimension = std::min(dimension_, other_array.dimension_);

        if(content_type().is_constructed_type() || block_size != other_block_size)
        {
            for(uint32_t i = 0; i < min_dimension; i++)
            {
                content_type().copy_instance_from_type(
                        target + i * block_size,
                        source + i * other_block_size,
                        other_array.content_type());
            }
        }
        else //optimization when the type is primitive with same block_size
        {
            std::memcpy(target, source, min_dimension * block_size);
        }
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source) const override
    {
        size_t block_size = content_type().memory_size();
        if(content_type().is_constructed_type())
        {
            for(uint32_t i = 0; i < dimension_; i++)
            {
                content_type().move_instance(target + i * block_size, source + i * block_size);
            }
        }
        else //optimization when the type is primitive
        {
            std::memcpy(target, source, dimension_ * block_size);
        }
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        if(content_type().is_constructed_type())
        {
            size_t block_size = content_type().memory_size();
            for(int32_t i = dimension_ - 1; i >= 0; i--)
            {
                content_type().destroy_instance(instance + i * block_size);
            }
        }
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        size_t block_size = content_type().memory_size();
        if(content_type().is_constructed_type())
        {
            bool comp = true;
            for(uint32_t i = 0; i < dimension_; i++)
            {
                comp &= content_type().compare_instance(instance + i * block_size, other_instance + i * block_size);
            }
            return comp;
        }
        else //optimization when the type is primitive
        {
            return std::memcmp(instance, other_instance, dimension_ * block_size) == 0;
        }
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if(other.kind() != TypeKind::ARRAY_TYPE)
        {
            return TypeConsistency::NONE;
        }

        const ArrayType& other_array = static_cast<const ArrayType&>(other);

        if(dimension() == other_array.dimension())
        {
            return TypeConsistency::EQUALS
                | content_type().is_compatible(other_array.content_type());
        }

        return TypeConsistency::IGNORE_ARRAY_BOUNDS
            | content_type().is_compatible(other_array.content_type());
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        visitor(node);
        size_t block_size = content_type().memory_size();
        for(uint32_t i = 0; i < dimension_; i++)
        {
            InstanceNode child(node, content_type(), node.instance + i * block_size, i, nullptr);
            content_type().for_each_instance(child, visitor);
        }
    }

    virtual uint8_t* get_instance_at(
            uint8_t* instance,
            size_t index) const override
    {
        return instance + index * content_type().memory_size();
    }

    virtual size_t get_instance_size(
            const uint8_t*) const override
    {
        return dimension_;
    }

protected:
    virtual DynamicType* clone() const override
    {
        return new ArrayType(*this);
    }

private:
    uint32_t dimension_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_ARRAY_TYPE_HPP_
