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

namespace dds {
namespace core {
namespace xtypes {

class DynamicDataConst
{
public:
    DynamicDataConst(
            const DynamicType& type)
        : type_(type)
        , instance_(new uint8_t[type.memory_size()])
        , is_loaned(false)
    {
        type_.init_instance(instance_);
    }

    DynamicDataConst(
            const DynamicDataConst& other)
        : type_(other.type_)
        , instance_(new uint8_t[other.type_.memory_size()])
        , is_loaned(false)
    {
        type_.copy_instance(instance_, other.instance_);
    }

    DynamicDataConst(
            DynamicDataConst&& other)
        : type_(std::move(other.type_))
        , instance_(std::move(other.instance_))
        , is_loaned(std::move(other.is_loaned))
    {
        other.is_loaned = true;
    }

    virtual ~DynamicDataConst()
    {
        if(!is_loaned)
        {
            type_.destroy_instance(instance_);
            delete[] instance_;
        }
    }

    const DynamicType& type() const { return type_; }

    template<typename T>
    const T& value() const
    {
        return *reinterpret_cast<T*>(instance_);
    }

    template<typename T>
    const T& value(
            const std::string& member_name) const
    {
        return *reinterpret_cast<T*>(instance_ + struct_member(member_name).offset());
    }

    DynamicDataConst loan_value(
            const std::string& member_name) const
    {
        const StructMember& member = struct_member(member_name);
        return DynamicDataConst(member.type(), instance_ + member.offset());
    }

    DynamicDataConst operator [] (const std::string& member_name) const
    {
        return loan_value(member_name);
    }

protected:
    const StructMember& struct_member(const std::string& name) const
    {
        if(type().kind() != TypeKind::STRUCTURE_TYPE)
        {
            throw "Exception Not implemented";
        }
        return static_cast<const StructType&>(type_).member(name);
    }

    DynamicDataConst(
            const DynamicType& type,
            uint8_t* source)
        : type_(type)
        , instance_(source)
        , is_loaned(true)
    {}

    DynamicDataConst(
            const DynamicDataConst& other,
            bool) //only for distinguish from copy constructor
        : type_(other.type_)
        , instance_(other.instance_)
        , is_loaned(true)
    {}

    const DynamicType& type_;
    uint8_t* instance_;
    bool is_loaned;
};

class DynamicData : public DynamicDataConst
{
public:
    DynamicData(
            const DynamicType& type)
        : DynamicDataConst(type)
    {}

    template<typename T>
    T& value()
    {
        return *reinterpret_cast<T*>(instance_);
    }

    template<typename T>
    void value(
            const T& value)
    {
        *reinterpret_cast<T*>(instance_) = value;
    }

    template<typename T>
    T& value(
            const std::string& member_name)
    {
        return *reinterpret_cast<T*>(instance_ + struct_member(member_name).offset());
    }

    template<typename T>
    void value(
            const std::string& member_name,
            const T& value)
    {
        *reinterpret_cast<T*>(instance_ + struct_member(member_name).offset()) = value;
    }

    DynamicData loan_value(
            const std::string& member_name)
    {
        return DynamicDataConst::loan_value(member_name);
    }

    DynamicData operator [] (
            const std::string& member_name)
    {
        return loan_value(member_name);
    }

private:
    DynamicData(DynamicDataConst&& other)
        : DynamicDataConst(std::move(other))
    {}
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
