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

#include <algorithm>
#include <initializer_list>
#include <iomanip>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>

#include <fastrtps/types/TypesBase.h>

#include "AnnotationDescriptorImpl.hpp"
#include "DynamicTypeBuilderFactoryImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include "TypeState.hpp"

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

#if defined(_MSC_VER) || defined(__cpp_aggregate_nsdmi)

TypeState::TypeState(
        const std::string& name,
        TypeKind kind)
    : TypeStateData{name, kind}
{
}

#else // __cpp_aggregate_nsdmi

TypeState::TypeState(
        const std::string& name,
        TypeKind kind)
{
    name_ = name;
    kind_ = kind;
}

#endif // __cpp_aggregate_nsdmi

TypeState::TypeState(
        const TypeState& other)
    : TypeStateData(other)
    , AnnotationManager(other)
{
    refresh_indexes();
}

TypeState::TypeState(
        const TypeDescriptor& descriptor)
{
    name_ = descriptor.name();

    kind_ = descriptor.kind();

    /*TODO(richiware)
       const DynamicType* type = descriptor.base_type();

       if (type != nullptr)
       {
        base_type_ = DynamicTypeImpl::get_implementation(*type).shared_from_this();
       }

       type = descriptor.get_discriminator_type();

       if (type != nullptr)
       {
        discriminator_type_ = DynamicTypeImpl::get_implementation(*type).shared_from_this();
       }

       type = descriptor.get_element_type();

       if (type != nullptr)
       {
        element_type_ = DynamicTypeImpl::get_implementation(*type).shared_from_this();
       }

       type = descriptor.get_key_element_type();
       if (type != nullptr)
       {
        key_element_type_ = DynamicTypeImpl::get_implementation(*type).shared_from_this();
       }

       uint32_t dims;
       const uint32_t* lenghts = descriptor.get_bounds(dims);
       bound_.assign(lenghts, lenghts + dims);
     */
}

/*TODO(richiware)
   TypeDescriptor TypeState::get_descriptor() const noexcept
   {
   TypeDescriptorImpl res;

   if (!name_.empty())
   {
    res.set_name(name_.c_str());
   }

   res.set_kind(kind_);

   if (base_type_)
   {
    res.set_base_type(&base_type_->get_interface());
   }

   if (discriminator_type_)
   {
    res.set_discriminator_type(&discriminator_type_->get_interface());
   }

   if (element_type_)
   {
    res.set_element_type(&element_type_->get_interface());
   }

   if (key_element_type_)
   {
    res.set_key_element_type(&key_element_type_->get_interface());
   }

   res.set_bounds(
    bound_.data(),
    static_cast<uint32_t>(bound_.size()));

   return res;
   }
 */

TypeState& TypeState::operator =(
        const TypeState& state) noexcept
{
    TypeStateData::operator =(state);
    refresh_indexes();
    return *this;
}

TypeState::~TypeState() noexcept
{
    clean();
}

void TypeState::refresh_indexes()
{
    member_by_id_.clear();
    member_by_name_.clear();

    // update indexes with member info
    /*TODO(richiware)
       for (DynamicTypeMemberImpl& m : members_)
       {
       member_by_id_[m.id()] = &m;
       member_by_name_[m.name().c_str()] = &m;
       }
     */
}

const DynamicTypeImpl& TypeState::resolve_alias_type(
        const DynamicTypeImpl& type)
{
    const DynamicTypeImpl* a = &type;

    while ( a->get_kind() == TK_ALIAS )
    {
        assert(a->base_type_);
        a = a->base_type_.get();
    }

    return *a;
}

void TypeState::clean()
{
    AnnotationManager::clean();

    base_type_.reset();
    discriminator_type_.reset();
    element_type_.reset();
    key_element_type_.reset();

    //TODO(richiware)members_.clear();
    member_by_id_.clear();
    member_by_name_.clear();
}

ReturnCode_t TypeState::copy_from(
        const TypeState& state) noexcept
{
    *this = state;
    return RETCODE_OK;
}

bool TypeState::operator ==(
        const TypeState& state) const
{
    // Resolve alias dependencies
    const TypeState* a = this;
    const TypeState* b = &state;

    if ( a == b )
    {
        return true;
    }

    while ( a->kind_ == TK_ALIAS )
    {
        assert(a->base_type_);
        a = a->base_type_.get();
    }

    while ( b->kind_ == TK_ALIAS )
    {
        assert(b->base_type_);
        b = b->base_type_.get();
    }

    return a->name_ == b->name_ &&
           a->kind_ == b->kind_ &&
           a->bound_ == b->bound_ &&
           a->AnnotationManager::operator ==(*b) &&
           //TODO(richiware)a->members_ == b->members_ &&
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

bool TypeState::operator !=(
        const TypeState& state) const
{
    return !operator ==(state);
}

bool TypeState::equals(
        const TypeState& state) const noexcept
{
    return *this == state;
}

void TypeState::set_base_type(
        const std::shared_ptr<const DynamicTypeImpl>& type)
{
    base_type_ = type;
}

void TypeState::set_base_type(
        std::shared_ptr<const DynamicTypeImpl>&& type)
{
    base_type_ = std::move(type);
}

std::shared_ptr<const DynamicTypeImpl> TypeState::get_base_type() const
{
    return base_type_;
}

uint32_t TypeState::get_bounds(
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

uint32_t TypeState::get_bounds_size() const
{
    return static_cast<uint32_t>(bound_.size());
}

std::shared_ptr<const DynamicTypeImpl> TypeState::get_discriminator_type() const
{
    return discriminator_type_;
}

std::shared_ptr<const DynamicTypeImpl> TypeState::get_element_type() const
{
    return element_type_;
}

std::shared_ptr<const DynamicTypeImpl> TypeState::get_key_element_type() const
{
    return key_element_type_;
}

TypeKind TypeState::get_kind() const noexcept
{
    return kind_;
}

const std::string& TypeState::get_name() const
{
    return name_;
}

uint32_t TypeState::get_total_bounds() const
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

bool TypeState::is_consistent(
        bool type /* = false*/) const
{
    // Enums should have at least one member
    /*TODO(richiware)
       if (type && kind_ == TK_ENUM && members_.empty())
       {
       return false;
       }
     */

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
    if (bound_.size() != 1 &&
            (kind_ == TK_SEQUENCE || kind_ == TK_MAP || kind_ == TK_BITMASK ||
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
    if (!element_type_ &&
            (kind_ == TK_ARRAY || kind_ == TK_SEQUENCE || kind_ == TK_STRING8 ||
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

    // Check members if any
    /*TODO(richiware)
       if (type && std::any_of(members_.begin(), members_.end(),
       [this](const DynamicTypeMemberImpl& m)
       {
        return !m.is_consistent(kind_);
       }))
       {
       // inconsistencies in the use of annotations
       return false;
       }
     */

    return true;
}

bool TypeState::is_primitive() const
{
    return kind_ > TK_NONE && kind_ <= TK_CHAR16 &&
           0u == get_annotation_count() &&
           //TODO(richiware) members_.empty() &&
           !base_type_ && !discriminator_type_ &&
           !element_type_ && !key_element_type_;
}

bool TypeState::is_subclass(
        const TypeState& state) const
{
    return state.kind_ == TK_STRUCTURE &&
           kind_ == TK_STRUCTURE &&
           base_type_ && (
        *base_type_ == state ||
        base_type_->is_subclass(state));
}

bool TypeState::is_type_name_consistent(
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

void TypeState::set_kind(
        TypeKind kind)
{
    kind_ = kind;
}

void TypeState::set_name(
        const std::string& name)
{
    name_ = name;
}

void TypeState::set_name(
        std::string&& name)
{
    name_ = std::move(name);
}

const std::list<const DynamicTypeMemberImpl*> TypeState::get_all_members() const
{
    std::list<const DynamicTypeMemberImpl*> res;

    // retrieve members from the base classes
    if (base_type_)
    {
        res = resolve_alias_type(*base_type_).get_all_members();
    }

    // populate the local members
    /*TODO(richiware)
       std::transform(
       members_.begin(),
       members_.end(),
       std::back_inserter(res),
       [](const DynamicTypeMemberImpl& m)
       {
       return &m;
       });
     */

    return res;
}

std::map<std::string, const DynamicTypeMemberImpl*> TypeState::get_all_members_by_name() const
{
    std::map<std::string, const DynamicTypeMemberImpl*> res;

    // retrieve members from the base classes
    if (base_type_)
    {
        res = resolve_alias_type(*base_type_).get_all_members_by_name();
    }

    // populate the local members
    res.insert(member_by_name_.begin(), member_by_name_.end());

    return res;
}

DynamicTypeMembersByName TypeState::get_all_members_by_name(
        ReturnCode_t* ec) const noexcept
{
    if (ec != nullptr)
    {
        *ec = ReturnCode_t{};
    }

    return DynamicTypeMembersByName{get_all_members_by_name()};
}

std::map<MemberId, const DynamicTypeMemberImpl*> TypeState::get_all_members_by_id() const
{
    std::map<MemberId, const DynamicTypeMemberImpl*> res;

    // retrieve members from the base classes
    if (base_type_)
    {
        res = resolve_alias_type(*base_type_).get_all_members_by_id();
    }

    // populate the local members
    res.insert(member_by_id_.begin(), member_by_id_.end());

    return res;
}

DynamicTypeMembersById TypeState::get_all_members_by_id(
        ReturnCode_t* ec) const noexcept
{
    if (ec != nullptr)
    {
        *ec = ReturnCode_t{};
    }

    return DynamicTypeMembersById{get_all_members_by_id()};
}

const DynamicTypeMemberImpl& TypeState::get_member_by_index(
        uint32_t index) const
{
    uint32_t offset = 0;

    // try the base type
    if (base_type_)
    {
        try
        {
            return resolve_alias_type(*base_type_).get_member_by_index(index);
        }
        catch (const std::system_error&)
        {
            offset = base_type_->get_member_count();
        }
    }

    /*TODO(richiware)
       if (index >= (offset + members_.size()))
       {
       //TODO(richiware) throw std::system_error(
       //TODO(richiware)           make_error_code(RETCODE_ERROR),
       //TODO(richiware)           "Error getting member by index, member not found.");
       }
     */

    /*TODO(richiware)
       auto it = members_.begin();
       std::advance(it, index - offset);
       return *it;
     */
}

const DynamicTypeMemberImpl& TypeState::get_member_by_name(
        const std::string& name) const
{
    auto it = member_by_name_.find(name);
    if (it != member_by_name_.end())
    {
        return *it->second;
    }
    else if (base_type_)
    {
        // try the base type
        try
        {
            return resolve_alias_type(*base_type_).get_member_by_name(name);
        }
        catch (const std::system_error&)
        {
        }
    }

    //TODO(richiware) throw std::system_error(
    //TODO(richiware)           make_error_code(RETCODE_ERROR),
    //TODO(richiware)           "Error getting member by name, member not found.");
}

const DynamicTypeMember* TypeState::get_member_by_name(
        const char* name,
        ReturnCode_t* ec) const noexcept
{
    try
    {
        if (ec)
        {
            *ec = RETCODE_OK;
        }

        return &get_member_by_name(name).get_interface();
    }
    catch (std::system_error& e)
    {
        if (ec)
        {
            *ec = static_cast<uint32_t>(e.code().value());
        }

        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());

        return nullptr;
    }
}

const DynamicTypeMemberImpl& TypeState::get_member(
        MemberId id) const
{
    auto it = member_by_id_.find(id);
    if (it != member_by_id_.end())
    {
        return *it->second;
    }
    else if (base_type_)
    {
        try
        {
            return resolve_alias_type(*base_type_).get_member(id);
        }
        catch (const std::system_error&)
        {
        }
    }

    //TODO(richiware) throw std::system_error(
    //TODO(richiware)           make_error_code(RETCODE_ERROR),
    //TODO(richiware)           "Error getting member, member not found.");
}

uint32_t TypeState::get_member_count() const
{
    /*TODO(richiware)
       std::size_t res{members_.size()};

       if (base_type_)
       {
       res += resolve_alias_type(*base_type_).get_member_count();
       }

       return static_cast<uint32_t>(res);
     */
}

MemberId TypeState::get_member_id_by_name(
        const std::string& name) const
{
    auto it = member_by_name_.find(name);

    if (it != member_by_name_.end())
    {
        return it->second->id();
    }
    else if (base_type_)
    {
        return resolve_alias_type(*base_type_).get_member_id_by_name(name);
    }

    return MEMBER_ID_INVALID;
}

MemberId TypeState::get_member_id_at_index(
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

    /*TODO(richiware)
       if (index >= (offset + members_.size()))
       {
       // index out of boundaries
       return MEMBER_ID_INVALID;
       }
     */

    // retrieve from local collection
    /*TODO(richiware)
       auto it = members_.begin();
       std::advance(it, index - offset);
       assert(it->index() == index);
       return it->id();
     */
}

bool TypeState::exists_member_by_name(
        const ObjectName& name) const
{
    auto base = get_base_type();
    if (base)
    {
        if (base->exists_member_by_name(name))
        {
            return true;
        }
    }
    //TODO(richiware) return member_by_name_.find(name) != member_by_name_.end();
}

bool TypeState::exists_member_by_id(
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

MemberId TypeState::get_id_from_label(
        uint64_t label) const
{
    // Check is a union
    if (TK_UNION != kind_)
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
    /*TODO(richiware)
       for (auto& m : members_)
       {
       auto& lbs = m.label();
       if (it != lbs.end())
       {
       return m.id();
       }
       }
     */

    return MEMBER_ID_INVALID;
}

std::ostream& operator <<(
        std::ostream& os,
        const TypeState& td)
{
    using namespace std;

    // indentation increment
    ++os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);

    auto manips = [](ostream& os) -> ostream&
            {
                long indent = os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);
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
        for (const AnnotationDescriptorImpl& d : td.get_all_annotations())
        {
        }
    }

    // Show members
    if (td.get_member_count())
    {
        // notify the members which object they belong to
        os.pword(DynamicTypeBuilderFactoryImpl::object_index) = (void*)&td;

        os << manips << "members:";
        for (const DynamicTypeMemberImpl* m : td.get_all_members())
        {
            os << *m;
        }

        os.pword(DynamicTypeBuilderFactoryImpl::object_index) = nullptr;
    }

    // indentation decrement
    --os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);

    return os;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
