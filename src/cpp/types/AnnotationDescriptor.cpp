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

#include <fastrtps/types/AnnotationDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

AnnotationDescriptor::AnnotationDescriptor()
{
}

ResponseCode AnnotationDescriptor::copy_from(const AnnotationDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        try
        {
        }
        catch(std::exception& e)
        {
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool AnnotationDescriptor::equals(const AnnotationDescriptor&) const
{
    return false;
    /*
    std::string mName;
    MemberId mId;
    DynamicType mType;
    std::string mDefaultValue;
    const uint32_t mIndex;
    uint64_t* mLabel;
    bool mDefaultLabel;
    */
}

bool AnnotationDescriptor::isConsistent() const
{
    return false;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
