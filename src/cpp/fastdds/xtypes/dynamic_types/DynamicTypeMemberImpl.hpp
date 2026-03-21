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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEMEMBERIMPL_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEMEMBERIMPL_HPP

#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "AnnotationDescriptorImpl.hpp"
#include "MemberDescriptorImpl.hpp"
#include "VerbatimTextDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicTypeMemberImpl : public virtual traits<DynamicTypeMember>::base_type
{
    friend class DynamicTypeBuilderImpl;

public:

    DynamicTypeMemberImpl(
            const MemberDescriptorImpl&) noexcept;

    ReturnCode_t get_descriptor(
            traits<MemberDescriptor>::ref_type& md) noexcept override;

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
            traits<DynamicTypeMember>::ref_type other) noexcept override;

    MemberId get_id() noexcept override;

    ObjectName get_name() noexcept override;

    MemberDescriptorImpl& get_descriptor() noexcept
    {
        return member_descriptor_;
    }

    const MemberDescriptorImpl& get_descriptor() const noexcept
    {
        return member_descriptor_;
    }

protected:

    traits<DynamicTypeMember>::ref_type _this();

private:

    std::vector<AnnotationDescriptorImpl> annotation_;

    MemberDescriptorImpl member_descriptor_;

    std::vector<VerbatimTextDescriptorImpl> verbatim_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEMEMBERIMPL_HPP
