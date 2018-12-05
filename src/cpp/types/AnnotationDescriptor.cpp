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
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

AnnotationDescriptor::AnnotationDescriptor()
: mType(nullptr)
{
}

AnnotationDescriptor::~AnnotationDescriptor()
{
    mType = nullptr;
}

AnnotationDescriptor::AnnotationDescriptor(const AnnotationDescriptor* descriptor)
{
    CopyFrom(descriptor);
}

AnnotationDescriptor::AnnotationDescriptor(DynamicType_ptr pType)
{
    mType = pType;
}

ResponseCode AnnotationDescriptor::CopyFrom(const AnnotationDescriptor* descriptor)
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
        logError(DYN_TYPES, "Error copying AnnotationDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
    return ResponseCode::RETCODE_OK;
}

bool AnnotationDescriptor::Equals(const AnnotationDescriptor* other) const
{
    if (other != nullptr && (mType == other->mType || (mType != nullptr && mType->Equals(other->mType.get()))))
    {
        if (mValue.size() != other->mValue.size())
        {
            return false;
        }

        for (auto it = mValue.begin(); it != mValue.end(); ++it)
        {
            auto it2 = other->mValue.find(it->first);
            if (it2 == other->mValue.end() || it2->second != it->second)
            {
                return false;
            }
        }
    }
    return true;
}

bool AnnotationDescriptor::GetKeyAnnotation() const
{
    auto it = mValue.find(ANNOTATION_KEY_ID);
    return (it != mValue.end() && it->second == "true");
}

ResponseCode AnnotationDescriptor::GetValue(std::string& value, const std::string& key)
{
    auto it = mValue.find(key);
    if (it != mValue.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode AnnotationDescriptor::GetAllValues(std::map<std::string, std::string>& value)
{
    value = mValue;
    return ResponseCode::RETCODE_OK;
}

bool AnnotationDescriptor::IsConsistent() const
{
    if (mType == nullptr || mType->GetKind() != TK_ANNOTATION)
    {
        return false;
    }

    //TODO: Check consistency of mValue
    return true;
}

void AnnotationDescriptor::SetType(DynamicType_ptr pType)
{
    mType = pType;
}

ResponseCode AnnotationDescriptor::SetValue(const std::string& key, const std::string& value)
{
    mValue[key] = value;
    return ResponseCode::RETCODE_OK;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
