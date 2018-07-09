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
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/log/Log.h>

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

} // namespace types
} // namespace fastrtps
} // namespace eprosima
