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

#include "DynamicTypeImpl.hpp"
#include "TypeValueConverter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

template<>
traits<AnnotationDescriptor>::ref_type traits<AnnotationDescriptor>::make_shared()
{
    return std::make_shared<AnnotationDescriptorImpl>();
}

ReturnCode_t AnnotationDescriptorImpl::get_value(
        ObjectName& value,
        const ObjectName& key) noexcept
{
    const AnnotationDescriptorImpl& myself = *this;
    return myself.get_value(value, key);
}

ReturnCode_t AnnotationDescriptorImpl::get_value(
        ObjectName& value,
        const ObjectName& key) const noexcept
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
    const AnnotationDescriptorImpl& myself = *this;
    return myself.get_all_value(value);
}

ReturnCode_t AnnotationDescriptorImpl::get_all_value(
        Parameters& value) const noexcept
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

    return copy_from(*traits<AnnotationDescriptor>::narrow<AnnotationDescriptorImpl>(descriptor));
}

ReturnCode_t AnnotationDescriptorImpl::copy_from(
        const AnnotationDescriptorImpl& descriptor) noexcept
{
    type_ = descriptor.type_;
    value_.clear();
    value_ = descriptor.value_;
    return RETCODE_OK;
}

bool AnnotationDescriptorImpl::equals(
        traits<AnnotationDescriptor>::ref_type descriptor) noexcept
{
    return equals(*traits<AnnotationDescriptor>::narrow<AnnotationDescriptorImpl>(descriptor));
}

bool AnnotationDescriptorImpl::equals(
        AnnotationDescriptorImpl& descriptor) noexcept
{
    return (type_ && type_->equals(descriptor.type_)) &&
           value_ == descriptor.value_;
}

bool AnnotationDescriptorImpl::is_consistent() noexcept
{
    if (!type_ || type_->get_kind() != TK_ANNOTATION)
    {
        return false;
    }

    auto type_impl {traits<DynamicType>::narrow<DynamicTypeImpl>(type_)};

    for (const auto& param : value_)
    {
        //{{{ Check the annotation parameter name is valid because it exits in the annotation type descriptor.
        const ObjectName* ann_param_value {nullptr};
        traits<DynamicTypeImpl>::ref_type ann_param_type;
        for (auto member{type_impl->get_all_members_by_index().cbegin()};
                !ann_param_value && member != type_impl->get_all_members_by_index().cend();
                ++member)
        {
            if (0 == member->get()->get_name().compare(param.first))
            {
                ann_param_value = &param.second;
                ann_param_type = traits<DynamicType>::narrow<DynamicTypeImpl>(member->get()->get_descriptor().type());
            }
        }

        if (!ann_param_value)
        {
            return false;
        }
        //}}}

        //{{{ Check the parameter value is convertible to its type.
        if (!TypeValueConverter::is_string_consistent(ann_param_type->get_kind(),
                ann_param_type->get_all_members_by_index(), ann_param_value->to_string()))
        {
            return false;
        }
        //}}}
    }

    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
