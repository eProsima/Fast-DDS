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
: type_(nullptr)
{
}

AnnotationDescriptor::~AnnotationDescriptor()
{
    type_ = nullptr;
}

AnnotationDescriptor::AnnotationDescriptor(const AnnotationDescriptor* descriptor)
{
    copy_from(descriptor);
}

AnnotationDescriptor::AnnotationDescriptor(DynamicType_ptr pType)
{
    type_ = pType;
}

ResponseCode AnnotationDescriptor::copy_from(const AnnotationDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        try
        {
            type_ = descriptor->type_;
            value_ = descriptor->value_;
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

bool AnnotationDescriptor::equals(const AnnotationDescriptor* other) const
{
    if (other != nullptr && (type_ == other->type_ || (type_ != nullptr && type_->equals(other->type_.get()))))
    {
        if (value_.size() != other->value_.size())
        {
            return false;
        }

        for (auto it = value_.begin(); it != value_.end(); ++it)
        {
            auto it2 = other->value_.find(it->first);
            if (it2 == other->value_.end() || it2->second != it->second)
            {
                return false;
            }
        }
    }
    return true;
}

bool AnnotationDescriptor::key_annotation() const
{
    auto it = value_.find(ANNOTATION_KEY_ID);
    if (it == value_.end())
    {
        it = value_.find(ANNOTATION_EPKEY_ID); // Legacy "@Key"
    }
    return (it != value_.end() && it->second == CONST_TRUE);
}

ResponseCode AnnotationDescriptor::get_value(std::string& value)
{
    return get_value(value, "value");
}

ResponseCode AnnotationDescriptor::get_value(
        std::string& value,
        const std::string& key)
{
    auto it = value_.find(key);
    if (it != value_.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode AnnotationDescriptor::get_all_value(std::map<std::string, std::string>& value) const
{
    value = value_;
    return ResponseCode::RETCODE_OK;
}

bool AnnotationDescriptor::is_consistent() const
{
    if (type_ == nullptr || type_->get_kind() != TK_ANNOTATION)
    {
        return false;
    }

    //TODO: Check consistency of value_
    return true;
}

void AnnotationDescriptor::set_type(DynamicType_ptr pType)
{
    type_ = pType;
}

ResponseCode AnnotationDescriptor::set_value(
        const std::string& key,
        const std::string& value)
{
    value_[key] = value;
    return ResponseCode::RETCODE_OK;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
