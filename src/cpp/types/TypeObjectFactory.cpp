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
        TypeObjectFactory::DeleteInstance();
    }
};

static TypeObjectFactoryReleaser s_releaser;
static TypeObjectFactory* g_instance = nullptr;
TypeObjectFactory* TypeObjectFactory::GetInstance()
{
    if (g_instance == nullptr)
    {
        g_instance = new TypeObjectFactory();
    }
    return g_instance;
}

ResponseCode TypeObjectFactory::DeleteInstance()
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
    TypeIdentifier *auxIdent;
    // TK_BOOLEAN:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BOOLEAN);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_BOOLEAN, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BYTE);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_BYTE, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BYTE);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT8, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_BYTE);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT8, auxIdent));
    // TK_INT16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_INT16);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT16, auxIdent));
    // TK_INT32:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_INT32);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT32, auxIdent));
    // TK_INT64:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_INT64);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT64, auxIdent));
    // TK_UINT16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_UINT16);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT16, auxIdent));
    // TK_UINT32:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_UINT32);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT32, auxIdent));
    // TK_UINT64:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_UINT64);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT64, auxIdent));
    // TK_FLOAT32:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_FLOAT32);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT32, auxIdent));
    // TK_FLOAT64:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_FLOAT64);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT64, auxIdent));
    // TK_FLOAT128:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_FLOAT128);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT128, auxIdent));
    // TK_CHAR8:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_CHAR8);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR8, auxIdent));
    // TK_CHAR16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_CHAR16);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR16, auxIdent));
    // TK_CHAR16:
    auxIdent = new TypeIdentifier;
    auxIdent->_d(TK_CHAR16);
    m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR16T, auxIdent));
}

TypeObjectFactory::~TypeObjectFactory()
{
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
        auto id_it = m_Identifiers.begin();
        while (id_it != m_Identifiers.end())
        {
            const TypeIdentifier* id = id_it->second;
            nullifyAllEntries(id);
            delete (id);
            ++id_it;
        }
        m_Identifiers.clear();

        auto idc_it = m_CompleteIdentifiers.begin();
        while (idc_it != m_CompleteIdentifiers.end())
        {
            const TypeIdentifier* id = idc_it->second;
            nullifyAllEntries(id);
            delete (id);
            ++idc_it;
        }
        m_CompleteIdentifiers.clear();
    }
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexObjects);
        auto obj_it = m_Objects.begin();
        while (obj_it != m_Objects.end())
        {
            delete (obj_it->second);
            ++obj_it;
        }
        m_Objects.clear();

        auto objc_it = m_CompleteObjects.begin();
        while (objc_it != m_CompleteObjects.end())
        {
            delete (objc_it->second);
            ++objc_it;
        }
        m_CompleteObjects.clear();
    }
}

void TypeObjectFactory::nullifyAllEntries(const TypeIdentifier *identifier)
{
    for (auto it = m_Identifiers.begin(); it != m_Identifiers.end(); ++it)
    {
        if (it->second == identifier)
        {
            it->second = nullptr;
        }
    }

    for (auto it = m_CompleteIdentifiers.begin(); it != m_CompleteIdentifiers.end(); ++it)
    {
        if (it->second == identifier)
        {
            it->second = nullptr;
        }
    }
}

const TypeObject* TypeObjectFactory::GetTypeObject(const std::string &type_name, bool complete) const
{
    const TypeIdentifier* identifier = GetTypeIdentifier(type_name, complete);
    if (identifier == nullptr)
    {
        return nullptr;
    }

    return GetTypeObject(identifier);
}

const TypeObject* TypeObjectFactory::GetTypeObject(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexObjects);
    if (identifier == nullptr) return nullptr;
    if (identifier->_d() == EK_COMPLETE)
    {
        if (m_CompleteObjects.find(identifier) != m_CompleteObjects.end())
        {
            return m_CompleteObjects.at(identifier);
        }
    }
    else
    {
        if (m_Objects.find(identifier) != m_Objects.end())
        {
            return m_Objects.at(identifier);
        }
    }

    // Maybe they are using an external TypeIdentifier?
    const TypeIdentifier* internalId = GetStoredTypeIdentifier(identifier);
    if (internalId != nullptr)
    {
        if (internalId == identifier)
        {
            return nullptr; // Type without object
        }
        return GetTypeObject(internalId);
    }

    return nullptr;
}

TypeKind TypeObjectFactory::GetTypeKind(const std::string &type_name) const
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
    else if (GetTypeIdentifier(type_name) != nullptr)
    {
        return EK_MINIMAL;
    }
    else
    {
        return TK_NONE;
    }
}

std::string TypeObjectFactory::GetTypeName(const TypeKind kind) const
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

const TypeIdentifier* TypeObjectFactory::GetPrimitiveTypeIdentifier(TypeKind kind) const
{
    std::string typeName = GetTypeName(kind);
    if (typeName.empty()) return nullptr;
    return GetTypeIdentifier(typeName);
}

/*
const TypeIdentifier* TypeObjectFactory::TryCreateTypeIdentifier(const std::string &type_name)
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    // TODO Makes sense here? I don't think so.
}
*/

const TypeIdentifier* TypeObjectFactory::GetTypeIdentifier(const std::string &type_name, bool complete) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);

    if (complete)
    {
        if (m_CompleteIdentifiers.find(type_name) != m_CompleteIdentifiers.end())
        {
            return m_CompleteIdentifiers.at(type_name);
        }
        /*else // Try it with minimal
        {
            return GetTypeIdentifier(type_name, false);
        }*/
    }
    else
    {
        if (m_Identifiers.find(type_name) != m_Identifiers.end())
        {
            return m_Identifiers.at(type_name);
        }
    }

    // Try with aliases
    if (m_Aliases.find(type_name) != m_Aliases.end())
    {
        return GetTypeIdentifier(m_Aliases.at(type_name), complete);
    }

    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::GetTypeIdentifierTryingComplete(const std::string &type_name) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);

    if (m_CompleteIdentifiers.find(type_name) != m_CompleteIdentifiers.end())
    {
        return m_CompleteIdentifiers.at(type_name);
    }
    else // Try it with minimal
    {
        return GetTypeIdentifier(type_name, false);
    }
}

const TypeIdentifier* TypeObjectFactory::GetStoredTypeIdentifier(const TypeIdentifier *identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    if (identifier == nullptr) return nullptr;
    if (identifier->_d() == EK_COMPLETE)
    {
        for (auto& it : m_CompleteIdentifiers)
        {
            if (*(it.second) == *identifier) return it.second;
        }
    }
    else
    {
        for (auto& it : m_Identifiers)
        {
            if (*(it.second) == *identifier) return it.second;
        }
    }
    return nullptr;
}

std::string TypeObjectFactory::GetTypeName(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    if (identifier == nullptr) return "<NULLPTR>";
    if (identifier->_d() == EK_COMPLETE)
    {
        for (auto& it : m_CompleteIdentifiers)
        {
            if (*(it.second) == *identifier) return it.first;
        }
    }
    else
    {
        for (auto& it : m_Identifiers)
        {
            if (*(it.second) == *identifier) return it.first;
        }
    }

    // Maybe they are using an external TypeIdentifier?
    const TypeIdentifier* internalId = GetStoredTypeIdentifier(identifier);
    if (internalId != nullptr)
    {
        return GetTypeName(internalId);
    }

    return "UNDEF";
}

const TypeIdentifier* TypeObjectFactory::TryGetComplete(const TypeIdentifier* identifier) const
{
    if (identifier->_d() == EK_COMPLETE)
    {
        return identifier;
    }

    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    std::string name = GetTypeName(identifier);
    return GetTypeIdentifierTryingComplete(name);
}

void TypeObjectFactory::AddTypeIdentifier(const std::string &type_name, const TypeIdentifier* identifier)
{
    const TypeIdentifier *alreadyExists = GetStoredTypeIdentifier(identifier);
    if (alreadyExists != nullptr)
    {
        // Don't copy
        if (alreadyExists->_d() == EK_COMPLETE)
        {
            m_CompleteIdentifiers[type_name] = alreadyExists;
        }
        else
        {
            m_Identifiers[type_name] = alreadyExists;
        }
        return;
    }

    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    //m_Identifiers.insert(std::pair<const std::string, const TypeIdentifier*>(type_name, identifier));
    if (identifier->_d() == EK_COMPLETE)
    {
        if (m_CompleteIdentifiers.find(type_name) == m_CompleteIdentifiers.end())
        {
            TypeIdentifier* id = new TypeIdentifier;
            *id = *identifier;
            m_CompleteIdentifiers[type_name] = id;
        }
    }
    else
    {
        if (m_Identifiers.find(type_name) == m_Identifiers.end())
        {
            TypeIdentifier* id = new TypeIdentifier;
            *id = *identifier;
            m_Identifiers[type_name] = id;
        }
    }
}

void TypeObjectFactory::AddTypeObject(const std::string &type_name, const TypeIdentifier* identifier,
    const TypeObject* object)
{
    AddTypeIdentifier(type_name, identifier);

    std::unique_lock<std::recursive_mutex> scopedObj(m_MutexObjects);

    if (object != nullptr)
    {
        if (object->_d() == EK_MINIMAL)
        {
            const TypeIdentifier* typeId = m_Identifiers[type_name];
            if (m_Objects.find(typeId) == m_Objects.end())
            {
                TypeObject* obj = new TypeObject;
                *obj = *object;
                m_Objects[typeId] = obj;
            }
        }
        else if (object->_d() == EK_COMPLETE)
        {
            const TypeIdentifier* typeId = m_CompleteIdentifiers[type_name];
            if (m_CompleteObjects.find(typeId) == m_CompleteObjects.end())
            {
                TypeObject* obj = new TypeObject;
                *obj = *object;
                m_CompleteObjects[typeId] = obj;
            }
        }
    }
}

const TypeIdentifier* TypeObjectFactory::GetStringIdentifier(uint32_t bound, bool wide)
{
    std::string type = TypeNamesGenerator::getStringTypeName(bound, wide, false);

    const TypeIdentifier* c_auxIdent = GetTypeIdentifier(type);

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
        //m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(type, auxIdent));
        //m_Identifiers[type] = auxIdent;
        AddTypeIdentifier(type, &auxIdent);
        return GetTypeIdentifier(type);
    }
    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::GetSequenceIdentifier(const std::string &type_name,
    uint32_t bound, bool complete)
{
    std::string auxType = TypeNamesGenerator::getSequenceTypeName(type_name, bound, false);

    const TypeIdentifier* c_auxIdent = GetTypeIdentifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* innerIdent = (complete)
            ? GetTypeIdentifierTryingComplete(type_name)
            : GetTypeIdentifier(type_name);

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
            auxIdent.seq_sdefn().header().equiv_kind(GetTypeKind(type_name));
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
            auxIdent.seq_ldefn().header().equiv_kind(GetTypeKind(type_name));
        }
        //m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //m_Identifiers[auxType] = auxIdent;
        AddTypeIdentifier(auxType, &auxIdent);
        return GetTypeIdentifier(auxType);
    }
    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::GetArrayIdentifier(const std::string &type_name,
    const std::vector<uint32_t> &bound, bool complete)
{
    uint32_t size;
    std::string auxType = TypeNamesGenerator::getArrayTypeName(type_name, bound, size, false);

    const TypeIdentifier* c_auxIdent = GetTypeIdentifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* innerIdent = (complete)
            ? GetTypeIdentifierTryingComplete(type_name)
            : GetTypeIdentifier(type_name);

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
            auxIdent.array_sdefn().header().equiv_kind(GetTypeKind(type_name));
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
            auxIdent.array_ldefn().header().equiv_kind(GetTypeKind(type_name));
        }
        //m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //m_Identifiers[auxType] = auxIdent;
        AddTypeIdentifier(auxType, &auxIdent);
        return GetTypeIdentifier(auxType);
    }
    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::GetMapIdentifier(const std::string &key_type_name,
    const std::string &value_type_name, uint32_t bound, bool complete)
{
    std::string auxType = TypeNamesGenerator::getMapTypeName(key_type_name, value_type_name, bound, false);

    const TypeIdentifier* c_auxIdent = GetTypeIdentifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* keyIdent = (complete)
            ? GetTypeIdentifierTryingComplete(key_type_name)
            : GetTypeIdentifier(key_type_name);
        const TypeIdentifier* valIdent = (complete)
            ? GetTypeIdentifierTryingComplete(value_type_name)
            : GetTypeIdentifier(value_type_name);

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
            auxIdent.map_sdefn().header().equiv_kind(GetTypeKind(value_type_name));
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
            auxIdent.map_ldefn().header().equiv_kind(GetTypeKind(value_type_name));
        }
        //m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //m_Identifiers[auxType] = auxIdent;
        AddTypeIdentifier(auxType, &auxIdent);
        return GetTypeIdentifier(auxType);
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

DynamicType_ptr TypeObjectFactory::BuildDynamicType(const std::string& name, const TypeIdentifier* identifier,
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
                descriptor.mBound.emplace_back(static_cast<uint32_t>(identifier->string_sdefn().bound()));
            }
            else
            {
                descriptor.mBound.emplace_back(identifier->string_ldefn().bound());
            }
            descriptor.mElementType = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Type();
            break;
        }
        case TK_STRING16:
        {
            if (identifier->_d() == TI_STRING16_SMALL)
            {
                descriptor.mBound.emplace_back(static_cast<uint32_t>(identifier->string_sdefn().bound()));
            }
            else
            {
                descriptor.mBound.emplace_back(identifier->string_ldefn().bound());
            }
            descriptor.mElementType = DynamicTypeBuilderFactory::GetInstance()->CreateChar16Type();
            break;
        }
        case TK_SEQUENCE:
        {
            if (identifier->_d() == TI_PLAIN_SEQUENCE_SMALL)
            {
                const TypeIdentifier *aux = TryGetComplete(identifier->seq_sdefn().element_identifier());
                descriptor.mBound.emplace_back(static_cast<uint32_t>(identifier->seq_sdefn().bound()));
                descriptor.mElementType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
            }
            else
            {
                const TypeIdentifier *aux = TryGetComplete(identifier->seq_ldefn().element_identifier());
                descriptor.mBound.emplace_back(identifier->seq_ldefn().bound());
                descriptor.mElementType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
            }
            break;
        }
        case TK_ARRAY:
        {
            if (identifier->_d() == TI_PLAIN_ARRAY_SMALL)
            {
                const TypeIdentifier *aux = TryGetComplete(identifier->array_sdefn().element_identifier());
                for (octet b : identifier->array_sdefn().array_bound_seq())
                {
                    descriptor.mBound.emplace_back(static_cast<uint32_t>(b));
                }
                descriptor.mElementType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
            }
            else
            {
                const TypeIdentifier *aux = identifier->array_ldefn().element_identifier();
                descriptor.mBound = identifier->array_ldefn().array_bound_seq();
                descriptor.mElementType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
            }
            break;
        }
        case TK_MAP:
        {
            if (identifier->_d() == TI_PLAIN_MAP_SMALL)
            {
                const TypeIdentifier *aux = TryGetComplete(identifier->map_sdefn().element_identifier());
                const TypeIdentifier *aux2 = TryGetComplete(identifier->map_sdefn().key_identifier());
                descriptor.mBound.emplace_back(static_cast<uint32_t>(identifier->map_sdefn().bound()));
                descriptor.mElementType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
                descriptor.mKeyElementType = BuildDynamicType(GetTypeName(aux), aux2, GetTypeObject(aux2));
            }
            else
            {
                const TypeIdentifier *aux = TryGetComplete(identifier->map_ldefn().element_identifier());
                const TypeIdentifier *aux2 = TryGetComplete(identifier->map_ldefn().key_identifier());
                descriptor.mBound.emplace_back(identifier->map_ldefn().bound());
                descriptor.mElementType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
                descriptor.mKeyElementType = BuildDynamicType(GetTypeName(aux), aux2, GetTypeObject(aux2));
            }
            break;
        }
        case EK_MINIMAL:
        case EK_COMPLETE:
            // A MinimalTypeObject cannot instantiate a valid TypeDescriptor, but maybe the object isn't minimal
            if (object != nullptr && object->_d() == EK_COMPLETE)
            {
                return BuildDynamicType(descriptor, object);
            }
            break;
        default:
            break;
    }

    DynamicTypeBuilder_ptr outputType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&descriptor);
    //outputType->SetName(name);
    return outputType->Build();
}

// TODO annotations
DynamicType_ptr TypeObjectFactory::BuildDynamicType(TypeDescriptor &descriptor, const TypeObject* object) const
{
    if (object == nullptr || object->_d() != EK_COMPLETE)
    {
        return nullptr;
    }

    // Change descriptor's kind
    descriptor.SetKind(object->complete()._d());

    switch (object->complete()._d())
    {
        // From here, we need TypeObject
        case TK_ALIAS:
        {
            const TypeIdentifier *aux =
                GetStoredTypeIdentifier(&object->complete().alias_type().body().common().related_type());
            descriptor.mBaseType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
            descriptor.SetName(object->complete().alias_type().header().detail().type_name());
            return DynamicTypeBuilderFactory::GetInstance()->CreateType(&descriptor);
        }
        case TK_STRUCTURE:
        {
            const TypeIdentifier *aux = &object->complete().struct_type().header().base_type();
            if (aux->_d() == TK_STRUCTURE)
            {
                descriptor.mBaseType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
            }

            DynamicTypeBuilder_ptr structType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&descriptor);

            //uint32_t order = 0;
            const CompleteStructMemberSeq& structVector = object->complete().struct_type().member_seq();
            for (auto member = structVector.begin(); member != structVector.end(); ++member)
            {
                //const TypeIdentifier *auxMem = &member.common().member_type_id();
                const TypeIdentifier *auxMem = GetStoredTypeIdentifier(&member->common().member_type_id());
                if (auxMem == nullptr)
                {
                    std::cout << "(Struct) auxMem is nullptr, but original member has " << (int)member->common().member_type_id()._d() << std::endl;
                }
                MemberDescriptor memDesc;
                memDesc.mId = member->common().member_id();
                memDesc.SetType(BuildDynamicType(GetTypeName(auxMem), auxMem, GetTypeObject(auxMem)));
                //memDesc.SetIndex(order++);
                memDesc.SetName(member->detail().name());
                structType->AddMember(&memDesc);
            }
            return structType->Build();
        }
        case TK_ENUM:
        {
            DynamicTypeBuilder_ptr enumType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&descriptor);

            uint32_t order = 0;
            const CompleteEnumeratedLiteralSeq& enumVector = object->complete().enumerated_type().literal_seq();
            for (auto member = enumVector.begin(); member != enumVector.end(); ++member)
            {
                enumType->AddEmptyMember(order++, member->detail().name());
            }
            return enumType->Build();
        }
        case TK_BITMASK:
        {
            DynamicTypeBuilder_ptr bitmaskType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&descriptor);

            const CompleteBitflagSeq& seq = object->complete().bitmask_type().flag_seq();
            for (auto member = seq.begin(); member != seq.end(); ++member)
            {
                MemberDescriptor memDesc;
                memDesc.mId = member->common().position();
                memDesc.SetName(member->detail().name());
                bitmaskType->AddMember(&memDesc);
            }
            return bitmaskType->Build();
        }
        case TK_BITSET:
        {
            /*
            const TypeIdentifier *aux = &object->complete().bitset_type().header().base_type();
            if (aux->_d() == TK_BITSET)
            {
                descriptor.mBaseType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));
            }

            DynamicTypeBuilder_ptr bitsetType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&descriptor);

            //uint32_t order = 0;
            const CompleteBitfieldSeq& fields = object->complete().bitset_type().field_seq();
            for (auto member = fields.begin(); member != fields.end(); ++member)
            {
                //const TypeIdentifier *auxMem = &member.common().member_type_id();
                const TypeIdentifier *auxMem = GetPrimitiveTypeIdentifier(member->common().holder_type());
                if (auxMem == nullptr)
                {
                    std::cout << "(Bitset) auxMem is nullptr, but original member has " << (int)member->common().holder_type() << std::endl;
                }
                MemberDescriptor memDesc;
                memDesc.mId = member->common().position();
                memDesc.SetType(BuildDynamicType(GetTypeName(auxMem), auxMem, GetTypeObject(auxMem)));
                memDesc.SetName(member->detail().name());
                // TODO Add bitbound!!
                bitsetType->AddMember(&memDesc);
            }
            return bitsetType->Build();
            */
           logError(XTYPES, "Bitset isn't supported by DynamicType");
           return nullptr;
        }
        case TK_UNION:
        {
            const TypeIdentifier *aux =
                GetStoredTypeIdentifier(&object->complete().union_type().discriminator().common().type_id());
            descriptor.mDiscriminatorType = BuildDynamicType(GetTypeName(aux), aux, GetTypeObject(aux));

            DynamicTypeBuilder_ptr unionType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&descriptor);

            //uint32_t order = 0;
            const CompleteUnionMemberSeq& unionVector = object->complete().union_type().member_seq();
            for (auto member = unionVector.begin(); member != unionVector.end(); ++member)
            {
                const TypeIdentifier *auxMem = GetStoredTypeIdentifier(&member->common().type_id());
                if (auxMem == nullptr)
                {
                    std::cout << "(Union) auxMem is nullptr, but original member has " << (int)member->common().type_id()._d() << std::endl;
                }
                MemberDescriptor memDesc;
                memDesc.SetType(BuildDynamicType(GetTypeName(auxMem), auxMem, GetTypeObject(auxMem)));
                //memDesc.SetIndex(order++);
                memDesc.mId = member->common().member_id();
                memDesc.SetName(member->detail().name());
                memDesc.SetDefaultUnionValue(member->common().member_flags().IS_DEFAULT());
                if (descriptor.mDiscriminatorType->GetKind() == TK_ENUM)
                {
                    DynamicTypeMember enumMember;
                    descriptor.mDiscriminatorType->GetMember(enumMember, memDesc.mId);
                    memDesc.mDefaultValue = enumMember.GetDescriptor()->mName;
                    for (uint32_t lab : member->common().label_seq())
                    {
                        memDesc.AddUnionCaseIndex(lab);
                    }
                }
                else
                {
                    memDesc.mDefaultValue = std::to_string(memDesc.mId);
                    for (uint32_t lab : member->common().label_seq())
                    {
                        memDesc.AddUnionCaseIndex(lab);
                    }
                }
                unionType->AddMember(&memDesc);
            }

            return unionType->Build();
        }
        case TK_ANNOTATION:
        {
            DynamicTypeBuilder_ptr annotationType =
                DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&descriptor);

            for (const CompleteAnnotationParameter &member : object->complete().annotation_type().member_seq())
            {
                const TypeIdentifier *auxMem = GetStoredTypeIdentifier(&member.common().member_type_id());
                if (auxMem == nullptr)
                {
                    std::cout << "(Annotation) auxMem is nullptr, but original member has " << (int)member.common().member_type_id()._d() << std::endl;
                }

                MemberDescriptor memDesc;
                memDesc.SetName(member.name());
                memDesc.SetType(BuildDynamicType(GetTypeName(auxMem), auxMem, GetTypeObject(auxMem)));
                // Member default values?
                //memDesc->default_value(); [...]
                annotationType->AddMember(&memDesc);
            }
            // Annotation inner definitions?

            return annotationType->Build();
        }
        default:
            break;
    }
    return nullptr;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
