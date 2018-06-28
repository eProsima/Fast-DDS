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
: mType(nullptr)
{
}

AnnotationDescriptor::AnnotationDescriptor(const AnnotationDescriptor* descriptor)
{
    copy_from(descriptor);
}

AnnotationDescriptor::AnnotationDescriptor(DynamicType* pType)
: mType(pType)
{
}

ResponseCode AnnotationDescriptor::copy_from(const AnnotationDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        try
        {
            mType = descriptor->mType;
            mValue = descriptor->mValue;
        }
        catch(std::exception& /*e*/)
        {
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
    return ResponseCode::RETCODE_OK;
}

bool AnnotationDescriptor::equals(const AnnotationDescriptor* other) const
{
    if (other != nullptr && mType == other->mType)
    {
        for (auto it = mValue.begin(); it != mValue.end(); ++it)
        {
            auto it2 = other->mValue.find(it->first);
            if (it2 == other->mValue.end() || it2->second != it->second)
            {
                return false;
            }
        }

        for (auto it = other->mValue.begin(); it != other->mValue.end(); ++it)
        {
            auto it2 = mValue.find(it->first);
            if (it2 == mValue.end() || it2->second != it->second)
            {
                return false;
            }
        }
    }
    return false;
}

bool AnnotationDescriptor::isConsistent() const
{
    if (mType == nullptr)
    {
        return false;
    }

    //TODO: Check consistency of mValue


    return true;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
