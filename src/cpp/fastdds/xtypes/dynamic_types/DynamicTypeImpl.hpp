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

#ifndef _FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_TYPE_IMPL_HPP_
#define _FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_TYPE_IMPL_HPP_

#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "AnnotationDescriptorImpl.hpp"
#include "DynamicTypeMemberImpl.hpp"
#include "TypeDescriptorImpl.hpp"
#include "VerbatimTextDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicTypeImpl : public virtual traits<DynamicType>::base_type
{
    friend class DynamicTypeBuilderImpl;

public:

    DynamicTypeImpl(
            const TypeDescriptorImpl& descriptor) noexcept;

    ReturnCode_t get_descriptor(
            traits<TypeDescriptor>::ref_type& descriptor) noexcept override;

    ObjectName get_name() noexcept override;

    TypeKind get_kind() noexcept override;

    ReturnCode_t get_member_by_name(
            traits<DynamicTypeMember>::ref_type& member,
            const ObjectName& name) noexcept override;

    ReturnCode_t get_all_members_by_name(
            DynamicTypeMembersByName& member) noexcept override;

    ReturnCode_t get_member(
            traits<DynamicTypeMember>::ref_type& member,
            MemberId id) noexcept override;

    ReturnCode_t get_all_members(
            DynamicTypeMembersById& member) noexcept override;

    uint32_t get_member_count() noexcept override;

    ReturnCode_t get_member_by_index(
            traits<DynamicTypeMember>::ref_type& member,
            uint32_t index) noexcept override;

    uint32_t get_annotation_count() noexcept override;

    ReturnCode_t get_annotation(
            traits<AnnotationDescriptor>::ref_type& descriptor,
            uint32_t idx) noexcept override;

    const std::vector<AnnotationDescriptorImpl> get_annotations() const
    {
        return annotation_;
    }

    uint32_t get_verbatim_text_count() noexcept override;

    ReturnCode_t get_verbatim_text(
            traits<VerbatimTextDescriptor>::ref_type& descriptor,
            uint32_t idx) noexcept override;

    bool equals(
            traits<DynamicType>::ref_type other) noexcept override;

    const DynamicTypeMembersById& get_all_members() const
    {
        return member_;
    }

    const std::vector<traits<DynamicTypeMemberImpl>::ref_type>& get_all_members_by_index() const
    {
        return members_;
    }

    const TypeDescriptorImpl& get_descriptor() const noexcept
    {
        return type_descriptor_;
    }

    uint32_t get_index_own_members() const noexcept
    {
        return index_own_members_;
    }

    int32_t default_value() const noexcept
    {
        return default_value_;
    }

    MemberId default_union_member() const noexcept
    {
        return default_union_member_;
    }

    traits<DynamicTypeImpl>::ref_type resolve_alias_enclosed_type() noexcept;

protected:

    traits<DynamicType>::ref_type _this();

private:

    //! Contains the annotations applied by the user.
    std::vector<AnnotationDescriptorImpl> annotation_;

    //! Contains the default value of discriminator (TK_UNION) or next literal (TK_ENUM).
    //! This is calculated while the type is being built.
    int32_t default_value_ {0};

    //! Points to the default union member.
    MemberId default_union_member_ {MEMBER_ID_INVALID};

    //! Index pointing the first own member, not inherited from a base_type.
    uint32_t index_own_members_ {0};

    //! Collection of all members sorted by MemberId.
    DynamicTypeMembersById member_;

    //! Collection of all members sorted by name.
    DynamicTypeMembersByName member_by_name_;

    //! Collection of all members sorted by index.
    std::vector<traits<DynamicTypeMemberImpl>::ref_type> members_;

    //! Copy of the TypeDescriptor provided by the user.
    TypeDescriptorImpl type_descriptor_;

    //! Contains the verbatim builtin annotation applied by the user.
    std::vector<VerbatimTextDescriptorImpl> verbatim_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_TYPE_IMPL_HPP_
