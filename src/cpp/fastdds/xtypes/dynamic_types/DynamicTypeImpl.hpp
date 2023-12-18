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

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

#include <vector>

#include "AnnotationDescriptorImpl.hpp"
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
            traits<TypeDescriptor>::ref_type descriptor) noexcept override;

    ObjectName get_name() noexcept override;

    TypeKind get_kind() noexcept override;

    ReturnCode_t get_member_by_name(
            traits<DynamicTypeMember>::ref_type member,
            const ObjectName& name) noexcept override;

    ReturnCode_t get_all_members_by_name(
            DynamicTypeMembersByName& member) noexcept override;

    ReturnCode_t get_member(
            traits<DynamicTypeMember>::ref_type member,
            MemberId id) noexcept override;

    ReturnCode_t get_all_members(
            DynamicTypeMembersById& member) noexcept override;

    uint32_t get_member_count() noexcept override;

    ReturnCode_t get_member_by_index(
            traits<DynamicTypeMember>::ref_type member,
            uint32_t index) noexcept override;

    uint32_t get_annotation_count() noexcept override;

    ReturnCode_t get_annotation(
            traits<AnnotationDescriptor>::ref_type descriptor,
            uint32_t idx) noexcept override;

    uint32_t get_verbatim_text_count() noexcept override;

    ReturnCode_t get_verbatim_text(
            traits<VerbatimTextDescriptor>::ref_type descriptor,
            uint32_t idx) noexcept override;

    bool equals(
            traits<DynamicType>::ref_type other) noexcept override;

protected:

    traits<DynamicType>::ref_type _this();

private:

    std::vector<AnnotationDescriptorImpl> annotation_;

    DynamicTypeMembersById member_;

    DynamicTypeMembersByName member_by_name_;

    std::vector<traits<DynamicTypeMember>::ref_type> members_;

    TypeDescriptorImpl type_descriptor_;

    std::vector<VerbatimTextDescriptorImpl> verbatim_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_TYPE_IMPL_HPP_
