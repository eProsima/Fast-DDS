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
 */

#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_

#include <dds/core/xtypes/StructType.hpp>

#include <cstring>
#include <iostream> //DELETE

namespace dds {
namespace core {
namespace xtypes {

class ConstDynamicData
{

};

class DynamicData
{
public:
    DynamicData(
            const DynamicType& type,
            bool cleaned = false)
        : type_(type)
        , data_((cleaned) ? new uint8_t[type.memory_size()]() : new uint8_t[type.memory_size()])
        , is_loaned(false)
    {
        type_.init_instance(data_);
    }

    DynamicData(
            const DynamicData& other)
        : type_(other.type_)
        , data_(new uint8_t[other.type_.memory_size()])
        , is_loaned(false)
    {
        std::cout << "copied" << std::endl;
        type_.copy_instance(data_, other.data_);
    }

    DynamicData(
            DynamicData&& other)
        : type_(std::move(other.type_))
        , data_(std::move(other.data_))
        , is_loaned(std::move(other.is_loaned))
    {
        other.is_loaned = true;
    }

    virtual ~DynamicData()
    {
        if(!is_loaned)
        {
            type_.destroy_instance(data_);
            delete[] data_;
        }
    }

    const DynamicType& type() const { return type_; }

    template<typename T>
    T& value()
    {
        return *reinterpret_cast<T*>(data_);
    }

    template<typename T>
    const T& value() const
    {
        return *reinterpret_cast<T*>(data_);
    }

    template<typename T>
    void value(
            const T& value)
    {
        *reinterpret_cast<T*>(data_) = value;
    }

    template<typename T>
    T& value(
            const std::string& member_name)
    {
        return *reinterpret_cast<T*>(data_ + struct_member(member_name).offset());
    }

    template<typename T>
    const T& value(
            const std::string& member_name) const
    {
        return *reinterpret_cast<T*>(data_ + struct_member(member_name).offset());
    }

    template<typename T>
    void value(
            const std::string& member_name,
            const T& value)
    {
        *reinterpret_cast<T*>(data_ + struct_member(member_name).offset()) = value;
    }

    DynamicData loan_value(
            const std::string& member_name)
    {
        const StructMember& member = struct_member(member_name);
        return DynamicData(member.type(), data_ + member.offset());
    }

    DynamicData operator [] (const std::string& member_name)
    {
        return loan_value(member_name);
    }

    //Iterator begin() { return Iterator(*this); }
    //Iterator end() { return  Iterator(); }

private:
    const StructMember& struct_member(const std::string& name) const
    {
        if(type().kind() != TypeKind::STRUCTURE_TYPE)
        {
            throw "Exception Not implemented";
        }
        return static_cast<const StructType&>(type_).member(name);
    }

    DynamicData(
            const DynamicType& type,
            uint8_t* source)
        : type_(type)
        , data_(source)
        , is_loaned(true)
    {}

    const DynamicType& type_;
    uint8_t* data_;
    bool is_loaned;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
