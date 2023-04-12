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

#ifndef TYPES_1_3_ANNOTATION_DESCRIPTOR_H
#define TYPES_1_3_ANNOTATION_DESCRIPTOR_H

#include <fastrtps/types/TypesBase.h>

#include <functional>

namespace eprosima{
namespace fastrtps{
namespace types{
namespace v1_3{

class MemberDescriptor;
class DynamicType;
class DynamicTypeBuilderFactory;

class AnnotationDescriptor final
{
protected:
    friend class DynamicTypeBuilderFactory;

    // Reference to the annotation type
    DynamicType_ptr type_;
	std::map<std::string, std::string> value_;

public:

    ReturnCode_t copy_from(const AnnotationDescriptor* other);
    bool operator==(const AnnotationDescriptor&) const;
    bool operator!=(const AnnotationDescriptor&) const;
    bool operator<(const AnnotationDescriptor&) const;
    bool equals(const AnnotationDescriptor*) const;
    bool is_consistent() const;
    bool key_annotation() const;

    ReturnCode_t get_value(
            std::string& value,
            const std::string& key) const;

    ReturnCode_t get_value(std::string& value) const; // key = "value"

    ReturnCode_t get_all_value(std::map<std::string, std::string>& value) const;

    const std::map<std::string, std::string>& get_all_values() const
    {
        return value_;
    }

    ReturnCode_t set_value(
        const std::string& key,
        const std::string& value);

    void set_type(const DynamicType_ptr& type)
    {
        type_ = type;
    }

    void set_type(DynamicType_ptr&& type)
    {
        type_ = std::move(type);
    }

    const DynamicType_ptr type() const
    {
        return type_;
    }
};

std::ostream& operator<<( std::ostream& os, const AnnotationDescriptor& md);

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_ANNOTATION_DESCRIPTOR_H
