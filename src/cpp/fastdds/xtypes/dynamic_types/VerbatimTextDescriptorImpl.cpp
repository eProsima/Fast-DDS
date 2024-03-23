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

#include "VerbatimTextDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

template<>
traits<VerbatimTextDescriptor>::ref_type traits<VerbatimTextDescriptor>::make_shared()
{
    return std::make_shared<VerbatimTextDescriptorImpl>();
}

ReturnCode_t VerbatimTextDescriptorImpl::copy_from(
        traits<VerbatimTextDescriptor>::ref_type descriptor) noexcept
{
    if (!descriptor)
    {
        return RETCODE_BAD_PARAMETER;
    }

    return copy_from(*traits<VerbatimTextDescriptor>::narrow<VerbatimTextDescriptorImpl>(descriptor));
}

ReturnCode_t VerbatimTextDescriptorImpl::copy_from(
        const VerbatimTextDescriptorImpl& descriptor) noexcept
{
    placement_ = descriptor.placement_;
    text_ = descriptor.text_;

    return RETCODE_OK;
}

bool VerbatimTextDescriptorImpl::equals(
        traits<VerbatimTextDescriptor>::ref_type descriptor) noexcept
{
    return equals(*traits<VerbatimTextDescriptor>::narrow<VerbatimTextDescriptorImpl>(descriptor));
}

bool VerbatimTextDescriptorImpl::equals(
        VerbatimTextDescriptorImpl& descriptor) noexcept
{
    return placement_ == descriptor.placement_ &&
           text_ == descriptor.text_;
}

bool VerbatimTextDescriptorImpl::is_consistent() noexcept
{
    //TODO
    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
