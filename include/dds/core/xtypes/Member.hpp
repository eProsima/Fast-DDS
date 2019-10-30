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

#ifndef OMG_DDS_CORE_XTYPES_MEMBER_HPP_
#define OMG_DDS_CORE_XTYPES_MEMBER_HPP_

#include <dds/core/xtypes/DynamicType.hpp>

#include <string>

namespace dds {
namespace core {
namespace xtypes {

class StructType;

/// \brief Class that represent a member.
/// A member can be added to any AggregationType.
class Member
{
    friend StructType;

public:
    /// \brief Construct a member.
    /// param[in] name Name of the member.
    /// param[in[ type Type of the member.
    Member(
            const std::string& name,
            const DynamicType& type)
        : name_(name)
        , type_(type)
        , id_(-1)
        , key_(false)
        , optional_(false)
    {}

    /// \brief Construct a member by a given rvalue type.
    /// param[in] name Name of the member.
    /// param[in[ type Type of the member.
    template<typename DynamicTypeImpl>
    Member(
            const std::string& name,
            const DynamicTypeImpl&& type)
        : name_(name)
        , type_(std::move(type))
        , id_(-1)
        , key_(false)
        , optional_(false)
    {}

    Member(const Member& other) = default;
    Member(Member&& other) = default;
    virtual ~Member() = default;

    /// \brief Name of the member.
    /// \returns the member name.
    const std::string& name() const { return name_; }

    /// \brief Type of the member.
    /// \returns A reference to the member type.
    const DynamicType& type() const { return *type_; }

    /// \brief Id annotation.
    /// \returns the id annotation
    int32_t get_id() const { return id_; }

    /// \brief Check the existance of an id annotation.
    /// \returns true if has an id annotation.
    bool has_id() const { return id_ >= 0; }

    /// \brief Check the existance of a key annotation.
    /// \returns true if has a key annotation.
    bool is_key() const { return key_; }

    /// \brief Check if the member is optional.
    /// \returns true if the member is optional.
    bool is_optional() const { return optional_; }

    /// \brief Used internally by DynamicData.
    /// Represents the memory offset in the AggregationType where this member is located.
    /// \returns memory offset.
    size_t offset() const { return offset_; }

    /// \brief Set an id annotation.
    /// \param[in] value Value of the id.
    /// \returns A reference to this member.
    Member& id(int32_t value)
    {
        id_ = value;
        return *this;
    }

    /// \brief Set member as key.
    /// \param[in] value Boolean indicating the key existance.
    /// \returns A reference to this member.
    Member& key(bool value = true)
    {
        key_ = value;
        return *this;
    }

    /// \brief Set member as optional.
    /// \param[in] value The optional value.
    /// \returns A reference to this member.
    Member& optional(bool value = true)
    {
        optional_ = value;
        return *this;
    }

private:
    std::string name_;
    DynamicType::Ptr type_;
    int32_t id_;
    bool key_;
    bool optional_;
    size_t offset_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_MEMBER_HPP_
