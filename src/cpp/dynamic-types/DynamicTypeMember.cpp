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

#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastdds/dds/log/Log.hpp>

#include <cassert>

using namespace eprosima::fastrtps::types;

ReturnCode_t DynamicTypeMember::apply_annotation(
        AnnotationDescriptor& descriptor)
{
    // Update the annotations on the member Dynamic Type.
    return descriptor_.apply_annotation(descriptor);
}

ReturnCode_t DynamicTypeMember::apply_annotation(
        const std::string& annotation_name,
        const std::string& key,
        const std::string& value)
{
    // Update the annotations on the member Dynamic Type.
    return descriptor_.apply_annotation(annotation_name, key, value);
}

bool DynamicTypeMember::operator==(const DynamicTypeMember& other) const
{
    return get_descriptor() == other.get_descriptor() && annotation_ == other.annotation_;
}

bool DynamicTypeMember::equals(
        const DynamicTypeMember& other) const
{
    return *this == other;
}

ReturnCode_t DynamicTypeMember::get_annotation(
        AnnotationDescriptor& descriptor,
        MemberId idx)
{
    assert(idx < get_annotation_count());
    descriptor = *std::advance(annotation_.begin(), idx);
    return ReturnCode_t::RETCODE_OK;
}

MemberId DynamicTypeMember::get_annotation_count()
{
    return annotation_.size();
}

ReturnCode_t DynamicTypeMember::get_descriptor(
        MemberDescriptor& descriptor) const
{
    descriptor = get_descriptor();
    return ReturnCode_t::RETCODE_OK;
}

// Annotations application
bool DynamicTypeMember::annotation_is_optional() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_OPTIONAL_ID);
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool DynamicTypeMember::annotation_is_key() const
{
    return annotation_get_key();
}

bool DynamicTypeMember::annotation_get_key() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_KEY_ID);
    if (ann == nullptr)
    {
        ann = get_annotation(ANNOTATION_EPKEY_ID);
    }
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool DynamicTypeMember::annotation_is_must_understand() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_MUST_UNDERSTAND_ID);
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool DynamicTypeMember::annotation_is_non_serialized() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_NON_SERIALIZED_ID);
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool DynamicTypeMember::annotation_is_value() const
{
    return get_annotation(ANNOTATION_VALUE_ID) != nullptr;
}

bool DynamicTypeMember::annotation_is_default_literal() const
{
    return get_annotation(ANNOTATION_DEFAULT_LITERAL_ID) != nullptr;
}

bool DynamicTypeMember::annotation_is_position() const
{
    return get_annotation(ANNOTATION_OPTIONAL_ID) != nullptr;
}

// Annotations getters
std::string DynamicTypeMember::annotation_get_value() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_VALUE_ID);
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return value;
        }
    }
    return "";
}

std::string DynamicTypeMember::annotation_get_default() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_DEFAULT_ID);
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return value;
        }
    }
    return "";
}

uint16_t DynamicTypeMember::annotation_get_position() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_POSITION_ID);
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return static_cast<uint16_t>(std::stoi(value));
        }
    }
    return static_cast<uint16_t>(-1);
}

// Annotations setters
void DynamicTypeMember::annotation_set_optional(
        bool optional)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_OPTIONAL_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor ann;
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_OPTIONAL_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_OPTIONAL_ID);
    }
    ann->set_value("value", optional ? "true" : "false");
}

void DynamicTypeMember::annotation_set_key(
        bool key)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_KEY_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_KEY_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_KEY_ID);
    }
    ann->set_value("value", key ? "true" : "false");
}

void DynamicTypeMember::annotation_set_must_understand(
        bool must_understand)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_MUST_UNDERSTAND_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_MUST_UNDERSTAND_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_MUST_UNDERSTAND_ID);
    }
    ann->set_value("value", must_understand ? "true" : "false");
}

void DynamicTypeMember::annotation_set_non_serialized(
        bool non_serialized)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_NON_SERIALIZED_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_NON_SERIALIZED_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_NON_SERIALIZED_ID);
    }
    ann->set_value("value", non_serialized ? "true" : "false");
}

void DynamicTypeMember::annotation_set_value(
        const std::string& value)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_VALUE_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_VALUE_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_VALUE_ID);
    }
    ann->set_value("value", value);
}

void DynamicTypeMember::annotation_set_default_literal()
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_DEFAULT_LITERAL_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_DEFAULT_LITERAL_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_DEFAULT_LITERAL_ID);
    }
    ann->set_value("value", "true");
}

void DynamicTypeMember::annotation_set_position(
        uint16_t position)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_POSITION_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_POSITION_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_POSITION_ID);
    }
    ann->set_value("value", std::to_string(position));
}

void DynamicTypeMember::annotation_set_default(
        const std::string& default_value)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_DEFAULT_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_DEFAULT_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_DEFAULT_ID);
    }
    ann->set_value("value", default_value);
}

bool DynamicTypeMember::annotation_is_bit_bound() const
{
    return get_annotation(ANNOTATION_BIT_BOUND_ID) != nullptr;
}

uint16_t DynamicTypeMember::annotation_get_bit_bound() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_BIT_BOUND_ID);
    if (ann != nullptr)
    {
        std::string value;
        if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
        {
            return static_cast<uint16_t>(std::stoi(value));
        }
    }
    return 32; // Default value
}

void DynamicTypeMember::annotation_set_bit_bound(
        uint16_t bit_bound)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_BIT_BOUND_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_BIT_BOUND_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_BIT_BOUND_ID);
    }
    ann->set_value("value", std::to_string(bit_bound));
}

ReturnCode_t DynamicTypeMember::apply_annotation(
        AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->copy_from(&descriptor);
        annotation_.push_back(pNewDescriptor);
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicTypeMember::apply_annotation(
        const std::string& annotation_name,
        const std::string& key,
        const std::string& value)
{
    AnnotationDescriptor* ann = get_annotation(annotation_name);
    if (ann != nullptr)
    {
        ann->set_value(key, value);
    }
    else
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(annotation_name));
        pNewDescriptor->set_value(key, value);
        annotation_.push_back(pNewDescriptor);
    }
    return ReturnCode_t::RETCODE_OK;
}

AnnotationDescriptor* DynamicTypeMember::get_annotation(
        const std::string& name)
{
    auto it = annotation_.begin();

    for (; it != annotation_.end(); ++it)
    {
        AnnotationDescriptor* ann = *it;
        if (ann->type()->get_name().compare(name) == 0)
        {
            return ann;
        }
    }
    return nullptr;
}

MemberId DynamicTypeMember::get_annotation_count()
{
    static_assert(false);
}

ReturnCode_t DynamicTypeMember::get_annotation(
        AnnotationDescriptor& descriptor,
        MemberId idx)
{
    static_assert(false);
}
