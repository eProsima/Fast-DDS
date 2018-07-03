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
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeMember::DynamicTypeMember()
    : mParent(nullptr)
    , mDescriptor(nullptr)
    , mId(MEMBER_ID_INVALID)
{
}

DynamicTypeMember::DynamicTypeMember(const MemberDescriptor* descriptor, MemberId id)
    : mParent(nullptr)
    , mId(id)
{
    mDescriptor = new MemberDescriptor(descriptor);
}

DynamicTypeMember::DynamicTypeMember(const DynamicTypeMember* other)
    : mParent(other->mParent)
    , mId(other->mId)
{
    mDescriptor = new MemberDescriptor(other->mDescriptor);
    for (auto it = other->mAnnotation.begin(); it != other->mAnnotation.end(); ++it)
    {
        AnnotationDescriptor* newDesc = new AnnotationDescriptor(*it);
        mAnnotation.push_back(newDesc);
    }
}

DynamicTypeMember::~DynamicTypeMember()
{
    mParent = nullptr;
    for (auto it = mAnnotation.begin(); it != mAnnotation.end(); ++it)
    {
        delete *it;
    }
    mAnnotation.clear();
}

ResponseCode DynamicTypeMember::get_descriptor(MemberDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        descriptor->copy_from(mDescriptor);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

uint32_t DynamicTypeMember::get_index() const
{
    if (mDescriptor != nullptr)
    {
        return mDescriptor->get_index();
    }
    return 0;
}

bool DynamicTypeMember::equals(const DynamicTypeMember* other) const
{
    if (other != nullptr && mParent == other->mParent && mAnnotation.size() == other->mAnnotation.size())
    {
        for (auto it = mAnnotation.begin(), it2 = other->mAnnotation.begin(); it != mAnnotation.end(); ++it, ++it2)
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

uint32_t DynamicTypeMember::get_annotation_count()
{
    return static_cast<uint32_t>(mAnnotation.size());
}

ResponseCode DynamicTypeMember::apply_annotation(AnnotationDescriptor& descriptor)
{
    if (descriptor.isConsistent())
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->copy_from(&descriptor);
        mAnnotation.push_back(pNewDescriptor);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicTypeMember::get_annotation(AnnotationDescriptor& descriptor, uint32_t idx)
{
    if (idx < mAnnotation.size())
    {
        descriptor.copy_from(mAnnotation[idx]);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

std::string DynamicTypeMember::get_name() const
{
    return mDescriptor->get_name();
}

MemberId DynamicTypeMember::get_id() const
{
    return mDescriptor->get_id();
}

void DynamicTypeMember::set_index(uint32_t index)
{
    if (mDescriptor != nullptr)
    {
        mDescriptor->set_index(index);
    }
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
