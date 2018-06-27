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

namespace eprosima {
namespace fastrtps {
namespace types {

MemberDescriptor::MemberDescriptor()
: mName("")
, mId(MEMBER_ID_INVALID)
, mType(nullptr)
, mDefaultValue("")
, mIndex(0)
, mDefaultLabel(false)
{
}

ResponseCode MemberDescriptor::copy_from(const MemberDescriptor* other)
{
    if (other != nullptr)
    {
        try
        {
            mName = other->mName;
            mId = other->mId;
            mType = other->mType;
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
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

bool MemberDescriptor::equals(const MemberDescriptor* other) const
{
    if (other != nullptr && mName == other->mName && mId == other->mId && mType == other->mType &&
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
bool MemberDescriptor::isConsistent() const
{
    return false;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
