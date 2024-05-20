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

#include "TypeDescriptorImpl.hpp"

#include <algorithm>
#include <cctype>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "DynamicTypeImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

template<>
traits<TypeDescriptor>::ref_type traits<TypeDescriptor>::make_shared()
{
    return std::make_shared<TypeDescriptorImpl>();
}

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

bool is_type_name_consistent(
        const ObjectName& sName)
{
    // Implement an FSM string parser to deal with both a plain type name
    // and a fully qualified name. According to the DDS xtypes standard,
    // type's fully qualified name is a concatenation of module names with
    // the name of a type inside of those modules.
    int currState = INVALID;
    for (uint32_t i = 0; i < sName.size(); ++i)
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

TypeDescriptorImpl::TypeDescriptorImpl(
        TypeKind kind,
        const ObjectName& name)
    : kind_(kind)
    , name_(name)
{
}

ReturnCode_t TypeDescriptorImpl::copy_from(
        traits<TypeDescriptor>::ref_type descriptor) noexcept
{
    if (!descriptor)
    {
        return RETCODE_BAD_PARAMETER;
    }

    return copy_from(*traits<TypeDescriptor>::narrow<TypeDescriptorImpl>(descriptor));
}

ReturnCode_t TypeDescriptorImpl::copy_from(
        const TypeDescriptorImpl& descriptor) noexcept
{
    kind_ = descriptor.kind_;
    name_ = descriptor.name_;
    base_type_ = descriptor.base_type_;
    discriminator_type_ = descriptor.discriminator_type_;
    bound_ = descriptor.bound_;
    element_type_ = descriptor.element_type_;
    key_element_type_ = descriptor.key_element_type_;
    extensibility_kind_ = descriptor.extensibility_kind_;
    is_nested_ = descriptor.is_nested_;
    is_extensibility_set_ = descriptor.is_extensibility_set_;

    return RETCODE_OK;
}

bool TypeDescriptorImpl::equals(
        traits<TypeDescriptor>::ref_type descriptor) noexcept
{
    return equals(*traits<TypeDescriptor>::narrow<TypeDescriptorImpl>(descriptor));
}

bool TypeDescriptorImpl::equals(
        TypeDescriptorImpl& descriptor) noexcept
{
    return kind_ == descriptor.kind_ &&
           name_ == descriptor.name_ &&
           ((!base_type_ && !descriptor.base_type_) || (base_type_ && base_type_->equals(descriptor.base_type_))) &&
           ((!discriminator_type_ && !descriptor.discriminator_type_) ||
           (discriminator_type_ && discriminator_type_->equals(descriptor.discriminator_type_))) &&
           bound_ == descriptor.bound_ &&
           ((!element_type_ && !descriptor.element_type_) ||
           (element_type_ && element_type_->equals(descriptor.element_type_))) &&
           ((!key_element_type_ && !descriptor.key_element_type_) ||
           (key_element_type_ && key_element_type_->equals(descriptor.key_element_type_))) &&
           extensibility_kind_ == descriptor.extensibility_kind_ &&
           is_nested_ == descriptor.is_nested_;
}

bool TypeDescriptorImpl::is_consistent() noexcept
{
    // Alias Types need the base type to indicate what type has been aliased.
    if (TK_ALIAS == kind_ && !base_type_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor describes an ALIAS but the base_type was not set");
        return false;
    }

    // Alias must have base type, and structures and bitsets optionally can have it.
    if (base_type_ &&
            TK_ALIAS != kind_ &&
            TK_BITSET != kind_ &&
            TK_STRUCTURE != kind_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Descriptor doesn't describe an ALIAS|BITSET|STRUCTURE but the base_type was set");
        return false;
    }

    // Check the parent is of the same kind.
    if (base_type_ && TK_ALIAS != kind_)
    {
        auto base_type =
                traits<DynamicType>::narrow<DynamicTypeImpl>(base_type_)->resolve_alias_enclosed_type();

        if (kind_ != base_type->get_kind())
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor type and the base_type are not of the same kind");
            return false;
        }

        // Check extensibility on structures.
        if (TK_STRUCTURE == kind_)
        {
            if (!is_extensibility_set_)
            {
                extensibility_kind_ = base_type->get_descriptor().extensibility_kind();
            }
            else if (extensibility_kind_ != base_type->get_descriptor().extensibility_kind())
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "ExtensibilityKind is different from that of base type.");
                return false;
            }
        }
    }

    if (0 < bound_.size() && (
                TK_ARRAY != kind_     &&
                TK_BITMASK != kind_   &&
                TK_BITSET != kind_    &&
                TK_MAP != kind_       &&
                TK_SEQUENCE != kind_  &&
                TK_STRING8 != kind_   &&
                TK_STRING16 != kind_  ))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor describes a type which not support bound");
        return false;
    }

    // Arrays need one or more bound fields with the lenghts of each dimension.
    if (TK_ARRAY == kind_ && 0 == bound_.size())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor describes an ARRAY but bound is empty");
        return false;
    }

    // These types need one bound with the length of the field.
    if (1 != bound_.size() && (
                TK_BITMASK == kind_   ||
                TK_MAP == kind_       ||
                TK_SEQUENCE == kind_  ||
                TK_STRING8 == kind_   ||
                TK_STRING16 == kind_  ))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Descriptor describes an SEQUENCE|MAP|BITMASK|STRING but bound doesn't contain only one element");
        return false;
    }

    // Check no bound is zero.
    if (bound_.end() != std::find_if(bound_.begin(), bound_.end(),
            [](uint32_t bound)
            {
                return 0 == bound;
            }))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Bounds contains a zero and it is not a valid value.");
        return false;
    }

    // Bitmask bound must be greater than zero and no greater than 64.
    if (TK_BITMASK == kind_ && (0 == bound_.at(0) || 64 < bound_.at(0)))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Descriptor describes an BITMASK but bound is not valid.");
        return false;
    }

    // Only union types need the discriminator of the union
    if (TK_UNION == kind_)
    {
        if (!discriminator_type_)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor describes an UNION but discriminant_type was not set");
            return false;
        }
        else
        {
            // Check discriminator kind and the type is a integer (labels are int32_t).
            // boolean, byte, char8, char16, int8, uint8, int16, uint16, int32, uint32, enum, alias
            TypeKind discriminator_kind =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(discriminator_type_)->resolve_alias_enclosed_type()
                            ->get_kind();

            switch (discriminator_kind)
            {
                case TK_BOOLEAN:
                case TK_BYTE:
                case TK_CHAR8:
                case TK_CHAR16:
                case TK_INT8:
                case TK_UINT8:
                case TK_INT16:
                case TK_UINT16:
                case TK_INT32:
                case TK_UINT32:
                case TK_INT64:
                case TK_UINT64:
                case TK_ENUM:
                    break;
                default:
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Discriminantor kind was not valid");
                    return false;
                    break;
            }
        }
    }
    else
    {
        if (discriminator_type_)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor doesn't describe an UNION but discriminant_type was set");
            return false;
        }
    }

    // ElementType is used by these types to set the "value" type of the element, otherwise it should be null.
    bool is_kind_with_element_type {
        TK_ARRAY == kind_    ||
        TK_BITMASK == kind_  ||
        TK_MAP == kind_      ||
        TK_SEQUENCE == kind_ ||
        TK_STRING8 == kind_  ||
        TK_STRING16 == kind_ };
    if (!element_type_ && is_kind_with_element_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Descriptor describes an ARRAY|SEQUENCE|MAP|BITMASK|STRING but element_type was not set");
        return false;
    }
    else if (element_type_ && !is_kind_with_element_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Descriptor not describes an ARRAY|SEQUENCE|MAP|BITMASK|STRING but element_type was set");
        return false;
    }
    else if (element_type_)
    {
        TypeKind element_kind =
                traits<DynamicType>::narrow<DynamicTypeImpl>(element_type_)->resolve_alias_enclosed_type()
                        ->get_kind();

        if (TK_STRING8 == kind_ && TK_CHAR8 != element_kind)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Elemend type of a TK_STRING8 must be TK_CHAR8.");
            return false;
        }
        else if (TK_STRING16 == kind_ && TK_CHAR16 != element_kind)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Elemend type of a TK_STRING16 must be TK_CHAR16.");
            return false;
        }
    }

    // For Bitmask types is mandatory that this element is boolean.
    if (TK_BITMASK == kind_ && TK_BOOLEAN != element_type_->get_kind())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor describes a BITMASK but element_type is not of BOOLEAN type");
        return false;
    }

    // Only map types need the keyElementType to store the "Key" type of the pair.
    if (TK_MAP == kind_ && !key_element_type_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor describes a MAP but key_element_type was not set");
        return false;
    }
    else if (TK_MAP != kind_ && key_element_type_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor doesn't describe a MAP but key_element_type was set");
        return false;
    }

    if (TK_MAP == kind_ && (TK_ANNOTATION == key_element_type()->get_kind() ||
            TK_ARRAY == key_element_type()->get_kind() ||
            TK_BITSET == key_element_type()->get_kind() ||
            TK_BITMASK == key_element_type()->get_kind() ||
            TK_MAP == key_element_type()->get_kind() ||
            TK_SEQUENCE == key_element_type()->get_kind() ||
            TK_STRING16 == key_element_type()->get_kind() ||
            TK_STRUCTURE == key_element_type()->get_kind() ||
            TK_UNION == key_element_type()->get_kind()
            ))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor describes a MAP with an invalid key_element_type");
        return false;
    }

    if (!is_type_name_consistent(name_))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor name is not a fully qualified name");
        return false;
    }

    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
