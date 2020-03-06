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

#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {

MemberDescriptor::MemberDescriptor()
: name_("")
, id_(MEMBER_ID_INVALID)
, type_(nullptr)
, default_value_("")
, index_(INDEX_INVALID)
, default_label_(false)
{
}

MemberDescriptor::MemberDescriptor(
        uint32_t index,
        const std::string& name)
    : name_(name)
    , id_(MEMBER_ID_INVALID)
    , type_(nullptr)
    , default_value_("")
    , index_(index)
    , default_label_(false)
{
}

MemberDescriptor::MemberDescriptor(const MemberDescriptor* descriptor)
: name_("")
, id_(MEMBER_ID_INVALID)
, type_(nullptr)
, default_value_("")
, index_(INDEX_INVALID)
, default_label_(false)
{
    copy_from(descriptor);
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type_)
    : name_(name)
    , id_(id)
    , type_(type_)
    , default_value_("")
    , index_(INDEX_INVALID)
    , default_label_(false)
{

}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type_,
        const std::string& defaultValue)
    : name_(name)
    , id_(id)
    , type_(type_)
    , default_value_(defaultValue)
    , index_(INDEX_INVALID)
    , default_label_(false)
{
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type_,
        const std::string& defaultValue,
        const std::vector<uint64_t>& unionLabels,
        bool isDefaultLabel)
    : name_(name)
    , id_(id)
    , type_(type_)
    , default_value_(defaultValue)
    , index_(INDEX_INVALID)
    , default_label_(isDefaultLabel)
{
    labels_ = unionLabels;
}

MemberDescriptor::~MemberDescriptor()
{
    for (auto it = annotation_.begin(); it != annotation_.end(); ++it)
    {
        delete *it;
    }
    annotation_.clear();
    type_ = nullptr;
}

void MemberDescriptor::add_union_case_index(uint64_t value)
{
    labels_.push_back(value);
}

bool MemberDescriptor::check_union_labels(const std::vector<uint64_t>& labels) const
{
    for (auto it = labels.begin(); it != labels.end(); ++it)
    {
        if (std::find(labels_.begin(), labels_.end(), *it) != labels_.end())
        {
            return false;
        }
    }
    return true;
}

ReturnCode_t MemberDescriptor::copy_from(const MemberDescriptor* other)
{
    if (other != nullptr)
    {
        try
        {
            // Clear annotations
            for (auto it = annotation_.begin(); it != annotation_.end(); ++it)
            {
                delete *it;
            }
            annotation_.clear();

            // Copy them
            for (auto it = other->annotation_.begin(); it != other->annotation_.end(); ++it)
            {
                AnnotationDescriptor* newDescriptor = new AnnotationDescriptor(*it);
                annotation_.push_back(newDescriptor);
            }

            type_ = other->type_;
            name_ = other->name_;
            id_ = other->id_;
            default_value_ = other->default_value_;
            index_ = other->index_;
            default_label_ = other->default_label_;
            labels_ = other->labels_;
            return ReturnCode_t::RETCODE_OK;
        }
        catch (std::exception& /*e*/)
        {
            return ReturnCode_t::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error copying MemberDescriptor, invalid input descriptor");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

bool MemberDescriptor::equals(const MemberDescriptor* other) const
{
    if (other != nullptr && name_ == other->name_ && id_ == other->id_ &&
        ((type_ == nullptr && other->type_ == nullptr) || type_->equals(other->type_.get())) &&
        default_value_ == other->default_value_ && index_ == other->index_ && default_label_ == other->default_label_ &&
        labels_.size() == other->labels_.size())
    {
        for (auto it = labels_.begin(), it2 = other->labels_.begin(); it != labels_.end(); ++it, ++it2)
        {
            if (*it != *it2)
                return false;
        }
        return true;
    }
    return false;
}

MemberId MemberDescriptor::get_id() const
{
    return id_;
}

uint32_t MemberDescriptor::get_index() const
{
    return index_;
}

TypeKind MemberDescriptor::get_kind() const
{
    if (type_ != nullptr)
    {
        return type_->get_kind();
    }
    return 0;
}

std::string MemberDescriptor::get_name() const
{
    return name_;
}

std::vector<uint64_t> MemberDescriptor::get_union_labels() const
{
    return labels_;
}

bool MemberDescriptor::is_consistent(TypeKind parentKind) const
{
    // The type field is mandatory in every type except bitmasks and enums.
    if ((parentKind != TK_BITMASK && parentKind != TK_ENUM) && type_ == nullptr)
    {
        return false;
    }

    // Only aggregated types must use the ID value.
    if (id_ != MEMBER_ID_INVALID && parentKind != TK_UNION && parentKind != TK_STRUCTURE &&
        parentKind != TK_BITSET && parentKind != TK_ANNOTATION)
    {
        return false;
    }

    if (!is_default_value_consistent(default_value_))
    {
        return false;
    }

    if (type_ != nullptr && !is_type_name_consistent(type_->name_)) // Enums and bitmask don't have type
    {
        return false;
    }

    // Only Unions need the field "label"
    if (labels_.size() != 0 && parentKind != TK_UNION)
    {
        return false;
    }
    // If the field isn't the default value for the union, it must have a label value.
    else if (parentKind == TK_UNION && default_label_ == false && labels_.size() == 0)
    {
        return false;
    }

    return true;
}

bool MemberDescriptor::is_default_union_value() const
{
    return default_label_;
}

bool MemberDescriptor::is_default_value_consistent(const std::string& sDefaultValue) const
{
    if (sDefaultValue.length() > 0)
    {
        try
        {
            switch (get_kind())
            {
            default:
                return true;
            case TK_INT32:
            {
                int32_t value(0);
                value = stoi(sDefaultValue);
                (void)value;
            }
            break;
            case TK_UINT32:
            {
                uint32_t value(0);
                value = stoul(sDefaultValue);
                (void)value;
            }
            break;
            case TK_INT16:
            {
                int16_t value(0);
                value = static_cast<int16_t>(stoi(sDefaultValue));
                (void)value;
            }
            break;
            case TK_UINT16:
            {
                uint16_t value(0);
                value = static_cast<uint16_t>(stoul(sDefaultValue));
                (void)value;
            }
            break;
            case TK_INT64:
            {
                int64_t value(0);
                value = stoll(sDefaultValue);
                (void)value;
            }
            break;
            case TK_UINT64:
            {
                uint64_t value(0);
                value = stoul(sDefaultValue);
                (void)value;
            }
            break;
            case TK_FLOAT32:
            {
                float value(0.0f);
                value = stof(sDefaultValue);
                (void)value;
            }
            break;
            case TK_FLOAT64:
            {
                double value(0.0f);
                value = stod(sDefaultValue);
                (void)value;
            }
            break;
            case TK_FLOAT128:
            {
                long double value(0.0f);
                value = stold(sDefaultValue);
                (void)value;
            }
            break;
            case TK_CHAR8: {   return sDefaultValue.length() >= 1; }
            case TK_CHAR16:
            {
                std::wstring temp = std::wstring(sDefaultValue.begin(), sDefaultValue.end());
                (void)temp;
            }
            break;
            case TK_BOOLEAN:
            {
                if (sDefaultValue == CONST_TRUE || sDefaultValue == CONST_FALSE)
                {
                    return true;
                }
                int value(0);
                value = stoi(sDefaultValue);
                (void)value;
            }
            break;
            case TK_BYTE: {   return sDefaultValue.length() >= 1; }   break;
            case TK_STRING16: {   return true;    }
            case TK_STRING8: {   return true;    }
            case TK_ENUM:
            {
                uint32_t value(0);
                value = stoul(sDefaultValue);
                (void)value;
            }
            break;
            case TK_BITMASK:
            {
                int value(0);
                value = stoi(sDefaultValue);
                (void)value;
            }
            break;
            case TK_ARRAY: {   return true;    }
            case TK_SEQUENCE: {   return true;    }
            case TK_MAP: {   return true;    }
            }
        }
        catch (...)
        {
            return false;
        }
    }
    return true;
}

bool MemberDescriptor::is_type_name_consistent(const std::string& sName) const
{
    // The first letter must start with a letter ( uppercase or lowercase )
    if (sName.length() > 0 && std::isalpha(sName[0]))
    {
        // All characters must be letters, numbers or underscore.
        for (uint32_t i = 1; i < sName.length(); ++i)
        {
            if (!std::isalnum(sName[i]) && sName[i] != 95)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

void MemberDescriptor::set_id(MemberId id)
{
    id_ = id;
}

void MemberDescriptor::set_index(uint32_t index)
{
    index_ = index;
}

void MemberDescriptor::set_name(const std::string& name)
{
    name_ = name;
}

void MemberDescriptor::set_type(DynamicType_ptr type)
{
    type_ = type;
}

void MemberDescriptor::set_default_union_value(bool bDefault)
{
    default_label_ = bDefault;
}

// Annotations application
bool MemberDescriptor::annotation_is_optional() const
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

bool MemberDescriptor::annotation_is_key() const
{
    return annotation_get_key();
}

bool MemberDescriptor::annotation_get_key() const
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

bool MemberDescriptor::annotation_is_must_understand() const
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

bool MemberDescriptor::annotation_is_non_serialized() const
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

bool MemberDescriptor::annotation_is_value() const
{
    return get_annotation(ANNOTATION_VALUE_ID) != nullptr;
}

bool MemberDescriptor::annotation_is_default_literal() const
{
    return get_annotation(ANNOTATION_DEFAULT_LITERAL_ID) != nullptr;
}

bool MemberDescriptor::annotation_is_position() const
{
    return get_annotation(ANNOTATION_OPTIONAL_ID) != nullptr;
}

// Annotations getters
std::string MemberDescriptor::annotation_get_value() const
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

std::string MemberDescriptor::annotation_get_default() const
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

uint16_t MemberDescriptor::annotation_get_position() const
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
void MemberDescriptor::annotation_set_optional(bool optional)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_OPTIONAL_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_OPTIONAL_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_OPTIONAL_ID);
    }
    ann->set_value("value", optional ? "true" : "false");
}

void MemberDescriptor::annotation_set_key(bool key)
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

void MemberDescriptor::annotation_set_must_understand(bool must_understand)
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

void MemberDescriptor::annotation_set_non_serialized(bool non_serialized)
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

void MemberDescriptor::annotation_set_value(const std::string& value)
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

void MemberDescriptor::annotation_set_default_literal()
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

void MemberDescriptor::annotation_set_position(uint16_t position)
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

void MemberDescriptor::annotation_set_default(const std::string& default_value)
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

bool MemberDescriptor::annotation_is_bit_bound() const
{
    return get_annotation(ANNOTATION_BIT_BOUND_ID) != nullptr;
}

uint16_t MemberDescriptor::annotation_get_bit_bound() const
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

void MemberDescriptor::annotation_set_bit_bound(uint16_t bit_bound)
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

ReturnCode_t MemberDescriptor::apply_annotation(AnnotationDescriptor& descriptor)
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
        logError(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t MemberDescriptor::apply_annotation(
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

AnnotationDescriptor* MemberDescriptor::get_annotation(const std::string& name) const
{
    auto it = annotation_.begin();

    for(; it != annotation_.end(); ++it)
    {
        AnnotationDescriptor* ann = *it;
        if (ann->type()->get_name().compare(name) == 0)
        {
            return ann;
        }
    }
    return nullptr;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
