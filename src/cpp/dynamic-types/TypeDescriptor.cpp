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

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/TypesBase.h>

using namespace eprosima::fastrtps::types;

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

TypeDescriptor::TypeDescriptor(
        const std::string& name,
        TypeKind kind)
    : TypeDescriptorData{name, kind}
{
}

TypeDescriptor::TypeDescriptor(
        const TypeDescriptor& other)
        : TypeDescriptorData(other)
{
    refresh_indexes();
}

TypeDescriptor& TypeDescriptor::operator=(
        const TypeDescriptor& descriptor)
{
    TypeDescriptorData::operator=(descriptor);
    refresh_indexes();
    return *this;
}

TypeDescriptor::~TypeDescriptor()
{
    clean();
}

void TypeDescriptor::refresh_indexes()
{
    member_by_id_.clear();
    member_by_name_.clear();

    // update indexes with member info
    for (DynamicTypeMember& m : members_)
    {
        member_by_id_[m.get_id()] = &m;
        member_by_name_[m.get_name()] = &m;
    }
}

void TypeDescriptor::clean()
{
    annotation_.clear();

    base_type_.reset();
    discriminator_type_.reset();
    element_type_.reset();
    key_element_type_.reset();

    members_.clear();
    member_by_id_.clear();
    member_by_name_.clear();
}

ReturnCode_t TypeDescriptor::copy_from(
        const TypeDescriptor& descriptor)
{
    *this = descriptor;
    return ReturnCode_t::RETCODE_OK;
}

bool TypeDescriptor::operator==(const TypeDescriptor& descriptor) const
{
    return name_ == descriptor.name_ &&
           kind_ == descriptor.kind_ &&
           base_type_ == descriptor.base_type_ &&
           discriminator_type_ == descriptor.discriminator_type_ &&
           bound_ == descriptor.bound_ &&
           element_type_ == descriptor.element_type_ &&
           key_element_type_ == descriptor.key_element_type_ &&
           annotation_ == descriptor.annotation_ &&
           members_ == descriptor.members_;
}

bool TypeDescriptor::equals(
        const TypeDescriptor& descriptor) const
{
    return *this == descriptor;
}

void TypeDescriptor::set_base_type(
        const DynamicType_ptr& type)
{
    base_type_ = type;
}

void TypeDescriptor::set_base_type(
        DynamicType_ptr&& type)
{
    base_type_ = std::move(type);
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
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error getting bounds value. Index out of range.");
        return BOUND_UNLIMITED;
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
    return BOUND_UNLIMITED;
}

bool TypeDescriptor::is_consistent() const
{
    // Alias Types need the base type to indicate what type has been aliased.
    if (kind_ == TK_ALIAS && !base_type_)
    {
        return false;
    }

    // Alias must have base type, and structures and bitsets optionally can have it.
    if (base_type_ && kind_ != TK_ALIAS && kind_ != TK_STRUCTURE && kind_ != TK_BITSET)
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
    if (!discriminator_type_ && kind_ == TK_UNION)
    {
        return false;
    }

    // ElementType is used by these types to set the "value" type of the element, otherwise it should be null.
    if (!element_type_ && (kind_ == TK_ARRAY || kind_ == TK_SEQUENCE || kind_ == TK_STRING8 ||
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
    if (!key_element_type_ && kind_ == TK_MAP)
    {
        return false;
    }

    if (!is_type_name_consistent(name_))
    {
        return false;
    }

    return true;
}

bool TypeDescriptor::is_primitive() const
{
    return kind_ > TK_NONE && kind_ <= TK_CHAR16;
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
        const std::string& name)
{
    name_ = name;
}

void TypeDescriptor::set_name(
        std::string&& name)
{
    name_ = std::move(name);
}

ReturnCode_t TypeDescriptor::get_all_members_by_name(
        std::map<std::string, const DynamicTypeMember*>& members) const
{
    members = std::map<std::string, const DynamicTypeMember*>(member_by_name_.begin(), member_by_name_.end());
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t TypeDescriptor::get_all_members(
        std::map<MemberId, const DynamicTypeMember*>& members) const
{
    members = std::map<MemberId, const DynamicTypeMember*>(member_by_id_.begin(), member_by_id_.end());
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t TypeDescriptor::get_member_by_name(
        MemberDescriptor& member,
        const std::string& name) const
{
    auto it = member_by_name_.find(name);
    if (it != member_by_name_.end())
    {
        member = it->second->get_descriptor();
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_WARNING(DYN_TYPES, "Error getting member by name, member not found.");
        return ReturnCode_t::RETCODE_ERROR;
    }
}

std::pair<const DynamicTypeMember*, bool>
TypeDescriptor::get_member(
        MemberId id) const
{
    auto it = member_by_id_.find(id);
    if (it != member_by_id_.end())
    {
        return {it->second, true};
    }
    else
    {
        return {nullptr, false};
    }
}

ReturnCode_t TypeDescriptor::get_member(
        MemberDescriptor& member,
        MemberId id) const
{
    const DynamicTypeMember* pM;
    bool found;

    std::tie(pM, found) = get_member(id);
    if(found)
    {
        member = pM->get_descriptor();
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_WARNING(DYN_TYPES, "Error getting member, member not found.");
        return ReturnCode_t::RETCODE_ERROR;
    }
}

uint32_t TypeDescriptor::get_members_count() const
{
    return static_cast<uint32_t>(members_.size());
}

MemberId TypeDescriptor::get_member_id_by_name(
        const std::string& name) const
{
    auto it = member_by_name_.find(name);

    if(it != member_by_name_.end())
    {
        return it->second->get_id();
    }

    return MEMBER_ID_INVALID;
}

MemberId TypeDescriptor::get_member_id_at_index(
        uint32_t index) const
{
    if(index >= members_.size())
    {
        return MEMBER_ID_INVALID;
    }

    auto it = members_.begin();
    std::advance(it, index);
    assert(it->get_index() == index);
    return it->get_id();
}

bool TypeDescriptor::exists_member_by_name(
        const std::string& name) const
{
    auto base = get_base_type();
    if (base)
    {
        if (base->exists_member_by_name(name))
        {
            return true;
        }
    }
    return member_by_name_.find(name) != member_by_name_.end();
}

bool TypeDescriptor::exists_member_by_id(
        MemberId id) const
{
    auto base = get_base_type();
    if (base)
    {
        if (base->exists_member_by_id(id))
        {
            return true;
        }
    }
    return member_by_id_.find(id) != member_by_id_.end();
}

