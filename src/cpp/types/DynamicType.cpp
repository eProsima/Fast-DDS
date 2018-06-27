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

#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicTypeMember.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicType::DynamicType()
{
}

ResponseCode DynamicType::get_descriptor(TypeDescriptor* descriptor) const
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

bool DynamicType::equals(const DynamicType& other) const
{
    return false;
}

std::string DynamicType::get_name() const
{
    return mName;
}

TypeKind DynamicType::get_kind() const
{
    return mKind;
}

ResponseCode DynamicType::get_member_by_name(DynamicTypeMember& member, const std::string name)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicType::get_all_members_by_name(std::map<std::string, DynamicTypeMember>& member)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicType::get_member(DynamicTypeMember& member, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicType::get_all_members(std::map<MemberId, DynamicTypeMember>& members)
{
    return ResponseCode::RETCODE_OK;
}

uint32_t DynamicType::get_annotation_count()
{
    return mAnnotation.size();
}

ResponseCode DynamicType::get_annotation(AnnotationDescriptor& descriptor, uint32_t idx)
{
    return ResponseCode::RETCODE_OK;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
