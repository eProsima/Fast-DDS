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

template<typename T>
using PrimitiveOrString = typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<std::string, T>::value>::type;

class ReadableDynamicDataRef
{
public:
    virtual ~ReadableDynamicDataRef() = default;

    bool has_type() const { return type_ != nullptr; }
    const DynamicType& type() const { return *type_; }

    size_t instance_id() const { return size_t(instance_); }

    template<typename T, class = PrimitiveOrString<T>>
    T& value() const // this = PrimitiveType
    {
        return *reinterpret_cast<T*>(instance_);
    }

    const std::string& string() const // this = PrimitiveType
    {
        return *reinterpret_cast<std::string*>(instance_);
    }

    ReadableDynamicDataRef operator [] (
            const std::string& member_name) const // this = StructType
    {
        const StructMember& member = static_cast<const StructType*>(type_)->member(member_name);
        return ReadableDynamicDataRef(member.type(), instance_ + member.offset());
    }

    ReadableDynamicDataRef operator [] (
            size_t index) const // this = SequenceType & ArrayType
    {
        const CollectionType* collection = static_cast<const CollectionType*>(type_);
        return ReadableDynamicDataRef(collection->content_type(), collection->get_instance_at(instance_, index));
    }

    size_t size() const // this = SequenceType & ArrayType
    {
        const CollectionType* collection = static_cast<const CollectionType*>(type_);
        return collection->get_instance_size(instance_);
    }

protected:
    ReadableDynamicDataRef(
            const DynamicType& type,
            uint8_t* source)
        : type_(&type)
        , instance_(source)
    {}

    const DynamicType* type_;
    uint8_t* instance_;

    const DynamicType& type(const ReadableDynamicDataRef& other) const { return *other.type_; }
    uint8_t* instance(const ReadableDynamicDataRef& other) const { return other.instance_; }
};


class WritableDynamicDataRef : public ReadableDynamicDataRef
{
public:
    WritableDynamicDataRef& operator = (
            const WritableDynamicDataRef& other)
    {
        type_->copy_instance(instance_, instance(other));
        return *this;
    }

    template<typename T, class = PrimitiveOrString<T>>
    const T& value() // this = PrimitiveType
    {
        return ReadableDynamicDataRef::value<T>();
    }

    const std::string& string() // this = PrimitiveType
    {
        return *reinterpret_cast<std::string*>(instance_);
    }

    WritableDynamicDataRef operator [] (
            const std::string& member_name) // this = StructType
    {
        return ReadableDynamicDataRef::operator[](member_name);
    }

    WritableDynamicDataRef operator [] (
            size_t index) // this = SequenceType & ArrayType
    {
        return ReadableDynamicDataRef::operator[](index);
    }

    template<typename T, class = PrimitiveOrString<T>>
    void value(const T& t) // this = PrimitiveType
    {
        type_->copy_instance(instance_, reinterpret_cast<const uint8_t*>(&t));
    }

    void string(const std::string& s) // this = StringType
    {
        type_->copy_instance(instance_, reinterpret_cast<const uint8_t*>(&s));
    }

    template<typename T, class = PrimitiveOrString<T>>
    WritableDynamicDataRef& push(const T& value) // this = SequenceType
    {
        const SequenceType* sequence = static_cast<const SequenceType*>(type_);
        sequence->push_instance(instance_, reinterpret_cast<const uint8_t*>(&value));
        return *this;
    }

    WritableDynamicDataRef& push(const WritableDynamicDataRef& data) // this = SequenceType
    {
        const SequenceType* sequence = static_cast<const SequenceType*>(type_);
        sequence->push_instance(instance_, instance(data));
        return *this;
    }

protected:
    WritableDynamicDataRef(
            const DynamicType& type,
            uint8_t* source)
        : ReadableDynamicDataRef(type, source)
    {}

    WritableDynamicDataRef(
            ReadableDynamicDataRef&& other)
        : ReadableDynamicDataRef(std::move(other))
    {}
};


class DynamicData : public WritableDynamicDataRef
{
public:
    DynamicData(
            const DynamicType& type)
        : WritableDynamicDataRef(type, new uint8_t[type.memory_size()])
    {
        std::cout << "DynamicData - ALLOC " << std::endl;
        type_->construct_instance(instance_);
    }

    DynamicData(const DynamicData& other) = delete;
    DynamicData(DynamicData&& other) = delete;

    virtual ~DynamicData()
    {
        type_->destroy_instance(instance_);
        delete[] instance_;
    }
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
