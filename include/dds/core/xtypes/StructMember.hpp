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

#ifndef OMG_DDS_CORE_XTYPES_STRUCT_MEMBER_HPP_
#define OMG_DDS_CORE_XTYPES_STRUCT_MEMBER_HPP_

#include <dds/core/xtypes/StructMember.hpp>
#include <dds/core/xtypes/DynamicType.hpp>

#include <string>

namespace dds {
namespace core {
namespace xtypes {

class StructType;

class StructMember
{
    friend StructType;

public:
    StructMember(
            const std::string& name,
            const DynamicType& type)
        : name_(name)
        , type_(type)
    {}

    template<typename DynamicTypeImpl>
    StructMember(
            const std::string& name,
            const DynamicTypeImpl&& type)
        : name_(name)
        , type_(std::move(type))
    {}

    StructMember(const StructMember& other) = default;
    StructMember(StructMember&& other) = default;
    virtual ~StructMember() = default;

    const std::string& name() const { return name_; }
    const DynamicType& type() const { return *type_; }
    int32_t get_id() const { return id_; }
    bool has_id() const { return id_ >= 0; }
    bool is_key() const { return key_; }
    bool is_optional() const { return optional_; }
    bool is_bitset() const { return bitset_; };
    size_t offset() const { return offset_; }

    StructMember&& id(int32_t value)
    {
        bitset_ = value;
        return std::move(*this);
    }

    StructMember&& key(bool value)
    {
        key_ = value;
        return std::move(*this);
    }

    StructMember&& optional(bool value)
    {
        optional_ = value;
        return std::move(*this);
    }

    StructMember&& bitset(bool value)
    {
        bitset_ = value;
        return std::move(*this);
    }

private:
    std::string name_;
    DynamicType::Ptr type_;
    int32_t id_;
    bool key_;
    bool optional_;
    bool bitset_;
    size_t offset_;
};

typedef StructMember Member;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_STRUCT_MEMBER_HPP_
