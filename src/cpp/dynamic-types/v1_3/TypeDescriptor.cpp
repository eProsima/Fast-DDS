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

#include <fastrtps/types/v1_3/AnnotationDescriptor.h>
#include <fastrtps/types/v1_3/DynamicType.h>
#include <fastrtps/types/v1_3/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/v1_3/TypeDescriptor.h>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/log/Log.hpp>

#include <iomanip>
#include <algorithm>

using namespace eprosima::fastrtps::types::v1_3;

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

#ifdef __cpp_aggregate_nsdm

TypeDescriptor::TypeDescriptor(
        const std::string& name,
        TypeKind kind)
    : TypeDescriptorData{name, kind}
{
}

#else // __cpp_aggregate_nsdm

TypeDescriptor::TypeDescriptor(
        const std::string& name,
        TypeKind kind)
{
    name_ = name;
    kind_ = kind;
}

#endif // __cpp_aggregate_nsdm


TypeDescriptor::TypeDescriptor(
        const TypeDescriptor& other)
    : TypeDescriptorData(other)
{
    refresh_indexes();
}

TypeDescriptor& TypeDescriptor::operator =(
        const TypeDescriptor& descriptor) noexcept
{
    TypeDescriptorData::operator =(descriptor);
    refresh_indexes();
    return *this;
}

TypeDescriptor::~TypeDescriptor() noexcept
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

const DynamicType& TypeDescriptor::resolve_alias_type(
        const DynamicType& type)
{
    const DynamicType* a = &type;

    while ( a->get_kind() == TypeKind::TK_ALIAS )
    {
        assert(a->base_type_);
        a = a->base_type_.get();
    }

    return *a;
}

void TypeDescriptor::clean()
{
    AnnotationManager::clean();

    base_type_.reset();
    discriminator_type_.reset();
    element_type_.reset();
    key_element_type_.reset();

    members_.clear();
    member_by_id_.clear();
    member_by_name_.clear();
}

ReturnCode_t TypeDescriptor::copy_from(
        const TypeDescriptor& descriptor) noexcept
{
    *this = descriptor;
    return ReturnCode_t::RETCODE_OK;
}

bool TypeDescriptor::operator ==(
        const TypeDescriptor& descriptor) const
{
    // Resolve alias dependencies
    const TypeDescriptor* a = this;
    const TypeDescriptor* b = &descriptor;

    while ( a->kind_ == TypeKind::TK_ALIAS )
    {
        assert(a->base_type_);
        a = a->base_type_.get();
    }

    while ( b->kind_ == TypeKind::TK_ALIAS )
    {
        assert(b->base_type_);
        b = b->base_type_.get();
    }

    return a->name_ == b->name_ &&
           a->kind_ == b->kind_ &&
           a->bound_ == b->bound_ &&
           a->AnnotationManager::operator ==(*b) &&
           a->members_ == b->members_ &&
           (a->base_type_ == b->base_type_ || (
               a->base_type_ &&
               b->base_type_ &&
               *a->base_type_ == *b->base_type_)) &&
           (a->discriminator_type_ == b->discriminator_type_ || (
               a->discriminator_type_ &&
               b->discriminator_type_ &&
               *a->discriminator_type_ == *b->discriminator_type_)) &&
           (a->element_type_ == b->element_type_ || (
               a->element_type_ &&
               b->element_type_ &&
               *a->element_type_ == *b->element_type_)) &&
           (a->key_element_type_ == b->key_element_type_ || (
               a->key_element_type_ &&
               b->key_element_type_ &&
               *a->key_element_type_ == *b->key_element_type_));
}

bool TypeDescriptor::operator !=(
        const TypeDescriptor& descriptor) const
{
    return !operator ==(descriptor);
}

bool TypeDescriptor::equals(
        const TypeDescriptor& descriptor) const noexcept
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

TypeKind TypeDescriptor::get_kind() const noexcept
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

bool TypeDescriptor::is_consistent(
        bool type /* = false*/) const
{
    // Enums should have at least one member
    if (type && kind_ == TypeKind::TK_ENUM && members_.empty())
    {
        return false;
    }

    // Alias Types need the base type to indicate what type has been aliased.
    if (kind_ == TypeKind::TK_ALIAS && !base_type_)
    {
        return false;
    }

    // Alias must have base type, and structures and bitsets optionally can have it.
    if (base_type_ && kind_ != TypeKind::TK_ALIAS && kind_ != TypeKind::TK_STRUCTURE && kind_ != TypeKind::TK_BITSET)
    {
        return false;
    }

    // Arrays need one or more bound fields with the lenghts of each dimension.
    if (kind_ == TypeKind::TK_ARRAY && bound_.size() == 0)
    {
        return false;
    }

    // These types need one bound with the length of the field.
    if (bound_.size() != 1 &&
            (kind_ == TypeKind::TK_SEQUENCE || kind_ == TypeKind::TK_MAP || kind_ == TypeKind::TK_BITMASK ||
            kind_ == TypeKind::TK_STRING8 || kind_ == TypeKind::TK_STRING16))
    {
        return false;
    }

    // Only union types need the discriminator of the union
    if (!discriminator_type_ && kind_ == TypeKind::TK_UNION)
    {
        return false;
    }

    // ElementType is used by these types to set the "value" type of the element, otherwise it should be null.
    if (!element_type_ &&
            (kind_ == TypeKind::TK_ARRAY || kind_ == TypeKind::TK_SEQUENCE || kind_ == TypeKind::TK_STRING8 ||
            kind_ == TypeKind::TK_STRING16 || kind_ == TypeKind::TK_MAP || kind_ == TypeKind::TK_BITMASK))
    {
        return false;
    }

    // For Bitmask types is mandatory that this element is boolean.
    if (kind_ == TypeKind::TK_BITMASK && (element_type_->get_kind() != TypeKind::TK_BOOLEAN))
    {
        return false;
    }

    // Only map types need the keyElementType to store the "Key" type of the pair.
    if (!key_element_type_ && kind_ == TypeKind::TK_MAP)
    {
        return false;
    }

    if (!is_type_name_consistent(name_))
    {
        return false;
    }

    // Check members if any
    if (type && std::any_of(members_.begin(), members_.end(),
            [this](const DynamicTypeMember& m)
            {
                return !m.is_consistent(kind_);
            }))
    {
        // inconsistencies in the use of annotations
        return false;
    }

    return true;
}

bool TypeDescriptor::is_primitive() const
{
    return kind_ > TypeKind::TK_NONE && kind_ <= TypeKind::TK_CHAR16 &&
           0u == get_annotation_count() &&
           members_.empty() &&
           !base_type_ && !discriminator_type_ &&
           !element_type_ && !key_element_type_;
}

bool TypeDescriptor::is_subclass(
        const TypeDescriptor& descriptor) const
{
    return descriptor.kind_ == TypeKind::TK_STRUCTURE &&
           kind_ == TypeKind::TK_STRUCTURE &&
           base_type_ && (
        *base_type_ == descriptor ||
        base_type_->is_subclass(descriptor));
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

const std::list<const DynamicTypeMember*> TypeDescriptor::get_all_members() const
{
    std::list<const DynamicTypeMember*> res;

    // retrieve members from the base classes
    if (base_type_)
    {
        res = resolve_alias_type(*base_type_).get_all_members();
    }

    // populate the local members
    std::transform(
        members_.begin(),
        members_.end(),
        std::back_inserter(res),
        [](const DynamicTypeMember& m)
        {
            return &m;
        });

    return res;
}

std::map<std::string, const DynamicTypeMember*> TypeDescriptor::get_all_members_by_name() const
{
    std::map<std::string, const DynamicTypeMember*> res;

    // retrieve members from the base classes
    if (base_type_)
    {
        res = resolve_alias_type(*base_type_).get_all_members_by_name();
    }

    // populate the local members
    res.insert(member_by_name_.begin(), member_by_name_.end());

    return res;
}

std::map<MemberId, const DynamicTypeMember*> TypeDescriptor::get_all_members_by_id() const
{
    std::map<MemberId, const DynamicTypeMember*> res;

    // retrieve members from the base classes
    if (base_type_)
    {
        res = resolve_alias_type(*base_type_).get_all_members_by_id();
    }

    // populate the local members
    res.insert(member_by_id_.begin(), member_by_id_.end());

    return res;
}

ReturnCode_t TypeDescriptor::get_member_by_index(
        MemberDescriptor& member,
        uint32_t index) const noexcept
{
    uint32_t offset = 0;

    // try the base type
    if (base_type_)
    {
        ReturnCode_t res = resolve_alias_type(*base_type_).get_member_by_index(member, index);
        if (!res)
        {
            // correct the offset
            offset = base_type_->get_member_count();
        }
        else
        {
            // we are done
            return res;
        }
    }

    if (index >= (offset + members_.size()))
    {
        EPROSIMA_LOG_WARNING(DYN_TYPES, "Error getting member by index, member not found.");
        return ReturnCode_t::RETCODE_ERROR;
    }

    auto it = members_.begin();
    std::advance(it, index - offset);
    member = *it;

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t TypeDescriptor::get_member_by_name(
        MemberDescriptor& member,
        const std::string& name) const noexcept
{
    // try the base type
    if (base_type_)
    {
        ReturnCode_t res = resolve_alias_type(*base_type_).get_member_by_name(member, name);
        if (!!res)
        {
            return res;
        }
    }

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
    else if (base_type_)
    {
        return resolve_alias_type(*base_type_).get_member(id);
    }
    else
    {
        return {nullptr, false};
    }
}

ReturnCode_t TypeDescriptor::get_member(
        MemberDescriptor& member,
        MemberId id) const noexcept
{
    const DynamicTypeMember* pM;
    bool found;

    std::tie(pM, found) = get_member(id);
    if (found)
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

uint32_t TypeDescriptor::get_member_count() const
{
    std::size_t res{members_.size()};

    if (base_type_)
    {
        res += resolve_alias_type(*base_type_).get_member_count();
    }

    return static_cast<uint32_t>(res);
}

MemberId TypeDescriptor::get_member_id_by_name(
        const std::string& name) const
{
    auto it = member_by_name_.find(name);

    if (it != member_by_name_.end())
    {
        return it->second->get_id();
    }
    else if (base_type_)
    {
        return resolve_alias_type(*base_type_).get_member_id_by_name(name);
    }

    return MEMBER_ID_INVALID;
}

MemberId TypeDescriptor::get_member_id_at_index(
        uint32_t index) const
{
    MemberId res = MEMBER_ID_INVALID;
    uint32_t offset = 0;

    // try the base type
    if (base_type_)
    {
        res = resolve_alias_type(*base_type_).get_member_id_at_index(index);
        if (res == MEMBER_ID_INVALID)
        {
            // correct the offset
            offset = base_type_->get_member_count();
        }
        else
        {
            // we are done
            return res;
        }
    }

    if (index >= (offset + members_.size()))
    {
        // index out of boundaries
        return MEMBER_ID_INVALID;
    }

    // retrieve from local collection
    auto it = members_.begin();
    std::advance(it, index - offset);
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

MemberId TypeDescriptor::get_id_from_label(
        uint64_t label) const
{
    // Check is a union
    if (TypeKind::TK_UNION != kind_)
    {
        return MEMBER_ID_INVALID;
    }

    // check the base class if any
    if (base_type_)
    {
        MemberId id = base_type_->get_id_from_label(label);
        if (MEMBER_ID_INVALID != id)
        {
            return id;
        }
    }

    // Check the members
    for (auto& m : members_)
    {
        auto& lbs = m.get_union_labels();
        auto it = lbs.find(label);
        if (it != lbs.end())
        {
            return m.get_id();
        }
    }

    return MEMBER_ID_INVALID;
}

std::ostream& eprosima::fastrtps::types::v1_3::operator <<(
        std::ostream& os,
        const TypeDescriptor& td)
{
    using namespace std;

    // indentation increment
    ++os.iword(DynamicTypeBuilderFactory::indentation_index);

    auto manips = [](ostream& os) -> ostream&
            {
                long indent = os.iword(DynamicTypeBuilderFactory::indentation_index);
                return os << string(indent, '\t') << setw(10) << left;
            };

    // TODO: Barro, add support for bounds & annotations

    os << endl
       << manips << "name:" << td.get_name() << endl
       << manips << "kind:" << td.get_kind() << endl
       << manips << "bounds:" << td.get_bounds_size() << endl;

    // Show base type
    auto bt = td.get_base_type();
    if (bt)
    {
        os << manips << "base type: ";
        os << *bt << endl;
    }

    // Show element type
    auto et = td.get_element_type();
    if (et)
    {
        os << manips << "element type: ";
        os << *et << endl;
    }

    // Show key type type
    auto kt = td.get_key_element_type();
    if (kt)
    {
        os << manips << "key element type: ";
        os << *kt << endl;
    }

    // Show discriminator type
    auto dt = td.get_discriminator_type();
    if (dt)
    {
        os << manips << "discriminator type: ";
        os << *dt << endl;
    }

    // Show annotations
    if (td.get_annotation_count())
    {
        os << manips << "annotations:" << endl;
        for (const AnnotationDescriptor& d : td.get_all_annotations())
        {
            os << d;
        }
    }

    // Show members
    if (td.get_member_count())
    {
        // notify the members which object they belong to
        os.pword(DynamicTypeBuilderFactory::object_index) = (void*)&td;

        os << manips << "members:";
        for (const DynamicTypeMember* m : td.get_all_members())
        {
            os << *m;
        }

        os.pword(DynamicTypeBuilderFactory::object_index) = nullptr;
    }

    // indentation decrement
    --os.iword(DynamicTypeBuilderFactory::indentation_index);

    return os;
}
