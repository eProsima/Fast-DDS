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

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeMember::DynamicTypeMember()
{
}

ResponseCode DynamicTypeMember::get_descriptor(MemberDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        //TODO: //ARCE: Fill descriptor with all the changes applied.
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool DynamicTypeMember::equals(const DynamicTypeMember& other) const
{
    if (mAnnotation.size() != other.mAnnotation.size())
    {
        return false;
    }
    else
    {
        for (auto it = mAnnotation.begin(), it2 = other.mAnnotation.begin(); it != mAnnotation.end(); ++it, ++it2)
        {

            if (!(*it)->equals(*it2))
            {
                return false;
            }
        }
    }
    return true;
}

std::string DynamicTypeMember::get_name() const
{
    return mParent->get_name();
}

MemberId DynamicTypeMember::get_id() const
{
    return mId;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
