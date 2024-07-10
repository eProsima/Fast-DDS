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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEBUILDERIMPL_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEBUILDERIMPL_HPP

#include <vector>

#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>

#include "AnnotationDescriptorImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include "TypeDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicTypeBuilderImpl : public traits<DynamicTypeBuilder>::base_type
{
public:

    DynamicTypeBuilderImpl(
            const TypeDescriptorImpl&) noexcept;

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

    bool equals(
            traits<DynamicType>::ref_type other) noexcept override;

    ReturnCode_t add_member(
            traits<MemberDescriptor>::ref_type descriptor) noexcept override;


    ReturnCode_t apply_annotation(
            traits<AnnotationDescriptor>::ref_type descriptor) noexcept override;

    ReturnCode_t apply_annotation_to_member(
            MemberId member_id,
            traits<AnnotationDescriptor>::ref_type descriptor) noexcept override;

    traits<DynamicType>::ref_type build() noexcept override;

    /*!
     * Initialize the instance copying the information from a @ref DynamicType.
     *
     * @param [in] @ref DynamicType reference used to initialize the instance.
     * @return Currently always return RETCODE_OK.
     */
    ReturnCode_t copy_from(
            traits<DynamicTypeImpl>::ref_type type);

    const TypeDescriptorImpl& get_descriptor() const noexcept
    {
        return type_descriptor_;
    }

    TypeDescriptorImpl& get_descriptor() noexcept
    {
        return type_descriptor_;
    }

    const std::vector<VerbatimTextDescriptorImpl>& get_verbatim() const noexcept
    {
        return verbatim_;
    }

    std::vector<VerbatimTextDescriptorImpl>& get_verbatim() noexcept
    {
        return verbatim_;
    }

protected:

    traits<DynamicTypeBuilder>::ref_type _this();

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

    //! Next MemberId to be used.
    MemberId next_id_ {0};

    //! Next index to be used.
    uint32_t next_index_ {0};

    //! Copy of the TypeDescriptor provided by the user.
    TypeDescriptorImpl type_descriptor_;

    std::vector<VerbatimTextDescriptorImpl> verbatim_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEBUILDERIMPL_HPP
