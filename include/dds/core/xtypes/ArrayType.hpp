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
#include <dds/core/xtypes/DynamicData.hpp>

#include <vector>

namespace dds {
namespace core {
namespace xtypes {

class ArrayType : public CollectionType
{
public:
    ArrayType(
            const DynamicType& content,
            uint32_t dimension = 0)
        : CollectionType(
                TypeKind::ARRAY_TYPE,
                "array_" + std::to_string(dimension),
                DynamicType::Ptr(content))
        , dimension_(dimension)
    {}

    template<typename DynamicTypeImpl>
    ArrayType(
            const DynamicTypeImpl&& content,
            uint32_t dimension)
        : CollectionType(
                TypeKind::ARRAY_TYPE,
                "array_" + std::to_string(dimension),
                DynamicType::Ptr(std::move(content)))
        , dimension_(dimension)
    {}

    ArrayType(const ArrayType& other) = default;
    ArrayType(ArrayType&& other) = default;

    virtual size_t memory_size() const
    {
        return dimension_ * content_type().memory_size();
    }

    virtual void init_instance(uint8_t* instance) const
    {
        if(content_type().is_constructed_type())
        {
            size_t block_size = content_type().memory_size();
            for(uint32_t i = 0; i < dimension_; i++)
            {
                content_type().init_instance(instance + i * block_size);
            }
        }
    }

    virtual void copy_instance(uint8_t* target, const uint8_t* source) const
    {
        if(content_type().is_constructed_type())
        {
            size_t block_size = content_type().memory_size();
            for(uint32_t i = 0; i < dimension_; i++)
            {
                content_type().copy_instance(target + i * block_size, source + i * block_size);
            }
        }
        else //optimization when the type is primitive
        {
            std::memcpy(target, source, dimension_ * content_type().memory_size());
        }
    }

    virtual void destroy_instance(uint8_t* instance) const
    {
        if(content_type().is_constructed_type())
        {
            size_t block_size = content_type().memory_size();
            for(uint32_t i = 0; i < dimension_; i++)
            {
                content_type().destroy_instance(instance + i * block_size);
            }
        }
    }

protected:
    virtual DynamicType* clone() const
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
