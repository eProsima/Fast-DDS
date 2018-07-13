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
#include <fastrtps/log/Log.h>
#include <sstream>

namespace eprosima{
namespace fastrtps{
namespace types{

std::string TypeObjectFactory::getStringTypeName(uint32_t bound, bool wide, bool generate_identifier)
{
    std::stringstream type;
    type << ((wide) ? "std::string" : "std::wstring");
    type << ((bound < 256) ? "s_" : "l_") << bound;
    if (generate_identifier) { GetStringIdentifier(bound, wide); }
    return type.str();
}

std::string TypeObjectFactory::getSequenceTypeName(const std::string &type_name, uint32_t bound,
    bool generate_identifier)
{
    std::stringstream auxType;
    auxType << ((bound < 256) ? "sequences_" : "sequencel_");
    auxType << type_name << "_" << bound;
    if (generate_identifier) { GetSequenceIdentifier(type_name, bound); }
    return auxType.str();
}

std::string TypeObjectFactory::getArrayTypeName(const std::string &type_name,
    const std::vector<uint32_t> &bound, bool generate_identifier)
{
    uint32_t unused;
    return getArrayTypeName(type_name, bound, unused, generate_identifier);
}

std::string TypeObjectFactory::getArrayTypeName(const std::string &type_name,
    const std::vector<uint32_t> &bound, uint32_t &ret_size, bool generate_identifier)
{
    std::stringstream auxType;
    std::stringstream auxType2;
    auxType2 << type_name;
    uint32_t size = 0;
    for (uint32_t b : bound)
    {
        auxType2 << "_" << b;
        size += b;
    }
    if (size < 256)
    {
        auxType << "arrays_";
    }
    else
    {
        auxType << "arrayl_";
    }
    auxType << auxType2.str();
    ret_size = size;
    if (generate_identifier) { GetArrayIdentifier(type_name, bound); }
    return auxType.str();
}

std::string TypeObjectFactory::getMapTypeName(const std::string &key_type_name,
    const std::string &value_type_name, uint32_t bound, bool generate_identifier)
{
    std::stringstream auxType;
    auxType << ((bound < 256) ? "maps_" : "mapl_");
    auxType << key_type_name << "_" << value_type_name << "_" << bound;
    if (generate_identifier) { GetMapIdentifier(key_type_name, value_type_name, bound); }
    return auxType.str();
}

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
    std::unique_lock<std::mutex> scoped(m_MutexIdentifiers);
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
}

TypeObjectFactory::~TypeObjectFactory()
{
    /*
    {
        std::unique_lock<std::mutex> scoped(m_MutexIdentifiers);
        auto id_it = m_Identifiers.begin();
        while (id_it != m_Identifiers.end())
        {
            delete (id_it->second);
            ++id_it;
        }
        m_Identifiers.clear();
    }
    {
        std::unique_lock<std::mutex> scoped(m_MutexObjects);
        auto obj_it = m_Objects.begin();
        while (obj_it != m_Objects.end())
        {
            delete (obj_it->second);
            ++obj_it;
        }
        m_Objects.clear();
    }
    */
}

const TypeObject* TypeObjectFactory::GetTypeObject(const std::string &type_name) const
{
    const TypeIdentifier* identifier = GetTypeIdentifier(type_name);
    if (identifier == nullptr)
    {
        return nullptr;
    }

    return GetTypeObject(identifier);
}

const TypeObject* TypeObjectFactory::GetTypeObject(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::mutex> scoped(m_MutexObjects);
    if (m_Objects.find(identifier) != m_Objects.end())
    {
        return m_Objects.at(identifier);
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
    else if (type_name == TKNAME_BYTE)
    {
        return TK_BYTE;
    }
    else if (type_name.find("std::strings_") == 0)
    {
        return TI_STRING8_SMALL;
    }
    else if (type_name.find("std::stringl_") == 0)
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
    else if (type_name == TKNAME_CHAR16)
    {
        return TK_CHAR16;
    }
    else if (type_name.find("std::wstrings_") == 0)
    {
        return TI_STRING16_SMALL;
    }
    else if (type_name.find("std::wstringl_") == 0)
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

const TypeIdentifier* TypeObjectFactory::GetPrimitiveTypeIdentifier(TypeKind kind)
{
    std::string typeName = GetTypeName(kind);
    if (typeName.empty()) return nullptr;
    return GetTypeIdentifier(typeName);
}

/*
const TypeIdentifier* TypeObjectFactory::TryCreateTypeIdentifier(const std::string &type_name)
{
    std::unique_lock<std::mutex> scoped(m_MutexIdentifiers);
    // TODO Makes sense here? I don't think so.
}
*/

const TypeIdentifier* TypeObjectFactory::GetTypeIdentifier(const std::string &type_name) const
{
    std::unique_lock<std::mutex> scoped(m_MutexIdentifiers);
    if (m_Identifiers.find(type_name) != m_Identifiers.end())
    {
        return m_Identifiers.at(type_name);
    }
    return nullptr;
}

void TypeObjectFactory::AddTypeIdentifier(const std::string &type_name, const TypeIdentifier* identifier)
{
    std::unique_lock<std::mutex> scoped(m_MutexIdentifiers);
    m_Identifiers.insert(std::pair<const std::string, const TypeIdentifier*>(type_name, identifier));
}

void TypeObjectFactory::AddTypeObject(const std::string &type_name, const TypeIdentifier* identifier,
        const TypeObject* object)
{
    std::unique_lock<std::mutex> scoped(m_MutexIdentifiers);
    m_Identifiers.insert(std::pair<std::string, const TypeIdentifier*>(type_name, identifier));
    std::unique_lock<std::mutex> scopedObj(m_MutexObjects);
    m_Objects.insert(std::pair<const TypeIdentifier*, const TypeObject*>(identifier, object));
}

const TypeIdentifier* TypeObjectFactory::GetStringIdentifier(uint32_t bound, bool wide)
{
    std::string type = getStringTypeName(bound, wide, false);

    TypeIdentifier* auxIdent;

    if (m_Identifiers.find(type) != m_Identifiers.end())
    {
        return m_Identifiers.at(type);
    }
    else
    {
        auxIdent = new TypeIdentifier;
        if (bound < 256)
        {
            auxIdent->_d(wide ? TI_STRING16_SMALL : TI_STRING8_SMALL);
            auxIdent->string_sdefn().bound(static_cast<octet>(bound));
        }
        else
        {
            auxIdent->_d(wide ? TI_STRING16_LARGE : TI_STRING8_LARGE);
            auxIdent->string_ldefn().bound(bound);
        }
        m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(type, auxIdent));
    }
    return auxIdent;
}

const TypeIdentifier* TypeObjectFactory::GetSequenceIdentifier(const std::string &type_name, uint32_t bound)
{
    std::string auxType = getSequenceTypeName(type_name, bound, false);

    TypeIdentifier* auxIdent;

    if (m_Identifiers.find(auxType) != m_Identifiers.end())
    {
        return m_Identifiers.at(auxType);
    }
    else
    {
        const TypeIdentifier* innerIdent = GetTypeIdentifier(type_name);

        auxIdent = new TypeIdentifier;
        if (bound < 256)
        {
            auxIdent->_d(TI_PLAIN_SEQUENCE_SMALL);
            auxIdent->seq_sdefn().bound(static_cast<octet>(bound));
            auxIdent->seq_sdefn().element_identifier(innerIdent);
            auxIdent->seq_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent->seq_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent->seq_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent->seq_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent->seq_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->seq_sdefn().header().element_flags().IS_KEY(false);
            auxIdent->seq_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent->seq_sdefn().header().equiv_kind(GetTypeKind(type_name));
        }
        else
        {
            auxIdent->_d(TI_PLAIN_SEQUENCE_LARGE);
            auxIdent->seq_ldefn().bound(bound);
            auxIdent->seq_ldefn().element_identifier(innerIdent);
            auxIdent->seq_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent->seq_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent->seq_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent->seq_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent->seq_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->seq_ldefn().header().element_flags().IS_KEY(false);
            auxIdent->seq_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent->seq_ldefn().header().equiv_kind(GetTypeKind(type_name));
        }
        m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
    }

    return auxIdent;
}

const TypeIdentifier* TypeObjectFactory::GetArrayIdentifier(const std::string &type_name, const std::vector<uint32_t> &bound)
{
    uint32_t size;
    std::string auxType = getArrayTypeName(type_name, bound, size, false);

    TypeIdentifier* auxIdent;

    if (m_Identifiers.find(auxType) != m_Identifiers.end())
    {
        return m_Identifiers.at(auxType);
    }
    else
    {
        const TypeIdentifier* innerIdent = GetTypeIdentifier(type_name);

        auxIdent = new TypeIdentifier;
        if (size < 256)
        {
            auxIdent->_d(TI_PLAIN_ARRAY_SMALL);
            for (uint32_t b : bound)
            {
                auxIdent->array_sdefn().array_bound_seq().push_back(static_cast<octet>(b));
            }
            auxIdent->array_sdefn().element_identifier(innerIdent);
            auxIdent->array_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent->array_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent->array_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent->array_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent->array_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->array_sdefn().header().element_flags().IS_KEY(false);
            auxIdent->array_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent->array_sdefn().header().equiv_kind(GetTypeKind(type_name));
        }
        else
        {
            auxIdent->_d(TI_PLAIN_ARRAY_LARGE);
            for (uint32_t b : bound)
            {
                auxIdent->array_ldefn().array_bound_seq().push_back(b);
            }
            auxIdent->array_ldefn().element_identifier(innerIdent);
            auxIdent->array_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent->array_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent->array_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent->array_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent->array_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->array_ldefn().header().element_flags().IS_KEY(false);
            auxIdent->array_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent->array_ldefn().header().equiv_kind(GetTypeKind(type_name));
        }
        m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
    }

    return auxIdent;
}

const TypeIdentifier* TypeObjectFactory::GetMapIdentifier(const std::string &key_type_name,
    const std::string &value_type_name, uint32_t bound)
{
    std::string auxType = getMapTypeName(key_type_name, value_type_name, bound, false);

    TypeIdentifier* auxIdent;

    if (m_Identifiers.find(auxType) != m_Identifiers.end())
    {
        return m_Identifiers.at(auxType);
    }
    else
    {
        const TypeIdentifier* keyIdent = GetTypeIdentifier(key_type_name);
        const TypeIdentifier* valIdent = GetTypeIdentifier(value_type_name);

        auxIdent = new TypeIdentifier;
        if (bound < 256)
        {
            auxIdent->_d(TI_PLAIN_MAP_SMALL);
            auxIdent->map_sdefn().bound(static_cast<octet>(bound));
            auxIdent->map_sdefn().element_identifier(valIdent);
            auxIdent->map_sdefn().key_identifier(keyIdent);
            auxIdent->map_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent->map_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent->map_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent->map_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent->map_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->map_sdefn().header().element_flags().IS_KEY(false);
            auxIdent->map_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent->map_sdefn().key_flags().TRY_CONSTRUCT1(false);
            auxIdent->map_sdefn().key_flags().TRY_CONSTRUCT2(false);
            auxIdent->map_sdefn().key_flags().IS_EXTERNAL(false);
            auxIdent->map_sdefn().key_flags().IS_OPTIONAL(false);
            auxIdent->map_sdefn().key_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->map_sdefn().key_flags().IS_KEY(false);
            auxIdent->map_sdefn().key_flags().IS_DEFAULT(false);
            auxIdent->map_sdefn().header().equiv_kind(GetTypeKind(value_type_name));
        }
        else
        {
            auxIdent->_d(TI_PLAIN_MAP_LARGE);
            auxIdent->map_ldefn().bound(bound);
            auxIdent->map_ldefn().element_identifier(valIdent);
            auxIdent->map_ldefn().key_identifier(keyIdent);
            auxIdent->map_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent->map_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent->map_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent->map_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent->map_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->map_ldefn().header().element_flags().IS_KEY(false);
            auxIdent->map_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent->map_ldefn().key_flags().TRY_CONSTRUCT1(false);
            auxIdent->map_ldefn().key_flags().TRY_CONSTRUCT2(false);
            auxIdent->map_ldefn().key_flags().IS_EXTERNAL(false);
            auxIdent->map_ldefn().key_flags().IS_OPTIONAL(false);
            auxIdent->map_ldefn().key_flags().IS_MUST_UNDERSTAND(false);
            auxIdent->map_ldefn().key_flags().IS_KEY(false);
            auxIdent->map_ldefn().key_flags().IS_DEFAULT(false);
            auxIdent->map_ldefn().header().equiv_kind(GetTypeKind(value_type_name));
        }
        m_Identifiers.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
    }

    return auxIdent;
}

static uint32_t s_typeNameCounter = 0;
static std::string GenerateTypeName(const std::string &kind)
{
    return kind + "_" + std::to_string(++s_typeNameCounter);
}

static TypeKind GetTypeKindFromIdentifier(const TypeIdentifier* identifier)
{

    switch(identifier->_d())
    {
        case TI_STRING8_SMALL:
        case TI_STRING8_LARGE:
            return TK_STRING8;
            break;
        case TI_STRING16_SMALL:
        case TI_STRING16_LARGE:
            return TK_STRING16;
            break;
        case TI_PLAIN_SEQUENCE_SMALL:
        case TI_PLAIN_SEQUENCE_LARGE:
            return TK_SEQUENCE;
            break;
        case TI_PLAIN_ARRAY_SMALL:
        case TI_PLAIN_ARRAY_LARGE:
            return TK_ARRAY;
            break;
        case TI_PLAIN_MAP_SMALL:
        case TI_PLAIN_MAP_LARGE:
            return TK_MAP;
            break;
        case TI_STRONGLY_CONNECTED_COMPONENT:
        case EK_COMPLETE:
            return TK_NONE;
            break;
        case EK_MINIMAL:
                return identifier->_d();
            break;
        default:
            return identifier->_d();
            break;
    }
}

//TypeDescriptor* TypeObjectFactory::BuildTypeDescriptorFromObject(TypeDescriptor* descriptor,
//    const TypeObject* object) const
//{
//    if (descriptor->mKind != object->_d())
//    {
//        logError(TYPE_OBJECT_FACTORY, "TypeDescriptor doesn't correspond with TypeObject.");
//        return descriptor;
//    }
//
//    switch(descriptor->mKind)
//    {
//        case EK_MINIMAL:
//            //TODO: BuildTypeDescriptorFromMinimalObject(descriptor, object->minimal());
//            break;
//        case EK_COMPLETE:
//            // TODO
//            break;
//    }
//
//    return descriptor;
//}

//void TypeObjectFactory::BuildTypeDescriptorFromMinimalObject(
//        TypeDescriptor* descriptor, const MinimalTypeObject &minimal) const
//{
//    descriptor->mKind = minimal._d(); // Must ignore "EK_MINIMAL" kind and use the inner kind
//    switch(minimal._d())
//    {
//        case TK_ALIAS:
//        {
//            const TypeIdentifier *aux = &minimal.alias_type().body().common().related_type();
//            descriptor->mBaseType = BuildDynamicType(aux, GetTypeObject(aux));
//            break;
//        }
//        case TK_STRUCTURE:
//        {
//            const TypeIdentifier *aux = &minimal.struct_type().header().base_type();
//            descriptor->mBaseType = BuildDynamicType(aux, GetTypeObject(aux));
//            uint32_t order = 0;
//            for (MinimalStructMember &member : minimal.struct_type().member_seq())
//            {
//                const TypeIdentifier *auxMem = &member.common().member_type_id();
//                MemberDescriptor *memDesc = new MemberDescriptor();
//                memDesc->mId = member.common().member_id();
//                memDesc->SetType(BuildDynamicType(auxMem, GetTypeObject(auxMem)));
//                memDesc->SetIndex(order++);
//                memDesc->SetName(GenerateTypeName(GetTypeName(GetTypeKindFromIdentifier(auxMem))));
//                memDesc->mType->mDescriptor->mBaseType = DynamicTypeBuilderFactory::GetInstance()->BuildType(descriptor);
//                // TODO descriptor->AddMember(memDesc);
//            }
//            break;
//        }
//        case TK_ENUM:
//        {
//            uint32_t order = 0;
//            for (MinimalEnumeratedLiteral &member : minimal.enumerated_type().literal_seq())
//            {
//                const TypeIdentifier *auxMem = GetTypeIdentifier("uint32_t");
//                MemberDescriptor *memDesc = new MemberDescriptor();
//                memDesc->SetType(BuildDynamicType(auxMem));
//                memDesc->SetIndex(order++);
//                std::stringstream ss;
//                ss << member.detail().name_hash()[0];
//                ss << member.detail().name_hash()[1];
//                ss << member.detail().name_hash()[2];
//                ss << member.detail().name_hash()[3];
//                memDesc->SetName(ss.str());
//                memDesc->mDefaultValue = std::to_string(memDesc->mIndex);
//                memDesc->mType->mDescriptor->mBaseType = DynamicTypeBuilderFactory::GetInstance()->BuildType(descriptor);
//                // TODO descriptor->AddMember(memDesc);
//            }
//            break;
//        }
//        case TK_BITMASK:
//        {
//            // TODO To implement
//            break;
//        }
//        case TK_BITSET:
//        {
//            // TODO To implement
//            break;
//        }
//        case TK_UNION:
//        {
//            const TypeIdentifier *aux  = &minimal.union_type().discriminator().common().type_id();
//            descriptor->mDiscriminatorType = BuildDynamicType(aux, GetTypeObject(aux));
//
//            uint32_t order = 0;
//            for (MinimalUnionMember &member : minimal.union_type().member_seq())
//            {
//                const TypeIdentifier *auxMem = &member.common().type_id();
//                MemberDescriptor *memDesc = new MemberDescriptor();
//                memDesc->SetType(BuildDynamicType(auxMem, GetTypeObject(auxMem)));
//                memDesc->SetIndex(order++);
//                memDesc->mId = member.common().member_id();
//                memDesc->SetName(GenerateTypeName(GetTypeName(GetTypeKindFromIdentifier(auxMem))));
//                memDesc->SetDefaultUnionValue(member.common().member_flags().IS_DEFAULT());
//                memDesc->mDefaultValue = std::to_string(memDesc->mIndex);
//                for (uint32_t lab : member.common().label_seq())
//                {
//                    memDesc->AddUnionCaseIndex(lab);
//                }
//                memDesc->mType->mDescriptor->mBaseType = DynamicTypeBuilderFactory::GetInstance()->BuildType(descriptor);
//                // TODO descriptor->AddMember(memDesc);
//            }
//            break;
//        }
//        case TK_ANNOTATION:
//        {
//            // TODO To implement
//            /*
//            uint64_t order = 0;
//            for (MinimalAnnotationParameter &member : minimal.annotation_type().member_seq())
//            {
//                const TypeIdentifier *auxMem = &member.common().type_id();
//
//                MemberDescriptor *memDesc = new MemberDescriptor();
//                memDesc->SetName(member.name());
//                memDesc->default_value(); [...]
//                // Copy pasted...
//                memDesc->SetType(BuildDynamicType(auxMem, GetTypeObject(auxMem)));
//                memDesc->SetIndex(order++);
//                memDesc->mId = member.common().member_id();
//                memDesc->SetDefaultUnionValue(member.common().member_flags().IS_DEFAULT());
//                memDesc->mDefaultValue = std::to_string(memDesc->mIndex);
//                for (uint32_t lab : member.common().label_seq())
//                {
//                    memDesc->AddUnionCaseIndex(lab);
//                }
//                memDesc->mType->mDescriptor->mBaseType = descriptor;
//            }
//            */
//            break;
//        }
//        // Impossible cases
//        case TK_STRING8:
//        case TK_STRING16:
//        case TK_SEQUENCE:
//        case TK_ARRAY:
//        case TK_MAP:
//        case EK_COMPLETE:
//        case EK_MINIMAL:
//        default:
//            descriptor->mKind = TK_NONE;
//            break;
//    }
//}

DynamicType* TypeObjectFactory::BuildDynamicType(const std::string& name, const TypeIdentifier* identifier,
    const TypeObject* object) const
{
    DynamicType* outputType = BuildDynamicType(identifier, object);
    if (outputType != nullptr)
    {
        outputType->SetName(name);
    }
    return outputType;
}

DynamicType* TypeObjectFactory::BuildDynamicType(const TypeIdentifier* identifier, const TypeObject* object) const
{
    TypeKind kind = GetTypeKindFromIdentifier(identifier);
    TypeDescriptor descriptor = new TypeDescriptor(GenerateTypeName(GetTypeName(kind)), kind);
    switch (kind)
    {
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
        break;
    }
    case TK_SEQUENCE:
    {
        if (identifier->_d() == TI_PLAIN_SEQUENCE_SMALL)
        {
            const TypeIdentifier *aux = identifier->seq_sdefn().element_identifier();
            descriptor.mBound.emplace_back(static_cast<uint32_t>(identifier->seq_sdefn().bound()));
            descriptor.mElementType = BuildDynamicType(aux, GetTypeObject(aux));
        }
        else
        {
            const TypeIdentifier *aux = identifier->seq_ldefn().element_identifier();
            descriptor.mBound.emplace_back(identifier->seq_ldefn().bound());
            descriptor.mElementType = BuildDynamicType(aux, GetTypeObject(aux));
        }
        break;
    }
    case TK_ARRAY:
    {
        if (identifier->_d() == TI_PLAIN_ARRAY_SMALL)
        {
            const TypeIdentifier *aux = identifier->array_sdefn().element_identifier();
            for (octet b : identifier->array_sdefn().array_bound_seq())
            {
                descriptor.mBound.emplace_back(static_cast<uint32_t>(b));
            }
            descriptor.mElementType = BuildDynamicType(aux, GetTypeObject(aux));
        }
        else
        {
            const TypeIdentifier *aux = identifier->array_ldefn().element_identifier();
            descriptor.mBound = identifier->array_ldefn().array_bound_seq();
            descriptor.mElementType = BuildDynamicType(aux, GetTypeObject(aux));
        }
        break;
    }
    case TK_MAP:
    {
        if (identifier->_d() == TI_PLAIN_MAP_SMALL)
        {
            const TypeIdentifier *aux = identifier->map_sdefn().element_identifier();
            const TypeIdentifier *aux2 = identifier->map_sdefn().key_identifier();
            descriptor.mBound.emplace_back(static_cast<uint32_t>(identifier->map_sdefn().bound()));
            descriptor.mElementType = BuildDynamicType(aux, GetTypeObject(aux));
            descriptor.mKeyElementType = BuildDynamicType(aux2, GetTypeObject(aux2));
        }
        else
        {
            const TypeIdentifier *aux = identifier->map_ldefn().element_identifier();
            const TypeIdentifier *aux2 = identifier->map_ldefn().key_identifier();
            descriptor.mBound.emplace_back(identifier->map_ldefn().bound());
            descriptor.mElementType = BuildDynamicType(aux, GetTypeObject(aux));
            descriptor.mKeyElementType = BuildDynamicType(aux2, GetTypeObject(aux2));
        }
        break;
    }
    // From here, we need TypeObject
    case TK_ALIAS:
    {
        if (object != nullptr)
        {
            const TypeIdentifier *aux = &object->minimal().alias_type().body().common().related_type();
            descriptor.mBaseType = BuildDynamicType(aux, GetTypeObject(aux));
        }
        break;
    }
    case TK_STRUCTURE:
    {
        if (object != nullptr)
        {
            const TypeIdentifier *aux = &object->minimal().struct_type().header().base_type();
            descriptor.mBaseType = BuildDynamicType(aux, GetTypeObject(aux));

            DynamicTypeBuilder* structType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomType(&descriptor);

            uint32_t order = 0;
            for (MinimalStructMember &member : object->minimal().struct_type().member_seq())
            {
                const TypeIdentifier *auxMem = &member.common().member_type_id();
                MemberDescriptor memDesc;
                memDesc.mId = member.common().member_id();
                memDesc.SetType(BuildDynamicType(auxMem, GetTypeObject(auxMem)));
                memDesc.SetIndex(order++);
                memDesc.SetName(GenerateTypeName(GetTypeName(GetTypeKindFromIdentifier(auxMem))));
                structType->AddMember(&memDesc);
            }
            return structType;
        }
        break;
    }
    case TK_ENUM:
    {
        if (object != nullptr)
        {
            DynamicTypeBuilder* enumType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomType(&descriptor);

            uint32_t order = 0;
            for (MinimalEnumeratedLiteral &member : object->minimal().enumerated_type().literal_seq())
            {
                std::stringstream ss;
                ss << member.detail().name_hash()[0];
                ss << member.detail().name_hash()[1];
                ss << member.detail().name_hash()[2];
                ss << member.detail().name_hash()[3];
                enumType->AddEmptyMember(order++, ss.str());
            }
            return enumType;

        }
        break;
    }
    case TK_BITMASK:
    {
        if (object != nullptr)
        {
            // TODO To implement
        }
        break;
    }
    case TK_BITSET:
    {
        if (object != nullptr)
        {
            // TODO To implement
        }
        break;
    }
    case TK_UNION:
    {
        if (object != nullptr)
        {
            const TypeIdentifier *aux = &object->minimal().union_type().discriminator().common().type_id();
            descriptor.mDiscriminatorType = BuildDynamicType(aux, GetTypeObject(aux));

            DynamicTypeBuilder* unionType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomType(&descriptor);

            uint32_t order = 0;
            for (MinimalUnionMember &member : object->minimal().union_type().member_seq())
            {
                const TypeIdentifier *auxMem = &member.common().type_id();
                MemberDescriptor memDesc;
                memDesc.SetType(BuildDynamicType(auxMem, GetTypeObject(auxMem)));
                memDesc.SetIndex(order++);
                memDesc.mId = member.common().member_id();
                memDesc.SetName(GenerateTypeName(GetTypeName(GetTypeKindFromIdentifier(auxMem))));
                memDesc.SetDefaultUnionValue(member.common().member_flags().IS_DEFAULT());
                memDesc.mDefaultValue = std::to_string(memDesc.mIndex);
                for (uint32_t lab : member.common().label_seq())
                {
                    memDesc.AddUnionCaseIndex(lab);
                }
                unionType->AddMember(&memDesc);
            }

            return unionType;
        }
        break;
    }
    case TK_ANNOTATION:
    {
        // TODO To implement

        if (object != nullptr)
        {
            /*
            uint64_t order = 0;
            for (MinimalAnnotationParameter &member : object->minimal().annotation_type().member_seq())
            {
            const TypeIdentifier *auxMem = &member.common().type_id();

            MemberDescriptor *memDesc = new MemberDescriptor();
            memDesc->SetName(member.name());
            memDesc->default_value(); [...]
            // Copy pasted...
            memDesc->SetType(BuildDynamicType(auxMem, GetTypeObject(auxMem)));
            memDesc->SetIndex(order++);
            memDesc->mId = member.common().member_id();
            memDesc->SetDefaultUnionValue(member.common().member_flags().IS_DEFAULT());
            memDesc->mDefaultValue = std::to_string(memDesc->mIndex);
            for (uint32_t lab : member.common().label_seq())
            {
            memDesc->AddUnionCaseIndex(lab);
            }
            memDesc->mType->mDescriptor->mBaseType = descriptor;
            }
            */
        }
        break;
    }
    case EK_COMPLETE:
    case EK_MINIMAL:
        if (object != nullptr)
        {
            //TODO: TO IMPLEMENT FOR EK_COMPLETE, MINIMALS ONLY CAN BE USED TO CHECK THE INTEGRITY
            //BuildTypeDescriptorFromObject(descriptor, object);
        }
        break;
    default:
        break;
    }

    DynamicTypeBuilder* outputType = DynamicTypeBuilderFactory::GetInstance()->CreateCustomType(&descriptor);
    return outputType;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima

