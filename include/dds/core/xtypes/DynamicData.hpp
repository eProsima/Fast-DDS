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
#include <dds/core/xtypes/PrimitiveTypes.hpp>

namespace dds {
namespace core {
namespace xtypes {

class DynamicDataConst
{
public:
    template<
        typename T,
        class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    DynamicDataConst(
            T value)
        : type_(&primitive_type<T>())
        , instance_(new uint8_t[type_->memory_size()])
        , is_loaned(false)
    {
        type_->init_instance(instance_);
        *reinterpret_cast<T*>(instance_) = value;
    }

    DynamicDataConst(
            const DynamicType& type)
        : type_(&type)
        , instance_(new uint8_t[type.memory_size()])
        , is_loaned(false)
    {
        type_->init_instance(instance_);
    }

    template<typename T>
    DynamicDataConst(
            const T&& type)
        : type_(type)
        , instance_(new uint8_t[type.memory_size()])
        , is_loaned(false)
    {
        type_->init_instance(instance_);
    }

    DynamicDataConst(
            const DynamicDataConst& other)
        : type_(other.type_)
        , instance_(new uint8_t[other.type_->memory_size()])
        , is_loaned(false)
    {
        type_->copy_instance(instance_, other.instance_);
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
            type_->destroy_instance(instance_);
            delete[] instance_;
        }
    }

    bool has_type() const { return type_ != nullptr; }
    const DynamicType& type() const { return *type_; }
    size_t instance_id() const { return size_t(instance_); }

    template<typename T>
    const T& value() const
    {
        return *reinterpret_cast<T*>(instance_);
    }

    const std::vector<DynamicDataConst>& values() const
    {
        return *reinterpret_cast<std::vector<DynamicDataConst>*>(instance_);
    }

    DynamicDataConst operator [] (
            const std::string& member_name) const
    {
        const StructMember& member = struct_member(member_name);
        return DynamicDataConst(member.type(), instance_ + member.offset());
    }

    const DynamicDataConst& operator [] (
            size_t index) const
    {
        return values()[index];
    }

protected:
    const StructMember& struct_member(const std::string& name) const
    {
        if(type().kind() != TypeKind::STRUCTURE_TYPE)
        {
            throw "Exception Not implemented"; //TODO: Exceptions and their checks only in Debug mode
        }
        return static_cast<const StructType*>(type_)->member(name);
    }

    DynamicDataConst(
            const DynamicType& type,
            uint8_t* source)
        : type_(&type)
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

    const DynamicType* type_;
    uint8_t* instance_;
    bool is_loaned;
};


class DynamicData : public DynamicDataConst
{
public:
    template<
        typename T,
        class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    DynamicData(
            T value)
        : DynamicDataConst(value)
    {}

    DynamicData(
            const DynamicType& type)
        : DynamicDataConst(type)
    {}

    template<typename T>
    T& value()
    {
        return const_cast<T&>(DynamicDataConst::value<T>());
    }

    template<typename T>
    DynamicData& value(const T& t)
    {
        const_cast<T&>(DynamicDataConst::value<T>()) = t;
        return *this;
    }

    template<typename T>
    DynamicData& push(const T& t)
    {
        values().push_back(t);
        return *this;
    }

    std::vector<DynamicData>& values() const
    {
        return const_cast<std::vector<DynamicData>&>(reinterpret_cast<const std::vector<DynamicData>&>(DynamicDataConst::values()));
    }

    DynamicData operator [] (
            const std::string& member_name)
    {
        return DynamicDataConst::operator[](member_name);
    }

    DynamicData& operator [] (
            size_t index) const
    {
        return const_cast<DynamicData&>(reinterpret_cast<const DynamicData&>(DynamicDataConst::operator[](index)));
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
