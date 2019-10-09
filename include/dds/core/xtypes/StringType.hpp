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

#ifndef OMG_DDS_CORE_XTYPES_STRING_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_STRING_TYPE_HPP_

#include <dds/core/xtypes/MutableCollectionType.hpp>
#include <dds/core/xtypes/PrimitiveTypes.hpp>
#include <dds/core/xtypes/SequenceInstance.hpp>

namespace dds {
namespace core {
namespace xtypes {

class StringType : public MutableCollectionType
{
public:
    StringType(
            int bounds = 0)
        : MutableCollectionType(
                TypeKind::STRING_TYPE,
                "std::string_" + ((bounds > 0) ? "_" + std::to_string(bounds) + "_" : ""),
                DynamicType::Ptr(primitive_type<char>()),
                bounds)
    {}

    virtual bool is_subset_of(const DynamicType& other) const
    {
        if(other.kind() != TypeKind::STRING_TYPE)
        {
            return false;
        }

        const StringType& other_string = static_cast<const StringType&>(other);
        return bounds() <= other_string.bounds();
    }

    virtual size_t memory_size() const
    {
        return sizeof(std::string);
    }

    virtual void construct_instance(uint8_t* instance) const
    {
        new (instance) std::string();
    }

    virtual void copy_instance(uint8_t* target, const uint8_t* source) const
    {
        new (target) std::string(*reinterpret_cast<const std::string*>(source));
    }

    virtual void destroy_instance(uint8_t* instance) const
    {
        reinterpret_cast<std::string*>(instance)->~basic_string();
    }

    virtual uint8_t* get_instance_at(uint8_t* instance, size_t index) const
    {
        void* char_addr = &reinterpret_cast<std::string*>(instance)->operator[](index);
        return static_cast<uint8_t*>(char_addr);
    }

    virtual size_t get_instance_size(const uint8_t* instance) const
    {
        return reinterpret_cast<const std::string*>(instance)->size();
    }

protected:
    virtual DynamicType* clone() const
    {
        return new StringType(*this);
    }
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_STRING_TYPE_HPP_
