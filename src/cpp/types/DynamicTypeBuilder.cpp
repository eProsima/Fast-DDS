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
#include <fastrtps/types/DynamicType.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeBuilder::DynamicTypeBuilder()
{
}

ResponseCode DynamicTypeBuilder::get_descriptor(TypeDescriptor* descriptor) const
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

bool DynamicTypeBuilder::equals(const DynamicType& /*other*/) const
{
    return false;
}

std::string DynamicTypeBuilder::get_name() const
{
    return "";
}

TypeKind DynamicTypeBuilder::get_kind() const
{
    return 0;
}

ResponseCode DynamicTypeBuilder::add_member(MemberDescriptor&)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicTypeBuilder::apply_annotation(AnnotationDescriptor&)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicTypeBuilder::apply_annotation_to_member(MemberId, AnnotationDescriptor&)
{
    return ResponseCode::RETCODE_OK;
}

DynamicType* DynamicTypeBuilder::build()
{
    return nullptr;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
