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

void MemberDescriptor::AddUnionCaseIndex(uint64_t value)
{
    mLabels.push_back(value);
}

bool MemberDescriptor::CheckUnionLabels(const std::vector<uint64_t>& labels) const
{
    for (auto it = labels.begin(); it != labels.end(); ++it)
    {
        if (std::find(mLabels.begin(), mLabels.end(), *it) != mLabels.end())
        {
            return false;
        }
    }
    return true;
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
            mLabels = other->mLabels;
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
        mLabels.size() == other->mLabels.size())
    {
        for (auto it = mLabels.begin(), it2 = other->mLabels.begin(); it != mLabels.end(); ++it, ++it2)
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

std::vector<uint64_t> MemberDescriptor::GetUnionLabels() const
{
    return mLabels;
}

bool MemberDescriptor::IsConsistent(TypeKind parentKind) const
{
    // The type field is mandatory in every type except bitmasks and enums.
    if ((parentKind != TK_BITMASK && parentKind != TK_ENUM) && mType == nullptr)
    {
        return false;
    }

    // Only aggregated types must use the ID value.
    if (mId != MEMBER_ID_INVALID && parentKind != TK_UNION && parentKind != TK_STRUCTURE &&
        parentKind != TK_ANNOTATION)
    {
        return false;
    }

    //TODO: Check default value.
    if (!IsTypeNameConsistent(mName))
    {
        return false;
    }

    // Only Unions need the field "label"
    if (mLabels.size() != 0 && parentKind != TK_UNION)
    {
        return false;
    }
    // If the field isn't the default value for the union, it must have a label value.
    else if (parentKind == TK_UNION && mDefaultLabel == false && mLabels.size() == 0)
    {
        return false;
    }

    return true;
}

bool MemberDescriptor::IsDefaultUnionValue() const
{
    return mDefaultLabel;
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

void MemberDescriptor::SetType(DynamicType* type)
{
    mType = type;
}

void MemberDescriptor::SetDefaultUnionValue(bool bDefault)
{
    mDefaultLabel = bDefault;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
