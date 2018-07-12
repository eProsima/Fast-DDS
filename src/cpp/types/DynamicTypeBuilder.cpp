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

#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeBuilder::DynamicTypeBuilder()
    : mCurrentMemberId(0)
    , mMaxIndex(0)
{
    mIsTypeObject = false;
}

DynamicTypeBuilder::DynamicTypeBuilder(const TypeDescriptor* descriptor)
    : DynamicType(descriptor)
    , mCurrentMemberId(0)
    , mMaxIndex(0)
{
    mIsTypeObject = false;

    RefreshMemberIds();
}

DynamicTypeBuilder::DynamicTypeBuilder(const DynamicType* pType)
    : DynamicType()
    , mCurrentMemberId(0)
    , mMaxIndex(0)
{
    mIsTypeObject = false;

    if (pType != nullptr)
    {
        CopyFromType(pType);
    }
    RefreshMemberIds();
}

DynamicTypeBuilder::~DynamicTypeBuilder()
{
}

ResponseCode DynamicTypeBuilder::AddMember(const MemberDescriptor* descriptor)
{
    if (mDescriptor != nullptr && descriptor != nullptr && descriptor->IsConsistent(mDescriptor->GetKind()))
    {
        if (mDescriptor->GetKind() == TK_ANNOTATION || mDescriptor->GetKind() == TK_BITMASK
            || mDescriptor->GetKind() == TK_ENUM || mDescriptor->GetKind() == TK_STRUCTURE
            || mDescriptor->GetKind() == TK_UNION)
        {
            if (!ExistsMemberByName(descriptor->GetName()))
            {
                if (CheckUnionConfiguration(descriptor))
                {
                    DynamicTypeMember* newMember = new DynamicTypeMember(descriptor, mCurrentMemberId);

                    // If the index of the new member is bigger than the current maximum, put it at the end.
                    if (newMember->GetIndex() > mMaxIndex)
                    {
                        newMember->SetIndex(++mMaxIndex);
                    }
                    else
                    {
                        // Move every member bigger than the current index to the right.
                        for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
                        {
                            if (it->second->GetIndex() >= newMember->GetIndex())
                            {
                                it->second->SetIndex(it->second->GetIndex() + 1);
                            }
                        }
                    }

                    mMemberById.insert(std::make_pair(mCurrentMemberId, newMember));
                    mMemberByName.insert(std::make_pair(newMember->GetName(), newMember));
                    ++mCurrentMemberId;
                    return ResponseCode::RETCODE_OK;
                }
                else
                {
                    logWarning(DYN_TYPES, "Error adding member, invalid union parameters.");
                    return ResponseCode::RETCODE_BAD_PARAMETER;
                }
            }
            else
            {
                logWarning(DYN_TYPES, "Error adding member, there is other member with the same name.");
                return ResponseCode::RETCODE_BAD_PARAMETER;
            }
        }
        else
        {
            logWarning(DYN_TYPES, "Error adding member, the current type " << mDescriptor->GetKind()
                << " doesn't support members.");
            return ResponseCode::RETCODE_PRECONDITION_NOT_MET;
        }
    }
    else
    {
        if (descriptor == nullptr)
        {
            logWarning(DYN_TYPES, "Error adding member, Invalid input descriptor.");
        }
        else
        {
            logWarning(DYN_TYPES, "Error adding member, The input descriptor isn't consistent.");
        }
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicTypeBuilder::ApplyAnnotation(AnnotationDescriptor& descriptor)
{
    if (descriptor.IsConsistent())
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->CopyFrom(&descriptor);
        mAnnotation.push_back(pNewDescriptor);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicTypeBuilder::ApplyAnnotationToMember(MemberId id, AnnotationDescriptor& descriptor)
{
    if (descriptor.IsConsistent())
    {
        auto it = mMemberById.find(id);
        if (it != mMemberById.end())
        {
            it->second->ApplyAnnotation(descriptor);
            return ResponseCode::RETCODE_OK;
        }
        else
        {
            logError(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation to member. The input descriptor isn't consistent.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

DynamicType* DynamicTypeBuilder::Build()
{
    if (mDescriptor->IsConsistent())
    {
        DynamicType* newType = DynamicTypeBuilderFactory::GetInstance()->BuildType(mDescriptor);
        newType->mName = mDescriptor->GetName();
        newType->mKind = mDescriptor->GetKind();
        for (auto it = mAnnotation.begin(); it != mAnnotation.end(); ++it)
        {
            AnnotationDescriptor* newAnnotation = new AnnotationDescriptor();
            newAnnotation->CopyFrom(*it);
            newType->mAnnotation.push_back(newAnnotation);
        }

        for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
        {
            DynamicTypeMember* newMember = new DynamicTypeMember(it->second);
            newMember->SetParent(newType);
            newType->mMemberById.insert(std::make_pair(newMember->GetId(), newMember));
            newType->mMemberByName.insert(std::make_pair(newMember->GetName(), newMember));
        }

        return newType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type. The current descriptor isn't consistent.");
        return nullptr;
    }
}

bool DynamicTypeBuilder::CheckUnionConfiguration(const MemberDescriptor* descriptor)
{
    if (mDescriptor->GetKind() == TK_UNION)
    {
        if (descriptor->GetUnionLabels().size() == 0)
        {
            return false;
        }
        for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
        {
            // Check that there isn't any member as default label and that there isn't any member with the same case.
            if ((descriptor->IsDefaultUnionValue() && it->second->IsDefaultUnionValue()) ||
                !descriptor->CheckUnionLabels(it->second->GetUnionLabels()))
            {
                return false;
            }
        }
    }
    return true;
}

ResponseCode DynamicTypeBuilder::CopyFrom(const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        Clear();

        ResponseCode res = CopyFromType(other);
        if (res == ResponseCode::RETCODE_OK)
        {
            mCurrentMemberId = other->mCurrentMemberId;
        }
        return res;
    }
    else
    {
        logError(DYN_TYPES, "Error copying DynamicTypeBuilder. Invalid input parameter.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

void DynamicTypeBuilder::Clear()
{
    DynamicType::Clear();

    mCurrentMemberId = 0;
}

void DynamicTypeBuilder::RefreshMemberIds()
{
    if (mDescriptor->GetKind() == TK_STRUCTURE && mDescriptor->GetBaseType() != nullptr)
    {
        mCurrentMemberId = mDescriptor->GetBaseType()->GetMembersCount();
    }
}

ResponseCode DynamicTypeBuilder::SetName(const std::string& name)
{
    if (mDescriptor != nullptr)
    {
        mDescriptor->SetName(name);
    }
    mName = name;
    return ResponseCode::RETCODE_OK;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
