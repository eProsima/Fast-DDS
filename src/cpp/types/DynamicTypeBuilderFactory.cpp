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
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/TypeNamesGenerator.h>
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
        /*
        case TK_STRING8: return TKNAME_STRING8;
        case TK_STRING16: return TKNAME_STRING16;
        case TK_ALIAS: return TKNAME_ALIAS;
        case TK_ENUM: return TKNAME_ENUM;
        */
        case TK_BITMASK: return TKNAME_BITMASK;
        /*
        case TK_ANNOTATION: return TKNAME_ANNOTATION;
        case TK_STRUCTURE: return TKNAME_STRUCTURE;
        case TK_UNION: return TKNAME_UNION;
        */
        case TK_BITSET: return TKNAME_BITSET;
        /*
        case TK_SEQUENCE: return TKNAME_SEQUENCE;
        case TK_ARRAY: return TKNAME_ARRAY;
        case TK_MAP: return TKNAME_MAP;
        */
        default:
            break;
    }
    return "UNDEF";
}

//static uint32_t s_typeNameCounter = 0;
static std::string GenerateTypeName(const std::string &kind)
{
    std::string tempKind = kind;
    std::replace(tempKind.begin(), tempKind.end(), ' ', '_');
    return tempKind;// + "_" + std::to_string(++s_typeNameCounter);
}

class DynamicTypeBuilderFactoryReleaser
{
public:
    ~DynamicTypeBuilderFactoryReleaser()
    {
        DynamicTypeBuilderFactory::DeleteInstance();
    }
};

static DynamicTypeBuilderFactoryReleaser s_releaser;
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
    std::unique_lock<std::recursive_mutex> scoped(mMutex);
    for (auto it = mBuildersList.begin(); it != mBuildersList.end(); ++it)
    {
        delete *it;
    }
    mBuildersList.clear();
#endif
}

void DynamicTypeBuilderFactory::AddBuilderToList(DynamicTypeBuilder* pBuilder)
{
    (void)pBuilder;
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::unique_lock<std::recursive_mutex> scoped(mMutex);
    mBuildersList.push_back(pBuilder);
#endif
}

DynamicType_ptr DynamicTypeBuilderFactory::BuildType(DynamicType_ptr other)
{
    return other;
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateType(const TypeDescriptor* descriptor, const std::string& name)
{
    if (descriptor != nullptr)
    {
        DynamicType_ptr pNewType = new DynamicType(descriptor);
        if (name.length() > 0)
        {
            pNewType->SetName(name);
        }
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type, invalid input descriptor");
        return nullptr;
    }
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateType(const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        DynamicType_ptr pNewType = new DynamicType(other);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type, invalid input parameter");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateAliasBuilder(DynamicTypeBuilder* base_type,
    const std::string& sName)
{
    if (base_type != nullptr)
    {
        DynamicType_ptr pType = CreateType(base_type);
        if (pType != nullptr)
        {
            return CreateAliasBuilder(pType, sName);
        }
        else
        {
            logError(DYN_TYPES, "Error creating alias type, Error creating dynamic type");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateAliasBuilder(DynamicType_ptr base_type, const std::string& sName)
{
    if (base_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ALIAS;
        pDescriptor.mBaseType = base_type;
        if (sName.length() > 0)
        {
            pDescriptor.mName = sName;
        }
        else
        {
            //pDescriptor.mName = GenerateTypeName(GetTypeName(TK_ALIAS));
            pDescriptor.mName = base_type->GetName();
        }

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateArrayBuilder(const DynamicTypeBuilder* element_type,
    const std::vector<uint32_t>& bounds)
{
    if (element_type != nullptr)
    {
        DynamicType_ptr pType = CreateType(element_type);
        if (pType != nullptr)
        {
            return CreateArrayBuilder(pType, bounds);
        }
        else
        {
            logError(DYN_TYPES, "Error creating array, error creating dynamic type");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error creating array, element_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateArrayBuilder(const DynamicType_ptr type,
    const std::vector<uint32_t>& bounds)
{
    if (type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ARRAY;
        pDescriptor.mName = TypeNamesGenerator::getArrayTypeName(type->GetName(), bounds, false);
        pDescriptor.mElementType = type;
        pDescriptor.mBound = bounds;

        for (uint32_t i = 0; i < pDescriptor.mBound.size(); ++i)
        {
            if (pDescriptor.mBound[i] == 0)
            {
                pDescriptor.mBound[i] = MAX_ELEMENTS_COUNT;
            }
        }


        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating array, element_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateBitmaskBuilder(uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pBoolDescriptor;
        pBoolDescriptor.mKind = TK_BOOLEAN;
        pBoolDescriptor.mName = GenerateTypeName(GetTypeName(TK_BOOLEAN));

        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_BITMASK;
        // TODO review on implementation for IDL
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_BITMASK));
        pDescriptor.mElementType = CreateType(&pBoolDescriptor);
        pDescriptor.mBound.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateBitsetBuilder(uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_BITSET;
        // TODO Review on implementation for IDL
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_BITSET));
        pDescriptor.mBound.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateBoolBuilder()
{
    TypeDescriptor pBoolDescriptor;
    pBoolDescriptor.mKind = TK_BOOLEAN;
    pBoolDescriptor.mName = GenerateTypeName(GetTypeName(TK_BOOLEAN));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pBoolDescriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateByteBuilder()
{
    TypeDescriptor pByteDescriptor;
    pByteDescriptor.mKind = TK_BYTE;
    pByteDescriptor.mName = GenerateTypeName(GetTypeName(TK_BYTE));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pByteDescriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChar8Builder()
{
    TypeDescriptor pChar8Descriptor;
    pChar8Descriptor.mKind = TK_CHAR8;
    pChar8Descriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR8));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar8Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChar16Builder()
{
    TypeDescriptor pChar16Descriptor;
    pChar16Descriptor.mKind = TK_CHAR16;
    pChar16Descriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar16Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateAnnotationPrimitive()
{
    TypeDescriptor pEnumDescriptor;
    pEnumDescriptor.mKind = TK_ANNOTATION;
    pEnumDescriptor.mName = GenerateTypeName(GetTypeName(TK_ANNOTATION));

    DynamicType_ptr pNewType = new DynamicType(&pEnumDescriptor);
    return pNewType;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateEnumBuilder()
{
    TypeDescriptor pEnumDescriptor;
    pEnumDescriptor.mKind = TK_ENUM;
    //pEnumDescriptor.mName = GenerateTypeName(GetTypeName(TK_ENUM));
    // Enum currently is an alias for uint32_t
    pEnumDescriptor.mName = GenerateTypeName(GetTypeName(TK_UINT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pEnumDescriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat32Builder()
{
    TypeDescriptor pFloat32Descriptor;
    pFloat32Descriptor.mKind = TK_FLOAT32;
    pFloat32Descriptor.mName = GenerateTypeName(GetTypeName(TK_FLOAT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat32Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat64Builder()
{
    TypeDescriptor pFloat64Descriptor;
    pFloat64Descriptor.mKind = TK_FLOAT64;
    pFloat64Descriptor.mName = GenerateTypeName(GetTypeName(TK_FLOAT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat64Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat128Builder()
{
    TypeDescriptor pFloat128Descriptor;
    pFloat128Descriptor.mKind = TK_FLOAT128;
    pFloat128Descriptor.mName = GenerateTypeName(GetTypeName(TK_FLOAT128));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat128Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt16Builder()
{
    TypeDescriptor pInt16Descriptor;
    pInt16Descriptor.mKind = TK_INT16;
    pInt16Descriptor.mName = GenerateTypeName(GetTypeName(TK_INT16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt16Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt32Builder()
{
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.mKind = TK_INT32;
    pInt32Descriptor.mName = GenerateTypeName(GetTypeName(TK_INT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt32Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt64Builder()
{
    TypeDescriptor pInt64Descriptor;
    pInt64Descriptor.mKind = TK_INT64;
    pInt64Descriptor.mName = GenerateTypeName(GetTypeName(TK_INT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt64Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateMapBuilder(DynamicTypeBuilder* key_element_type,
    DynamicTypeBuilder* element_type, uint32_t bound)
{
    if (key_element_type != nullptr && element_type != nullptr)
    {
        DynamicType_ptr pKeyType = CreateType(key_element_type);
        DynamicType_ptr pValueType = CreateType(element_type);
        if (pKeyType != nullptr && pValueType != nullptr)
        {
            return CreateMapBuilder(pKeyType, pValueType, bound);
        }
        else
        {
            logError(DYN_TYPES, "Error creating map, Error creating dynamic types.");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error creating map, element_type and key_element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateMapBuilder(DynamicType_ptr key_type,
    DynamicType_ptr value_type, uint32_t bound)
{
    if (key_type != nullptr && value_type != nullptr)
    {
        if (bound == 0)
        {
            bound = MAX_ELEMENTS_COUNT;
        }

        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_MAP;
        //pDescriptor.mName = GenerateTypeName(GetTypeName(TK_MAP));
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mKeyElementType = key_type;
        pDescriptor.mElementType = value_type;

        pDescriptor.mName = TypeNamesGenerator::getMapTypeName(key_type->GetName(), value_type->GetName(),
            bound, false);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating map, element_type and key_element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateSequenceBuilder(const DynamicTypeBuilder* element_type,
    uint32_t bound)
{
    if (element_type != nullptr)
    {
        DynamicType_ptr pType = CreateType(element_type);
        if (pType != nullptr)
        {
            return CreateSequenceBuilder(pType, bound);
        }
        else
        {
            logError(DYN_TYPES, "Error creating sequence, error creating dynamic type.");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error creating sequence, element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateSequenceBuilder(const DynamicType_ptr type, uint32_t bound)
{
    if (type != nullptr)
    {
        if (bound == 0)
        {
            bound = MAX_ELEMENTS_COUNT;
        }

        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_SEQUENCE;
        pDescriptor.mName = TypeNamesGenerator::getSequenceTypeName(type->GetName(), bound, false);
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mElementType = type;

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating sequence, element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateStringBuilder(uint32_t bound)
{
    if (bound == 0)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor pCharDescriptor;
    pCharDescriptor.mKind = TK_CHAR8;
    pCharDescriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR8));

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING8;
    //pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRING8));
    pDescriptor.mElementType = CreateType(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    pDescriptor.mName = TypeNamesGenerator::getStringTypeName(bound, false, true);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChildStructBuilder(DynamicTypeBuilder* parent_type)
{
    if (parent_type != nullptr && parent_type->GetKind() == TK_STRUCTURE)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_STRUCTURE;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRUCTURE));
        pDescriptor.mBaseType = CreateType(parent_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating child struct, invalid input type.");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateStructBuilder()
{
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRUCTURE;
    pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRUCTURE));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateCustomBuilder(const TypeDescriptor* descriptor,
    const std::string& name /*= ""*/)
{
    if (descriptor != nullptr)
    {
        TypeKind kind = descriptor->GetKind();
        if (kind == TK_BOOLEAN || kind == TK_BYTE || kind == TK_INT16 || kind == TK_INT32 ||
            kind == TK_INT64 || kind == TK_UINT16 || kind == TK_UINT32 || kind == TK_UINT64 ||
            kind == TK_FLOAT32 || kind == TK_FLOAT64 || kind == TK_FLOAT128 || kind == TK_CHAR8 ||
            kind == TK_CHAR16 || kind == TK_STRING8 || kind == TK_STRING16 || kind == TK_ALIAS ||
            kind == TK_ENUM || kind == TK_BITMASK || kind == TK_STRUCTURE || kind == TK_UNION ||
            kind == TK_BITSET || kind == TK_SEQUENCE || kind == TK_ARRAY || kind == TK_MAP)
        {
            DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(descriptor);
            if (pNewType != nullptr && name.length() > 0)
            {
                pNewType->SetName(name);
            }
            AddBuilderToList(pNewType);
            return pNewType;
        }
        else
        {
            logError(DYN_TYPES, "Error creating type, unsupported type kind.");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error creating type, invalid input descriptor.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateBuilderCopy(const DynamicTypeBuilder* type)
{
    if (type != nullptr)
    {
        DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(type);
        AddBuilderToList(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error creating type, invalid input type.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint16Builder()
{
    TypeDescriptor pUInt16Descriptor;
    pUInt16Descriptor.mKind = TK_UINT16;
    pUInt16Descriptor.mName = GenerateTypeName(GetTypeName(TK_UINT16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt16Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint32Builder()
{
    TypeDescriptor pUInt32Descriptor;
    pUInt32Descriptor.mKind = TK_UINT32;
    pUInt32Descriptor.mName = GenerateTypeName(GetTypeName(TK_UINT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt32Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint64Builder()
{
    TypeDescriptor pUInt64Descriptor;
    pUInt64Descriptor.mKind = TK_UINT64;
    pUInt64Descriptor.mName = GenerateTypeName(GetTypeName(TK_UINT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt64Descriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUnionBuilder(DynamicTypeBuilder* discriminator_type)
{
    if (discriminator_type != nullptr && discriminator_type->IsDiscriminatorType())
    {
        DynamicType_ptr pType = CreateType(discriminator_type);
        if (pType != nullptr)
        {
            return CreateUnionBuilder(pType);
        }
        else
        {
            logError(DYN_TYPES, "Error building Union, Error creating discriminator type");
            return nullptr;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error building Union, invalid discriminator type");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUnionBuilder(DynamicType_ptr discriminator_type)
{
    if (discriminator_type != nullptr && discriminator_type->IsDiscriminatorType())
    {
        TypeDescriptor pUnionDescriptor;
        pUnionDescriptor.mKind = TK_UNION;
        pUnionDescriptor.mName = GenerateTypeName(GetTypeName(TK_UNION));
        pUnionDescriptor.mDiscriminatorType = discriminator_type;

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUnionDescriptor);
        AddBuilderToList(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error building Union, invalid discriminator type");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateWstringBuilder(uint32_t bound)
{
    if (bound == 0)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor pCharDescriptor;
    pCharDescriptor.mKind = TK_CHAR16;
    pCharDescriptor.mName = GenerateTypeName(GetTypeName(TK_CHAR16));

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING16;
    //pDescriptor.mName = GenerateTypeName(GetTypeName(TK_STRING16));
    pDescriptor.mElementType = CreateType(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    pDescriptor.mName = TypeNamesGenerator::getStringTypeName(bound, true, true);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    AddBuilderToList(pNewTypeBuilder);
    return pNewTypeBuilder;
}

ResponseCode DynamicTypeBuilderFactory::DeleteBuilder(DynamicTypeBuilder* builder)
{
    if (builder != nullptr)
    {
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        std::unique_lock<std::recursive_mutex> scoped(mMutex);
        auto it = std::find(mBuildersList.begin(), mBuildersList.end(), builder);
        if (it != mBuildersList.end())
        {
            mBuildersList.erase(it);
            delete builder;
        }
        else
        {
            logWarning(DYN_TYPES, "The given type has been deleted previously.");
            return ResponseCode::RETCODE_ALREADY_DELETED;
        }
#else
        delete builder;
#endif
    }
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicTypeBuilderFactory::DeleteType(DynamicType* type)
{
    if (type != nullptr)
    {
        delete type;
    }
    return ResponseCode::RETCODE_OK;
}

DynamicType_ptr DynamicTypeBuilderFactory::GetPrimitiveType(TypeKind kind)
{
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = kind;
    pDescriptor.mName = GenerateTypeName(GetTypeName(kind));
    return CreateType(&pDescriptor);
}

bool DynamicTypeBuilderFactory::IsEmpty() const
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    return mBuildersList.empty();
#else
    return true;
#endif
}

void DynamicTypeBuilderFactory::BuildTypeIdentifier(const DynamicType_ptr type, TypeIdentifier& identifier,
        bool complete) const
{
    const TypeDescriptor *descriptor = type->getTypeDescriptor();
    BuildTypeIdentifier(descriptor, identifier, complete);
}

void DynamicTypeBuilderFactory::BuildTypeIdentifier(const TypeDescriptor* descriptor, TypeIdentifier& identifier,
        bool complete) const
{
    const TypeIdentifier *id2 = (complete)
        ? TypeObjectFactory::GetInstance()->GetTypeIdentifierTryingComplete(descriptor->GetName())
        : TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->GetName());
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
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id, complete);
                        identifier.seq_sdefn().element_identifier(&elem_id);
                    }
                    else
                    {
                        identifier._d(TI_PLAIN_SEQUENCE_LARGE);
                        identifier.seq_ldefn().bound(descriptor->mBound[0]);
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id, complete);
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
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id, complete);
                        identifier.array_sdefn().element_identifier(&elem_id);
                    }
                    else
                    {
                        identifier._d(TI_PLAIN_ARRAY_LARGE);
                        identifier.array_ldefn().array_bound_seq(descriptor->mBound);
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id, complete);
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
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id, complete);
                        identifier.map_sdefn().element_identifier(&elem_id);
                        TypeIdentifier key_id;
                        BuildTypeIdentifier(descriptor->GetKeyElementType()->mDescriptor, key_id, complete);
                        identifier.map_sdefn().key_identifier(&key_id);
                    }
                    else
                    {
                        identifier._d(TI_PLAIN_MAP_LARGE);
                        identifier.map_ldefn().bound(static_cast<SBound>(descriptor->mBound[0]));
                        TypeIdentifier elem_id;
                        BuildTypeIdentifier(descriptor->GetElementType()->mDescriptor, elem_id, complete);
                        identifier.map_ldefn().element_identifier(&elem_id);
                        TypeIdentifier key_id;
                        BuildTypeIdentifier(descriptor->GetKeyElementType()->mDescriptor, key_id, complete);
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
                    // and return them as EK_MINIMAL or EK_COMPLETE
                    logInfo(DYN_TYPE_FACTORY, "Complex types must be built from CompleteTypeObjects.");
                }
                break;
        }

        TypeObjectFactory::GetInstance()->AddTypeIdentifier(descriptor->GetName(), &identifier);
    }
}

void DynamicTypeBuilderFactory::BuildTypeObject(const DynamicType_ptr type, TypeObject &object,
                                                bool complete) const
{
    const TypeDescriptor *descriptor = type->getTypeDescriptor();

    std::map<MemberId, DynamicTypeMember*> membersMap;
    type->GetAllMembers(membersMap);
    std::vector<const MemberDescriptor*> members;
    for (auto it : membersMap)
    {
        members.push_back(it.second->GetDescriptor());
    }

    BuildTypeObject(descriptor, object, &members, complete);
}

void DynamicTypeBuilderFactory::BuildTypeObject(const TypeDescriptor* descriptor, TypeObject &object,
                                                const std::vector<const MemberDescriptor*> *members,
                                                bool complete) const
{
    const TypeObject *obj2 = TypeObjectFactory::GetInstance()->GetTypeObject(descriptor->GetName(), complete);
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
                    BuildAliasTypeObject(descriptor, object, complete);
                }
                break;
            // Enumerated TKs
            case TK_ENUM:
                {
                    BuildEnumTypeObject(descriptor, object, *members, complete);
                }
                break;
            case TK_BITMASK:
                {
                    BuildBitmaskTypeObject(descriptor, object, *members, complete);
                }
                break;
            // Structured TKs
            case TK_ANNOTATION:
                {
                    BuildAnnotationTypeObject(descriptor, object, *members, complete);
                }
                break;
            case TK_STRUCTURE:
                {
                    BuildStructTypeObject(descriptor, object, *members, complete);
                }
                break;
            case TK_UNION:
                {
                    BuildUnionTypeObject(descriptor, object, *members, complete);
                }
                break;
            case TK_BITSET:
                {
                    BuildBitsetTypeObject(descriptor, object, *members, complete);
                }
                break;
        }
    }
}

void DynamicTypeBuilderFactory::BuildAliasTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(EK_COMPLETE);
        object.complete()._d(TK_ALIAS);
        object.complete().alias_type().alias_flags().IS_FINAL(false);
        object.complete().alias_type().alias_flags().IS_APPENDABLE(false);
        object.complete().alias_type().alias_flags().IS_MUTABLE(false);
        object.complete().alias_type().alias_flags().IS_NESTED(false);
        object.complete().alias_type().alias_flags().IS_AUTOID_HASH(false);

        object.complete().alias_type().header().detail().type_name(descriptor->GetName());
        object.complete().alias_type().body().common().related_flags().TRY_CONSTRUCT1(false);
        object.complete().alias_type().body().common().related_flags().TRY_CONSTRUCT2(false);
        object.complete().alias_type().body().common().related_flags().IS_EXTERNAL(false);
        object.complete().alias_type().body().common().related_flags().IS_OPTIONAL(false);
        object.complete().alias_type().body().common().related_flags().IS_MUST_UNDERSTAND(false);
        object.complete().alias_type().body().common().related_flags().IS_KEY(false);
        object.complete().alias_type().body().common().related_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //BuildTypeIdentifier(descriptor->GetBaseType()->mDescriptor, ident);
        TypeObject obj;
        BuildTypeObject(descriptor->GetBaseType(), obj, complete);
        TypeIdentifier ident = *TypeObjectFactory::GetInstance()->GetTypeIdentifier(
                                    descriptor->GetBaseType()->GetName());

        object.complete().alias_type().body().common().related_type(ident);

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
            CompleteAliasType::getCdrSerializedSize(object.complete().alias_type()) + 4));
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
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        // Add our alias
        TypeObjectFactory::GetInstance()->AddAlias(descriptor->GetName(), descriptor->GetBaseType()->GetName());

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
    else
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

        //TypeIdentifier ident;
        //BuildTypeIdentifier(descriptor->GetBaseType()->mDescriptor, ident);
        TypeObject obj;
        BuildTypeObject(descriptor->GetBaseType()->mDescriptor, obj);
        TypeIdentifier ident = *TypeObjectFactory::GetInstance()->GetTypeIdentifier(
                                    descriptor->GetBaseType()->GetName());

        object.minimal().alias_type().body().common().related_type(ident);

        TypeIdentifier identifier;
        identifier._d(EK_MINIMAL);

        SerializedPayload_t payload(static_cast<uint32_t>(
            MinimalAliasType::getCdrSerializedSize(object.minimal().alias_type()) + 4));
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
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        // Add our alias
        TypeObjectFactory::GetInstance()->AddAlias(descriptor->GetName(), descriptor->GetBaseType()->GetName());

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::BuildEnumTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
                                                    const std::vector<const MemberDescriptor*> members,
                                                    bool complete) const
{
    if (complete)
    {
        object._d(EK_COMPLETE);
        object.complete()._d(TK_ENUM);
        object.complete().enumerated_type().header().common().bit_bound(32); // TODO fixed by IDL, isn't?
        object.complete().enumerated_type().header().detail().type_name(descriptor->GetName());

        for (const MemberDescriptor* member : members)
        {
            CompleteEnumeratedLiteral mel;
            mel.common().flags().TRY_CONSTRUCT1(false);
            mel.common().flags().TRY_CONSTRUCT2(false);
            mel.common().flags().IS_EXTERNAL(false);
            mel.common().flags().IS_OPTIONAL(false);
            mel.common().flags().IS_MUST_UNDERSTAND(false);
            mel.common().flags().IS_KEY(false);
            mel.common().flags().IS_DEFAULT(false);
            mel.common().value(member->GetIndex());
            mel.detail().name(member->GetName());
            object.complete().enumerated_type().literal_seq().emplace_back(mel);
        }

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
            CompleteEnumeratedType::getCdrSerializedSize(object.complete().enumerated_type()) + 4));
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
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
    else
    {
        object._d(EK_MINIMAL);
        object.minimal()._d(TK_ENUM);
        object.minimal().enumerated_type().header().common().bit_bound(32); // TODO fixed by IDL, isn't?

        for (const MemberDescriptor* member : members)
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

        TypeIdentifier identifier;
        identifier._d(EK_MINIMAL);

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
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::BuildStructTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
                                                    const std::vector<const MemberDescriptor*> members,
                                                    bool complete) const
{
    if (complete)
    {
        object._d(EK_COMPLETE);
        object.complete()._d(TK_STRUCTURE);

        object.complete().struct_type().struct_flags().IS_FINAL(false);
        object.complete().struct_type().struct_flags().IS_APPENDABLE(false);
        object.complete().struct_type().struct_flags().IS_MUTABLE(false);
        object.complete().struct_type().struct_flags().IS_NESTED(false);
        object.complete().struct_type().struct_flags().IS_AUTOID_HASH(false);

        for (const MemberDescriptor* member : members)
        {
            CompleteStructMember msm;
            msm.common().member_id(member->GetId());
            msm.common().member_flags().TRY_CONSTRUCT1(false);
            msm.common().member_flags().TRY_CONSTRUCT2(false);
            msm.common().member_flags().IS_EXTERNAL(false);
            msm.common().member_flags().IS_OPTIONAL(false);
            msm.common().member_flags().IS_MUST_UNDERSTAND(false);
            msm.common().member_flags().IS_KEY(false);
            msm.common().member_flags().IS_DEFAULT(false);
            //TypeIdentifier memIdent;
            //BuildTypeIdentifier(member->mType->mDescriptor, memIdent);

            std::map<MemberId, DynamicTypeMember*> membersMap;
            member->mType->GetAllMembers(membersMap);
            std::vector<const MemberDescriptor*> innerMembers;
            for (auto it : membersMap)
            {
                innerMembers.push_back(it.second->GetDescriptor());
            }

            TypeObject memObj;
            BuildTypeObject(member->mType->mDescriptor, memObj, &innerMembers);
            const TypeIdentifier *typeId = TypeObjectFactory::GetInstance()->GetTypeIdentifierTryingComplete(member->mType->GetName());
            if (typeId == nullptr)
            {
                logError(DYN_TYPES, "Member " << member->GetName() << " of struct " << descriptor->GetName() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }

            msm.detail().name(member->GetName());
            object.complete().struct_type().member_seq().emplace_back(msm);
        }

        object.complete().struct_type().header().detail().type_name(descriptor->GetName());
        //object.complete().struct_type().header().detail().ann_builtin()...
        //object.complete().struct_type().header().detail().ann_custom()...

        if (descriptor->GetBaseType().get() != nullptr)
        {
            TypeIdentifier parent;
            BuildTypeIdentifier(descriptor->GetBaseType(), parent);
            object.complete().struct_type().header().base_type(parent);
        }
        //object.complete().struct_type().header().base_type().equivalence_hash()[0..13];

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
           CompleteStructType::getCdrSerializedSize(object.complete().struct_type()) + 4));
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteStructMember &st : object.complete().struct_type().member_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for(int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
    else
    {
        object._d(EK_MINIMAL);
        object.minimal()._d(TK_STRUCTURE);

        object.minimal().struct_type().struct_flags().IS_FINAL(false);
        object.minimal().struct_type().struct_flags().IS_APPENDABLE(false);
        object.minimal().struct_type().struct_flags().IS_MUTABLE(false);
        object.minimal().struct_type().struct_flags().IS_NESTED(false);
        object.minimal().struct_type().struct_flags().IS_AUTOID_HASH(false);

        for (const MemberDescriptor* member : members)
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
            //TypeIdentifier memIdent;
            //BuildTypeIdentifier(member->mType->mDescriptor, memIdent);

            std::map<MemberId, DynamicTypeMember*> membersMap;
            member->mType->GetAllMembers(membersMap);
            std::vector<const MemberDescriptor*> innerMembers;
            for (auto it : membersMap)
            {
                innerMembers.push_back(it.second->GetDescriptor());
            }

            TypeObject memObj;
            BuildTypeObject(member->mType->mDescriptor, memObj, &innerMembers);
            const TypeIdentifier *typeId = TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName());
            if (typeId == nullptr)
            {
                logError(DYN_TYPES, "Member " << member->GetName() << " of struct " << descriptor->GetName() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }
            //msm.common().member_type_id(*TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName()));
            MD5 hash(member->GetName());
            for(int i = 0; i < 4; ++i)
            {
                msm.detail().name_hash()[i] = hash.digest[i];
            }
            object.minimal().struct_type().member_seq().emplace_back(msm);
        }

        if (descriptor->GetBaseType().get() != nullptr)
        {
            TypeIdentifier parent;
            BuildTypeIdentifier(descriptor->GetBaseType(), parent);
            object.minimal().struct_type().header().base_type(parent);
        }

        TypeIdentifier identifier;
        identifier._d(EK_MINIMAL);

        SerializedPayload_t payload(static_cast<uint32_t>(
           MinimalStructType::getCdrSerializedSize(object.minimal().struct_type()) + 4));
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
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
}


void DynamicTypeBuilderFactory::BuildUnionTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
                                                    const std::vector<const MemberDescriptor*> members,
                                                    bool complete) const
{
    if (complete)
    {
        object._d(EK_COMPLETE);
        object.complete()._d(TK_UNION);

        object.complete().union_type().union_flags().IS_FINAL(false);
        object.complete().union_type().union_flags().IS_APPENDABLE(false);
        object.complete().union_type().union_flags().IS_MUTABLE(false);
        object.complete().union_type().union_flags().IS_NESTED(false);
        object.complete().union_type().union_flags().IS_AUTOID_HASH(false);

        object.complete().union_type().discriminator().common().member_flags().TRY_CONSTRUCT1(false);
        object.complete().union_type().discriminator().common().member_flags().TRY_CONSTRUCT2(false);
        object.complete().union_type().discriminator().common().member_flags().IS_EXTERNAL(false);
        object.complete().union_type().discriminator().common().member_flags().IS_OPTIONAL(false);
        object.complete().union_type().discriminator().common().member_flags().IS_MUST_UNDERSTAND(false);
        object.complete().union_type().discriminator().common().member_flags().IS_KEY(false);
        object.complete().union_type().discriminator().common().member_flags().IS_DEFAULT(false);

        TypeObject discObj;
        BuildTypeObject(descriptor->mDiscriminatorType->mDescriptor, discObj);
        TypeIdentifier discIdent =
            *TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->mDiscriminatorType->GetName());
        object.complete().union_type().discriminator().common().type_id(discIdent);
            //*TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->mDiscriminatorType->GetName()));

        for (const MemberDescriptor* member : members)
        {
            CompleteUnionMember mum;
            mum.common().member_id(member->GetId());
            mum.common().member_flags().TRY_CONSTRUCT1(false);
            mum.common().member_flags().TRY_CONSTRUCT2(false);
            mum.common().member_flags().IS_EXTERNAL(false);
            mum.common().member_flags().IS_OPTIONAL(false);
            mum.common().member_flags().IS_MUST_UNDERSTAND(false);
            mum.common().member_flags().IS_KEY(false);
            mum.common().member_flags().IS_DEFAULT(member->IsDefaultUnionValue());

            //TypeIdentifier memIdent;
            //BuildTypeIdentifier(member->mType->mDescriptor, memIdent);

            std::map<MemberId, DynamicTypeMember*> membersMap;
            member->mType->GetAllMembers(membersMap);
            std::vector<const MemberDescriptor*> innerMembers;
            for (auto it : membersMap)
            {
                innerMembers.push_back(it.second->GetDescriptor());
            }

            TypeObject memObj;
            BuildTypeObject(member->mType->mDescriptor, memObj, &innerMembers);
            const TypeIdentifier *typeId = TypeObjectFactory::GetInstance()->GetTypeIdentifierTryingComplete(member->mType->GetName());
            if (typeId == nullptr)
            {
                logError(DYN_TYPES, "Member " << member->GetName() << " of union " << descriptor->GetName() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                mum.common().type_id(memIdent);
            }
            //TypeIdentifier memIdent = *TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName());
            //mum.common().type_id(memIdent);
            //mum.common().type_id(*TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName()));
            for (uint64_t lab : member->GetUnionLabels())
            {
                mum.common().label_seq().emplace_back(static_cast<uint32_t>(lab));
            }
            mum.detail().name(member->GetName());
            object.complete().union_type().member_seq().emplace_back(mum);
        }

        object.complete().union_type().header().detail().type_name(descriptor->GetName());

        TypeIdentifier identifier;
        identifier._d(EK_MINIMAL);

        SerializedPayload_t payload(static_cast<uint32_t>(
            CompleteUnionType::getCdrSerializedSize(object.complete().union_type()) + 4));
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
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
    else
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

        TypeObject discObj;
        BuildTypeObject(descriptor->mDiscriminatorType->mDescriptor, discObj);
        TypeIdentifier discIdent =
            *TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->mDiscriminatorType->GetName());
        object.minimal().union_type().discriminator().common().type_id(discIdent);
            //*TypeObjectFactory::GetInstance()->GetTypeIdentifier(descriptor->mDiscriminatorType->GetName()));

        for (const MemberDescriptor* member : members)
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

            //TypeIdentifier memIdent;
            //BuildTypeIdentifier(member->mType->mDescriptor, memIdent);

            std::map<MemberId, DynamicTypeMember*> membersMap;
            member->mType->GetAllMembers(membersMap);
            std::vector<const MemberDescriptor*> innerMembers;
            for (auto it : membersMap)
            {
                innerMembers.push_back(it.second->GetDescriptor());
            }

            TypeObject memObj;
            BuildTypeObject(member->mType->mDescriptor, memObj, &innerMembers);
            const TypeIdentifier *typeId = TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName());
            if (typeId == nullptr)
            {
                logError(DYN_TYPES, "Member " << member->GetName() << " of union " << descriptor->GetName() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                mum.common().type_id(memIdent);
            }
            //TypeIdentifier memIdent = *TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName());
            //mum.common().type_id(memIdent);
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

        TypeIdentifier identifier;
        identifier._d(EK_MINIMAL);

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
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::BuildBitsetTypeObject(
        const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members,
        bool complete) const
{
    if (complete)
    {
        object._d(EK_COMPLETE);
        object.complete()._d(TK_BITSET);

        object.complete().bitset_type().bitset_flags().IS_FINAL(false);
        object.complete().bitset_type().bitset_flags().IS_APPENDABLE(false);
        object.complete().bitset_type().bitset_flags().IS_MUTABLE(false);
        object.complete().bitset_type().bitset_flags().IS_NESTED(false);
        object.complete().bitset_type().bitset_flags().IS_AUTOID_HASH(false);

        for (const MemberDescriptor* member : members)
        {
            CompleteBitfield msm;
            msm.common().position(member->GetId());
            //msm.common().bitcount(member->); // No available field
            msm.common().holder_type(member->mType->GetKind());
            msm.detail().name(member->GetName());
            object.complete().bitset_type().field_seq().emplace_back(msm);
        }

        object.complete().bitset_type().header().detail().type_name(descriptor->GetName());
        //object.complete().bitset_type().header().detail().ann_builtin()...
        //object.complete().bitset_type().header().detail().ann_custom()...

        if (descriptor->GetBaseType().get() != nullptr)
        {
            TypeIdentifier parent;
            BuildTypeIdentifier(descriptor->GetBaseType(), parent);
            object.complete().bitset_type().header().base_type(parent);
        }
        //object.complete().bitset_type().header().base_type().equivalence_hash()[0..13];

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
           CompleteBitsetType::getCdrSerializedSize(object.complete().bitset_type()) + 4));
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteBitfield &st : object.complete().bitset_type().field_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for(int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
    else
    {
        object._d(EK_COMPLETE);
        object.minimal()._d(TK_BITSET);

        object.minimal().bitset_type().bitset_flags().IS_FINAL(false);
        object.minimal().bitset_type().bitset_flags().IS_APPENDABLE(false);
        object.minimal().bitset_type().bitset_flags().IS_MUTABLE(false);
        object.minimal().bitset_type().bitset_flags().IS_NESTED(false);
        object.minimal().bitset_type().bitset_flags().IS_AUTOID_HASH(false);

        for (const MemberDescriptor* member : members)
        {
            MinimalBitfield msm;
            msm.common().position(member->GetId());
            //msm.common().bitcount(member->); // No available field
            msm.common().holder_type(member->mType->GetKind());
            MD5 parent_bitfield_hash(member->GetName());
            for(int i = 0; i < 4; ++i)
            {
                msm.name_hash()[i] = parent_bitfield_hash.digest[i];
            }
            object.minimal().bitset_type().field_seq().emplace_back(msm);
        }

        //object.minimal().bitset_type().header().detail().ann_builtin()...
        //object.minimal().bitset_type().header().detail().ann_custom()...

        if (descriptor->GetBaseType().get() != nullptr)
        {
            TypeIdentifier parent;
            BuildTypeIdentifier(descriptor->GetBaseType(), parent);
            object.minimal().bitset_type().header().base_type(parent);
        }
        //object.minimal().bitset_type().header().base_type().equivalence_hash()[0..13];

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
           MinimalBitsetType::getCdrSerializedSize(object.minimal().bitset_type()) + 4));
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (MinimalBitfield &st : object.minimal().bitset_type().field_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for(int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::BuildBitmaskTypeObject(
        const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members,
        bool complete) const
{
    if (complete)
    {
        object._d(EK_COMPLETE);
        object.complete()._d(TK_BITMASK);

        object.complete().bitmask_type().bitmask_flags().IS_FINAL(false);
        object.complete().bitmask_type().bitmask_flags().IS_APPENDABLE(false);
        object.complete().bitmask_type().bitmask_flags().IS_MUTABLE(false);
        object.complete().bitmask_type().bitmask_flags().IS_NESTED(false);
        object.complete().bitmask_type().bitmask_flags().IS_AUTOID_HASH(false);

        for (const MemberDescriptor* member : members)
        {
            CompleteBitflag msm;
            msm.common().position(member->GetId());
            msm.detail().name(member->GetName());
            object.complete().bitmask_type().flag_seq().emplace_back(msm);
        }

        object.complete().bitmask_type().header().detail().type_name(descriptor->GetName());

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
           CompleteBitmaskType::getCdrSerializedSize(object.complete().bitmask_type()) + 4));
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteBitflag &st : object.complete().bitmask_type().flag_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for(int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
    else
    {
        object._d(EK_COMPLETE);
        object.minimal()._d(TK_BITMASK);

        object.minimal().bitmask_type().bitmask_flags().IS_FINAL(false);
        object.minimal().bitmask_type().bitmask_flags().IS_APPENDABLE(false);
        object.minimal().bitmask_type().bitmask_flags().IS_MUTABLE(false);
        object.minimal().bitmask_type().bitmask_flags().IS_NESTED(false);
        object.minimal().bitmask_type().bitmask_flags().IS_AUTOID_HASH(false);

        for (const MemberDescriptor* member : members)
        {
            MinimalBitflag msm;
            msm.common().position(member->GetId());
            MD5 parent_bitfield_hash(member->GetName());
            for(int i = 0; i < 4; ++i)
            {
                msm.detail().name_hash()[i] = parent_bitfield_hash.digest[i];
            }
            object.minimal().bitmask_type().flag_seq().emplace_back(msm);
        }

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
           MinimalBitmaskType::getCdrSerializedSize(object.minimal().bitmask_type()) + 4));
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (MinimalBitflag &st : object.minimal().bitmask_type().flag_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for(int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::BuildAnnotationTypeObject(
        const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members,
        bool complete) const
{
    if (complete)
    {
        object._d(EK_COMPLETE);
        object.complete()._d(TK_ANNOTATION);

        for (const MemberDescriptor* member : members)
        {
            CompleteAnnotationParameter msm;
            msm.name(member->GetName());
            //msm.default_value(member->); // No equivalence field

            TypeObject memObj;
            BuildTypeObject(member->mType->mDescriptor, memObj);
            const TypeIdentifier *typeId = TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName());
            if (typeId == nullptr)
            {
                logError(DYN_TYPES, "Member " << member->GetName() << " of annotation " << descriptor->GetName() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }

            object.complete().annotation_type().member_seq().emplace_back(msm);
        }

        object.complete().annotation_type().header().annotation_name(descriptor->GetName());

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
           CompleteAnnotationType::getCdrSerializedSize(object.complete().annotation_type()) + 4));
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteAnnotationParameter &st : object.complete().annotation_type().member_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for(int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
    else
    {
        object._d(EK_COMPLETE);
        object.minimal()._d(TK_ANNOTATION);

        for (const MemberDescriptor* member : members)
        {
            MinimalAnnotationParameter msm;
            msm.name(member->GetName());
            //msm.default_value(member->); // No equivalence field

            TypeObject memObj;
            BuildTypeObject(member->mType->mDescriptor, memObj);
            const TypeIdentifier *typeId = TypeObjectFactory::GetInstance()->GetTypeIdentifier(member->mType->GetName());
            if (typeId == nullptr)
            {
                logError(DYN_TYPES, "Member " << member->GetName() << " of annotation " << descriptor->GetName() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }

            object.minimal().annotation_type().member_seq().emplace_back(msm);
        }

        TypeIdentifier identifier;
        identifier._d(EK_COMPLETE);

        SerializedPayload_t payload(static_cast<uint32_t>(
           MinimalAnnotationType::getCdrSerializedSize(object.minimal().annotation_type()) + 4));
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size); // Object that manages the raw buffer.

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (MinimalAnnotationParameter &st : object.minimal().annotation_type().member_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for(int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::GetInstance()->AddTypeObject(descriptor->GetName(), &identifier, &object);
    }
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateAliasType(DynamicTypeBuilder* base_type, const std::string& sName)
{
    if (base_type != nullptr)
    {
        DynamicType_ptr pType = CreateType(base_type);
        if (pType != nullptr)
        {
            return CreateAliasType(pType, sName);
        }
        else
        {
            logError(DYN_TYPES, "Error creating alias type, Error creating dynamic type");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateAliasType(DynamicType_ptr base_type, const std::string& sName)
{
    if (base_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ALIAS;
        pDescriptor.mBaseType = base_type;
        if (sName.length() > 0)
        {
            pDescriptor.mName = sName;
        }
        else
        {
            pDescriptor.mName = base_type->GetName();
        }

        return CreateType(&pDescriptor, sName);
    }
    else
    {
        logError(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateInt32Type()
{
    TypeDescriptor pInt32Descriptor(GenerateTypeName(GetTypeName(TK_INT32)), TK_INT32);
    return new DynamicType(&pInt32Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateUint32Type()
{
    TypeDescriptor pUint32Descriptor(GenerateTypeName(GetTypeName(TK_UINT32)), TK_UINT32);
    return new DynamicType(&pUint32Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateInt16Type()
{
    TypeDescriptor pInt16Descriptor(GenerateTypeName(GetTypeName(TK_INT16)), TK_INT16);
    return new DynamicType(&pInt16Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateUint16Type()
{
    TypeDescriptor pUint16Descriptor(GenerateTypeName(GetTypeName(TK_UINT16)), TK_UINT16);
    return new DynamicType(&pUint16Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateInt64Type()
{
    TypeDescriptor pInt64Descriptor(GenerateTypeName(GetTypeName(TK_INT64)), TK_INT64);
    return new DynamicType(&pInt64Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateUint64Type()
{
    TypeDescriptor pUint64Descriptor(GenerateTypeName(GetTypeName(TK_UINT64)), TK_UINT64);
    return new DynamicType(&pUint64Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateFloat32Type()
{
    TypeDescriptor pFloat32Descriptor(GenerateTypeName(GetTypeName(TK_FLOAT32)), TK_FLOAT32);
    return new DynamicType(&pFloat32Descriptor);
}


DynamicType_ptr DynamicTypeBuilderFactory::CreateFloat64Type()
{
    TypeDescriptor pFloat64Descriptor(GenerateTypeName(GetTypeName(TK_FLOAT64)), TK_FLOAT64);
    return new DynamicType(&pFloat64Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateFloat128Type()
{
    TypeDescriptor pFloat128Descriptor(GenerateTypeName(GetTypeName(TK_FLOAT128)), TK_FLOAT128);
    return new DynamicType(&pFloat128Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateChar8Type()
{
    TypeDescriptor pChar8Descriptor(GenerateTypeName(GetTypeName(TK_CHAR8)), TK_CHAR8);
    return new DynamicType(&pChar8Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateChar16Type()
{
    TypeDescriptor pChar16Descriptor(GenerateTypeName(GetTypeName(TK_CHAR16)), TK_CHAR16);
    return new DynamicType(&pChar16Descriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateBoolType()
{
    TypeDescriptor pBoolDescriptor(GenerateTypeName(GetTypeName(TK_BOOLEAN)), TK_BOOLEAN);
    return new DynamicType(&pBoolDescriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateByteType()
{
    TypeDescriptor pByteDescriptor(GenerateTypeName(GetTypeName(TK_BYTE)), TK_BYTE);
    return new DynamicType(&pByteDescriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateStringType(uint32_t bound /*= MAX_STRING_LENGTH*/)
{
    if (bound == 0)
    {
        bound = MAX_STRING_LENGTH;
    }
    TypeDescriptor pStringDescriptor("", TK_STRING8);
    pStringDescriptor.mName = TypeNamesGenerator::getStringTypeName(bound, false, true);
    pStringDescriptor.mElementType = CreateChar8Type();
    pStringDescriptor.mBound.push_back(bound);

    return new DynamicType(&pStringDescriptor);
}


DynamicType_ptr DynamicTypeBuilderFactory::CreateWstringType(uint32_t bound /*= MAX_STRING_LENGTH*/)
{
    if (bound == 0)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor pStringDescriptor("", TK_STRING16);
    pStringDescriptor.mName = TypeNamesGenerator::getStringTypeName(bound, true, true);
    pStringDescriptor.mElementType = CreateChar16Type();
    pStringDescriptor.mBound.push_back(bound);

    return new DynamicType(&pStringDescriptor);
}

DynamicType_ptr DynamicTypeBuilderFactory::CreateBitsetType(uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_BITSET;
        pDescriptor.mName = GenerateTypeName(GetTypeName(TK_BITSET));
        pDescriptor.mBound.push_back(bound);
        return CreateType(&pDescriptor, pDescriptor.mName);
    }
    else
    {
        logError(DYN_TYPES, "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return nullptr;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
