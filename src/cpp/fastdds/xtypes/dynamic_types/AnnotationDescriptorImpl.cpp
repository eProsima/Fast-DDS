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

#include "AnnotationDescriptorImpl.hpp"

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t AnnotationDescriptorImpl::get_value(
        ObjectName& value,
        const ObjectName& key) noexcept
{
    auto it = value_.find(key);

    if (it != value_.end())
    {
        value = it->second;
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t AnnotationDescriptorImpl::get_all_value(
        Parameters& value) noexcept
{
    value = value_;
    return RETCODE_OK;
}

ReturnCode_t AnnotationDescriptorImpl::set_value(
        const ObjectName& key,
        const ObjectName& value) noexcept
{
    value_[key] = value;
    return RETCODE_OK;
}

ReturnCode_t AnnotationDescriptorImpl::copy_from(
        traits<AnnotationDescriptor>::ref_type descriptor) noexcept
{
    if (!descriptor)
    {
        return RETCODE_BAD_PARAMETER;
    }

    type_ = descriptor->type();
    value_.clear();
    descriptor->get_all_value(value_);
    return RETCODE_OK;
}

bool AnnotationDescriptorImpl::equals(
        traits<AnnotationDescriptor>::ref_type descriptor) noexcept
{
    //TODO: Check consistency of value_
    return true;
}

bool AnnotationDescriptorImpl::is_consistent() noexcept
{
    if (!type_ || type_->get_kind() != TK_ANNOTATION)
    {
        return false;
    }

    //TODO: Check consistency of value_
    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
