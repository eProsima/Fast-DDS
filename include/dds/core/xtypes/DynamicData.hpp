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

#include <cassert>

namespace dds {
namespace core {
namespace xtypes {

template<typename T>
using PrimitiveOrString = typename std::enable_if<
    std::is_arithmetic<T>::value ||
    std::is_same<std::string, T>::value ||
    std::is_same<std::wstring, T>::value
    >::type;

class ReadableDynamicDataRef
{
public:
    virtual ~ReadableDynamicDataRef() = default;

    bool operator == (
            const ReadableDynamicDataRef& other) const
    {
        return type_.compare_instance(instance_, other.instance_);
    }

    bool operator != (
            const ReadableDynamicDataRef& other) const
    {
        return !(*this == other);
    }

    const DynamicType& type() const { return type_; }
    size_t instance_id() const { return size_t(instance_); }

    inline std::string to_string() const; // Into DynamicDataImpl.hpp

    template<typename T, class = PrimitiveOrString<T>>
    T& value() const // this = PrimitiveType
    {
        assert((type_.kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (type_.kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
            || (type_.kind() == primitive_type<T>().kind()));
        return *reinterpret_cast<T*>(instance_);
    }

    const std::string& string() const // this = StringType
    {
        assert(type_.kind() == TypeKind::STRING_TYPE);
        return *reinterpret_cast<std::string*>(instance_);
    }

    const std::wstring& wstring() const // this = WStringType
    {
        assert(type_.kind() == TypeKind::WSTRING_TYPE);
        return *reinterpret_cast<std::wstring*>(instance_);
    }

    ReadableDynamicDataRef operator [] (
            const std::string& member_name) const // this = AggregationType
    {
        assert(type_.is_aggregation_type());
        const AggregationType& aggregation = static_cast<const AggregationType&>(type_);
        assert(aggregation.has_member(member_name));

        const Member& member = aggregation.member(member_name);
        return ReadableDynamicDataRef(member.type(), instance_ + member.offset());
    }

    ReadableDynamicDataRef operator [] (
            size_t index) const // this = CollectionType, AggregationType
    {
        assert((type_.is_aggregation_type() || type_.is_collection_type()) && index < size());
        if(type_.is_collection_type())
        {
            const CollectionType& collection = static_cast<const CollectionType&>(type_);
            return ReadableDynamicDataRef(collection.content_type(), collection.get_instance_at(instance_, index));
        }

        const AggregationType& aggregation = static_cast<const StructType&>(type_);
        const Member& member = aggregation.member(index);
        return ReadableDynamicDataRef(member.type(), instance_ + member.offset());
    }

    size_t size() const
    {
        assert(type_.is_collection_type() || type_.is_aggregation_type());
        if(type_.is_collection_type())
        {
            const CollectionType& collection = static_cast<const CollectionType&>(type_);
            return collection.get_instance_size(instance_);
        }
        if(type_.is_aggregation_type())
        {
            const AggregationType& aggregation = static_cast<const AggregationType&>(type_);
            return aggregation.members().size();
        }
        return 1;
    }

    template<typename T, class = PrimitiveOrString<T>>
    std::vector<T> as_vector() const // this = CollectionType with PrimitiveOrString content
    {
        const CollectionType& collection = static_cast<const CollectionType&>(type_);
        assert(type_.is_collection_type());
        assert((collection.content_type().kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (collection.content_type().kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
            || (collection.content_type().kind() == primitive_type<T>().kind()));

        const T* location = reinterpret_cast<T*>(collection.get_instance_at(instance_, 0));
        return std::vector<T>(location, location + size());
    }

    class ReadableNode
    {
    public:
        ReadableNode(const Instanceable::InstanceNode& instance_node) : internal_(instance_node) {}
        bool has_parent() const { return internal_.parent != nullptr; }
        ReadableNode parent() const { return ReadableNode(*internal_.parent); }
        ReadableDynamicDataRef data() const { return ReadableDynamicDataRef(internal_.type, internal_.instance); }
        const DynamicType& type() const { return internal_.type; }
        size_t deep() const { return internal_.deep; }
        size_t from_index() const { return internal_.from_index; }
        const Member* from_member() const { return internal_.from_member; }
    private:
        const Instanceable::InstanceNode& internal_;
    };

    bool for_each(std::function<void(const ReadableNode& node)> visitor) const
    {
        Instanceable::InstanceNode root(type_, instance_);
        try
        {
            type_.for_each_instance(root, [&](const Instanceable::InstanceNode& instance_node)
            {
                visitor(ReadableNode(instance_node));
            });
            return true;
        }
        catch(bool value) { return value; }
    }

protected:
    ReadableDynamicDataRef(
            const DynamicType& type,
            uint8_t* source)
        : type_(type)
        , instance_(source)
    {}

    const DynamicType& type_;
    uint8_t* instance_;

    const DynamicType& p_type(const ReadableDynamicDataRef& other) const { return other.type_; }
    uint8_t* p_instance(const ReadableDynamicDataRef& other) const { return other.instance_; }
};


class WritableDynamicDataRef : public ReadableDynamicDataRef
{
public:
    WritableDynamicDataRef& operator = (
            const WritableDynamicDataRef& other)
    {
        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, p_instance(other));
        return *this;
    }

    template<typename T, class = PrimitiveOrString<T>>
    WritableDynamicDataRef& operator = (
            const T& other)
    {
        value(other);
        return *this;
    }

    WritableDynamicDataRef& operator = (
            const std::string& other)
    {
        string(other);
        return *this;
    }

    WritableDynamicDataRef& operator = (
            const std::wstring& other)
    {
        wstring(other);
        return *this;
    }

    ReadableDynamicDataRef cref() const { return ReadableDynamicDataRef(*this); }

    template<typename T, class = PrimitiveOrString<T>>
    const T& value() // this = PrimitiveType & StringType
    {
        return ReadableDynamicDataRef::value<T>();
    }

    const std::string& string() // this = StringType
    {
        return ReadableDynamicDataRef::string();
    }

    ReadableDynamicDataRef operator [] (
            const std::string& member_name) const // this = StructType
    {
        return ReadableDynamicDataRef::operator[](member_name);
    }

    ReadableDynamicDataRef operator [] (
            size_t index) const // this = SequenceType & ArrayType
    {
        return ReadableDynamicDataRef::operator[](index);
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
    void value(const T& t) // this = PrimitiveType & StringType
    {
        assert((type_.kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (type_.kind() == TypeKind::STRING_TYPE && std::is_same<std::wstring, T>::value)
            || (type_.kind() == primitive_type<T>().kind()));

        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, reinterpret_cast<const uint8_t*>(&t));
    }

    void string(const std::string& s) // this = StringType
    {
        assert(type_.kind() == TypeKind::STRING_TYPE);
        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, reinterpret_cast<const uint8_t*>(&s));
    }

    void wstring(const std::wstring& s) // this = WStringType
    {
        assert(type_.kind() == TypeKind::WSTRING_TYPE);
        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, reinterpret_cast<const uint8_t*>(&s));
    }

    template<typename T, class = PrimitiveOrString<T>>
    WritableDynamicDataRef& push(const T& value) // this = SequenceType
    {
        assert(type_.kind() == TypeKind::SEQUENCE_TYPE);
        const SequenceType& sequence = static_cast<const SequenceType&>(type_);
        assert((sequence.content_type().kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (sequence.content_type().kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
            || (sequence.content_type().kind() == primitive_type<T>().kind()));

        uint8_t* element = sequence.push_instance(instance_, reinterpret_cast<const uint8_t*>(&value));
        assert(element != nullptr);
        return *this;
    }

    WritableDynamicDataRef& push(const WritableDynamicDataRef& data) // this = SequenceType
    {
        assert(type_.kind() == TypeKind::SEQUENCE_TYPE);
        const SequenceType& sequence = static_cast<const SequenceType&>(type_);

        uint8_t* element = sequence.push_instance(instance_, p_instance(data));
        assert(element != nullptr);
        return *this;
    }

    WritableDynamicDataRef& resize(size_t size) // this = SequenceType
    {
        assert(type_.kind() == TypeKind::SEQUENCE_TYPE);
        const SequenceType& sequence = static_cast<const SequenceType&>(type_);

        sequence.resize_instance(instance_, size);
        return *this;
    }

    bool for_each(std::function<void(const ReadableNode& node)> visitor) const
    {
        return ReadableDynamicDataRef::for_each(visitor);
    }

    class WritableNode : public ReadableNode
    {
    public:
        WritableNode(const Instanceable::InstanceNode& instance_node) : ReadableNode(instance_node) {}
        WritableDynamicDataRef data() { return ReadableNode::data(); }
    };

    bool for_each(std::function<void(WritableNode& node)> visitor)
    {
        Instanceable::InstanceNode root(type_, instance_);
        try
        {
            type_.for_each_instance(root, [&](const Instanceable::InstanceNode& instance_node)
            {
                WritableNode node(instance_node);
                visitor(node);
            });
            return true;
        }
        catch(bool value) { return value; }
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
        type_.construct_instance(instance_);
    }

    DynamicData(
            const ReadableDynamicDataRef& other,
            const DynamicType& type)
        : WritableDynamicDataRef(type, new uint8_t[type.memory_size()])
    {
        assert(type_.is_compatible(other.type()) != TypeConsistency::NONE);
        type_.copy_instance_from_type(instance_, p_instance(other), p_type(other));
    }

    DynamicData(const DynamicData& other)
        : WritableDynamicDataRef(other.type_, new uint8_t[other.type_.memory_size()])
    {
        assert(type_.is_compatible(other.type()) == TypeConsistency::EQUALS);
        type_.copy_instance(instance_, p_instance(other));
    }

    DynamicData(DynamicData&& other)
        : WritableDynamicDataRef(other.type_, new uint8_t[other.type_.memory_size()])
    {
        assert(type_.is_compatible(other.type()) == TypeConsistency::EQUALS);
        type_.move_instance(instance_, p_instance(other));
    }

    DynamicData& operator = (
            const DynamicData& other)
    {
        assert(type_.is_compatible(other.type()) == TypeConsistency::EQUALS);
        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, p_instance(other));
        return *this;
    }

    virtual ~DynamicData() override
    {
        type_.destroy_instance(instance_);
        delete[] instance_;
    }

    WritableDynamicDataRef ref() const { return WritableDynamicDataRef(*this); }
};

} //namespace xtypes
} //namespace core
} //namespace dds

#include <dds/core/xtypes/DynamicDataImpl.hpp>

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
