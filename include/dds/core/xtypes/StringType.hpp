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

class StringType : public DynamicType
{
public:
    StringType()
        : DynamicType(TypeKind::STRING_TYPE, "std::string")
    {}

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
