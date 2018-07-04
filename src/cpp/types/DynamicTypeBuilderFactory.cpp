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

static uint32_t s_typeNameCounter = 0;
static std::string GenerateTypeName()
{
    return "DynamicType" + std::to_string(++s_typeNameCounter);
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

DynamicType* DynamicTypeBuilderFactory::BuildType(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        DynamicType* pNewType = new DynamicType(descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewType);
#endif
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
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewType);
#endif
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type, invalid input parameter");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateAliasType(DynamicType* base_type, std::string sName /*= ""*/)
{
    if (base_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ARRAY;
        pDescriptor.mBaseType = base_type;
        if (sName.length() > 0)
        {
            pDescriptor.mName = sName;
        }
        else
        {
            pDescriptor.mName = GenerateTypeName();
        }

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewTypeBuilder);
#endif
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
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound = bounds;
        pDescriptor.mElementType = BuildType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewTypeBuilder);
#endif
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
        pBoolDescriptor.mName = GenerateTypeName();

        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_BITMASK;
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mElementType = BuildType(&pBoolDescriptor);
        pDescriptor.mBound.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewTypeBuilder);
#endif
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
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewTypeBuilder);
#endif
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
    pBoolDescriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pBoolDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateByteType()
{
    TypeDescriptor pByteDescriptor;
    pByteDescriptor.mKind = TK_BYTE;
    pByteDescriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pByteDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChar8Type()
{
    TypeDescriptor pChar8Descriptor;
    pChar8Descriptor.mKind = TK_CHAR8;
    pChar8Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar8Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateChar16Type()
{
    TypeDescriptor pChar16Descriptor;
    pChar16Descriptor.mKind = TK_CHAR16;
    pChar16Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar16Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat32Type()
{
    TypeDescriptor pFloat32Descriptor;
    pFloat32Descriptor.mKind = TK_FLOAT32;
    pFloat32Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat32Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat64Type()
{
    TypeDescriptor pFloat64Descriptor;
    pFloat64Descriptor.mKind = TK_FLOAT64;
    pFloat64Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat64Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateFloat128Type()
{
    TypeDescriptor pFloat128Descriptor;
    pFloat128Descriptor.mKind = TK_FLOAT128;
    pFloat128Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat128Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt16Type()
{
    TypeDescriptor pInt16Descriptor;
    pInt16Descriptor.mKind = TK_INT16;
    pInt16Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt16Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt32Type()
{
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.mKind = TK_INT32;
    pInt32Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt32Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateInt64Type()
{
    TypeDescriptor pInt64Descriptor;
    pInt64Descriptor.mKind = TK_INT64;
    pInt64Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt64Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateMapType(DynamicType* key_element_type,
    DynamicType* element_type, uint32_t bound)
{
    if (key_element_type != nullptr && element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_MAP;
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mKeyElementType = BuildType(key_element_type);
        pDescriptor.mElementType = BuildType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewTypeBuilder);
#endif
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
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mElementType = BuildType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewTypeBuilder);
#endif
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
    pCharDescriptor.mName = GenerateTypeName();

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING8;
    pDescriptor.mName = GenerateTypeName();
    pDescriptor.mElementType = BuildType(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateType(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewType);
#endif
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
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        mTypesList.push_back(pNewType);
#endif
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
    pUInt16Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt16Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint32Type()
{
    TypeDescriptor pUInt32Descriptor;
    pUInt32Descriptor.mKind = TK_UINT32;
    pUInt32Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt32Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateUint64Type()
{
    TypeDescriptor pUInt64Descriptor;
    pUInt64Descriptor.mKind = TK_UINT64;
    pUInt64Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt64Descriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::CreateWstringType(uint32_t bound)
{
    TypeDescriptor pCharDescriptor;
    pCharDescriptor.mKind = TK_CHAR16;
    pCharDescriptor.mName = GenerateTypeName();

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING16;
    pDescriptor.mName = GenerateTypeName();
    pDescriptor.mElementType = BuildType(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    mTypesList.push_back(pNewTypeBuilder);
#endif
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
    pDescriptor.mName = GenerateTypeName();
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
