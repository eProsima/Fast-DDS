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
DynamicTypeBuilderFactory* DynamicTypeBuilderFactory::get_instance()
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
    for (auto it = mTypesList.begin(); it != mTypesList.end(); ++it)
    {
        delete *it;
    }
    mTypesList.clear();
}

DynamicType* DynamicTypeBuilderFactory::build_type(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        DynamicType* pNewType = new DynamicType(descriptor);
        mTypesList.push_back(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type, invalid input descriptor");
        return nullptr;
    }
}

DynamicType* DynamicTypeBuilderFactory::build_type(const DynamicType* other)
{
    if (other != nullptr)
    {
        DynamicType* pNewType = new DynamicType(other);
        mTypesList.push_back(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error building type, invalid input parameter");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_alias_type(DynamicType* base_type, std::string sName /*= ""*/)
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
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_array_type(const DynamicType* element_type,
    std::vector<uint32_t> bounds)
{
    if (element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ARRAY;
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound = bounds;
        pDescriptor.mElementType = build_type(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating array, element_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bitmask_type(uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pBoolDescriptor;
        pBoolDescriptor.mKind = TK_BOOLEAN;
        pBoolDescriptor.mName = GenerateTypeName();

        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_BITMASK;
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mElementType = build_type(&pBoolDescriptor);
        pDescriptor.mBound.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bool_type()
{
    TypeDescriptor pBoolDescriptor;
    pBoolDescriptor.mKind = TK_BOOLEAN;
    pBoolDescriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pBoolDescriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_byte_type()
{
    TypeDescriptor pByteDescriptor;
    pByteDescriptor.mKind = TK_BYTE;
    pByteDescriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pByteDescriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_char8_type()
{
    TypeDescriptor pChar8Descriptor;
    pChar8Descriptor.mKind = TK_CHAR8;
    pChar8Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar8Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_char16_type()
{
    TypeDescriptor pChar16Descriptor;
    pChar16Descriptor.mKind = TK_CHAR16;
    pChar16Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar16Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float32_type()
{
    TypeDescriptor pFloat32Descriptor;
    pFloat32Descriptor.mKind = TK_FLOAT32;
    pFloat32Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat32Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float64_type()
{
    TypeDescriptor pFloat64Descriptor;
    pFloat64Descriptor.mKind = TK_FLOAT64;
    pFloat64Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat64Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float128_type()
{
    TypeDescriptor pFloat128Descriptor;
    pFloat128Descriptor.mKind = TK_FLOAT128;
    pFloat128Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat128Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int16_type()
{
    TypeDescriptor pInt16Descriptor;
    pInt16Descriptor.mKind = TK_INT16;
    pInt16Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt16Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int32_type()
{
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.mKind = TK_INT32;
    pInt32Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt32Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int64_type()
{
    TypeDescriptor pInt64Descriptor;
    pInt64Descriptor.mKind = TK_INT64;
    pInt64Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt64Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_map_type(DynamicType* key_element_type,
    DynamicType* element_type, uint32_t bound)
{
    if (key_element_type != nullptr && element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_MAP;
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mKeyElementType = build_type(key_element_type);
        pDescriptor.mElementType = build_type(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating map, element_type and key_element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_sequence_type(const DynamicType* element_type, uint32_t bound)
{
    if (element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_SEQUENCE;
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound.push_back(bound);
        pDescriptor.mElementType = build_type(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        logError(DYN_TYPES, "Error creating sequence, element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_string_type(uint32_t bound)
{
    TypeDescriptor pCharDescriptor;
    pCharDescriptor.mKind = TK_CHAR8;
    pCharDescriptor.mName = GenerateTypeName();

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING8;
    pDescriptor.mName = GenerateTypeName();
    pDescriptor.mElementType = build_type(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_type(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(descriptor);
        mTypesList.push_back(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error creating type, invalid input descriptor.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_type_copy(const DynamicType* type)
{
    if (type != nullptr)
    {
        DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(type);
        mTypesList.push_back(pNewType);
        return pNewType;
    }
    else
    {
        logError(DYN_TYPES, "Error creating type, invalid input type.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint16_type()
{
    TypeDescriptor pUInt16Descriptor;
    pUInt16Descriptor.mKind = TK_UINT16;
    pUInt16Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt16Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint32_type()
{
    TypeDescriptor pUInt32Descriptor;
    pUInt32Descriptor.mKind = TK_UINT32;
    pUInt32Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt32Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint64_type()
{
    TypeDescriptor pUInt64Descriptor;
    pUInt64Descriptor.mKind = TK_UINT64;
    pUInt64Descriptor.mName = GenerateTypeName();

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt64Descriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_wstring_type(uint32_t bound)
{
    TypeDescriptor pCharDescriptor;
    pCharDescriptor.mKind = TK_CHAR16;
    pCharDescriptor.mName = GenerateTypeName();

    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING16;
    pDescriptor.mName = GenerateTypeName();
    pDescriptor.mElementType = build_type(&pCharDescriptor);
    pDescriptor.mBound.push_back(bound);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    mTypesList.push_back(pNewTypeBuilder);
    return pNewTypeBuilder;
}

//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_type_object(TypeObject type_object)
//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_uri(const std::string& document_url, const std::string& type_name, IncludePathSeq include_paths);
//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_document(const std::string& document, const std::string& type_name, IncludePathSeq include_paths);

ResponseCode DynamicTypeBuilderFactory::delete_type(DynamicType* type)
{
    if (type != nullptr)
    {
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
    }
    return ResponseCode::RETCODE_OK;
}

DynamicType* DynamicTypeBuilderFactory::get_primitive_type(TypeKind kind)
{
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = kind;
    pDescriptor.mName = GenerateTypeName();
    DynamicType* pNewType = build_type(&pDescriptor);
    return pNewType;
}

bool DynamicTypeBuilderFactory::IsEmpty() const
{
    return mTypesList.empty();
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
