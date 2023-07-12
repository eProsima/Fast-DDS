// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_1_3_ANNOTATION_DESCRIPTOR_IMPL_H
#define TYPES_1_3_ANNOTATION_DESCRIPTOR_IMPL_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/AnnotationDescriptor.hpp>

#include <functional>
#include <map>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicTypeImpl;
class DynamicTypeBuilderFactoryImpl;

class AnnotationDescriptorImpl final
{
protected:

    friend class DynamicTypeBuilderFactoryImpl;

    // Reference to the annotation type
    std::shared_ptr<const DynamicTypeImpl> type_;
    std::map<std::string, std::string> value_;

public:

    bool operator ==(
            const AnnotationDescriptorImpl&) const;
    bool operator !=(
            const AnnotationDescriptorImpl&) const;
    bool operator <(
            const AnnotationDescriptorImpl&) const;
    bool is_consistent() const;
    bool key_annotation() const;

    ReturnCode_t get_value(
            std::string& value,
            const std::string& key) const;

    ReturnCode_t get_value(
            std::string& value) const;                // key = "value"

    ReturnCode_t get_all_value(
            std::map<std::string, std::string>& value) const;

    const std::map<std::string, std::string>& get_all_values() const
    {
        return value_;
    }

    ReturnCode_t set_value(
            const std::string& key,
            const std::string& value);

    template<class D,
             typename std::enable_if<
                 std::is_constructible<std::shared_ptr<const DynamicTypeImpl>, D>::value,
                 bool>::type = true>
    void set_type(
            const D& type)
    {
        std::shared_ptr<const DynamicTypeImpl> tmp{type};
        set_type(std::move(tmp));
    }

    void set_type(
            std::shared_ptr<const DynamicTypeImpl>&& type)
    {
        type_ = std::move(type);
    }

    const std::shared_ptr<const DynamicTypeImpl> type() const
    {
        return type_;
    }

    AnnotationDescriptor get_descriptor() const;

};

std::ostream& operator <<(
        std::ostream& os,
        const AnnotationDescriptorImpl& md);

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_ANNOTATION_DESCRIPTOR_IMPL_H
