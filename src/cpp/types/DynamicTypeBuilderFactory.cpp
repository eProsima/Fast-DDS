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

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_array_type(const DynamicType* element_type,
    std::vector<uint32_t> bounds)
{
    if (element_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.mKind = TK_ARRAY;
        pDescriptor.mName = GenerateTypeName();
        pDescriptor.mBound = bounds;
        pDescriptor.mElementType = new DynamicType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bitmask_type(uint32_t bound)
{
    //TODO: Check bound is out of range
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_BITMASK;
    pDescriptor.mName = GenerateTypeName();
    pDescriptor.mBound.push_back(bound);

    TypeDescriptor pBoolDescriptor;
    pBoolDescriptor.mKind = TK_BOOLEAN;
    pBoolDescriptor.mName = GenerateTypeName();
    pDescriptor.mElementType = new DynamicType(&pBoolDescriptor);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
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
        pDescriptor.mKeyElementType = new DynamicType(key_element_type);
        pDescriptor.mElementType = new DynamicType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
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
        pDescriptor.mElementType = new DynamicType(element_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        mTypesList.push_back(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_string_type(uint32_t bound)
{
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING8;
    pDescriptor.mName = GenerateTypeName();
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
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_wstring_type(uint32_t bound)
{
    TypeDescriptor pDescriptor;
    pDescriptor.mKind = TK_STRING16;
    pDescriptor.mName = GenerateTypeName();
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
    DynamicType* pNewType = new DynamicType(&pDescriptor);
    mTypesList.push_back(pNewType);
    return pNewType;
}



} // namespace types
} // namespace fastrtps
} // namespace eprosima
