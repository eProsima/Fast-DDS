// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeMember::DynamicTypeMember()
    : parent_(nullptr)
    , id_(MEMBER_ID_INVALID)
{
}

DynamicTypeMember::DynamicTypeMember(
        const MemberDescriptor* descriptor,
        MemberId id)
    : parent_(nullptr)
    , id_(id)
{
    descriptor_.copy_from(descriptor);
    descriptor_.set_id(id);
}

DynamicTypeMember::DynamicTypeMember(const DynamicTypeMember* other)
    : parent_(other->parent_)
    , id_(other->id_)
{
    descriptor_.copy_from(&other->descriptor_);
}

DynamicTypeMember::~DynamicTypeMember()
{
    parent_ = nullptr;
}

ReturnCode_t DynamicTypeMember::apply_annotation(AnnotationDescriptor& descriptor)
{
    // Update the annotations on the member Dynamic Type.
    return descriptor_.apply_annotation(descriptor);
}

ReturnCode_t DynamicTypeMember::apply_annotation(
        const std::string& annotation_name,
        const std::string& key,
        const std::string& value)
{
    // Update the annotations on the member Dynamic Type.
    return descriptor_.apply_annotation(annotation_name, key, value);
}

bool DynamicTypeMember::equals(const DynamicTypeMember* other) const
{
    if (other != nullptr && descriptor_.annotation_.size() == other->descriptor_.annotation_.size())
    {
        for (auto it = descriptor_.annotation_.begin(),
                it2 = other->descriptor_.annotation_.begin();
                it != descriptor_.annotation_.end(); ++it, ++it2)
        {
            if (!(*it)->equals(*it2))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

ReturnCode_t DynamicTypeMember::get_annotation(
        AnnotationDescriptor& descriptor,
        uint32_t idx)
{
    if (idx < descriptor_.annotation_.size())
    {
        descriptor.copy_from(descriptor_.annotation_[idx]);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

uint32_t DynamicTypeMember::get_annotation_count()
{
    return static_cast<uint32_t>(descriptor_.annotation_.size());
}

bool DynamicTypeMember::key_annotation() const
{
    return descriptor_.annotation_is_key();
}

std::vector<uint64_t> DynamicTypeMember::get_union_labels() const
{
    return descriptor_.get_union_labels();
}

ReturnCode_t DynamicTypeMember::get_descriptor(MemberDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        descriptor->copy_from(&descriptor_);
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error getting MemberDescriptor, invalid input descriptor");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

uint32_t DynamicTypeMember::get_index() const
{
    return descriptor_.get_index();
}

std::string DynamicTypeMember::get_name() const
{
    return descriptor_.get_name();
}

MemberId DynamicTypeMember::get_id() const
{
    return descriptor_.get_id();
}

bool DynamicTypeMember::is_default_union_value() const
{
    return descriptor_.is_default_union_value();
}

void DynamicTypeMember::set_index(uint32_t index)
{
    descriptor_.set_index(index);
}

void DynamicTypeMember::set_parent(DynamicType* pType)
{
    parent_ = pType;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
