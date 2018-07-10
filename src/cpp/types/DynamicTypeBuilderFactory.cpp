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

#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/TypeObject.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/log/Log.h>

#include <fastrtps/rtps/common/SerializedPayload.h>
#include <fastrtps/utils/md5.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastrtps {
namespace types {

static std::string GetTypeName(TypeKind kind)
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

        case TK_ALIAS: return TKNAME_ALIAS;
        case TK_ENUM: return TKNAME_ENUM;
        case TK_BITMASK: return TKNAME_BITMASK;
        case TK_ANNOTATION: return TKNAME_ANNOTATION;
        case TK_STRUCTURE: return TKNAME_STRUCTURE;
        case TK_UNION: return TKNAME_UNION;
        case TK_BITSET: return TKNAME_BITSET;
        case TK_SEQUENCE: return TKNAME_SEQUENCE;
        case TK_ARRAY: return TKNAME_ARRAY;
        case TK_MAP: return TKNAME_MAP;

        default:
            break;
    }
    return "UNDEF";
}

static uint32_t s_typeNameCounter = 0;
static std::string GenerateTypeName(const std::string &kind)
{
    std::string tempKind = kind;
    std::replace(tempKind.begin(), tempKind.end(), ' ', '_');
    return tempKind + "_" + std::to_string(++s_typeNameCounter);
}

static DynamicTypeBuilderFactory* g_instance = nullptr;
DynamicTypeBuilderFactory* DynamicTypeBuilderFactory::GetInstance()
{
    if (g_instance == nullptr)
    {
        g_instance = new DynamicTypeBuilderFactory();
    }
    return g_instance;
}

ResponseCode DynamicTypeBuilderFactory::DeleteInstance()
{
    if (g_instance != nullptr)
    {
        delete g_instance;
        g_instance = nullptr;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_ERROR;
}

DynamicTypeBuilderFactory::DynamicTypeBuilderFactory()
{
}

DynamicTypeBuilderFactory::~DynamicTypeBuilderFactory()
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    for (auto it = mTypesList.begin(); it != mTypesList.end(); ++it)
    {
        delete *it;
    }
    mTypesList.clear();
#endif
}

void DynamicTypeBuilderFactory::AddTypeToList(DynamicType* pType)
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pType);
#endif
}

DynamicType* DynamicTypeBuilderFactory::BuildType(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        DynamicType* pNewType = new DynamicType(descriptor);
        AddTypeToList(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type, invalid input descriptor");
        return nullptr;
    }
}

DynamicType* DynamicTypeBuilderFactory::BuildType(const DynamicType* other)
{
    if (other != nullptr)
    {
        DynamicType* pNewType = new DynamicType(other);
        AddTypeToList(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type, invalid input parameter");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateAliasType(DynamicType* base_type, const std::string& sName)
{
    if (base_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ALIAS;
        pDescriptor.mBaseType = BuildType(base_type);
        if (sName.length() > 0)
        {
            pDescriptor.mName = sName;
        }
        else
        {
            pDescriptor.mName = GenerateTypeName(GetTypeName(TK_ALIAS));
        }

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateArrayType(const DynamicType* element_type,
    std::vector<uint32_t> bounds)
{
    if (element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ARRAY;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_ARRAY));
        pDescriptor.mBound = bounds;
        pDescriptor.mElementType = BuildType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating array, element_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateBitmaskType(uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pBoolDescriptor;
        pBoolDescriptor.mKind = TK_BOOLEAN;
        pBoolDescriptor.mName = GenerateTypeName(GetTypeName(TK_BOOLEAN));

        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_BITMASK;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_BITMASK));
        pDescriptor.mElementType = BuildType(&pBoolDescriptor);
        pDescriptor.mBound.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateBitsetType(uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_BITSET;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_BITSET));
        pDescriptor.mBound.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateBoolType()
{
    TypeDescriptor pBoolDescriptor;
    pBoolDescriptor.mKind = TK_BOOLEAN;
    pBoolDescriptor.mName = GenerateTypeName(GetTypeName(TK_BOOLEAN));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pBoolDescriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateByteType()
{
    TypeDescriptor pByteDescriptor;
    pByteDescriptor.mKind = TK_BYTE;
    pByteDescriptor.mName = GenerateTypeName(GetTypeName(TK_BYTE));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pByteDescriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChar8Type()
{
    TypeDescriptor pChar8Descriptor;
    pChar8Descriptor.mKind = TK_CHAR8;
    pChar8Descriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR8));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar8Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChar16Type()
{
    TypeDescriptor pChar16Descriptor;
    pChar16Descriptor.mKind = TK_CHAR16;
    pChar16Descriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar16Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateEnumType()
{
    TypeDescriptor pEnumDescriptor;
    pEnumDescriptor.mKind = TK_ENUM;
    pEnumDescriptor.mName = GenerateTypeName(GetTypeName(TK_ENUM));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pEnumDescriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat32Type()
{
    TypeDescriptor pFloat32Descriptor;
    pFloat32Descriptor.mKind = TK_FLOAT32;
    pFloat32Descriptor.mName = GenerateTypeName(GetTypeName(TK_FLOAT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat32Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat64Type()
{
    TypeDescriptor pFloat64Descriptor;
    pFloat64Descriptor.mKind = TK_FLOAT64;
    pFloat64Descriptor.mName = GenerateTypeName(GetTypeName(TK_FLOAT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat64Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat128Type()
{
    TypeDescriptor pFloat128Descriptor;
    pFloat128Descriptor.mKind = TK_FLOAT128;
    pFloat128Descriptor.mName = GenerateTypeName(GetTypeName(TK_FLOAT128));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat128Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt16Type()
{
    TypeDescriptor pInt16Descriptor;
    pInt16Descriptor.mKind = TK_INT16;
    pInt16Descriptor.mName = GenerateTypeName(GetTypeName(TK_INT16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt16Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt32Type()
{
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.mKind = TK_INT32;
    pInt32Descriptor.mName = GenerateTypeName(GetTypeName(TK_INT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt32Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt64Type()
{
    TypeDescriptor pInt64Descriptor;
    pInt64Descriptor.mKind = TK_INT64;
    pInt64Descriptor.mName = GenerateTypeName(GetTypeName(TK_INT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt64Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateMapType(DynamicType* key_element_type,
    DynamicType* element_type, uint32_t bound)
{
    if (key_element_type != nullptr && element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_MAP;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_MAP));
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mKeyElementType = BuildType(key_element_type);
        pDescriptor.mElementType = BuildType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating map, element_type and key_element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateSequenceType(const DynamicType* element_type, uint32_t bound)
{
    if (element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_SEQUENCE;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_SEQUENCE));
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mElementType = BuildType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating sequence, element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateStringType(uint32_t bound)
{
    TypeDescriptor pCharDescriptor;
    pCharDescriptor.mKind = TK_CHAR8;
    pCharDescriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR8));

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING8;
    pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRING8));
    pDescriptor.mElementType = BuildType(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChildStructType(DynamicType* parent_type)
{
    if (parent_type->GetKind() == TK_STRUCTURE)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_STRUCTURE;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRUCTURE));
        pDescriptor.mBaseType = BuildType(parent_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating child struct, invalid input type.");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateStructType()
{
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRUCTURE;
    pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRUCTURE));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateType(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(descriptor);
        AddTypeToList(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error creating type, invalid input descriptor.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateTypeCopy(const DynamicType* type)
{
    if (type != nullptr)
    {
        DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(type);
        AddTypeToList(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error creating type, invalid input type.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint16Type()
{
    TypeDescriptor pUInt16Descriptor;
    pUInt16Descriptor.mKind = TK_UINT16;
    pUInt16Descriptor.mName = GenerateTypeName(GetTypeName(TK_UINT16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt16Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint32Type()
{
    TypeDescriptor pUInt32Descriptor;
    pUInt32Descriptor.mKind = TK_UINT32;
    pUInt32Descriptor.mName = GenerateTypeName(GetTypeName(TK_UINT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt32Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint64Type()
{
    TypeDescriptor pUInt64Descriptor;
    pUInt64Descriptor.mKind = TK_UINT64;
    pUInt64Descriptor.mName = GenerateTypeName(GetTypeName(TK_UINT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt64Descriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUnionType(DynamicType* discriminator_type)
{
    if (discriminator_type != nullptr && discriminator_type->IsDiscriminatorType())
    {
        TypeDescriptor pUnionDescriptor;
        pUnionDescriptor.mKind = TK_UNION;
        pUnionDescriptor.mName = GenerateTypeName(GetTypeName(TK_UNION));
        pUnionDescriptor.mDiscriminatorType = BuildType(discriminator_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUnionDescriptor);
        AddTypeToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error building Union, invalid discriminator type");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateWstringType(uint32_t bound)
{
    TypeDescriptor pCharDescriptor;
    pCharDescriptor.mKind = TK_CHAR16;
    pCharDescriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR16));

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING16;
    pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRING16));
    pDescriptor.mElementType = BuildType(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    AddTypeToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_type_object(TypeObject type_object)
//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_uri(const std::string& document_url, const std::string& type_name, IncludePathSeq include_paths);
//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_document(const std::string& document, const std::string& type_name, IncludePathSeq include_paths);

ResponseCode DynamicTypeBuilderFactory::DeleteType(DynamicType* type)
{
    if (type != nullptr)
    {
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        auto it = std::find(mTypesList.begin(), mTypesList.end(), type);
        if (it != mTypesList.end())
        {
            mTypesList.erase(it);
            delete type;
        }
        else
        {
            logWarning(DYN_TYPES, "The given type has been deleted previously.");
            return ResponseCode::RETCODE_ALREADY_DELETED;
        }
#else
        delete type;
#endif
    }
    return ResponseCode::RETCODE_OK;
}

DynamicType* DynamicTypeBuilderFactory::GetPrimitiveType(TypeKind kind)
{
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = kind;
    pDescriptor.mName = GenerateTypeName(GetTypeName(kind));
    DynamicType* pNewType = BuildType(&pDescriptor);
    return pNewType;
}

bool DynamicTypeBuilderFactory::IsEmpty() const
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    return mTypesList.empty();
#else
    return true;
#endif
}


void DynamicTypeBuilderFactory::BuildTypeIdentifier(const TypeDescriptor* descriptor, TypeIdentifier& identifier) const
{
    const TypeIdentifier *id2 = TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->GetName());
    if (id2 != nullptr)
    {
        identifier = *id2;
    }
    else
    {
        switch(descriptor->mKind)
        {
            // Basic types
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
                {
                    identifier._d(descriptor->mKind);
                }
                break;
            // String TKs
            case TK_STRING8:
                {
                    if (descriptor->mBound[0] < 256)
                    {
                        identifier._d(TI_STRING8_SMALL);
                        identifier.string_sdefn().bound(static_cast<SBound>(descriptor->mBound[0]));
                    }
                    else
                    {
                        identifier._d(TI_STRING8_LARGE);
                        identifier.string_ldefn().bound(descriptor->mBound[0]);
                    }
                }
                break;
            case TK_STRING16:
                {
                    if (descriptor->mBound[0] < 256)
                    {
                        identifier._d(TI_STRING16_SMALL);
                        identifier.string_sdefn().bound(static_cast<SBound>(descriptor->mBound[0]));
                    }
                    else
                    {
                        identifier._d(TI_STRING16_LARGE);
                        identifier.string_ldefn().bound(descriptor->mBound[0]);
                    }
                }
                break;
            // Collection TKs
            case TK_SEQUENCE:
                {
                    if (descriptor->mBound[0] < 256)
                    {
                        identifier._d(TI_PLAIN_SEQUENCE_SMALL);
                        identifier.seq_sdefn().bound(static_cast<SBound>(descriptor->mBound[0]));
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id);
                        identifier.seq_sdefn().element_identifier(&elem_id);
                    }
                    else
                    {
                        identifier._d(TI_PLAIN_SEQUENCE_LARGE);
                        identifier.seq_ldefn().bound(descriptor->mBound[0]);
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id);
                        identifier.seq_ldefn().element_identifier(&elem_id);
                    }
                }
                break;
            case TK_ARRAY:
                {
                    uint32_t size = 0;
                    for (uint32_t s : descriptor->mBound)
                    {
                        size += s;
                    }

                    if (size < 256)
                    {
                        identifier._d(TI_PLAIN_ARRAY_SMALL);
                        for (uint32_t b : descriptor->mBound)
                        {
                            identifier.array_sdefn().array_bound_seq().emplace_back(static_cast<SBound>(b));
                        }
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id);
                        identifier.array_sdefn().element_identifier(&elem_id);
                    }
                    else
                    {
                        identifier._d(TI_PLAIN_ARRAY_LARGE);
                        identifier.array_ldefn().array_bound_seq(descriptor->mBound);
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id);
                        identifier.array_ldefn().element_identifier(&elem_id);
                    }
                }
                break;
            case TK_MAP:
                {
                    if (descriptor->mBound[0] < 256)
                    {
                        identifier._d(TI_PLAIN_MAP_SMALL);
                        identifier.map_sdefn().bound(static_cast<SBound>(descriptor->mBound[0]));
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id);
                        identifier.map_sdefn().element_identifier(&elem_id);
                        TypeIdentifier key_id;
                        BuildTypeIdentifier(descriptor->GetKeyElementType()->mDescriptor, key_id);
                        identifier.map_sdefn().key_identifier(&key_id);
                    }
                    else
                    {
                        identifier._d(TI_PLAIN_MAP_LARGE);
                        identifier.map_ldefn().bound(static_cast<SBound>(descriptor->mBound[0]));
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id);
                        identifier.map_ldefn().element_identifier(&elem_id);
                        TypeIdentifier key_id;
                        BuildTypeIdentifier(descriptor->GetKeyElementType()->mDescriptor, key_id);
                        identifier.map_ldefn().key_identifier(&key_id);
                    }
                }
                break;
            // Constructed/Named types
            case TK_ALIAS:
            // Enumerated TKs
            case TK_ENUM:
            case TK_BITMASK:
            // Structured TKs
            case TK_ANNOTATION:
            case TK_STRUCTURE:
            case TK_UNION:
            case TK_BITSET:
                {
                    // Need to be registered as TypeObject first
                }
                break;
        }

        TypeObjectFactory::GetInstance()->AddTypeIdentifier(descriptor->GetName(), &identifier);
    }
}

void DynamicTypeBuilderFactory::BuildTypeObject(const TypeDescriptor* descriptor, TypeObject &object,
                                                const std::vector<MemberDescriptor*> *members) const
{
    const TypeObject *obj2 = TypeObjectFactory::GetInstance()->GetTypeObject(descriptor->GetName());
    if (obj2 != nullptr)
    {
        object = *obj2;
    }
    else
    {
        switch(descriptor->mKind)
        {
            // Basic types
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
            // String TKs
            case TK_STRING8:
            case TK_STRING16:
            // Collection TKs
            case TK_SEQUENCE:
            case TK_ARRAY:
            case TK_MAP:
                break;

            // Constructed/Named types
            case TK_ALIAS:
                {
                    BuildAliasTypeObject(descriptor, object);
                }
                break;
            // Enumerated TKs
            case TK_ENUM:
                {
                    BuildEnumTypeObject(descriptor, object, *members);
                }
                break;
            case TK_BITMASK:
                {
                    // Not implemented
                }
                break;
            // Structured TKs
            case TK_ANNOTATION:
                {
                    // Not implemented
                }
                break;
            case TK_STRUCTURE:
                {
                    BuildStructTypeObject(descriptor, object, *members);
                }
                break;
            case TK_UNION:
                {
                    BuildUnionTypeObject(descriptor, object, *members);
                }
                break;
            case TK_BITSET:
                {
                    // Not implemented
                }
                break;
        }
    }
}

void DynamicTypeBuilderFactory::BuildAliasTypeObject(const TypeDescriptor* descriptor, TypeObject& object) const
{
    object._d(EK_MINIMAL);
    object.minimal()._d(TK_ALIAS);
    object.minimal().alias_type().alias_flags().IS_FINAL(false);
    object.minimal().alias_type().alias_flags().IS_APPENDABLE(false);
    object.minimal().alias_type().alias_flags().IS_MUTABLE(false);
    object.minimal().alias_type().alias_flags().IS_NESTED(false);
    object.minimal().alias_type().alias_flags().IS_AUTOID_HASH(false);

    object.minimal().alias_type().body().common().related_flags().TRY_CONSTRUCT1(false);
    object.minimal().alias_type().body().common().related_flags().TRY_CONSTRUCT2(false);
    object.minimal().alias_type().body().common().related_flags().IS_EXTERNAL(false);
    object.minimal().alias_type().body().common().related_flags().IS_OPTIONAL(false);
    object.minimal().alias_type().body().common().related_flags().IS_MUST_UNDERSTAND(false);
    object.minimal().alias_type().body().common().related_flags().IS_KEY(false);
    object.minimal().alias_type().body().common().related_flags().IS_DEFAULT(false);

    TypeIdentifier ident;
    BuildTypeIdentifier(descriptor->GetBaseType()->mDescriptor, ident);
    object.minimal().alias_type().body().common().related_type(ident);
    //object.minimal().alias_type().body().common().related_type() =
    //    *(TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->GetBaseType()->GetName()));

    TypeIdentifier *identifier = &object.minimal().alias_type().body().common().related_type();

    TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), identifier, &object);
}

void DynamicTypeBuilderFactory::BuildEnumTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
                                                    const std::vector<MemberDescriptor*> members) const
{
    object._d(EK_MINIMAL);
    object.minimal()._d(TK_ENUM);
    object.minimal().enumerated_type().header().common().bit_bound(32); // TODO fixed by IDL, isn't?

    for (MemberDescriptor* member : members)
    {
        MinimalEnumeratedLiteral mel;
        mel.common().flags().TRY_CONSTRUCT1(false);
        mel.common().flags().TRY_CONSTRUCT2(false);
        mel.common().flags().IS_EXTERNAL(false);
        mel.common().flags().IS_OPTIONAL(false);
        mel.common().flags().IS_MUST_UNDERSTAND(false);
        mel.common().flags().IS_KEY(false);
        mel.common().flags().IS_DEFAULT(false);
        mel.common().value(member->GetIndex());
        MD5 hash(member->GetName());
        for(int i = 0; i < 4; ++i)
        {
            mel.detail().name_hash()[i] = hash.digest[i];
        }
        object.minimal().enumerated_type().literal_seq().emplace_back(mel);
    }

    TypeIdentifier* identifier = new TypeIdentifier();
    identifier->_d(EK_MINIMAL);

    SerializedPayload_t payload(static_cast<uint32_t>(
        MinimalEnumeratedType::getCdrSerializedSize(object.minimal().enumerated_type()) + 4));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);
    // Fixed endian (Page 221, EquivalenceHash definition of Extensible and Dynamic Topic Types for DDS document)
    eprosima::fastcdr::Cdr ser(
        fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
        eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = CDR_LE;

    object.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
    MD5 objectHash;
    objectHash.update((char*)payload.data, payload.length);
    objectHash.finalize();
    for(int i = 0; i < 14; ++i)
    {
        identifier->equivalence_hash()[i] = objectHash.digest[i];
    }

    TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), identifier, &object);
}

void DynamicTypeBuilderFactory::BuildStructTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
                                                    const std::vector<MemberDescriptor*> members) const
{
    object._d(EK_MINIMAL);
    object.minimal()._d(TK_STRUCTURE);

    object.minimal().struct_type().struct_flags().IS_FINAL(false);
    object.minimal().struct_type().struct_flags().IS_APPENDABLE(false);
    object.minimal().struct_type().struct_flags().IS_MUTABLE(false);
    object.minimal().struct_type().struct_flags().IS_NESTED(false);
    object.minimal().struct_type().struct_flags().IS_AUTOID_HASH(false);

    for (MemberDescriptor* member : members)
    {
        MinimalStructMember msm;
        msm.common().member_id(member->GetId());
        msm.common().member_flags().TRY_CONSTRUCT1(false);
        msm.common().member_flags().TRY_CONSTRUCT2(false);
        msm.common().member_flags().IS_EXTERNAL(false);
        msm.common().member_flags().IS_OPTIONAL(false);
        msm.common().member_flags().IS_MUST_UNDERSTAND(false);
        msm.common().member_flags().IS_KEY(false);
        msm.common().member_flags().IS_DEFAULT(false);
        TypeIdentifier memIdent;
        BuildTypeIdentifier(member->mType->mDescriptor, memIdent);
        msm.common().member_type_id(memIdent);
        //msm.common().member_type_id(*TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName()));
        MD5 hash(member->GetName());
        for(int i = 0; i < 4; ++i)
        {
            msm.detail().name_hash()[i] = hash.digest[i];
        }
        object.minimal().struct_type().member_seq().emplace_back(msm);
    }

    object.minimal().struct_type().header().base_type()._d(EK_MINIMAL);


    SerializedPayload_t payload(static_cast<uint32_t>(
        object.minimal().struct_type().member_seq().size() * sizeof(MinimalStructMember) + 4));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = CDR_LE;
    // Serialize encapsulation

    for (MinimalStructMember &st : object.minimal().struct_type().member_seq())
    {
        ser << st;
    }
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
    MD5 objectHash;
    objectHash.update((char*)payload.data, payload.length);
    objectHash.finalize();
    for(int i = 0; i < 14; ++i)
    {
        object.minimal().struct_type().header().base_type().equivalence_hash()[i] = objectHash.digest[i];
    }

    TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(),
        &object.minimal().struct_type().header().base_type(), &object);
}


void DynamicTypeBuilderFactory::BuildUnionTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
                                                    const std::vector<MemberDescriptor*> members) const
{
    object._d(EK_MINIMAL);
    object.minimal()._d(TK_UNION);

    object.minimal().union_type().union_flags().IS_FINAL(false);
    object.minimal().union_type().union_flags().IS_APPENDABLE(false);
    object.minimal().union_type().union_flags().IS_MUTABLE(false);
    object.minimal().union_type().union_flags().IS_NESTED(false);
    object.minimal().union_type().union_flags().IS_AUTOID_HASH(false);

    object.minimal().union_type().discriminator().common().member_flags().TRY_CONSTRUCT1(false);
    object.minimal().union_type().discriminator().common().member_flags().TRY_CONSTRUCT2(false);
    object.minimal().union_type().discriminator().common().member_flags().IS_EXTERNAL(false);
    object.minimal().union_type().discriminator().common().member_flags().IS_OPTIONAL(false);
    object.minimal().union_type().discriminator().common().member_flags().IS_MUST_UNDERSTAND(false);
    object.minimal().union_type().discriminator().common().member_flags().IS_KEY(false);
    object.minimal().union_type().discriminator().common().member_flags().IS_DEFAULT(false);

    TypeIdentifier discIdent;
    BuildTypeIdentifier(descriptor->mDiscriminatorType->mDescriptor, discIdent);
    object.minimal().union_type().discriminator().common().type_id(discIdent);
        //*TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->mDiscriminatorType->GetName()));

    for (MemberDescriptor* member : members)
    {
        MinimalUnionMember mum;
        mum.common().member_id(member->GetId());
        mum.common().member_flags().TRY_CONSTRUCT1(false);
        mum.common().member_flags().TRY_CONSTRUCT2(false);
        mum.common().member_flags().IS_EXTERNAL(false);
        mum.common().member_flags().IS_OPTIONAL(false);
        mum.common().member_flags().IS_MUST_UNDERSTAND(false);
        mum.common().member_flags().IS_KEY(false);
        mum.common().member_flags().IS_DEFAULT(member->IsDefaultUnionValue());

        TypeIdentifier memIdent;
        BuildTypeIdentifier(member->mType->mDescriptor, memIdent);
        mum.common().type_id(memIdent);
        //mum.common().type_id(*TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName()));
        for (uint64_t lab : member->GetUnionLabels())
        {
            mum.common().label_seq().emplace_back(static_cast<uint32_t>(lab));
        }
        MD5 hash(member->GetName());
        for(int i = 0; i < 4; ++i)
        {
            mum.detail().name_hash()[i] = hash.digest[i];
        }
        object.minimal().union_type().member_seq().emplace_back(mum);
    }

    TypeIdentifier* identifier = new TypeIdentifier();
    identifier->_d(EK_MINIMAL);

    SerializedPayload_t payload(static_cast<uint32_t>(
        MinimalUnionType::getCdrSerializedSize(object.minimal().union_type()) + 4));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);
    // Fixed endian (Page 221, EquivalenceHash definition of Extensible and Dynamic Topic Types for DDS document)
    eprosima::fastcdr::Cdr ser(
        fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
        eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = CDR_LE;

    object.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
    MD5 objectHash;
    objectHash.update((char*)payload.data, payload.length);
    objectHash.finalize();
    for(int i = 0; i < 14; ++i)
    {
        identifier->equivalence_hash()[i] = objectHash.digest[i];
    }

    TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), identifier, &object);
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
