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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/md5.h>
#include <fastrtps/utils/string_convert.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {

static std::string get_type_name(
        octet kind)
{
    switch (kind)
    {
        // Primitive types, already defined (never will be asked, but ok)
        case eprosima::fastdds::dds::xtypes::TK_BOOLEAN: return TKNAME_BOOLEAN;
        case eprosima::fastdds::dds::xtypes::TK_INT16: return TKNAME_INT16;
        case eprosima::fastdds::dds::xtypes::TK_INT32: return TKNAME_INT32;
        case eprosima::fastdds::dds::xtypes::TK_UINT16: return TKNAME_UINT16;
        case eprosima::fastdds::dds::xtypes::TK_UINT32: return TKNAME_UINT32;
        case eprosima::fastdds::dds::xtypes::TK_FLOAT32: return TKNAME_FLOAT32;
        case eprosima::fastdds::dds::xtypes::TK_FLOAT64: return TKNAME_FLOAT64;
        case eprosima::fastdds::dds::xtypes::TK_CHAR8: return TKNAME_CHAR8;
        case eprosima::fastdds::dds::xtypes::TK_BYTE: return TKNAME_BYTE;
        case eprosima::fastdds::dds::xtypes::TK_INT64: return TKNAME_INT64;
        case eprosima::fastdds::dds::xtypes::TK_UINT64: return TKNAME_UINT64;
        case eprosima::fastdds::dds::xtypes::TK_FLOAT128: return TKNAME_FLOAT128;
        case eprosima::fastdds::dds::xtypes::TK_CHAR16: return TKNAME_CHAR16;
        /*
           case eprosima::fastdds::dds::xtypes::TK_STRING8: return TKNAME_STRING8;
           case eprosima::fastdds::dds::xtypes::TK_STRING16: return TKNAME_STRING16;
           case eprosima::fastdds::dds::xtypes::TK_ALIAS: return TKNAME_ALIAS;
           case eprosima::fastdds::dds::xtypes::TK_ENUM: return TKNAME_ENUM;
         */
        case eprosima::fastdds::dds::xtypes::TK_BITMASK: return TKNAME_BITMASK;
        /*
           case eprosima::fastdds::dds::xtypes::TK_ANNOTATION: return TKNAME_ANNOTATION;
           case eprosima::fastdds::dds::xtypes::TK_STRUCTURE: return TKNAME_STRUCTURE;
           case eprosima::fastdds::dds::xtypes::TK_UNION: return TKNAME_UNION;
         */
        case eprosima::fastdds::dds::xtypes::TK_BITSET: return TKNAME_BITSET;
        /*
           case eprosima::fastdds::dds::xtypes::TK_SEQUENCE: return TKNAME_SEQUENCE;
           case eprosima::fastdds::dds::xtypes::TK_ARRAY: return TKNAME_ARRAY;
           case eprosima::fastdds::dds::xtypes::TK_MAP: return TKNAME_MAP;
         */
        default:
            break;
    }
    return "UNDEF";
}

//static uint32_t s_typeNameCounter = 0;
static std::string GenerateTypeName(
        const std::string& kind)
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
        DynamicTypeBuilderFactory::delete_instance();
    }

};

static DynamicTypeBuilderFactoryReleaser s_releaser;
static DynamicTypeBuilderFactory* g_instance = nullptr;
DynamicTypeBuilderFactory* DynamicTypeBuilderFactory::get_instance()
{
    if (g_instance == nullptr)
    {
        g_instance = new DynamicTypeBuilderFactory();
    }
    return g_instance;
}

ReturnCode_t DynamicTypeBuilderFactory::delete_instance()
{
    if (g_instance != nullptr)
    {
        delete g_instance;
        g_instance = nullptr;
        return eprosima::fastdds::dds::RETCODE_OK;
    }
    return eprosima::fastdds::dds::RETCODE_ERROR;
}

DynamicTypeBuilderFactory::DynamicTypeBuilderFactory()
{
}

DynamicTypeBuilderFactory::~DynamicTypeBuilderFactory()
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::unique_lock<std::recursive_mutex> scoped(mutex_);
    for (auto it = builders_list_.begin(); it != builders_list_.end(); ++it)
    {
        delete *it;
    }
    builders_list_.clear();
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
}

void DynamicTypeBuilderFactory::add_builder_to_list(
        DynamicTypeBuilder* pBuilder)
{
    (void)pBuilder;
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::unique_lock<std::recursive_mutex> scoped(mutex_);
    builders_list_.push_back(pBuilder);
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
}

DynamicType_ptr DynamicTypeBuilderFactory::build_type(
        DynamicType_ptr other)
{
    return other;
}

DynamicType_ptr DynamicTypeBuilderFactory::create_type(
        const TypeDescriptor* descriptor,
        const std::string& name)
{
    if (descriptor != nullptr)
    {
        DynamicType_ptr pNewType(new DynamicType(descriptor));
        if (name.length() > 0)
        {
            pNewType->set_name(name);
        }
        return pNewType;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building type, invalid input descriptor");
        return DynamicType_ptr(nullptr);
    }
}

DynamicType_ptr DynamicTypeBuilderFactory::create_type(
        const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        DynamicType_ptr pNewType(new DynamicType(other));
        return pNewType;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building type, invalid input parameter");
        return DynamicType_ptr(nullptr);
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_alias_builder(
        DynamicTypeBuilder* base_type,
        const std::string& sName)
{
    if (base_type != nullptr)
    {
        DynamicType_ptr pType = create_type(base_type);
        if (pType != nullptr)
        {
            return create_alias_builder(pType, sName);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, Error creating dynamic type");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_alias_builder(
        DynamicType_ptr base_type,
        const std::string& sName)
{
    if (base_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_ALIAS;
        pDescriptor.base_type_ = base_type;
        if (sName.length() > 0)
        {
            pDescriptor.name_ = sName;
        }
        else
        {
            //pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_ALIAS));
            pDescriptor.name_ = base_type->get_name();
        }

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        add_builder_to_list(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_array_builder(
        const DynamicTypeBuilder* element_type,
        const std::vector<uint32_t>& bounds)
{
    if (element_type != nullptr)
    {
        DynamicType_ptr pType = create_type(element_type);
        if (pType != nullptr)
        {
            return create_array_builder(pType, bounds);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating array, error creating dynamic type");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating array, element_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_array_builder(
        const DynamicType_ptr type,
        const std::vector<uint32_t>& bounds)
{
    if (type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_ARRAY;
        // pDescriptor.name_ = TypeNamesGenerator::get_array_type_name(type->get_name(), bounds, false);
        pDescriptor.element_type_ = type;
        pDescriptor.bound_ = bounds;

        for (uint32_t i = 0; i < pDescriptor.bound_.size(); ++i)
        {
            if (pDescriptor.bound_[i] == 0)
            {
                pDescriptor.bound_[i] = MAX_ELEMENTS_COUNT;
            }
        }


        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        add_builder_to_list(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating array, element_type must be valid");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bitmask_builder(
        uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pBoolDescriptor;
        pBoolDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_BOOLEAN;
        pBoolDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BOOLEAN));

        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_BITMASK;
        // TODO review on implementation for IDL
        pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BITMASK));
        pDescriptor.element_type_ = create_type(&pBoolDescriptor);
        pDescriptor.bound_.push_back(bound);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        add_builder_to_list(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bitset_builder()
{
    TypeDescriptor pDescriptor;
    pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_BITSET;
    // TODO Review on implementation for IDL
    pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BITSET));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bool_builder()
{
    TypeDescriptor pBoolDescriptor;
    pBoolDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_BOOLEAN;
    pBoolDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BOOLEAN));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pBoolDescriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_byte_builder()
{
    TypeDescriptor pByteDescriptor;
    pByteDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_BYTE;
    pByteDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BYTE));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pByteDescriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_char8_builder()
{
    TypeDescriptor pChar8Descriptor;
    pChar8Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_CHAR8;
    pChar8Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_CHAR8));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar8Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_char16_builder()
{
    TypeDescriptor pChar16Descriptor;
    pChar16Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_CHAR16;
    pChar16Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_CHAR16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pChar16Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicType_ptr DynamicTypeBuilderFactory::create_annotation_primitive(
        const std::string& name)
{
    TypeDescriptor pEnumDescriptor;
    pEnumDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_ANNOTATION;
    //pEnumDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_ANNOTATION));
    pEnumDescriptor.name_ = name;

    return DynamicType_ptr(new DynamicType(&pEnumDescriptor));
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_enum_builder()
{
    TypeDescriptor pEnumDescriptor;
    pEnumDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_ENUM;
    //pEnumDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_ENUM));
    // Enum currently is an alias for uint32_t
    pEnumDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UINT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pEnumDescriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float32_builder()
{
    TypeDescriptor pFloat32Descriptor;
    pFloat32Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_FLOAT32;
    pFloat32Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_FLOAT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat32Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float64_builder()
{
    TypeDescriptor pFloat64Descriptor;
    pFloat64Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_FLOAT64;
    pFloat64Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_FLOAT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat64Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float128_builder()
{
    TypeDescriptor pFloat128Descriptor;
    pFloat128Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_FLOAT128;
    pFloat128Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_FLOAT128));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pFloat128Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int16_builder()
{
    TypeDescriptor pInt16Descriptor;
    pInt16Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_INT16;
    pInt16Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_INT16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt16Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int32_builder()
{
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_INT32;
    pInt32Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_INT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt32Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int64_builder()
{
    TypeDescriptor pInt64Descriptor;
    pInt64Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_INT64;
    pInt64Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_INT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pInt64Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_map_builder(
        DynamicTypeBuilder* key_element_type,
        DynamicTypeBuilder* element_type,
        uint32_t bound)
{
    if (key_element_type != nullptr && element_type != nullptr)
    {
        DynamicType_ptr pKeyType = create_type(key_element_type);
        DynamicType_ptr pValueType = create_type(element_type);
        if (pKeyType != nullptr && pValueType != nullptr)
        {
            return create_map_builder(pKeyType, pValueType, bound);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating map, Error creating dynamic types.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating map, element_type and key_element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_map_builder(
        DynamicType_ptr key_type,
        DynamicType_ptr value_type,
        uint32_t bound)
{
    if (key_type != nullptr && value_type != nullptr)
    {
        if (bound == BOUND_UNLIMITED)
        {
            bound = MAX_ELEMENTS_COUNT;
        }

        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_MAP;
        //pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_MAP));
        pDescriptor.bound_.push_back(bound);
        pDescriptor.key_element_type_ = key_type;
        pDescriptor.element_type_ = value_type;

        // pDescriptor.name_ = TypeNamesGenerator::get_map_type_name(key_type->get_name(), value_type->get_name(),
        //                 bound, false);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        add_builder_to_list(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating map, element_type and key_element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_sequence_builder(
        const DynamicTypeBuilder* element_type,
        uint32_t bound)
{
    if (element_type != nullptr)
    {
        DynamicType_ptr pType = create_type(element_type);
        if (pType != nullptr)
        {
            return create_sequence_builder(pType, bound);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating sequence, error creating dynamic type.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating sequence, element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_sequence_builder(
        const DynamicType_ptr type,
        uint32_t bound)
{
    if (type != nullptr)
    {
        if (bound == BOUND_UNLIMITED)
        {
            bound = MAX_ELEMENTS_COUNT;
        }

        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_SEQUENCE;
        // pDescriptor.name_ = TypeNamesGenerator::get_sequence_type_name(type->get_name(), bound, false);
        pDescriptor.bound_.push_back(bound);
        pDescriptor.element_type_ = type;

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        add_builder_to_list(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating sequence, element_type must be valid.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_string_builder(
        uint32_t bound)
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor pCharDescriptor;
    pCharDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_CHAR8;
    pCharDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_CHAR8));

    TypeDescriptor pDescriptor;
    pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_STRING8;
    //pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_STRING8));
    pDescriptor.element_type_ = create_type(&pCharDescriptor);
    pDescriptor.bound_.push_back(bound);

    // pDescriptor.name_ = TypeNamesGenerator::get_string_type_name(bound, false, true);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_child_struct_builder(
        DynamicTypeBuilder* parent_type)
{
    if (parent_type != nullptr &&
            (parent_type->get_kind() == eprosima::fastdds::dds::xtypes::TK_STRUCTURE ||
            parent_type->get_kind() == eprosima::fastdds::dds::xtypes::TK_BITSET))
    {
        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = parent_type->get_kind();
        pDescriptor.name_ = GenerateTypeName(get_type_name(parent_type->get_kind()));
        pDescriptor.base_type_ = create_type(parent_type);

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
        add_builder_to_list(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating child struct, invalid input type.");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_struct_builder()
{
    TypeDescriptor pDescriptor;
    pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_STRUCTURE;
    pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_STRUCTURE));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_custom_builder(
        const TypeDescriptor* descriptor,
        const std::string& name /*= ""*/)
{
    if (descriptor != nullptr)
    {
        TypeKind kind = descriptor->get_kind();
        if (kind == eprosima::fastdds::dds::xtypes::TK_BOOLEAN || kind == eprosima::fastdds::dds::xtypes::TK_BYTE ||
                kind == eprosima::fastdds::dds::xtypes::TK_INT16 ||
                kind == eprosima::fastdds::dds::xtypes::TK_INT32 ||
                kind == eprosima::fastdds::dds::xtypes::TK_INT64 || kind == eprosima::fastdds::dds::xtypes::TK_UINT16 ||
                kind == eprosima::fastdds::dds::xtypes::TK_UINT32 ||
                kind == eprosima::fastdds::dds::xtypes::TK_UINT64 ||
                kind == eprosima::fastdds::dds::xtypes::TK_FLOAT32 ||
                kind == eprosima::fastdds::dds::xtypes::TK_FLOAT64 ||
                kind == eprosima::fastdds::dds::xtypes::TK_FLOAT128 ||
                kind == eprosima::fastdds::dds::xtypes::TK_CHAR8 ||
                kind == eprosima::fastdds::dds::xtypes::TK_CHAR16 ||
                kind == eprosima::fastdds::dds::xtypes::TK_STRING8 ||
                kind == eprosima::fastdds::dds::xtypes::TK_STRING16 ||
                kind == eprosima::fastdds::dds::xtypes::TK_ALIAS ||
                kind == eprosima::fastdds::dds::xtypes::TK_ENUM || kind == eprosima::fastdds::dds::xtypes::TK_BITMASK ||
                kind == eprosima::fastdds::dds::xtypes::TK_STRUCTURE ||
                kind == eprosima::fastdds::dds::xtypes::TK_UNION ||
                kind == eprosima::fastdds::dds::xtypes::TK_BITSET ||
                kind == eprosima::fastdds::dds::xtypes::TK_SEQUENCE ||
                kind == eprosima::fastdds::dds::xtypes::TK_ARRAY ||
                kind == eprosima::fastdds::dds::xtypes::TK_MAP ||
                kind == eprosima::fastdds::dds::xtypes::TK_ANNOTATION)
        {
            DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(descriptor);
            if (pNewType != nullptr && name.length() > 0)
            {
                pNewType->set_name(name);
            }
            add_builder_to_list(pNewType);
            return pNewType;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error creating type, unsupported type kind: " << static_cast<uint32_t>(kind));
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating type, invalid input descriptor.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_builder_copy(
        const DynamicTypeBuilder* type)
{
    if (type != nullptr)
    {
        DynamicTypeBuilder* pNewType = new DynamicTypeBuilder(type);
        add_builder_to_list(pNewType);
        return pNewType;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating type, invalid input type.");
    }
    return nullptr;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint16_builder()
{
    TypeDescriptor pUInt16Descriptor;
    pUInt16Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_UINT16;
    pUInt16Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UINT16));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt16Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint32_builder()
{
    TypeDescriptor pUInt32Descriptor;
    pUInt32Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_UINT32;
    pUInt32Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UINT32));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt32Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint64_builder()
{
    TypeDescriptor pUInt64Descriptor;
    pUInt64Descriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_UINT64;
    pUInt64Descriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UINT64));

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUInt64Descriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_union_builder(
        DynamicTypeBuilder* discriminator_type)
{
    if (discriminator_type != nullptr && discriminator_type->is_discriminator_type())
    {
        DynamicType_ptr pType = create_type(discriminator_type);
        if (pType != nullptr)
        {
            return create_union_builder(pType);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building Union, Error creating discriminator type");
            return nullptr;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building Union, invalid discriminator type");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_union_builder(
        DynamicType_ptr discriminator_type)
{
    if (discriminator_type != nullptr && discriminator_type->is_discriminator_type())
    {
        TypeDescriptor pUnionDescriptor;
        pUnionDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_UNION;
        pUnionDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UNION));
        pUnionDescriptor.discriminator_type_ = discriminator_type;

        DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pUnionDescriptor);
        add_builder_to_list(pNewTypeBuilder);
        return pNewTypeBuilder;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building Union, invalid discriminator type");
        return nullptr;
    }
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_wstring_builder(
        uint32_t bound)
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor pCharDescriptor;
    pCharDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_CHAR16;
    pCharDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_CHAR16));

    TypeDescriptor pDescriptor;
    pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_STRING16;
    //pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_STRING16));
    pDescriptor.element_type_ = create_type(&pCharDescriptor);
    pDescriptor.bound_.push_back(bound);

    // pDescriptor.name_ = TypeNamesGenerator::get_string_type_name(bound, true, true);

    DynamicTypeBuilder* pNewTypeBuilder = new DynamicTypeBuilder(&pDescriptor);
    add_builder_to_list(pNewTypeBuilder);
    return pNewTypeBuilder;
}

ReturnCode_t DynamicTypeBuilderFactory::delete_builder(
        DynamicTypeBuilder* builder)
{
    if (builder != nullptr)
    {
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        std::unique_lock<std::recursive_mutex> scoped(mutex_);
        auto it = std::find(builders_list_.begin(), builders_list_.end(), builder);
        if (it != builders_list_.end())
        {
            builders_list_.erase(it);
            delete builder;
        }
        else
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "The given type has been deleted previously.");
            return eprosima::fastdds::dds::RETCODE_ALREADY_DELETED;
        }
#else
        delete builder;
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilderFactory::delete_type(
        DynamicType* type)
{
    if (type != nullptr)
    {
        delete type;
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

DynamicType_ptr DynamicTypeBuilderFactory::get_primitive_type(
        octet kind)
{
    TypeDescriptor pDescriptor;
    pDescriptor.kind_ = kind;
    pDescriptor.name_ = GenerateTypeName(get_type_name(kind));
    return create_type(&pDescriptor);
}

bool DynamicTypeBuilderFactory::is_empty() const
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    return builders_list_.empty();
#else
    return true;
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
}

void DynamicTypeBuilderFactory::set_annotation_default_value(
        AnnotationParameterValue& apv,
        const MemberDescriptor* member) const
{
    switch (member->get_kind())
    {
        case eprosima::fastdds::dds::xtypes::TK_BOOLEAN:
        {
            std::string value = member->get_default_value();
            std::transform(value.begin(), value.end(), value.begin(),
                    [](unsigned char c)
                    {
                        return static_cast<char>(std::tolower(c));
                    });
            apv.boolean_value(value.compare("0") != 0 || value.compare(CONST_TRUE) == 0);
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_BYTE:
        {
            apv.byte_value(static_cast<uint8_t>(std::stoul(member->get_default_value())));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_INT16:
        {
            apv.int16_value(static_cast<int16_t>(std::stoi(member->get_default_value())));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_INT32:
        {
            apv.int32_value(static_cast<int32_t>(std::stoi(member->get_default_value())));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_INT64:
        {
            apv.int64_value(static_cast<int64_t>(std::stoll(member->get_default_value())));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_UINT16:
        {
            apv.uint_16_value(static_cast<uint16_t>(std::stoul(member->get_default_value())));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_UINT32:
        {
            apv.uint32_value(static_cast<uint32_t>(std::stoul(member->get_default_value())));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_UINT64:
        {
            apv.uint64_value(static_cast<uint64_t>(std::stoull(member->get_default_value())));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_FLOAT32:
        {
            apv.float32_value(std::stof(member->get_default_value()));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_FLOAT64:
        {
            apv.float64_value(std::stod(member->get_default_value()));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_FLOAT128:
        {
            apv.float128_value(std::stold(member->get_default_value()));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_CHAR8:
        {
            apv.char_value(member->get_default_value().c_str()[0]);
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_CHAR16:
        {
            apv.wchar_value(wstring_from_bytes(member->get_default_value()).c_str()[0]);
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_STRING8:
        {
            apv.string8_value(member->get_default_value());
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_STRING16:
        {
            apv.string16_value(wstring_from_bytes(member->get_default_value()));
        }
        break;
        case eprosima::fastdds::dds::xtypes::TK_ENUM:
        {
            // TODO Translate from enum value name to integer value
            apv.enumerated_value(static_cast<int32_t>(std::stoul(member->get_default_value())));
        }
        break;
        default:
            break;
    }
}

DynamicType_ptr DynamicTypeBuilderFactory::create_alias_type(
        DynamicTypeBuilder* base_type,
        const std::string& sName)
{
    if (base_type != nullptr)
    {
        DynamicType_ptr pType = create_type(base_type);
        if (pType != nullptr)
        {
            return create_alias_type(pType, sName);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, Error creating dynamic type");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return DynamicType_ptr(nullptr);
}

DynamicType_ptr DynamicTypeBuilderFactory::create_alias_type(
        DynamicType_ptr base_type,
        const std::string& sName)
{
    if (base_type != nullptr)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_ALIAS;
        pDescriptor.base_type_ = base_type;
        if (sName.length() > 0)
        {
            pDescriptor.name_ = sName;
        }
        else
        {
            pDescriptor.name_ = base_type->get_name();
        }

        return create_type(&pDescriptor, sName);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, base_type must be valid");
    }
    return DynamicType_ptr(nullptr);
}

DynamicType_ptr DynamicTypeBuilderFactory::create_int32_type()
{
    TypeDescriptor pInt32Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_INT32)),
            eprosima::fastdds::dds::xtypes::TK_INT32);
    return DynamicType_ptr(new DynamicType(&pInt32Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_uint32_type()
{
    TypeDescriptor pUint32Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UINT32)),
            eprosima::fastdds::dds::xtypes::TK_UINT32);
    return DynamicType_ptr(new DynamicType(&pUint32Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_int16_type()
{
    TypeDescriptor pInt16Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_INT16)),
            eprosima::fastdds::dds::xtypes::TK_INT16);
    return DynamicType_ptr(new DynamicType(&pInt16Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_uint16_type()
{
    TypeDescriptor pUint16Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UINT16)),
            eprosima::fastdds::dds::xtypes::TK_UINT16);
    return DynamicType_ptr(new DynamicType(&pUint16Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_int64_type()
{
    TypeDescriptor pInt64Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_INT64)),
            eprosima::fastdds::dds::xtypes::TK_INT64);
    return DynamicType_ptr(new DynamicType(&pInt64Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_uint64_type()
{
    TypeDescriptor pUint64Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_UINT64)),
            eprosima::fastdds::dds::xtypes::TK_UINT64);
    return DynamicType_ptr(new DynamicType(&pUint64Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_float32_type()
{
    TypeDescriptor pFloat32Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_FLOAT32)),
            eprosima::fastdds::dds::xtypes::TK_FLOAT32);
    return DynamicType_ptr(new DynamicType(&pFloat32Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_float64_type()
{
    TypeDescriptor pFloat64Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_FLOAT64)),
            eprosima::fastdds::dds::xtypes::TK_FLOAT64);
    return DynamicType_ptr(new DynamicType(&pFloat64Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_float128_type()
{
    TypeDescriptor pFloat128Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_FLOAT128)),
            eprosima::fastdds::dds::xtypes::TK_FLOAT128);
    return DynamicType_ptr(new DynamicType(&pFloat128Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_char8_type()
{
    TypeDescriptor pChar8Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_CHAR8)),
            eprosima::fastdds::dds::xtypes::TK_CHAR8);
    return DynamicType_ptr(new DynamicType(&pChar8Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_char16_type()
{
    TypeDescriptor pChar16Descriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_CHAR16)),
            eprosima::fastdds::dds::xtypes::TK_CHAR16);
    return DynamicType_ptr(new DynamicType(&pChar16Descriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_bool_type()
{
    TypeDescriptor pBoolDescriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BOOLEAN)),
            eprosima::fastdds::dds::xtypes::TK_BOOLEAN);
    return DynamicType_ptr(new DynamicType(&pBoolDescriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_byte_type()
{
    TypeDescriptor pByteDescriptor(GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BYTE)),
            eprosima::fastdds::dds::xtypes::TK_BYTE);
    return DynamicType_ptr(new DynamicType(&pByteDescriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_string_type(
        uint32_t bound /*= MAX_STRING_LENGTH*/)
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_STRING_LENGTH;
    }
    TypeDescriptor pStringDescriptor("", eprosima::fastdds::dds::xtypes::TK_STRING8);
    // pStringDescriptor.name_ = TypeNamesGenerator::get_string_type_name(bound, false, true);
    pStringDescriptor.element_type_ = create_char8_type();
    pStringDescriptor.bound_.push_back(bound);

    return DynamicType_ptr(new DynamicType(&pStringDescriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_wstring_type(
        uint32_t bound /*= MAX_STRING_LENGTH*/)
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor pStringDescriptor("", eprosima::fastdds::dds::xtypes::TK_STRING16);
    // pStringDescriptor.name_ = TypeNamesGenerator::get_string_type_name(bound, true, true);
    pStringDescriptor.element_type_ = create_char16_type();
    pStringDescriptor.bound_.push_back(bound);

    return DynamicType_ptr(new DynamicType(&pStringDescriptor));
}

DynamicType_ptr DynamicTypeBuilderFactory::create_bitset_type(
        uint32_t bound)
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor pDescriptor;
        pDescriptor.kind_ = eprosima::fastdds::dds::xtypes::TK_BITSET;
        pDescriptor.name_ = GenerateTypeName(get_type_name(eprosima::fastdds::dds::xtypes::TK_BITSET));
        pDescriptor.bound_.push_back(bound);
        return create_type(&pDescriptor, pDescriptor.name_);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return DynamicType_ptr(nullptr);
}

// void DynamicTypeBuilderFactory::apply_type_annotations(
//         AppliedAnnotationSeq& annotations,
//         const TypeDescriptor* descriptor) const
// {
//     for (const AnnotationDescriptor* annotation : descriptor->annotation_)
//     {
//         AppliedAnnotation ann;
//         ann.annotation_typeid(
//             *TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(annotation->type_->get_name()));
//         std::map<std::string, std::string> values;
//         annotation->get_all_value(values);
//         for (auto it : values)
//         {
//             AppliedAnnotationParameter ann_param;
//             MD5 message_hash(it.first);
//             for (int i = 0; i < 4; ++i)
//             {
//                 ann_param.paramname_hash()[i] = message_hash.digest[i];
//             }
//             AnnotationParameterValue param_value;
//             param_value._d(annotation->type_->get_kind());
//             param_value.from_string(it.second);
//             ann_param.value(param_value);
//             ann.param_seq().push_back(ann_param);
//         }
//         annotations.push_back(ann);
//     }
// }

} // namespace types
} // namespace fastrtps
} // namespace eprosima
