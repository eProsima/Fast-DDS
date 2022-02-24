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

#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/types/TypesBase.h>

#include <dds/core/LengthUnlimited.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {

enum FSM_INPUTS
{
    LETTER = 1,
    NUMBER,
    UNDERSCORE,
    COLON,
    OTHER
};

enum FSM_STATES
{
    INVALID = 0,
    SINGLECOLON,
    DOUBLECOLON,
    VALID
};

static const int stateTable[4][6] =
{
    /* Input:     letter,  number,  underscore, colon,       other */
    {INVALID,     VALID,   INVALID, INVALID,    INVALID,     INVALID},
    {SINGLECOLON, INVALID, INVALID, INVALID,    DOUBLECOLON, INVALID},
    {DOUBLECOLON, VALID,   INVALID, INVALID,    INVALID,     INVALID},
    {VALID,       VALID,   VALID,   VALID,      SINGLECOLON, INVALID}
};

TypeDescriptor::TypeDescriptor()
    : kind_(0)
    , name_("")
    , base_type_(nullptr)
    , discriminator_type_(nullptr)
    , element_type_(nullptr)
    , key_element_type_(nullptr)
{
}

TypeDescriptor::TypeDescriptor(
        const std::string& name,
        TypeKind kind)
    : kind_(kind)
    , name_(name)
    , base_type_(nullptr)
    , discriminator_type_(nullptr)
    , element_type_(nullptr)
    , key_element_type_(nullptr)
{
}

TypeDescriptor::TypeDescriptor(
        const TypeDescriptor* other)
    : kind_(0)
    , name_("")
    , base_type_(nullptr)
    , discriminator_type_(nullptr)
    , element_type_(nullptr)
    , key_element_type_(nullptr)
{
    copy_from(other);
}

TypeDescriptor::~TypeDescriptor()
{
    clean();
}

void TypeDescriptor::clean()
{
    for (auto it = annotation_.begin(); it != annotation_.end(); ++it)
    {
        delete *it;
    }
    annotation_.clear();

    base_type_ = nullptr;
    discriminator_type_ = nullptr;
    element_type_ = nullptr;
    key_element_type_ = nullptr;
}

ReturnCode_t TypeDescriptor::copy_from(
        const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        try
        {
            clean();

            for (auto it = descriptor->annotation_.begin(); it != descriptor->annotation_.end(); ++it)
            {
                AnnotationDescriptor* newDescriptor = new AnnotationDescriptor(*it);
                annotation_.push_back(newDescriptor);
            }

            kind_ = descriptor->kind_;
            name_ = descriptor->name_;
            base_type_ = descriptor->base_type_;
            discriminator_type_ = descriptor->discriminator_type_;
            bound_ = descriptor->bound_;
            element_type_ = descriptor->element_type_;
            key_element_type_ = descriptor->key_element_type_;
            return ReturnCode_t::RETCODE_OK;
        }
        catch (std::exception& /*e*/)
        {
            return ReturnCode_t::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error copying TypeDescriptor, invalid input descriptor");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

bool TypeDescriptor::equals(
        const TypeDescriptor* descriptor) const
{
    return descriptor != nullptr && name_ == descriptor->name_ && kind_ == descriptor->kind_ &&
           base_type_ == descriptor->base_type_ && discriminator_type_ == descriptor->discriminator_type_ &&
           bound_ == descriptor->bound_ && element_type_ == descriptor->element_type_ &&
           key_element_type_ == descriptor->key_element_type_;
}

DynamicType_ptr TypeDescriptor::get_base_type() const
{
    return base_type_;
}

uint32_t TypeDescriptor::get_bounds(
        uint32_t index /*=0*/) const
{
    if (index < bound_.size())
    {
        return bound_[index];
    }
    else
    {
        logError(DYN_TYPES, "Error getting bounds value. Index out of range.");
        return ::dds::core::LENGTH_UNLIMITED;
    }
}

uint32_t TypeDescriptor::get_bounds_size() const
{
    return static_cast<uint32_t>(bound_.size());
}

DynamicType_ptr TypeDescriptor::get_discriminator_type() const
{
    return discriminator_type_;
}

DynamicType_ptr TypeDescriptor::get_element_type() const
{
    return element_type_;
}

DynamicType_ptr TypeDescriptor::get_key_element_type() const
{
    return key_element_type_;
}

TypeKind TypeDescriptor::get_kind() const
{
    return kind_;
}

std::string TypeDescriptor::get_name() const
{
    return name_;
}

uint32_t TypeDescriptor::get_total_bounds() const
{
    if (bound_.size() >= 1)
    {
        uint32_t bounds = 1;
        for (uint32_t i = 0; i < bound_.size(); ++i)
        {
            bounds *= bound_[i];
        }
        return bounds;
    }
    return ::dds::core::LENGTH_UNLIMITED;
}

bool TypeDescriptor::is_consistent() const
{
    // Alias Types need the base type to indicate what type has been aliased.
    if (kind_ == TK_ALIAS && base_type_ == nullptr)
    {
        return false;
    }

    // Alias must have base type, and structures and bitsets optionally can have it.
    if (base_type_ != nullptr && kind_ != TK_ALIAS && kind_ != TK_STRUCTURE && kind_ != TK_BITSET)
    {
        return false;
    }

    // Arrays need one or more bound fields with the lenghts of each dimension.
    if (kind_ == TK_ARRAY && bound_.size() == 0)
    {
        return false;
    }

    // These types need one bound with the length of the field.
    if (bound_.size() != 1 && (kind_ == TK_SEQUENCE || kind_ == TK_MAP || kind_ == TK_BITMASK ||
            kind_ == TK_STRING8 || kind_ == TK_STRING16))
    {
        return false;
    }

    // Only union types need the discriminator of the union
    if ((discriminator_type_ == nullptr) == (kind_ == TK_UNION))
    {
        return false;
    }

    // ElementType is used by these types to set the "value" type of the element, otherwise it should be null.
    if ((element_type_ == nullptr) == (kind_ == TK_ARRAY || kind_ == TK_SEQUENCE || kind_ == TK_STRING8 ||
            kind_ == TK_STRING16 || kind_ == TK_MAP || kind_ == TK_BITMASK))
    {
        return false;
    }

    // For Bitmask types is mandatory that this element is boolean.
    if (kind_ == TK_BITMASK && (element_type_->get_kind() != TK_BOOLEAN))
    {
        return false;
    }

    // Only map types need the keyElementType to store the "Key" type of the pair.
    if ((key_element_type_ == nullptr) == (kind_ == TK_MAP))
    {
        return false;
    }

    if (!is_type_name_consistent(name_))
    {
        return false;
    }

    return true;
}

bool TypeDescriptor::is_type_name_consistent(
        const std::string& sName)
{
    // Implement an FSM string parser to deal with both a plain type name
    // and a fully qualified name. According to the DDS xtypes standard,
    // type's fully qualified name is a concatenation of module names with
    // the name of a type inside of those modules.
    int currState = INVALID;
    for (uint32_t i = 0; i < sName.length(); ++i)
    {
        int col = 0;
        if (std::isalpha(sName[i]))
        {
            col = LETTER;
        }
        else if (std::isdigit(sName[i]))
        {
            col = NUMBER;
        }
        else if (sName[i] == '_')
        {
            col = UNDERSCORE;
        }
        else if (sName[i] == ':')
        {
            col = COLON;
        }
        else
        {
            col = OTHER;
        }
        currState = stateTable[currState][col];
        if (currState == INVALID)
        {
            return false;
        }
    }
    return true;
}

void TypeDescriptor::set_kind(
        TypeKind kind)
{
    kind_ = kind;
}

void TypeDescriptor::set_name(
        std::string name)
{
    name_ = name;
}

ReturnCode_t TypeDescriptor::apply_annotation(
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
        logError(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t TypeDescriptor::apply_annotation(
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

AnnotationDescriptor* TypeDescriptor::get_annotation(
        const std::string& name) const
{
    auto it = annotation_.begin();

    for (; it != annotation_.end(); ++it)
    {
        AnnotationDescriptor* ann = *it;
        //if (ann->type()->get_name().compare(name) == 0)
        if (ann != nullptr
                && ann->type() != nullptr
                && ann->type()->kind_ > 0
                && !ann->type()->get_name().empty()
                && ann->type()->get_name().compare(name) == 0)
        {
            return ann;
        }
    }
    return nullptr;
}

// Annotations application
bool TypeDescriptor::annotation_is_extensibility() const
{
    return get_annotation(ANNOTATION_EXTENSIBILITY_ID) != nullptr;
}

bool TypeDescriptor::annotation_is_mutable() const
{
    if (get_annotation(ANNOTATION_MUTABLE_ID) != nullptr)
    {
        return true;
    }
    else
    {
        AnnotationDescriptor* ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
        if (ann != nullptr)
        {
            std::string value;
            if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
            {
                return value.compare(EXTENSIBILITY_MUTABLE) == 0;
            }
        }
    }
    return false;
}

bool TypeDescriptor::annotation_is_final() const
{
    if (get_annotation(ANNOTATION_FINAL_ID) != nullptr)
    {
        return true;
    }
    else
    {
        AnnotationDescriptor* ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
        if (ann != nullptr)
        {
            std::string value;
            if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
            {
                return value.compare(EXTENSIBILITY_FINAL) == 0;
            }
        }
    }
    return false;
}

bool TypeDescriptor::annotation_is_appendable() const
{
    if (get_annotation(ANNOTATION_APPENDABLE_ID) != nullptr)
    {
        return true;
    }
    else
    {
        AnnotationDescriptor* ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
        if (ann != nullptr)
        {
            std::string value;
            if (ann->get_value(value) == ReturnCode_t::RETCODE_OK)
            {
                return value.compare(EXTENSIBILITY_APPENDABLE) == 0;
            }
        }
    }
    return false;
}

bool TypeDescriptor::annotation_is_nested() const
{
    return get_annotation(ANNOTATION_NESTED_ID) != nullptr;
}

bool TypeDescriptor::annotation_is_bit_bound() const
{
    return get_annotation(ANNOTATION_BIT_BOUND_ID) != nullptr;
}

bool TypeDescriptor::annotation_is_key() const
{
    return get_annotation(ANNOTATION_KEY_ID) != nullptr || get_annotation(ANNOTATION_EPKEY_ID) != nullptr;
}

bool TypeDescriptor::annotation_is_non_serialized() const
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

// Annotation getters
std::string TypeDescriptor::annotation_get_extensibility() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
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

bool TypeDescriptor::annotation_get_nested() const
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_NESTED_ID);
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

bool TypeDescriptor::annotation_get_key() const
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

uint16_t TypeDescriptor::annotation_get_bit_bound() const
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

// Annotation setters
void TypeDescriptor::annotation_set_extensibility(
        const std::string& extensibility)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_EXTENSIBILITY_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
    }
    ann->set_value("value", extensibility);
}

void TypeDescriptor::annotation_set_mutable()
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_MUTABLE_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_MUTABLE_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_MUTABLE_ID);
    }
    ann->set_value("value", CONST_TRUE);
}

void TypeDescriptor::annotation_set_final()
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_FINAL_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_FINAL_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_FINAL_ID);
    }
    ann->set_value("value", CONST_TRUE);
}

void TypeDescriptor::annotation_set_appendable()
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_APPENDABLE_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_APPENDABLE_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_APPENDABLE_ID);
    }
    ann->set_value("value", CONST_TRUE);
}

void TypeDescriptor::annotation_set_nested(
        bool nested)
{
    AnnotationDescriptor* ann = get_annotation(ANNOTATION_NESTED_ID);
    if (ann == nullptr)
    {
        ann = new AnnotationDescriptor();
        ann->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_NESTED_ID));
        apply_annotation(*ann);
        delete ann;
        ann = get_annotation(ANNOTATION_NESTED_ID);
    }
    ann->set_value("value", nested ? CONST_TRUE : CONST_FALSE);
}

void TypeDescriptor::annotation_set_key(
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
    ann->set_value("value", key ? CONST_TRUE : CONST_FALSE);
}

void TypeDescriptor::annotation_set_bit_bound(
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

void TypeDescriptor::annotation_set_non_serialized(
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
    ann->set_value("value", non_serialized ? CONST_TRUE : CONST_FALSE);
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
