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
    : mDescriptor(nullptr)
    , mCurrentMemberId(0)
    , mMaxIndex(0)
{
}

DynamicTypeBuilder::DynamicTypeBuilder(const DynamicTypeBuilder* builder)
    : mCurrentMemberId(0)
    , mMaxIndex(0)
{
    CopyFromBuilder(builder);
}

DynamicTypeBuilder::DynamicTypeBuilder(const TypeDescriptor* descriptor)
    : mCurrentMemberId(0)
    , mMaxIndex(0)
{
    mDescriptor = new TypeDescriptor(descriptor);
    try
    {
        mName = descriptor->GetName();
        mKind = descriptor->GetKind();
    }
    catch (...)
    {
        mName = "";
        mKind = TK_NONE;
    }

    // Alias types use the same members than it's base class.
    if (mKind == TK_ALIAS)
    {
        for (auto it = mDescriptor->GetBaseType()->mMemberById.begin();
            it != mDescriptor->GetBaseType()->mMemberById.end(); ++it)
        {
            mMemberByName.insert(std::make_pair(it->second->GetName(), it->second));
        }
    }

    RefreshMemberIds();
}

DynamicTypeBuilder::~DynamicTypeBuilder()
{
    mName = "";
    mKind = 0;
    if (mDescriptor != nullptr)
    {
        delete mDescriptor;
        mDescriptor = nullptr;
    }

    for (auto it = mAnnotation.begin(); it != mAnnotation.end(); ++it)
    {
        delete *it;
    }
    mAnnotation.clear();

    for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
    {
        delete it->second;
    }
    mMemberById.clear();
    mMemberByName.clear();
}

ResponseCode DynamicTypeBuilder::AddEmptyMember(uint32_t index, const std::string& name)
{
    MemberDescriptor descriptor(index, name);
    return AddMember(&descriptor);
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
                        newMember->SetIndex(mMaxIndex++);
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

ResponseCode DynamicTypeBuilder::AddMember(MemberId id, const std::string& name, DynamicTypeBuilder* mType)
{
    if (mType != nullptr)
    {
        MemberDescriptor descriptor(id, name, DynamicTypeBuilderFactory::GetInstance()->CreateType(mType));
        return AddMember(&descriptor);
    }
    else
    {
        MemberDescriptor descriptor(id, name, nullptr);
        return AddMember(&descriptor);
    }
}

ResponseCode DynamicTypeBuilder::AddMember(MemberId id, const std::string& name, DynamicTypeBuilder* mType,
    const std::string& defaultValue)
{
    MemberDescriptor descriptor(id, name, DynamicTypeBuilderFactory::GetInstance()->CreateType(mType), defaultValue);
    return AddMember(&descriptor);
}

ResponseCode DynamicTypeBuilder::AddMember(MemberId id, const std::string& name, DynamicTypeBuilder* mType,
    const std::string& defaultValue, const std::vector<uint64_t>& unionLabels, bool isDefaultLabel)
{
    MemberDescriptor descriptor(id, name, DynamicTypeBuilderFactory::GetInstance()->CreateType(mType),
        defaultValue, unionLabels, isDefaultLabel);
    return AddMember(&descriptor);
}

ResponseCode DynamicTypeBuilder::AddMember(MemberId id, const std::string& name, DynamicType_ptr mType)
{
    MemberDescriptor descriptor(id, name, mType);
    return AddMember(&descriptor);
}

ResponseCode DynamicTypeBuilder::AddMember(MemberId id, const std::string& name, DynamicType_ptr mType,
    const std::string& defaultValue)
{
    MemberDescriptor descriptor(id, name, mType, defaultValue);
    return AddMember(&descriptor);
}

ResponseCode DynamicTypeBuilder::AddMember(MemberId id, const std::string& name, DynamicType_ptr mType,
    const std::string& defaultValue, const std::vector<uint64_t>& unionLabels, bool isDefaultLabel)
{
    MemberDescriptor descriptor(id, name, mType, defaultValue, unionLabels, isDefaultLabel);
    return AddMember(&descriptor);
}

ResponseCode DynamicTypeBuilder::ApplyAnnotation(AnnotationDescriptor& descriptor)
{
    return _ApplyAnnotation(descriptor);
}

ResponseCode DynamicTypeBuilder::ApplyAnnotation(std::string key, std::string value)
{
    return _ApplyAnnotation(key, value);
}

ResponseCode DynamicTypeBuilder::ApplyAnnotationToMember(MemberId id, AnnotationDescriptor& descriptor)
{
    return _ApplyAnnotationToMember(id, descriptor);
}

ResponseCode DynamicTypeBuilder::ApplyAnnotationToMember(MemberId id, std::string key, std::string value)
{
    return _ApplyAnnotationToMember(id, key, value);
}

DynamicType_ptr DynamicTypeBuilder::Build()
{
    if (mDescriptor->IsConsistent())
    {
        return DynamicTypeBuilderFactory::GetInstance()->CreateType(this);
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
        if (!descriptor->IsDefaultUnionValue() && descriptor->GetUnionLabels().size() == 0)
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

        ResponseCode res = CopyFromBuilder(other);
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


ResponseCode DynamicTypeBuilder::CopyFromBuilder(const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        Clear();

        mName = other->mName;
        mKind = other->mKind;
        mDescriptor = new TypeDescriptor(other->mDescriptor);

        for (auto it = other->mAnnotation.begin(); it != other->mAnnotation.end(); ++it)
        {
            AnnotationDescriptor* newDescriptor = new AnnotationDescriptor(*it);
            mAnnotation.push_back(newDescriptor);
        }

        for (auto it = other->mMemberById.begin(); it != other->mMemberById.end(); ++it)
        {
            DynamicTypeMember* newMember = new DynamicTypeMember(it->second);
            mMemberById.insert(std::make_pair(newMember->GetId(), newMember));
            mMemberByName.insert(std::make_pair(newMember->GetName(), newMember));
        }

        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error copying DynamicType, invalid input type");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

void DynamicTypeBuilder::Clear()
{
    mName = "";
    mKind = 0;
    if (mDescriptor != nullptr)
    {
        delete mDescriptor;
        mDescriptor = nullptr;
    }

    for (auto it = mAnnotation.begin(); it != mAnnotation.end(); ++it)
    {
        delete *it;
    }
    mAnnotation.clear();

    for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
    {
        delete it->second;
    }
    mMemberById.clear();
    mMemberByName.clear();
    mCurrentMemberId = 0;
}

bool DynamicTypeBuilder::ExistsMemberByName(const std::string& name) const
{
    if (mDescriptor->GetBaseType() != nullptr)
    {
        if (mDescriptor->GetBaseType()->ExistsMemberByName(name))
        {
            return true;
        }
    }
    return mMemberByName.find(name) != mMemberByName.end();
}

ResponseCode DynamicTypeBuilder::GetAllMembers(std::map<MemberId, DynamicTypeMember*>& members)
{
    members = mMemberById;
    return ResponseCode::RETCODE_OK;
}

std::string DynamicTypeBuilder::GetName() const
{
    return mName;
}

bool DynamicTypeBuilder::IsConsistent() const
{
    return mDescriptor->IsConsistent();
}

bool DynamicTypeBuilder::IsDiscriminatorType() const
{
    if (mKind == TK_ALIAS && mDescriptor != nullptr && mDescriptor->GetBaseType() != nullptr)
    {
        return mDescriptor->GetBaseType()->IsDiscriminatorType();
    }
    return mKind == TK_BOOLEAN || mKind == TK_BYTE || mKind == TK_INT16 || mKind == TK_INT32 ||
        mKind == TK_INT64 || mKind == TK_UINT16 || mKind == TK_UINT32 || mKind == TK_UINT64 ||
        mKind == TK_FLOAT32 || mKind == TK_FLOAT64 || mKind == TK_FLOAT128 || mKind == TK_CHAR8 ||
        mKind == TK_CHAR16 || mKind == TK_STRING8 || mKind == TK_STRING16 || mKind == TK_ENUM || mKind == TK_BITMASK;
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

ResponseCode DynamicTypeBuilder::_ApplyAnnotation(AnnotationDescriptor& descriptor)
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

ResponseCode DynamicTypeBuilder::_ApplyAnnotation(const std::string& key, const std::string& value)
{
    auto it = mAnnotation.begin();
    if (it != mAnnotation.end())
    {
        (*it)->SetValue(key, value);
    }
    else
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->SetType(DynamicTypeBuilderFactory::GetInstance()->CreateAnnotationPrimitive());
        pNewDescriptor->SetValue(key, value);
        mAnnotation.push_back(pNewDescriptor);
    }

    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicTypeBuilder::_ApplyAnnotationToMember(MemberId id, AnnotationDescriptor& descriptor)
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

ResponseCode DynamicTypeBuilder::_ApplyAnnotationToMember(MemberId id, const std::string& key, const std::string& value)
{
    auto it = mMemberById.find(id);
    if (it != mMemberById.end())
    {
        it->second->ApplyAnnotation(key, value);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}
} // namespace types
} // namespace fastrtps
} // namespace eprosima
