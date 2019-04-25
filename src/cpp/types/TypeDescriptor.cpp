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
#include <fastrtps/log/Log.h>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastrtps {
namespace types {

TypeDescriptor::TypeDescriptor()
    : kind_(0)
    , name_("")
    , base_type_(nullptr)
    , discriminator_type_(nullptr)
    , element_type_(nullptr)
    , key_element_type_(nullptr)
{
}

TypeDescriptor::TypeDescriptor(const std::string& name, TypeKind kind)
    : kind_(kind)
    , name_(name)
    , base_type_(nullptr)
    , discriminator_type_(nullptr)
    , element_type_(nullptr)
    , key_element_type_(nullptr)
{
}

TypeDescriptor::TypeDescriptor(const TypeDescriptor* other)
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
    base_type_ = nullptr;
    discriminator_type_ = nullptr;
    element_type_ = nullptr;
    key_element_type_ = nullptr;
}

ResponseCode TypeDescriptor::copy_from(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        try
        {
            clean();

            kind_ = descriptor->kind_;
            name_ = descriptor->name_;
            base_type_ = descriptor->base_type_;
            discriminator_type_ = descriptor->discriminator_type_;
            bound_ = descriptor->bound_;
            element_type_ = descriptor->element_type_;
            key_element_type_ = descriptor->key_element_type_;
            return ResponseCode::RETCODE_OK;
        }
        catch (std::exception& /*e*/)
        {
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error copying TypeDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool TypeDescriptor::equals(const TypeDescriptor* descriptor) const
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

uint32_t TypeDescriptor::get_bounds(uint32_t index /*=0*/) const
{
    if (index < bound_.size())
    {
        return bound_[index];
    }
    else
    {
        logError(DYN_TYPES, "Error getting bounds value. Index out of range.");
        return LENGTH_UNLIMITED;
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
    return LENGTH_UNLIMITED;
}

bool TypeDescriptor::is_consistent() const
{
    // Alias Types need the base type to indicate what type has been aliased.
    if (kind_ == TK_ALIAS && base_type_ == nullptr)
    {
        return false;
    }

    // Alias must have base type and structures optionally can have it.
    if (base_type_ != nullptr && kind_ != TK_ALIAS && kind_ != TK_STRUCTURE)
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

bool TypeDescriptor::is_type_name_consistent(const std::string& sName) const
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


void TypeDescriptor::set_kind(TypeKind kind)
{
    kind_ = kind;
}

void TypeDescriptor::set_name(std::string name)
{
    name_ = name;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
