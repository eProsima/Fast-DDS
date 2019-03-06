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

#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeNamesGenerator.h>
#include <fastrtps/log/Log.h>
#include <sstream>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeObjectFactoryReleaser
{
public:
    ~TypeObjectFactoryReleaser()
    {
        TypeObjectFactory::delete_instance();
    }
};

static TypeObjectFactoryReleaser s_releaser;
static TypeObjectFactory* g_instance = nullptr;
TypeObjectFactory* TypeObjectFactory::get_instance()
{
    if (g_instance == nullptr)
    {
        g_instance = new TypeObjectFactory();
    }
    return g_instance;
}

ResponseCode TypeObjectFactory::delete_instance()
{
    if (g_instance != nullptr)
    {
        delete g_instance;
        g_instance = nullptr;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_ERROR;
}

TypeObjectFactory::TypeObjectFactory()
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    // Generate basic TypeIdentifiers
    TypeIdentifier* auxIdent;
    // TK_BOOLEAN:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BOOLEAN);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_BOOLEAN, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BYTE);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_BYTE, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BYTE);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT8, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BYTE);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT8, auxIdent));
    // TK_INT16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_INT16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT16, auxIdent));
    // TK_INT32:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_INT32);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT32, auxIdent));
    // TK_INT64:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_INT64);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT64, auxIdent));
    // TK_UINT16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_UINT16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT16, auxIdent));
    // TK_UINT32:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_UINT32);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT32, auxIdent));
    // TK_UINT64:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_UINT64);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT64, auxIdent));
    // TK_FLOAT32:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_FLOAT32);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT32, auxIdent));
    // TK_FLOAT64:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_FLOAT64);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT64, auxIdent));
    // TK_FLOAT128:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_FLOAT128);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT128, auxIdent));
    // TK_CHAR8:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_CHAR8);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR8, auxIdent));
    // TK_CHAR16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_CHAR16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR16, auxIdent));
    // TK_CHAR16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_CHAR16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR16T, auxIdent));
}

TypeObjectFactory::~TypeObjectFactory()
{
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
        auto id_it = identifiers_.begin();
        while (id_it != identifiers_.end())
        {
            const TypeIdentifier* id = id_it->second;
            nullify_all_entries(id);
            delete (id);
            ++id_it;
        }
        identifiers_.clear();

        auto idc_it = complete_identifiers_.begin();
        while (idc_it != complete_identifiers_.end())
        {
            const TypeIdentifier* id = idc_it->second;
            nullify_all_entries(id);
            delete (id);
            ++idc_it;
        }
        complete_identifiers_.clear();
    }
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexObjects);
        auto obj_it = objects_.begin();
        while (obj_it != objects_.end())
        {
            delete (obj_it->second);
            ++obj_it;
        }
        objects_.clear();

        auto objc_it = complete_objects_.begin();
        while (objc_it != complete_objects_.end())
        {
            delete (objc_it->second);
            ++objc_it;
        }
        complete_objects_.clear();
    }
}

void TypeObjectFactory::nullify_all_entries(const TypeIdentifier* identifier)
{
    for (auto it = identifiers_.begin(); it != identifiers_.end(); ++it)
    {
        if (it->second == identifier)
        {
            it->second = nullptr;
        }
    }

    for (auto it = complete_identifiers_.begin(); it != complete_identifiers_.end(); ++it)
    {
        if (it->second == identifier)
        {
            it->second = nullptr;
        }
    }
}

const TypeObject* TypeObjectFactory::get_type_object(const std::string& type_name, bool complete) const
{
    const TypeIdentifier* identifier = get_type_identifier(type_name, complete);
    if (identifier == nullptr)
    {
        return nullptr;
    }

    return get_type_object(identifier);
}

const TypeObject* TypeObjectFactory::get_type_object(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexObjects);
    if (identifier == nullptr) return nullptr;
    if (identifier->_d() == EK_COMPLETE)
    {
        if (complete_objects_.find(identifier) != complete_objects_.end())
        {
            return complete_objects_.at(identifier);
        }
    }
    else
    {
        if (objects_.find(identifier) != objects_.end())
        {
            return objects_.at(identifier);
        }
    }

    // Maybe they are using an external TypeIdentifier?
    const TypeIdentifier* internalId = get_stored_type_identifier(identifier);
    if (internalId != nullptr)
    {
        if (internalId == identifier)
        {
            return nullptr; // Type without object
        }
        return get_type_object(internalId);
    }

    return nullptr;
}

TypeKind TypeObjectFactory::get_type_kind(const std::string& type_name) const
{
    if (type_name == TKNAME_BOOLEAN)
    {
        return TK_BOOLEAN;
    }
    else if (type_name == TKNAME_INT16)
    {
        return TK_INT16;
    }
    else if (type_name == TKNAME_INT32)
    {
        return TK_INT32;
    }
    else if (type_name == TKNAME_UINT16)
    {
        return TK_UINT16;
    }
    else if (type_name == TKNAME_UINT32)
    {
        return TK_UINT32;
    }
    else if (type_name == TKNAME_FLOAT32)
    {
        return TK_FLOAT32;
    }
    else if (type_name == TKNAME_FLOAT64)
    {
        return TK_FLOAT64;
    }
    else if (type_name == TKNAME_CHAR8)
    {
        return TK_CHAR8;
    }
    else if (type_name == TKNAME_BYTE || type_name == TKNAME_INT8 || type_name == TKNAME_UINT8)
    {
        return TK_BYTE;
    }
    else if (type_name.find("strings_") == 0)
    {
        return TI_STRING8_SMALL;
    }
    else if (type_name.find("stringl_") == 0)
    {
        return TI_STRING8_LARGE;
    }
    else if (type_name.find("sets_") == 0)
    {
        return TI_PLAIN_SEQUENCE_SMALL;
    }
    else if (type_name.find("setl_") == 0)
    {
        return TI_PLAIN_SEQUENCE_LARGE;
    }
    else if (type_name.find("arrays_") == 0)
    {
        return TI_PLAIN_ARRAY_SMALL;
    }
    else if (type_name.find("arrayl_") == 0)
    {
        return TI_PLAIN_ARRAY_LARGE;
    }
    else if (type_name == TKNAME_INT64)
    {
        return TK_INT64;
    }
    else if (type_name == TKNAME_UINT64)
    {
        return TK_UINT64;
    }
    else if (type_name == TKNAME_FLOAT128)
    {
        return TK_FLOAT128;
    }
    else if (type_name == TKNAME_CHAR16 || type_name == TKNAME_CHAR16T)
    {
        return TK_CHAR16;
    }
    else if (type_name.find("wstrings_") == 0)
    {
        return TI_STRING16_SMALL;
    }
    else if (type_name.find("wstringl_") == 0)
    {
        return TI_STRING16_LARGE;
    }
    else if (type_name.find("sequences_") == 0)
    {
        return TI_PLAIN_SEQUENCE_SMALL;
    }
    else if (type_name.find("sequencel_") == 0)
    {
        return TI_PLAIN_SEQUENCE_LARGE;
    }
    else if (type_name.find("maps_") == 0)
    {
        return TI_PLAIN_MAP_SMALL;
    }
    else if (type_name.find("mapl_") == 0)
    {
        return TI_PLAIN_MAP_LARGE;
    }
    else if (get_type_identifier(type_name) != nullptr)
    {
        return EK_MINIMAL;
    }
    else
    {
        return TK_NONE;
    }
}

std::string TypeObjectFactory::get_type_name(const TypeKind kind) const
{
    switch (kind)
    {
        // Primitive types, already defined (never will be asked, but ok)
        case TK_BOOLEAN: return TKNAME_BOOLEAN;
        case TK_INT16: return TKNAME_INT16;
        case TK_INT32: return TKNAME_INT32;
        case TK_UINT16: return TKNAME_UINT16;
        case TK_UINT32: return TKNAME_UINT32;
        case TK_FLOAT32: return TKNAME_FLOAT32;
        case TK_FLOAT64: return TKNAME_FLOAT64;
        case TK_CHAR8: return TKNAME_CHAR8;
        case TK_BYTE: return TKNAME_BYTE;
        case TK_INT64: return TKNAME_INT64;
        case TK_UINT64: return TKNAME_UINT64;
        case TK_FLOAT128: return TKNAME_FLOAT128;
        case TK_CHAR16: return TKNAME_CHAR16;
        default:
            break;
    }
    return "";
}

const TypeIdentifier* TypeObjectFactory::get_primitive_type_identifier(TypeKind kind) const
{
    std::string typeName = get_type_name(kind);
    if (typeName.empty()) return nullptr;
    return get_type_identifier(typeName);
}

/*
const TypeIdentifier* TypeObjectFactory::TryCreateTypeIdentifier(const std::string& type_name)
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    // TODO Makes sense here? I don't think so.
}
*/

const TypeIdentifier* TypeObjectFactory::get_type_identifier(const std::string& type_name, bool complete) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);

    if (complete)
    {
        if (complete_identifiers_.find(type_name) != complete_identifiers_.end())
        {
            return complete_identifiers_.at(type_name);
        }
        /*else // Try it with minimal
        {
            return get_type_identifier(type_name, false);
        }*/
    }
    else
    {
        if (identifiers_.find(type_name) != identifiers_.end())
        {
            return identifiers_.at(type_name);
        }
    }

    // Try with aliases
    if (aliases_.find(type_name) != aliases_.end())
    {
        return get_type_identifier(aliases_.at(type_name), complete);
    }

    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::get_type_identifier_trying_complete(const std::string& type_name) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);

    if (complete_identifiers_.find(type_name) != complete_identifiers_.end())
    {
        return complete_identifiers_.at(type_name);
    }
    else // Try it with minimal
    {
        return get_type_identifier(type_name, false);
    }
}

const TypeIdentifier* TypeObjectFactory::get_stored_type_identifier(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    if (identifier == nullptr) return nullptr;
    if (identifier->_d() == EK_COMPLETE)
    {
        for (auto& it : complete_identifiers_)
        {
            if (*(it.second) == *identifier) return it.second;
        }
    }
    else
    {
        for (auto& it : identifiers_)
        {
            if (*(it.second) == *identifier) return it.second;
        }
    }
    return nullptr;
}

std::string TypeObjectFactory::get_type_name(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    if (identifier == nullptr) return "<NULLPTR>";
    if (identifier->_d() == EK_COMPLETE)
    {
        for (auto& it : complete_identifiers_)
        {
            if (*(it.second) == *identifier) return it.first;
        }
    }
    else
    {
        for (auto& it : identifiers_)
        {
            if (*(it.second) == *identifier) return it.first;
        }
    }

    // Maybe they are using an external TypeIdentifier?
    const TypeIdentifier* internalId = get_stored_type_identifier(identifier);
    if (internalId != nullptr)
    {
        return get_type_name(internalId);
    }

    return "UNDEF";
}

const TypeIdentifier* TypeObjectFactory::try_get_complete(const TypeIdentifier* identifier) const
{
    if (identifier->_d() == EK_COMPLETE)
    {
        return identifier;
    }

    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    std::string name = get_type_name(identifier);
    return get_type_identifier_trying_complete(name);
}

void TypeObjectFactory::add_type_identifier(const std::string& type_name, const TypeIdentifier* identifier)
{
    const TypeIdentifier* alreadyExists = get_stored_type_identifier(identifier);
    if (alreadyExists != nullptr)
    {
        // Don't copy
        if (alreadyExists->_d() == EK_COMPLETE)
        {
            complete_identifiers_[type_name] = alreadyExists;
        }
        else
        {
            identifiers_[type_name] = alreadyExists;
        }
        return;
    }

    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    //identifiers_.insert(std::pair<const std::string, const TypeIdentifier*>(type_name, identifier));
    if (identifier->_d() == EK_COMPLETE)
    {
        if (complete_identifiers_.find(type_name) == complete_identifiers_.end())
        {
            TypeIdentifier* id = new TypeIdentifier;
            *id = *identifier;
            complete_identifiers_[type_name] = id;
        }
    }
    else
    {
        if (identifiers_.find(type_name) == identifiers_.end())
        {
            TypeIdentifier* id = new TypeIdentifier;
            *id = *identifier;
            identifiers_[type_name] = id;
        }
    }
}

void TypeObjectFactory::add_type_object(const std::string& type_name, const TypeIdentifier* identifier,
    const TypeObject* object)
{
    add_type_identifier(type_name, identifier);

    std::unique_lock<std::recursive_mutex> scopedObj(m_MutexObjects);

    if (object != nullptr)
    {
        if (object->_d() == EK_MINIMAL)
        {
            const TypeIdentifier* typeId = identifiers_[type_name];
            if (objects_.find(typeId) == objects_.end())
            {
                TypeObject* obj = new TypeObject;
                *obj = *object;
                objects_[typeId] = obj;
            }
        }
        else if (object->_d() == EK_COMPLETE)
        {
            const TypeIdentifier* typeId = complete_identifiers_[type_name];
            if (complete_objects_.find(typeId) == complete_objects_.end())
            {
                TypeObject* obj = new TypeObject;
                *obj = *object;
                complete_objects_[typeId] = obj;
            }
        }
    }
}

const TypeIdentifier* TypeObjectFactory::get_string_identifier(
        uint32_t bound,
        bool wide)
{
    std::string type = TypeNamesGenerator::get_string_type_name(bound, wide, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(type);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        TypeIdentifier auxIdent;
        if (bound < 256)
        {
            auxIdent._d(wide ? TI_STRING16_SMALL : TI_STRING8_SMALL);
            auxIdent.string_sdefn().bound(static_cast<octet>(bound));
        }
        else
        {
            auxIdent._d(wide ? TI_STRING16_LARGE : TI_STRING8_LARGE);
            auxIdent.string_ldefn().bound(bound);
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(type, auxIdent));
        //identifiers_[type] = auxIdent;
        add_type_identifier(type, &auxIdent);
        return get_type_identifier(type);
    }
    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::get_sequence_identifier(const std::string& type_name,
    uint32_t bound, bool complete)
{
    std::string auxType = TypeNamesGenerator::get_sequence_type_name(type_name, bound, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* innerIdent = (complete)
            ? get_type_identifier_trying_complete(type_name)
            : get_type_identifier(type_name);

        TypeIdentifier auxIdent;
        if (bound < 256)
        {
            auxIdent._d(TI_PLAIN_SEQUENCE_SMALL);
            auxIdent.seq_sdefn().bound(static_cast<octet>(bound));
            auxIdent.seq_sdefn().element_identifier(innerIdent);
            auxIdent.seq_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.seq_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.seq_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.seq_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.seq_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.seq_sdefn().header().element_flags().IS_KEY(false);
            auxIdent.seq_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.seq_sdefn().header().equiv_kind(get_type_kind(type_name));
        }
        else
        {
            auxIdent._d(TI_PLAIN_SEQUENCE_LARGE);
            auxIdent.seq_ldefn().bound(bound);
            auxIdent.seq_ldefn().element_identifier(innerIdent);
            auxIdent.seq_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.seq_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.seq_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.seq_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.seq_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.seq_ldefn().header().element_flags().IS_KEY(false);
            auxIdent.seq_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.seq_ldefn().header().equiv_kind(get_type_kind(type_name));
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //identifiers_[auxType] = auxIdent;
        add_type_identifier(auxType, &auxIdent);
        return get_type_identifier(auxType);
    }
    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::get_array_identifier(const std::string& type_name,
    const std::vector<uint32_t> &bound, bool complete)
{
    uint32_t size;
    std::string auxType = TypeNamesGenerator::get_array_type_name(type_name, bound, size, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* innerIdent = (complete)
            ? get_type_identifier_trying_complete(type_name)
            : get_type_identifier(type_name);

        TypeIdentifier auxIdent;
        if (size < 256)
        {
            auxIdent._d(TI_PLAIN_ARRAY_SMALL);
            for (uint32_t b : bound)
            {
                auxIdent.array_sdefn().array_bound_seq().push_back(static_cast<octet>(b));
            }
            auxIdent.array_sdefn().element_identifier(innerIdent);
            auxIdent.array_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.array_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.array_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.array_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.array_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.array_sdefn().header().element_flags().IS_KEY(false);
            auxIdent.array_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.array_sdefn().header().equiv_kind(get_type_kind(type_name));
        }
        else
        {
            auxIdent._d(TI_PLAIN_ARRAY_LARGE);
            for (uint32_t b : bound)
            {
                auxIdent.array_ldefn().array_bound_seq().push_back(b);
            }
            auxIdent.array_ldefn().element_identifier(innerIdent);
            auxIdent.array_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.array_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.array_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.array_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.array_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.array_ldefn().header().element_flags().IS_KEY(false);
            auxIdent.array_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.array_ldefn().header().equiv_kind(get_type_kind(type_name));
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //identifiers_[auxType] = auxIdent;
        add_type_identifier(auxType, &auxIdent);
        return get_type_identifier(auxType);
    }
    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::get_map_identifier(const std::string& key_type_name,
    const std::string& value_type_name, uint32_t bound, bool complete)
{
    std::string auxType = TypeNamesGenerator::get_map_type_name(key_type_name, value_type_name, bound, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* keyIdent = (complete)
            ? get_type_identifier_trying_complete(key_type_name)
            : get_type_identifier(key_type_name);
        const TypeIdentifier* valIdent = (complete)
            ? get_type_identifier_trying_complete(value_type_name)
            : get_type_identifier(value_type_name);

        TypeIdentifier auxIdent;
        if (bound < 256)
        {
            auxIdent._d(TI_PLAIN_MAP_SMALL);
            auxIdent.map_sdefn().bound(static_cast<octet>(bound));
            auxIdent.map_sdefn().element_identifier(valIdent);
            auxIdent.map_sdefn().key_identifier(keyIdent);
            auxIdent.map_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.map_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.map_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_sdefn().header().element_flags().IS_KEY(false);
            auxIdent.map_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.map_sdefn().key_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_sdefn().key_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_sdefn().key_flags().IS_EXTERNAL(false);
            auxIdent.map_sdefn().key_flags().IS_OPTIONAL(false);
            auxIdent.map_sdefn().key_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_sdefn().key_flags().IS_KEY(false);
            auxIdent.map_sdefn().key_flags().IS_DEFAULT(false);
            auxIdent.map_sdefn().header().equiv_kind(get_type_kind(value_type_name));
        }
        else
        {
            auxIdent._d(TI_PLAIN_MAP_LARGE);
            auxIdent.map_ldefn().bound(bound);
            auxIdent.map_ldefn().element_identifier(valIdent);
            auxIdent.map_ldefn().key_identifier(keyIdent);
            auxIdent.map_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.map_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.map_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_ldefn().header().element_flags().IS_KEY(false);
            auxIdent.map_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.map_ldefn().key_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_ldefn().key_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_ldefn().key_flags().IS_EXTERNAL(false);
            auxIdent.map_ldefn().key_flags().IS_OPTIONAL(false);
            auxIdent.map_ldefn().key_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_ldefn().key_flags().IS_KEY(false);
            auxIdent.map_ldefn().key_flags().IS_DEFAULT(false);
            auxIdent.map_ldefn().header().equiv_kind(get_type_kind(value_type_name));
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //identifiers_[auxType] = auxIdent;
        add_type_identifier(auxType, &auxIdent);
        return get_type_identifier(auxType);
    }
    return nullptr;
}

static TypeKind GetTypeKindFromIdentifier(const TypeIdentifier* identifier)
{
    if (identifier == nullptr) return TK_NONE;
    switch (identifier->_d())
    {
        case TI_STRING8_SMALL:
        case TI_STRING8_LARGE:
            return TK_STRING8;
        case TI_STRING16_SMALL:
        case TI_STRING16_LARGE:
            return TK_STRING16;
        case TI_PLAIN_SEQUENCE_SMALL:
        case TI_PLAIN_SEQUENCE_LARGE:
            return TK_SEQUENCE;
        case TI_PLAIN_ARRAY_SMALL:
        case TI_PLAIN_ARRAY_LARGE:
            return TK_ARRAY;
        case TI_PLAIN_MAP_SMALL:
        case TI_PLAIN_MAP_LARGE:
            return TK_MAP;
        case TI_STRONGLY_CONNECTED_COMPONENT:
            return TK_NONE;
        case EK_COMPLETE:
        case EK_MINIMAL:
        default:
            return identifier->_d();
    }
}

DynamicType_ptr TypeObjectFactory::build_dynamic_type(const std::string& name, const TypeIdentifier* identifier,
    const TypeObject* object) const
{
    TypeKind kind = GetTypeKindFromIdentifier(identifier);
    TypeDescriptor descriptor(name, kind);
    switch (kind)
    {
    // Basic types goes as default!
    /*
    case TK_NONE:
    case TK_BOOLEAN:
    case TK_BYTE:
    case TK_INT16:
    case TK_INT32:
    case TK_INT64:
    case TK_UINT16:
    case TK_UINT32:
    case TK_UINT64:
    case TK_FLOAT32:
    case TK_FLOAT64:
    case TK_FLOAT128:
    case TK_CHAR8:
    case TK_CHAR16:
        break;
    */
        case TK_STRING8:
        {
            if (identifier->_d() == TI_STRING8_SMALL)
            {
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->string_sdefn().bound()));
            }
            else
            {
                descriptor.bound_.emplace_back(identifier->string_ldefn().bound());
            }
            descriptor.element_type_ = DynamicTypeBuilderFactory::get_instance()->create_char8_type();
            break;
        }
        case TK_STRING16:
        {
            if (identifier->_d() == TI_STRING16_SMALL)
            {
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->string_sdefn().bound()));
            }
            else
            {
                descriptor.bound_.emplace_back(identifier->string_ldefn().bound());
            }
            descriptor.element_type_ = DynamicTypeBuilderFactory::get_instance()->create_char16_type();
            break;
        }
        case TK_SEQUENCE:
        {
            if (identifier->_d() == TI_PLAIN_SEQUENCE_SMALL)
            {
                const TypeIdentifier* aux = try_get_complete(identifier->seq_sdefn().element_identifier());
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->seq_sdefn().bound()));
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            else
            {
                const TypeIdentifier* aux = try_get_complete(identifier->seq_ldefn().element_identifier());
                descriptor.bound_.emplace_back(identifier->seq_ldefn().bound());
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            break;
        }
        case TK_ARRAY:
        {
            if (identifier->_d() == TI_PLAIN_ARRAY_SMALL)
            {
                const TypeIdentifier* aux = try_get_complete(identifier->array_sdefn().element_identifier());
                for (octet b : identifier->array_sdefn().array_bound_seq())
                {
                    descriptor.bound_.emplace_back(static_cast<uint32_t>(b));
                }
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            else
            {
                const TypeIdentifier* aux = identifier->array_ldefn().element_identifier();
                descriptor.bound_ = identifier->array_ldefn().array_bound_seq();
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            break;
        }
        case TK_MAP:
        {
            if (identifier->_d() == TI_PLAIN_MAP_SMALL)
            {
                const TypeIdentifier* aux = try_get_complete(identifier->map_sdefn().element_identifier());
                const TypeIdentifier* aux2 = try_get_complete(identifier->map_sdefn().key_identifier());
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->map_sdefn().bound()));
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
                descriptor.key_element_type_ = build_dynamic_type(get_type_name(aux), aux2, get_type_object(aux2));
            }
            else
            {
                const TypeIdentifier* aux = try_get_complete(identifier->map_ldefn().element_identifier());
                const TypeIdentifier* aux2 = try_get_complete(identifier->map_ldefn().key_identifier());
                descriptor.bound_.emplace_back(identifier->map_ldefn().bound());
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
                descriptor.key_element_type_ = build_dynamic_type(get_type_name(aux), aux2, get_type_object(aux2));
            }
            break;
        }
        case EK_MINIMAL:
        case EK_COMPLETE:
            // A MinimalTypeObject cannot instantiate a valid TypeDescriptor, but maybe the object isn't minimal
            if (object != nullptr && object->_d() == EK_COMPLETE)
            {
                return build_dynamic_type(descriptor, object);
            }
            break;
        default:
            break;
    }

    DynamicTypeBuilder_ptr outputType = DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);
    //outputType->set_name(name);
    return outputType->build();
}

// TODO annotations
DynamicType_ptr TypeObjectFactory::build_dynamic_type(
        TypeDescriptor& descriptor,
        const TypeObject* object) const
{
    if (object == nullptr || object->_d() != EK_COMPLETE)
    {
        return nullptr;
    }

    // Change descriptor's kind
    descriptor.set_kind(object->complete()._d());

    switch (object->complete()._d())
    {
        // From here, we need TypeObject
        case TK_ALIAS:
        {
            const TypeIdentifier* aux =
                get_stored_type_identifier(&object->complete().alias_type().body().common().related_type());
            descriptor.base_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            descriptor.set_name(object->complete().alias_type().header().detail().type_name());
            return DynamicTypeBuilderFactory::get_instance()->create_type(&descriptor);
        }
        case TK_STRUCTURE:
        {
            const TypeIdentifier* aux = &object->complete().struct_type().header().base_type();
            if (aux->_d() == TK_STRUCTURE)
            {
                descriptor.base_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }

            DynamicTypeBuilder_ptr structType = DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            //uint32_t order = 0;
            const CompleteStructMemberSeq& structVector = object->complete().struct_type().member_seq();
            for (auto member = structVector.begin(); member != structVector.end(); ++member)
            {
                //const TypeIdentifier* auxMem = &member.common().member_type_id();
                const TypeIdentifier* auxMem = get_stored_type_identifier(&member->common().member_type_id());
                if (auxMem == nullptr)
                {
                    std::cout << "(Struct) auxMem is nullptr, but original member has " << (int)member->common().member_type_id()._d() << std::endl;
                }
                MemberDescriptor memDesc;
                memDesc.id_ = member->common().member_id();
                memDesc.set_type(build_dynamic_type(get_type_name(auxMem), auxMem, get_type_object(auxMem)));
                //memDesc.set_index(order++);
                memDesc.set_name(member->detail().name());
                structType->add_member(&memDesc);
            }
            return structType->build();
        }
        case TK_ENUM:
        {
            DynamicTypeBuilder_ptr enumType = DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            uint32_t order = 0;
            const CompleteEnumeratedLiteralSeq& enumVector = object->complete().enumerated_type().literal_seq();
            for (auto member = enumVector.begin(); member != enumVector.end(); ++member)
            {
                enumType->add_empty_member(order++, member->detail().name());
            }
            return enumType->build();
        }
        case TK_BITMASK:
        {
            DynamicTypeBuilder_ptr bitmaskType = DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            const CompleteBitflagSeq& seq = object->complete().bitmask_type().flag_seq();
            for (auto member = seq.begin(); member != seq.end(); ++member)
            {
                MemberDescriptor memDesc;
                memDesc.id_ = member->common().position();
                memDesc.set_name(member->detail().name());
                bitmaskType->add_member(&memDesc);
            }
            return bitmaskType->build();
        }
        case TK_BITSET:
        {
            /*
            const TypeIdentifier* aux = &object->complete().bitset_type().header().base_type();
            if (aux->_d() == TK_BITSET)
            {
                descriptor.base_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }

            DynamicTypeBuilder_ptr bitsetType = DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            //uint32_t order = 0;
            const CompleteBitfieldSeq& fields = object->complete().bitset_type().field_seq();
            for (auto member = fields.begin(); member != fields.end(); ++member)
            {
                //const TypeIdentifier* auxMem = &member.common().member_type_id();
                const TypeIdentifier* auxMem = get_primitive_type_identifier(member->common().holder_type());
                if (auxMem == nullptr)
                {
                    std::cout << "(Bitset) auxMem is nullptr, but original member has " << (int)member->common().holder_type() << std::endl;
                }
                MemberDescriptor memDesc;
                memDesc.id_ = member->common().position();
                memDesc.set_type(build_dynamic_type(get_type_name(auxMem), auxMem, get_type_object(auxMem)));
                memDesc.set_name(member->detail().name());
                // TODO Add bitbound!!
                bitsetType->add_member(&memDesc);
            }
            return bitsetType->build();
            */
            logError(XTYPES, "Bitset isn't supported by DynamicType");
            return nullptr;
        }
        case TK_UNION:
        {
            const TypeIdentifier* aux =
                get_stored_type_identifier(&object->complete().union_type().discriminator().common().type_id());
            descriptor.discriminator_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));

            DynamicTypeBuilder_ptr unionType = DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            //uint32_t order = 0;
            const CompleteUnionMemberSeq& unionVector = object->complete().union_type().member_seq();
            for (auto member = unionVector.begin(); member != unionVector.end(); ++member)
            {
                const TypeIdentifier* auxMem = get_stored_type_identifier(&member->common().type_id());
                if (auxMem == nullptr)
                {
                    std::cout << "(Union) auxMem is nullptr, but original member has " << (int)member->common().type_id()._d() << std::endl;
                }
                MemberDescriptor memDesc;
                memDesc.set_type(build_dynamic_type(get_type_name(auxMem), auxMem, get_type_object(auxMem)));
                //memDesc.set_index(order++);
                memDesc.id_ = member->common().member_id();
                memDesc.set_name(member->detail().name());
                memDesc.set_default_union_value(member->common().member_flags().IS_DEFAULT());
                if (descriptor.discriminator_type_->get_kind() == TK_ENUM)
                {
                    DynamicTypeMember enumMember;
                    descriptor.discriminator_type_->get_member(enumMember, memDesc.id_);
                    memDesc.default_value_ = enumMember.get_descriptor()->name_;
                    for (uint32_t lab : member->common().label_seq())
                    {
                        memDesc.add_union_case_index(lab);
                    }
                }
                else
                {
                    memDesc.default_value_ = std::to_string(memDesc.id_);
                    for (uint32_t lab : member->common().label_seq())
                    {
                        memDesc.add_union_case_index(lab);
                    }
                }
                unionType->add_member(&memDesc);
            }

            return unionType->build();
        }
        case TK_ANNOTATION:
        {
            DynamicTypeBuilder_ptr annotationType =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            for (const CompleteAnnotationParameter& member : object->complete().annotation_type().member_seq())
            {
                const TypeIdentifier* auxMem = get_stored_type_identifier(&member.common().member_type_id());
                if (auxMem == nullptr)
                {
                    std::cout << "(Annotation) auxMem is nullptr, but original member has " << (int)member.common().member_type_id()._d() << std::endl;
                }

                MemberDescriptor memDesc;
                memDesc.set_name(member.name());
                memDesc.set_type(build_dynamic_type(get_type_name(auxMem), auxMem, get_type_object(auxMem)));
                memDesc.set_default_value(member.default_value().to_string());
                annotationType->add_member(&memDesc);
            }
            // Annotation inner definitions?

            return annotationType->build();
        }
        default:
            break;
    }
    return nullptr;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
