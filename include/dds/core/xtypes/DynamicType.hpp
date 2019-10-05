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
#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_

#include <dds/core/xtypes/TypeKind.hpp>

#include <string>

namespace dds {
namespace core {
namespace xtypes {

class DynamicType
{
public:
    virtual ~DynamicType() = default;

    const TypeKind& kind() const { return kind_; }

    bool is_primitive_type() const
    {
        return (int(kind_) & int(TypeKind::PRIMITIVE_TYPE)) != 0;
    }

    bool is_collection_type() const
    {
        return (int(kind_) & int(TypeKind::COLLECTION_TYPE)) != 0;
    }

    bool is_aggregation_type() const
    {
        return (int(kind_) & int(TypeKind::AGGREGATION_TYPE)) != 0;
    }

    bool is_constructed_type() const
    {
        return (int(kind_) & int(TypeKind::CONSTRUCTED_TYPE)) != 0;
    }

    virtual size_t memory_size() const = 0;
    virtual DynamicType* clone() const = 0;

protected:
    DynamicType(
            TypeKind kind)
        : kind_(kind)
    {}

    DynamicType(const DynamicType& other) = default;
    DynamicType(DynamicType&& other) = default;

private:
    TypeKind kind_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_
