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
    : mParent(nullptr)
    , mId(MEMBER_ID_INVALID)
{
}

DynamicTypeMember::DynamicTypeMember(const MemberDescriptor* descriptor, MemberId id)
    : mParent(nullptr)
    , mId(id)
{
    mDescriptor.CopyFrom(descriptor);
    mDescriptor.SetId(id);
}

DynamicTypeMember::DynamicTypeMember(const DynamicTypeMember* other)
    : mParent(other->mParent)
    , mId(other->mId)
{
    mDescriptor.CopyFrom(&other->mDescriptor);
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

ResponseCode DynamicTypeMember::ApplyAnnotation(AnnotationDescriptor& descriptor)
{
    if (descriptor.IsConsistent())
    {
        // Store the annotation in the list of the member.
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->CopyFrom(&descriptor);
        mAnnotation.push_back(pNewDescriptor);

        // Update the annotations on the member Dynamic Type.
        mDescriptor.mType->ApplyAnnotation(descriptor);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicTypeMember::ApplyAnnotation(const std::string& key, const std::string& value)
{
    // Update the annotations on the member Dynamic Type.
    mDescriptor.mType->ApplyAnnotation(key, value);

    auto it = mAnnotation.begin();
    if (it != mAnnotation.end())
    {
        return (*it)->SetValue(key, value);
    }
    else
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->SetType(DynamicTypeBuilderFactory::GetInstance()->CreateAnnotationPrimitive());
        pNewDescriptor->SetValue(key, value);
        mAnnotation.push_back(pNewDescriptor);
        return ResponseCode::RETCODE_OK;
    }
}

bool DynamicTypeMember::Equals(const DynamicTypeMember* other) const
{
    if (other != nullptr && mAnnotation.size() == other->mAnnotation.size())
    {
        for (auto it = mAnnotation.begin(), it2 = other->mAnnotation.begin(); it != mAnnotation.end(); ++it, ++it2)
        {
            if (!(*it)->Equals(*it2))
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

ResponseCode DynamicTypeMember::GetAnnotation(AnnotationDescriptor& descriptor, uint32_t idx)
{
    if (idx < mAnnotation.size())
    {
        descriptor.CopyFrom(mAnnotation[idx]);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

uint32_t DynamicTypeMember::GetAnnotationCount()
{
    return static_cast<uint32_t>(mAnnotation.size());
}

bool DynamicTypeMember::GetKeyAnnotation() const
{
    for (auto anIt = mAnnotation.begin(); anIt != mAnnotation.end(); ++anIt)
    {
        if ((*anIt)->GetKeyAnnotation())
        {
            return true;
        }
    }
    return false;
}

std::vector<uint64_t> DynamicTypeMember::GetUnionLabels() const
{
    return mDescriptor.GetUnionLabels();
}

ResponseCode DynamicTypeMember::GetDescriptor(MemberDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        descriptor->CopyFrom(&mDescriptor);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error getting MemberDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

uint32_t DynamicTypeMember::GetIndex() const
{
    return mDescriptor.GetIndex();
}

std::string DynamicTypeMember::GetName() const
{
    return mDescriptor.GetName();
}

MemberId DynamicTypeMember::GetId() const
{
    return mDescriptor.GetId();
}

bool DynamicTypeMember::IsDefaultUnionValue() const
{
    return mDescriptor.IsDefaultUnionValue();
}

void DynamicTypeMember::SetIndex(uint32_t index)
{
    mDescriptor.SetIndex(index);
}

void DynamicTypeMember::SetParent(DynamicType* pType)
{
    mParent = pType;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
