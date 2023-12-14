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

#include "DynamicTypeMemberImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t DynamicTypeMemberImpl::get_descriptor(
        traits<MemberDescriptor>::ref_type md) noexcept
{
    if (!md)
    {
        return RETCODE_BAD_PARAMETER;
    }

    traits<MemberDescriptor>::narrow<MemberDescriptorImpl>(md)->copy_from(member_descriptor_);
    return RETCODE_OK;
}

uint32_t DynamicTypeMemberImpl::get_annotation_count() noexcept
{
    return static_cast<uint32_t>(annotation_.size());
}

ReturnCode_t DynamicTypeMemberImpl::get_annotation(
        traits<AnnotationDescriptor>::ref_type descriptor,
        const uint32_t idx) noexcept
{
    if (!descriptor || idx >= annotation_.size())
    {
        return RETCODE_BAD_PARAMETER;
    }

    traits<AnnotationDescriptor>::narrow<AnnotationDescriptorImpl>(descriptor)->copy_from(annotation_.at(idx));
    return RETCODE_OK;
}

uint32_t DynamicTypeMemberImpl::get_verbatim_text_count() noexcept
{
    return static_cast<uint32_t>(verbatim_.size());
}

ReturnCode_t DynamicTypeMemberImpl::get_verbatim_text(
        traits<VerbatimTextDescriptor>::ref_type descriptor,
        const uint32_t idx) noexcept
{
    if (!descriptor || idx >= verbatim_.size())
    {
        return RETCODE_BAD_PARAMETER;
    }

    traits<VerbatimTextDescriptor>::narrow<VerbatimTextDescriptorImpl>(descriptor)->copy_from(verbatim_.at(idx));
    return RETCODE_OK;
}

bool DynamicTypeMemberImpl::equals(
        traits<DynamicTypeMember>::ref_type descriptor) noexcept
{
    bool ret_value = true;
    auto impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(descriptor);

    if (annotation_.size() == impl->annotation_.size())
    {
        for (size_t count {0}; count < annotation_.size(); ++count)
        {
            ret_value &= annotation_.at(count).equals(impl->annotation_.at(count));
        }
    }

    ret_value &= member_descriptor_.equals(impl->member_descriptor_);

    return ret_value;
}

MemberId DynamicTypeMemberImpl::get_id() noexcept
{
    return member_descriptor_.id();
}

ObjectName DynamicTypeMemberImpl::get_name() noexcept
{
    return member_descriptor_.name();
}

traits<DynamicTypeMember>::ref_type DynamicTypeMemberImpl::_this ()
{
    return shared_from_this();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
