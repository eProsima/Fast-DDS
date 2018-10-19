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

class MemberDescriptor;
class DynamicType;

namespace eprosima{
namespace fastrtps{
namespace types{

class AnnotationDescriptor
{
public:
    AnnotationDescriptor();

    ResponseCode get_value(std::string& value, const std::string& key);
    ResponseCode get_all_value(std::map<std::string, std::string>& value);
    ResponseCode set_value(const std::string& key, const std::string& value);

	ResponseCode copy_from(const AnnotationDescriptor* other);
	bool equals(const AnnotationDescriptor*) const;
	bool isConsistent() const;
protected:

	DynamicType* mType;
	std::map<std::string, std::string> mValue;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_ANNOTATION_DESCRIPTOR_H
