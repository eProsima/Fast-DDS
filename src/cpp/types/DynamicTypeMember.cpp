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
    }
    else
    {
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool DynamicTypeMember::equals(const DynamicTypeMember& other) const
{
    return false;
}

std::string DynamicTypeMember::get_name() const
{
    return mName;
}

MemberId DynamicTypeMember::get_id() const
{
    return mId;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
