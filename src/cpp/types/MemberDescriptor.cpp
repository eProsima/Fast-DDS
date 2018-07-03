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
, mIndex(0)
, mDefaultLabel(false)
{
}

MemberDescriptor::MemberDescriptor(const MemberDescriptor* descriptor)
{
    copy_from(descriptor);
}

MemberDescriptor::~MemberDescriptor()
{
    if (mType != nullptr)
    {
        delete mType;
    }
}

ResponseCode MemberDescriptor::copy_from(const MemberDescriptor* other)
{
    if (other != nullptr)
    {
        try
        {
            if (mType != nullptr)
            {
                delete mType;
            }
            mType = new DynamicType(other->mType);
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

bool MemberDescriptor::equals(const MemberDescriptor* other) const
{
    if (other != nullptr && mName == other->mName && mId == other->mId &&
        ((mType == nullptr && other->mType == nullptr) || mType->equals(other->mType)) &&
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

MemberId MemberDescriptor::get_id() const
{
    return mId;
}

uint32_t MemberDescriptor::get_index() const
{
    return mIndex;
}

void MemberDescriptor::set_index(uint32_t index)
{
    mIndex = index;
}

TypeKind MemberDescriptor::get_kind() const
{
    if (mType != nullptr)
    {
        return mType->get_kind();
    }
    return 0;
}

std::string MemberDescriptor::get_name() const
{
    return mName;
}

bool MemberDescriptor::isConsistent() const
{
    // The type field is mandatory.
    if (mType == nullptr)
    {
        return false;
    }

    // Only aggregated types must use the ID value.
    if (mId != MEMBER_ID_INVALID && mType->get_kind() != TK_UNION && mType->get_kind() != TK_STRUCTURE &&
        mType->get_kind() != TK_ANNOTATION)
    {
        return false;
    }

    //TODO: Check default value.
    if (!IsTypeNameConsistent(mName))
    {
        return false;
    }

    // Only Unions need the field "label"
    if (mLabel.size() != 0 && mType->get_kind() != TK_UNION)
    {
        return false;
    }
    // If the field ins't de default value for the union, it must have a label value.
    else if (mType->get_kind() == TK_UNION && mDefaultLabel == false && mLabel.size() == 0)
    {
        return false;
    }

    return false;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
