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
#include <dds/core/xtypes/CollectionType.hpp>
#include <dds/core/xtypes/SequenceType.hpp>
#include <dds/core/xtypes/PrimitiveTypes.hpp>

namespace dds {
namespace core {
namespace xtypes {

class DynamicDataConst
{
public:
    DynamicDataConst(
            const DynamicType& type)
        : type_(&type)
        , instance_(new uint8_t[type.memory_size()])
        , managed_(true)
    {
        std::cout << "DynamicData - ALLOC" << std::endl;
        type_->construct_instance(instance_);
    }

    DynamicDataConst(
            const DynamicDataConst& other)
        : type_(other.type_)
        , instance_(other.managed_ ? new uint8_t[other.type_->memory_size()] : other.instance_)
        , managed_(other.managed_)
    {
        if(other.managed_)
        {
            std::cout << "DynamicData - ALLOC" << std::endl;
            type_->copy_instance(instance_, other.instance_);
        }
    }

    DynamicDataConst(
            DynamicDataConst&& other)
        : type_(std::move(other.type_))
        , instance_(std::move(other.instance_))
        , managed_(std::move(other.managed_))
    {
        other.managed_ = false;
    }

    virtual ~DynamicDataConst()
    {
        if(managed_)
        {
            type_->destroy_instance(instance_);
            delete[] instance_;
        }
    }

    bool has_type() const { return type_ != nullptr; }
    const DynamicType& type() const { return *type_; }

    size_t instance_id() const { return size_t(instance_); }
    bool is_ref() const { return !managed_; }

    DynamicDataConst get_ref() const
    {
        return DynamicDataConst(*type_, instance_);
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    const T& value() const // this = PrimitiveType
    {
        return *reinterpret_cast<T*>(instance_);
    }

    DynamicDataConst operator [] (
            const std::string& member_name) const // this = StructType
    {
        const StructMember& member = static_cast<const StructType*>(type_)->member(member_name);
        return DynamicDataConst(member.type(), instance_ + member.offset());
    }

    DynamicDataConst operator [] (
            size_t index) const // this = SequenceType & ArrayType
    {
        const CollectionType* collection = static_cast<const CollectionType*>(type_);
        return DynamicDataConst(collection->content_type(), collection->get_instance_at(instance_, index));
    }

protected:
    DynamicDataConst(
            const DynamicType& type,
            uint8_t* source)
        : type_(&type)
        , instance_(source)
        , managed_(false)
    {}

    const DynamicType* type_;
    uint8_t* instance_;
    bool managed_;

    const DynamicType& type(const DynamicDataConst& other) const { return *other.type_; }
    uint8_t* instance(const DynamicDataConst& other) const { return other.instance_; }
};


class DynamicData : public DynamicDataConst
{
public:
    DynamicData(
            const DynamicType& type)
        : DynamicDataConst(type)
    {}

    DynamicData& operator = (
            const DynamicData& other)
    {
        type_->copy_instance(instance_, instance(other));
        return *this;
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    T& value() // this = PrimitiveType
    {
        return const_cast<T&>(DynamicDataConst::value<T>());
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    DynamicData& value(const T& t) // this = PrimitiveType
    {
        const_cast<T&>(DynamicDataConst::value<T>()) = t;
        return *this;
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    DynamicData push(T value) // this = SequenceType
    {
        const SequenceType* sequence = static_cast<const SequenceType*>(type_);
        sequence->push_instance(instance_, reinterpret_cast<uint8_t*>(&value));
        return *this;
    }

    DynamicData push(const DynamicData& data) // this = SequenceType
    {
        const SequenceType* sequence = static_cast<const SequenceType*>(type_);
        sequence->push_instance(instance_, instance(data));
        return *this;
    }

    DynamicData operator [] (
            const std::string& member_name) // this = StructType
    {
        return DynamicDataConst::operator[](member_name);
    }

    DynamicData operator [] (
            size_t index) // this = SequenceType & ArrayType
    {
        return DynamicDataConst::operator[](index);
    }

private:
    DynamicData(
            DynamicDataConst&& other)
        : DynamicDataConst(std::move(other))
    {}
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
