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

#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

static bool IsTypeNameConsistent(const std::string& sName)
{
    // The first letter must start with a letter ( uppercase or lowercase )
    if (sName.length() > 0 && std::isalpha(sName[0]))
    {
        // All characters must be letters, numbers or underscore.
        for (uint32_t i = 1; i < sName.length(); ++i)
        {
            if (!std::isalnum(sName[i]) && sName[i] != 95)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

MemberDescriptor::MemberDescriptor()
: mName("")
, mId(MEMBER_ID_INVALID)
, mType(nullptr)
, mDefaultValue("")
, mIndex(INDEX_INVALID)
, mDefaultLabel(false)
{
}

MemberDescriptor::MemberDescriptor(const MemberDescriptor* descriptor)
{
    CopyFrom(descriptor);
}

MemberDescriptor::~MemberDescriptor()
{
    if (mType != nullptr)
    {
        DynamicTypeBuilderFactory::GetInstance()->DeleteType(mType);
    }
}

ResponseCode MemberDescriptor::CopyFrom(const MemberDescriptor* other)
{
    if (other != nullptr)
    {
        try
        {
            if (mType != nullptr)
            {
                DynamicTypeBuilderFactory::GetInstance()->DeleteType(mType);
            }
            mType = DynamicTypeBuilderFactory::GetInstance()->BuildType(other->mType);
            mName = other->mName;
            mId = other->mId;
            mDefaultValue = other->mDefaultValue;
            mIndex = other->mIndex;
            mDefaultLabel = other->mDefaultLabel;
            mLabel = other->mLabel;
            return ResponseCode::RETCODE_OK;
        }
        catch (std::exception& /*e*/)
        {
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error copying MemberDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool MemberDescriptor::Equals(const MemberDescriptor* other) const
{
    if (other != nullptr && mName == other->mName && mId == other->mId &&
        ((mType == nullptr && other->mType == nullptr) || mType->Equals(other->mType)) &&
        mDefaultValue == other->mDefaultValue && mIndex == other->mIndex && mDefaultLabel == other->mDefaultLabel &&
        mLabel.size() == other->mLabel.size())
    {
        for (auto it = mLabel.begin(), it2 = other->mLabel.begin(); it != mLabel.end(); ++it, ++it2)
        {
            if (*it != *it2)
                return false;
        }
        return true;
    }
    return false;
}

MemberId MemberDescriptor::GetId() const
{
    return mId;
}

uint32_t MemberDescriptor::GetIndex() const
{
    return mIndex;
}

TypeKind MemberDescriptor::GetKind() const
{
    if (mType != nullptr)
    {
        return mType->GetKind();
    }
    return 0;
}

std::string MemberDescriptor::GetName() const
{
    return mName;
}

bool MemberDescriptor::IsConsistent() const
{
    // The type field is mandatory.
    if (mType == nullptr)
    {
        return false;
    }

    // Only aggregated types must use the ID value.
    if (mId != MEMBER_ID_INVALID && mType->GetKind() != TK_UNION && mType->GetKind() != TK_STRUCTURE &&
        mType->GetKind() != TK_ANNOTATION)
    {
        return false;
    }

    //TODO: Check default value.
    if (!IsTypeNameConsistent(mName))
    {
        return false;
    }

    // Only Unions need the field "label"
    if (mLabel.size() != 0 && mType->GetKind() != TK_UNION)
    {
        return false;
    }
    // If the field ins't de default value for the union, it must have a label value.
    else if (mType->GetKind() == TK_UNION && mDefaultLabel == false && mLabel.size() == 0)
    {
        return false;
    }

    return false;
}

void MemberDescriptor::SetId(MemberId id)
{
    mId = id;
}

void MemberDescriptor::SetIndex(uint32_t index)
{
    mIndex = index;
}

void MemberDescriptor::SetName(const std::string& name)
{
    mName = name;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
