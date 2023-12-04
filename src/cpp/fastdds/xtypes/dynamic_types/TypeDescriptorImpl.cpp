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

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

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

ReturnCode_t TypeDescriptorImpl::copy_from(
        traits<TypeDescriptor>::ref_type descriptor) noexcept
{
    if (!descriptor)
    {
        return RETCODE_BAD_PARAMETER;
    }

    kind_ = descriptor->kind();
    name_ = descriptor->name();
    base_type_ = descriptor->base_type();
    discriminator_type_ = descriptor->discriminator_type();
    bound_ = descriptor->bound();
    element_type_ = descriptor->element_type();
    key_element_type_ = descriptor->key_element_type();
    extensibility_kind_ = descriptor->extensibility_kind();
    is_nested_ = descriptor->is_nested();

    return RETCODE_OK;
}

bool TypeDescriptorImpl::equals(
        traits<TypeDescriptor>::ref_type descriptor) noexcept
{
    return kind_ == descriptor->kind() &&
           name_ == descriptor->name() &&
           base_type_ == descriptor->base_type() && // TODO(richiware) change when dynamictype has equals.
           discriminator_type_ == descriptor->discriminator_type() &&
           bound_ == descriptor->bound() &&
           element_type_ == descriptor->element_type() &&
           key_element_type_ == descriptor->key_element_type() &&
           extensibility_kind_ == descriptor->extensibility_kind() &&
           is_nested_ == descriptor->is_nested();
}

bool TypeDescriptorImpl::is_consistent() noexcept
{
    //TODO(richiware) when dynamic_type has is_consisten.
    // Alias Types need the base type to indicate what type has been aliased.
    if (TK_ALIAS == kind_ && !base_type_)
    {
        return false;
    }

    // Alias must have base type, and structures and bitsets optionally can have it.
    if (base_type_ && TK_ALIAS != kind_ && TK_STRUCTURE != kind_ && TK_BITSET != kind_)
    {
        return false;
    }

    // Arrays need one or more bound fields with the lenghts of each dimension.
    if (TK_ARRAY == kind_ && 0 == bound_.size())
    {
        return false;
    }

    // These types need one bound with the length of the field.
    if (1 != bound_.size() && (
                TK_SEQUENCE == kind_  ||
                TK_MAP == kind_       ||
                TK_BITMASK == kind_   ||
                TK_STRING8 == kind_   ||
                TK_STRING16 == kind_))
    {
        return false;
    }

    // Only union types need the discriminator of the union
    if (TK_UNION == kind_ && !discriminator_type_)
    {
        return false;
    }

    // ElementType is used by these types to set the "value" type of the element, otherwise it should be null.
    if (!element_type_ && (
                TK_ARRAY == kind_    ||
                TK_SEQUENCE == kind_ ||
                TK_STRING8 == kind_  ||
                TK_STRING16 == kind_ ||
                TK_MAP == kind_      ||
                TK_BITMASK == kind_))
    {
        return false;
    }

    // For Bitmask types is mandatory that this element is boolean.
    if (TK_BITMASK == kind_ && TK_BOOLEAN != element_type_->get_kind())
    {
        return false;
    }

    // Only map types need the keyElementType to store the "Key" type of the pair.
    if (TK_MAP == kind_ && !key_element_type_)
    {
        return false;
    }

    if (!is_type_name_consistent(name_))
    {
        return false;
    }

    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
