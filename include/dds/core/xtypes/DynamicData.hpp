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

#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_

#include <dds/core/xtypes/StructType.hpp>

namespace dds {
namespace core {
namespace xtypes {

class DynamicData
{
public:
    DynamicData(
            const DynamicType& type)
        : type_(type)
        , data_(new uint8_t[type.memory_size()])
        , is_loaned(false)
    {}

    ~DynamicData()
    {
        if(!is_loaned)
        {
            delete[] data_;
        }
    }

    const DynamicType& type() const { return type_; }

    template<typename T>
    T& value() const
    {
        return *reinterpret_cast<T*>(data_);
    }

    template<typename T>
    void value(
            const T& value)
    {
        return *reinterpret_cast<T*>(data_) = value;
    }

    template<typename T>
    T& value(
            const std::string& member_name) const
    {
        return *reinterpret_cast<T*>(data_ + member_offset(member_name));
    }

    template<typename T>
    void value(
            const std::string& member_name,
            const T& value)
    {
        return *reinterpret_cast<T*>(data_ + member_offset(member_name)) = value;
    }

    DynamicData loan_value(
            const std::string& member_name) const
    {
        return DynamicData(type_, data_ + member_offset(member_name));
    }

private:
    DynamicData(
            const DynamicType& type,
            uint8_t* source)
        : type_(type)
        , data_(source)
        , is_loaned(true)
    {}

    size_t member_offset(const std::string& name) const
    {
        if(type().kind() == TypeKind::STRUCTURE_TYPE)
        {
            static_cast<const StructType&>(type_).member(name).offset();
        }
        throw "Exception Not implemented";
    }

    const DynamicType& type_;
    uint8_t* data_;
    bool is_loaned;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
