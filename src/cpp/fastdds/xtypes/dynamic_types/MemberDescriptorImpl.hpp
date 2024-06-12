// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _FASTDDS_XTYPES_DYNAMIC_TYPES_MEMBER_DESCRIPTOR_IMPL_HPP_
#define _FASTDDS_XTYPES_DYNAMIC_TYPES_MEMBER_DESCRIPTOR_IMPL_HPP_

#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class MemberDescriptorImpl : public virtual MemberDescriptor
{
    //! Default value of the member in string form.
    std::string default_value_;

    //! MemberId, it should be filled automatically when the member is added if not set (MEMBER_ID_INVALID).
    MemberId id_ {MEMBER_ID_INVALID};

    //! Definition order of the member inside its parent.
    uint32_t index_ {0xFFFFFFFF};

    //! If the union member is default.
    bool is_default_label_ {false};

    //! If the member is key.
    bool is_key_ {false};

    //! If the member is must_understand.
    bool is_must_understand_ {false};

    //! If the member is optional.
    bool is_optional_ {false};

    //! If the member is shared (external).
    bool is_shared_ {false};

    //! Case Labels for unions.
    UnionCaseLabelSeq label_;

    //! Name of the member
    ObjectName name_;

    //! Kind of the DynamicType which will contain this member.
    TypeKind parent_kind_ {TK_NONE};

    //! @ref TryConstructKind
    TryConstructKind try_construct_kind_ {TryConstructKind::DISCARD};

    bool is_try_construct_kind_set_ {false};

    //! Member's type
    traits<DynamicType>::ref_type type_;

public:

    MemberDescriptorImpl() noexcept = default;

    MemberDescriptorImpl(
            const MemberDescriptorImpl& descriptor) noexcept = default;

    MemberDescriptorImpl(
            MemberDescriptorImpl&& descriptor) noexcept = default;

    virtual ~MemberDescriptorImpl() noexcept = default;

    ObjectName& name() noexcept override
    {
        return name_;
    }

    const ObjectName& name() const noexcept override
    {
        return name_;
    }

    void name(
            const ObjectName& name) noexcept override
    {
        name_ = name;
    }

    virtual void name(
            ObjectName&& name) noexcept override
    {
        name_ = std::move(name);
    }

    MemberId id() const noexcept override
    {
        return id_;
    }

    MemberId& id() noexcept override
    {
        return id_;
    }

    void id(
            MemberId id) noexcept override
    {
        id_ = id;
    }

    traits<DynamicType>::ref_type type() const noexcept override
    {
        return type_;
    }

    traits<DynamicType>::ref_type& type() noexcept override
    {
        return type_;
    }

    void type(
            traits<DynamicType>::ref_type type) noexcept override
    {
        type_ = type;
    }

    std::string& default_value() noexcept override
    {
        return default_value_;
    }

    const std::string& default_value() const noexcept override
    {
        return default_value_;
    }

    void default_value(
            const std::string& default_value) noexcept override
    {
        default_value_ = default_value;
    }

    virtual void default_value(
            std::string&& default_value) noexcept override
    {
        default_value_ = std::move(default_value);
    }

    uint32_t index() const noexcept override
    {
        return index_;
    }

    uint32_t& index() noexcept override
    {
        return index_;
    }

    void index(
            uint32_t index) noexcept
    {
        index_ = index;
    }

    const UnionCaseLabelSeq& label() const noexcept override
    {
        return label_;
    }

    UnionCaseLabelSeq& label() noexcept override
    {
        return label_;
    }

    void label(
            const UnionCaseLabelSeq& label) noexcept override
    {
        label_ = label;
    }

    void label(
            UnionCaseLabelSeq&& label) noexcept override
    {
        label_ = std::move(label);
    }

    TypeKind parent_kind() const noexcept
    {
        return parent_kind_;
    }

    TypeKind& parent_kind() noexcept
    {
        return parent_kind_;
    }

    void parent_kind(
            TypeKind parent_kind) noexcept
    {
        parent_kind_ = parent_kind;
    }

    TryConstructKind try_construct_kind() const noexcept override
    {
        return try_construct_kind_;
    }

    TryConstructKind& try_construct_kind() noexcept override
    {
        return try_construct_kind_;
    }

    void try_construct_kind(
            TryConstructKind try_construct_kind) noexcept override
    {
        try_construct_kind_ = try_construct_kind;
        is_try_construct_kind_set_ = true;
    }

    bool is_key() const noexcept override
    {
        return is_key_;
    }

    bool& is_key() noexcept override
    {
        return is_key_;
    }

    void is_key(
            bool is_key) noexcept override
    {
        is_key_ = is_key;
    }

    bool is_optional() const noexcept override
    {
        return is_optional_;
    }

    bool& is_optional() noexcept override
    {
        return is_optional_;
    }

    void is_optional(
            bool is_optional) noexcept override
    {
        is_optional_ = is_optional;
    }

    bool is_must_understand() const noexcept override
    {
        return is_must_understand_;
    }

    bool& is_must_understand() noexcept override
    {
        return is_must_understand_;
    }

    void is_must_understand(
            bool is_must_understand) noexcept override
    {
        is_must_understand_ = is_must_understand;
    }

    bool is_shared() const noexcept override
    {
        return is_shared_;
    }

    bool& is_shared() noexcept override
    {
        return is_shared_;
    }

    void is_shared(
            bool is_shared) noexcept override
    {
        is_shared_ = is_shared;
    }

    bool is_default_label() const noexcept override
    {
        return is_default_label_;
    }

    bool& is_default_label() noexcept override
    {
        return is_default_label_;
    }

    void is_default_label(
            bool is_default_label) noexcept override
    {
        is_default_label_ = is_default_label;
    }

    ReturnCode_t copy_from(
            traits<MemberDescriptor>::ref_type descriptor) noexcept override;

    ReturnCode_t copy_from(
            const MemberDescriptorImpl& descriptor) noexcept;

    bool equals(
            traits<MemberDescriptor>::ref_type descriptor) noexcept override;

    bool equals(
            MemberDescriptorImpl& descriptor) noexcept;

    bool equal_labels(
            UnionCaseLabelSeq& label) noexcept;

    bool is_consistent() noexcept override;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_XTYPES_DYNAMIC_TYPES_MEMBER_DESCRIPTOR_IMPL_HPP_
