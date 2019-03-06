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
#include <fastrtps/log/Log.h>

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
    for (auto it = other->annotation_.begin(); it != other->annotation_.end(); ++it)
    {
        AnnotationDescriptor* newDesc = new AnnotationDescriptor(*it);
        annotation_.push_back(newDesc);
    }
}

DynamicTypeMember::~DynamicTypeMember()
{
    parent_ = nullptr;
    for (auto it = annotation_.begin(); it != annotation_.end(); ++it)
    {
        delete *it;
    }
    annotation_.clear();
}

ResponseCode DynamicTypeMember::apply_annotation(AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        // Store the annotation in the list of the member.
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->copy_from(&descriptor);
        annotation_.push_back(pNewDescriptor);

        // Update the annotations on the member Dynamic Type.
        descriptor_.type_->apply_annotation(descriptor);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicTypeMember::apply_annotation(const std::string& key, const std::string& value)
{
    // Update the annotations on the member Dynamic Type.
    descriptor_.type_->apply_annotation(key, value);

    auto it = annotation_.begin();
    if (it != annotation_.end())
    {
        return (*it)->set_value(key, value);
    }
    else
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive());
        pNewDescriptor->set_value(key, value);
        annotation_.push_back(pNewDescriptor);
        return ResponseCode::RETCODE_OK;
    }
}

bool DynamicTypeMember::equals(const DynamicTypeMember* other) const
{
    if (other != nullptr && annotation_.size() == other->annotation_.size())
    {
        for (auto it = annotation_.begin(), it2 = other->annotation_.begin(); it != annotation_.end(); ++it, ++it2)
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

ResponseCode DynamicTypeMember::get_annotation(
        AnnotationDescriptor& descriptor,
        uint32_t idx)
{
    if (idx < annotation_.size())
    {
        descriptor.copy_from(annotation_[idx]);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

uint32_t DynamicTypeMember::get_annotation_count()
{
    return static_cast<uint32_t>(annotation_.size());
}

bool DynamicTypeMember::key_annotation() const
{
    for (auto anIt = annotation_.begin(); anIt != annotation_.end(); ++anIt)
    {
        if ((*anIt)->key_annotation())
        {
            return true;
        }
    }
    return false;
}

std::vector<uint64_t> DynamicTypeMember::get_union_labels() const
{
    return descriptor_.get_union_labels();
}

ResponseCode DynamicTypeMember::get_descriptor(MemberDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        descriptor->copy_from(&descriptor_);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error getting MemberDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
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
