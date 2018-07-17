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

#ifndef TYPES_ANNOTATION_DESCRIPTOR_H
#define TYPES_ANNOTATION_DESCRIPTOR_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class MemberDescriptor;
class DynamicType;

class AnnotationDescriptor
{
public:
    AnnotationDescriptor();
    ~AnnotationDescriptor();
    AnnotationDescriptor(const AnnotationDescriptor* descriptor);
    AnnotationDescriptor(DynamicType_ptr pType);

    ResponseCode CopyFrom(const AnnotationDescriptor* other);
    bool Equals(const AnnotationDescriptor*) const;
    bool IsConsistent() const;
    bool GetKeyAnnotation() const;

    ResponseCode GetValue(std::string& value, const std::string& key);
    ResponseCode GetAllValues(std::map<std::string, std::string>& value);
    ResponseCode SetValue(const std::string& key, const std::string& value);
    void SetType(DynamicType_ptr pType);

protected:

    DynamicType_ptr mType;
	std::map<std::string, std::string> mValue;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_ANNOTATION_DESCRIPTOR_H
