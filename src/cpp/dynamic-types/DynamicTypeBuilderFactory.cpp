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
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/TypeNamesGenerator.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/utils/md5.h>
#include <fastrtps/utils/string_convert.hpp>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastrtps {
namespace types {

void dtypes_memory_check::reset() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    builders_list_.clear();
    types_list_.clear();
}

bool dtypes_memory_check::is_empty() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    return builders_list_.empty() && types_list_.empty();
}

void dtypes_memory_check::add_primitive(const DynamicTypeBuilder* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    primitive_builders_list_.insert(pBuilder);
}

void dtypes_memory_check::add_primitive(const DynamicType* pType) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    primitive_types_list_.insert(pType);
}

bool dtypes_memory_check::add(const DynamicTypeBuilder* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if(!builders_list_.insert(pBuilder).second)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type builder has been inserted previously.");
        return false;
    }
    return true;
}

bool dtypes_memory_check::remove(const DynamicTypeBuilder* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if(!builders_list_.erase(pBuilder)
        && primitive_builders_list_.find(pBuilder) != primitive_builders_list_.end())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type builder has been already removed.");
        return false;
    }
    return true;
}

bool dtypes_memory_check::add(const DynamicType* pType) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if(!types_list_.insert(pType).second)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type has been inserted previously.");
        return false;
    }
    return true;
}

bool dtypes_memory_check::remove(const DynamicType* type) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if(!types_list_.erase(type)
        && primitive_types_list_.find(type) != primitive_types_list_.end())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type has been already removed.");
        return false;
    }
    return true;
}

static std::string get_type_name(
        TypeKind kind)
{
    switch (kind)
    {
        // Primitive types, already defined (never will be asked, but ok)
        case TypeKind::TK_BOOLEAN: return TKNAME_BOOLEAN;
        case TypeKind::TK_INT16: return TKNAME_INT16;
        case TypeKind::TK_INT32: return TKNAME_INT32;
        case TypeKind::TK_UINT16: return TKNAME_UINT16;
        case TypeKind::TK_UINT32: return TKNAME_UINT32;
        case TypeKind::TK_FLOAT32: return TKNAME_FLOAT32;
        case TypeKind::TK_FLOAT64: return TKNAME_FLOAT64;
        case TypeKind::TK_CHAR8: return TKNAME_CHAR8;
        case TypeKind::TK_BYTE: return TKNAME_BYTE;
        case TypeKind::TK_INT64: return TKNAME_INT64;
        case TypeKind::TK_UINT64: return TKNAME_UINT64;
        case TypeKind::TK_FLOAT128: return TKNAME_FLOAT128;
        case TypeKind::TK_CHAR16: return TKNAME_CHAR16;
        /*
           case TypeKind::TK_STRING8: return TKNAME_STRING8;
           case TypeKind::TK_STRING16: return TKNAME_STRING16;
           case TypeKind::TK_ALIAS: return TKNAME_ALIAS;
           case TypeKind::TK_ENUM: return TKNAME_ENUM;
         */
        case TypeKind::TK_BITMASK: return TKNAME_BITMASK;
        /*
           case TypeKind::TK_ANNOTATION: return TKNAME_ANNOTATION;
           case TypeKind::TK_STRUCTURE: return TKNAME_STRUCTURE;
           case TypeKind::TK_UNION: return TKNAME_UNION;
         */
        case TypeKind::TK_BITSET: return TKNAME_BITSET;
        /*
           case TypeKind::TK_SEQUENCE: return TKNAME_SEQUENCE;
           case TypeKind::TK_ARRAY: return TKNAME_ARRAY;
           case TypeKind::TK_MAP: return TKNAME_MAP;
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

DynamicTypeBuilderFactory& DynamicTypeBuilderFactory::get_instance() noexcept
{
    // C++ standard requires preserve global construction order
    // make sure the dynamic tracker lifespan is larger than the factory one
    dynamic_tracker_interface::get_dynamic_tracker();

    // C++11 guarantees the construction to be atomic
    static DynamicTypeBuilderFactory instance;
    return instance;
}

ReturnCode_t DynamicTypeBuilderFactory::delete_instance() noexcept
{
    get_instance().reset();
    return ReturnCode_t::RETCODE_OK;
}

void DynamicTypeBuilderFactory::reset()
{
    dynamic_tracker_interface::get_dynamic_tracker().reset();
}

DynamicTypeBuilderFactory::~DynamicTypeBuilderFactory()
{
    assert(is_empty());
    reset();
}

void DynamicTypeBuilderFactory::after_construction(DynamicTypeBuilder* pBuilder)
{
    dynamic_tracker_interface::get_dynamic_tracker().add(pBuilder);
}

void DynamicTypeBuilderFactory::before_destruction(DynamicTypeBuilder* builder)
{
    dynamic_tracker_interface::get_dynamic_tracker().remove(builder);
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_builder(const TypeDescriptor& td) noexcept
{
    if (td.is_consistent())
    {
        return std::allocate_shared<DynamicTypeBuilder>(
            get_allocator(),
            DynamicTypeBuilder::use_the_create_method{},
            td);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building type builder. The current descriptor isn't consistent.");
    }

    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_builder_copy(const DynamicType& type) noexcept
{
    assert(type.is_consistent());
    return create_builder(type);
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_builder_copy(const DynamicTypeBuilder& builder) noexcept
{
    assert(builder.is_consistent());
    return create_builder(builder);
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_alias_builder(
        const DynamicTypeBuilder& base_type,
        const std::string& sName)
{
    DynamicType_ptr pType = base_type.build();
    if (pType)
    {
        return create_alias_builder(*pType, sName);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, Error creating dynamic type");
    }

    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_alias_builder(
        const DynamicType& base_type,
        const std::string& sName)
{
    // Get a builder copy of base_type
    DynamicTypeBuilder_ptr builder = create_builder_copy(base_type);

    if(builder)
    {
        builder->set_kind(TypeKind::TK_ALIAS);
        builder->set_base_type(base_type.shared_from_this());

        if (sName.length() > 0)
        {
            builder->set_name(sName);
        }

        return builder;
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating alias type, Error creating dynamic type builder");

    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_array_builder(
        const DynamicTypeBuilder& element_type,
        const std::vector<uint32_t>& bounds)
{
    DynamicType_ptr pType = element_type.build();
    if (pType)
    {
        return create_array_builder(*pType, bounds);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating array, error creating dynamic type");
    }

    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_array_builder(
        const DynamicType& type,
        const std::vector<uint32_t>& bounds) noexcept
{
    TypeDescriptor descriptor;
    descriptor.set_kind(TypeKind::TK_ARRAY);
    descriptor.set_name(TypeNamesGenerator::get_array_type_name(type.get_name(), bounds, false));
    descriptor.element_type_ = type.shared_from_this();
    descriptor.bound_ = bounds;

    for (uint32_t i = 0; i < descriptor.bound_.size(); ++i)
    {
        if (descriptor.bound_[i] == 0)
        {
            descriptor.bound_[i] = MAX_ELEMENTS_COUNT;
        }
    }

    return create_builder(descriptor);
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_bitmask_builder(
        uint32_t bound) noexcept
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor descriptor;
        descriptor.set_kind(TypeKind::TK_BITMASK);
        // TODO review on implementation for IDL
        descriptor.set_name(GenerateTypeName(get_type_name(TypeKind::TK_BITMASK)));
        descriptor.element_type_ = create_bool_type();
        descriptor.bound_.push_back(bound);

        return create_builder(descriptor);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }
    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_bitset_builder(uint32_t bound) noexcept
{
    if (bound <= MAX_BITMASK_LENGTH)
    {
        TypeDescriptor descriptor;
        descriptor.set_kind(TypeKind::TK_BITSET);
        // TODO Review on implementation for IDL
        descriptor.set_name(GenerateTypeName(get_type_name(TypeKind::TK_BITSET)));
        descriptor.bound_.push_back(bound);
        return create_builder(descriptor);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error creating bitmask, length exceeds the maximum value '" << MAX_BITMASK_LENGTH << "'");
    }

    return {};
}

// Beware! this method doesn't return a static object but creates it
DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::new_primitive_builder(TypeKind kind) noexcept
{
    // create on heap
    DynamicTypeBuilder_ptr builder = std::make_shared<DynamicTypeBuilder>(
                DynamicTypeBuilder::use_the_create_method{},
                TypeDescriptor{GenerateTypeName(get_type_name(kind)), kind},
                true); // will be a static object
    // notify the tracker
    dynamic_tracker_interface::get_dynamic_tracker().add_primitive(builder.get());
    return builder;
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_bool_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_BOOLEAN>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_byte_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_BYTE>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_char8_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_CHAR8>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_char16_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_CHAR16>();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_annotation_primitive(
        const std::string& name)
{
    TypeDescriptor descriptor;
    descriptor.set_kind(TypeKind::TK_ANNOTATION);
    descriptor.set_name(name);
    return create_builder(descriptor)->build();
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_enum_builder()
{
    TypeDescriptor pEnumDescriptor;
    pEnumDescriptor.set_kind(TypeKind::TK_ENUM);
    // Enum currently is an alias for uint32_t
    pEnumDescriptor.set_name(GenerateTypeName(get_type_name(TypeKind::TK_UINT32)));
    return create_builder(pEnumDescriptor);
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_float32_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_FLOAT32>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_float64_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_FLOAT64>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_float128_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_FLOAT128>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_int16_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_INT16>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_int32_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_INT32>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_int64_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_INT64>();
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_map_builder(
        const DynamicTypeBuilder& key_element_type,
        const DynamicTypeBuilder& element_type,
        uint32_t bound)
{
    DynamicType_ptr pKeyType = key_element_type.build();
    DynamicType_ptr pValueType = element_type.build();
    if (pKeyType && pValueType)
    {
        return create_map_builder(*pKeyType, *pValueType, bound);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating map, Error creating dynamic types.");
    }

    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_map_builder(
        const DynamicType& key_type,
        const DynamicType& value_type,
        uint32_t bound) noexcept
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_ELEMENTS_COUNT;
    }

    TypeDescriptor descriptor;
    descriptor.set_kind(TypeKind::TK_MAP);
    descriptor.bound_.push_back(bound);
    descriptor.key_element_type_ = key_type.shared_from_this();
    descriptor.element_type_ = value_type.shared_from_this();
    descriptor.set_name(
            TypeNamesGenerator::get_map_type_name(
                    key_type.get_name(),
                    value_type.get_name(),
            bound, false));
    return create_builder(descriptor);
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_sequence_builder(
        const DynamicTypeBuilder& element_type,
        uint32_t bound)
{
    DynamicType_ptr pType = element_type.build();
    if (pType)
    {
        return create_sequence_builder(*pType, bound);
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating sequence, error creating dynamic type.");
    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_sequence_builder(
        const DynamicType& type,
        uint32_t bound) noexcept
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_ELEMENTS_COUNT;
    }

    TypeDescriptor descriptor;
    descriptor.set_kind(TypeKind::TK_SEQUENCE);
    descriptor.set_name(TypeNamesGenerator::get_sequence_type_name(type.get_name(), bound, false));
    descriptor.bound_.push_back(bound);
    descriptor.element_type_ = type.shared_from_this();

    return create_builder(descriptor);
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_string_builder(
        uint32_t bound)
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor descriptor;
    descriptor.set_kind(TypeKind::TK_STRING8);
    descriptor.set_name(TypeNamesGenerator::get_string_type_name(bound, false, true));
    descriptor.element_type_ = create_char8_type();
    descriptor.bound_.push_back(bound);

    return create_builder(descriptor);
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_child_struct_builder(
        const DynamicTypeBuilder& parent_type)
{
    auto kind = parent_type.get_kind();
    if (kind == TypeKind::TK_STRUCTURE || kind == TypeKind::TK_BITSET)
    {
        TypeDescriptor descriptor;
        descriptor.set_kind(kind);
        descriptor.set_name(GenerateTypeName(get_type_name(kind)));
        descriptor.base_type_ = parent_type.build();

        return create_builder(descriptor);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating child struct, invalid input type.");
        return {};
    }
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_struct_builder()
{
    TypeDescriptor descriptor;
    descriptor.set_kind(TypeKind::TK_STRUCTURE);
    descriptor.set_name(GenerateTypeName(get_type_name(TypeKind::TK_STRUCTURE)));

    return create_builder(descriptor);
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_uint16_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_UINT16>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_uint32_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_UINT32>();
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_uint64_builder() noexcept
{
    return create_primitive_builder<TypeKind::TK_UINT64>();
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_union_builder(
        const DynamicTypeBuilder& discriminator_type)
{
    if (discriminator_type.is_discriminator_type())
    {
        DynamicType_ptr pType = discriminator_type.build();
        if (pType)
        {
            return create_union_builder(*pType);
        }

        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building Union, Error creating discriminator type");
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building Union, invalid discriminator type");
    }

    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_union_builder(
        const DynamicType& discriminator_type)
{
    if (discriminator_type.is_discriminator_type())
    {
        TypeDescriptor descriptor;
        descriptor.set_kind(TypeKind::TK_UNION);
        descriptor.set_name(GenerateTypeName(get_type_name(TypeKind::TK_UNION)));
        descriptor.discriminator_type_ = discriminator_type.shared_from_this();

        return create_builder(descriptor);
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building Union, invalid discriminator type");
    return {};
}

DynamicTypeBuilder_ptr DynamicTypeBuilderFactory::create_wstring_builder(
        uint32_t bound)
{
    if (bound == BOUND_UNLIMITED)
    {
        bound = MAX_STRING_LENGTH;
    }

    TypeDescriptor descriptor;
    descriptor.kind_ = TypeKind::TK_STRING16;
    descriptor.name_ = TypeNamesGenerator::get_string_type_name(bound, true, true);
    descriptor.element_type_ = create_char16_type();
    descriptor.bound_.push_back(bound);

    return create_builder(descriptor);
}

ReturnCode_t DynamicTypeBuilderFactory::delete_builder(
        DynamicTypeBuilder* builder) noexcept
{
    return dynamic_tracker_interface::get_dynamic_tracker().remove(builder)
            ? ReturnCode_t::RETCODE_OK : ReturnCode_t::RETCODE_ALREADY_DELETED;
}

ReturnCode_t DynamicTypeBuilderFactory::delete_type(
        DynamicType* type) noexcept
{
    return dynamic_tracker_interface::get_dynamic_tracker().remove(type)
            ? ReturnCode_t::RETCODE_OK : ReturnCode_t::RETCODE_ALREADY_DELETED;
}

DynamicTypeBuilder_cptr& DynamicTypeBuilderFactory::create_primitive_builder(TypeKind kind) noexcept
{
    static DynamicTypeBuilder_cptr empty_means_failure;

    switch(kind)
    {
        case TypeKind::TK_BOOLEAN:
            return create_bool_builder();
        case TypeKind::TK_BYTE:
            return create_byte_builder();
        case TypeKind::TK_INT16:
            return create_int16_builder();
        case TypeKind::TK_INT32:
            return create_int32_builder();
        case TypeKind::TK_INT64:
            return create_int64_builder();
        case TypeKind::TK_UINT16:
            return create_uint16_builder();
        case TypeKind::TK_UINT32:
            return create_uint32_builder();
        case TypeKind::TK_UINT64:
            return create_uint64_builder();
        case TypeKind::TK_FLOAT32:
            return create_float32_builder();
        case TypeKind::TK_FLOAT64:
            return create_float64_builder();
        case TypeKind::TK_FLOAT128:
            return create_float128_builder();
        case TypeKind::TK_CHAR8:
            return create_char8_builder();
        case TypeKind::TK_CHAR16:
            return create_char16_builder();
        default:
            EPROSIMA_LOG_ERROR(DYN_TYPES, "The type provided " << int(kind) << " is not primitive");
            return empty_means_failure;
    }
}

DynamicType_ptr DynamicTypeBuilderFactory::get_primitive_type(TypeKind kind) noexcept
{
    DynamicTypeBuilder_cptr builder = create_primitive_builder(kind);
    if(builder)
    {
        return builder->build();
    }
    return {};
}

bool DynamicTypeBuilderFactory::is_empty() const
{
    return dynamic_tracker_interface::get_dynamic_tracker().is_empty();
}

void DynamicTypeBuilderFactory::build_type_identifier(
        const TypeDescriptor& descriptor,
        TypeIdentifier& identifier,
        bool complete) const
{
    const TypeIdentifier* id2 = (complete)
        ? TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(descriptor.get_name())
        : TypeObjectFactory::get_instance()->get_type_identifier(descriptor.get_name());
    if (id2 != nullptr)
    {
        identifier = *id2;
    }
    else
    {
        switch (descriptor.kind_)
        {
            // Basic types
            case TypeKind::TK_NONE:
            case TypeKind::TK_BOOLEAN:
            case TypeKind::TK_BYTE:
            case TypeKind::TK_INT16:
            case TypeKind::TK_INT32:
            case TypeKind::TK_INT64:
            case TypeKind::TK_UINT16:
            case TypeKind::TK_UINT32:
            case TypeKind::TK_UINT64:
            case TypeKind::TK_FLOAT32:
            case TypeKind::TK_FLOAT64:
            case TypeKind::TK_FLOAT128:
            case TypeKind::TK_CHAR8:
            case TypeKind::TK_CHAR16:
            {
                identifier._d(descriptor.kind_);
            }
            break;
            // String TKs
            case TypeKind::TK_STRING8:
            {
                if (descriptor.bound_[0] < 256)
                {
                    identifier._d(TypeKind::TI_STRING8_SMALL);
                    identifier.string_sdefn().bound(static_cast<SBound>(descriptor.bound_[0]));
                }
                else
                {
                    identifier._d(TypeKind::TI_STRING8_LARGE);
                    identifier.string_ldefn().bound(descriptor.bound_[0]);
                }
            }
            break;
            case TypeKind::TK_STRING16:
            {
                if (descriptor.bound_[0] < 256)
                {
                    identifier._d(TypeKind::TI_STRING16_SMALL);
                    identifier.string_sdefn().bound(static_cast<SBound>(descriptor.bound_[0]));
                }
                else
                {
                    identifier._d(TypeKind::TI_STRING16_LARGE);
                    identifier.string_ldefn().bound(descriptor.bound_[0]);
                }
            }
            break;
            // Collection TKs
            case TypeKind::TK_SEQUENCE:
            {
                if (descriptor.bound_[0] < 256)
                {
                    identifier._d(TypeKind::TI_PLAIN_SEQUENCE_SMALL);
                    identifier.seq_sdefn().bound(static_cast<SBound>(descriptor.bound_[0]));
                    TypeIdentifier elem_id;
                    build_type_identifier(*descriptor.get_element_type(), elem_id, complete);
                    identifier.seq_sdefn().element_identifier(&elem_id);
                }
                else
                {
                    identifier._d(TypeKind::TI_PLAIN_SEQUENCE_LARGE);
                    identifier.seq_ldefn().bound(descriptor.bound_[0]);
                    TypeIdentifier elem_id;
                    build_type_identifier(*descriptor.get_element_type(), elem_id, complete);
                    identifier.seq_ldefn().element_identifier(&elem_id);
                }
            }
            break;
            case TypeKind::TK_ARRAY:
            {
                uint32_t size = 0;
                for (uint32_t s : descriptor.bound_)
                {
                    size += s;
                }

                if (size < 256)
                {
                    identifier._d(TypeKind::TI_PLAIN_ARRAY_SMALL);
                    for (uint32_t b : descriptor.bound_)
                    {
                        identifier.array_sdefn().array_bound_seq().emplace_back(static_cast<SBound>(b));
                    }
                    TypeIdentifier elem_id;
                    build_type_identifier(*descriptor.get_element_type(), elem_id, complete);
                    identifier.array_sdefn().element_identifier(&elem_id);
                }
                else
                {
                    identifier._d(TypeKind::TI_PLAIN_ARRAY_LARGE);
                    identifier.array_ldefn().array_bound_seq(descriptor.bound_);
                    TypeIdentifier elem_id;
                    build_type_identifier(*descriptor.get_element_type(), elem_id, complete);
                    identifier.array_ldefn().element_identifier(&elem_id);
                }
            }
            break;
            case TypeKind::TK_MAP:
            {
                if (descriptor.bound_[0] < 256)
                {
                    identifier._d(TypeKind::TI_PLAIN_MAP_SMALL);
                    identifier.map_sdefn().bound(static_cast<SBound>(descriptor.bound_[0]));
                    TypeIdentifier elem_id;
                    build_type_identifier(*descriptor.get_element_type(), elem_id, complete);
                    identifier.map_sdefn().element_identifier(&elem_id);
                    TypeIdentifier key_id;
                    build_type_identifier(*descriptor.get_key_element_type(), key_id, complete);
                    identifier.map_sdefn().key_identifier(&key_id);
                }
                else
                {
                    identifier._d(TypeKind::TI_PLAIN_MAP_LARGE);
                    identifier.map_ldefn().bound(static_cast<SBound>(descriptor.bound_[0]));
                    TypeIdentifier elem_id;
                    build_type_identifier(*descriptor.get_element_type(), elem_id, complete);
                    identifier.map_ldefn().element_identifier(&elem_id);
                    TypeIdentifier key_id;
                    build_type_identifier(*descriptor.get_key_element_type(), key_id, complete);
                    identifier.map_ldefn().key_identifier(&key_id);
                }
            }
            break;
            // Constructed/Named types
            case TypeKind::TK_ALIAS:
            // Enumerated TKs
            case TypeKind::TK_ENUM:
            case TypeKind::TK_BITMASK:
            // Structured TKs
            case TypeKind::TK_ANNOTATION:
            case TypeKind::TK_STRUCTURE:
            case TypeKind::TK_UNION:
            case TypeKind::TK_BITSET:
            {
                // Need to be registered as TypeObject first
                // and return them as EK_MINIMAL or TypeKind::EK_COMPLETE
                EPROSIMA_LOG_INFO(DYN_TYPE_FACTORY, "Complex types must be built from CompleteTypeObjects.");
            }
            break;
            // TODO:BARRO handle this specific cases
            case TypeKind::TI_STRING8_SMALL:
            case TypeKind::TI_STRING8_LARGE:
            case TypeKind::TI_STRING16_SMALL:
            case TypeKind::TI_STRING16_LARGE:
            case TypeKind::TI_PLAIN_SEQUENCE_LARGE:
            case TypeKind::TI_PLAIN_SEQUENCE_SMALL:
            case TypeKind::TI_PLAIN_ARRAY_SMALL:
            case TypeKind::TI_PLAIN_ARRAY_LARGE:
            case TypeKind::TI_PLAIN_MAP_SMALL:
            case TypeKind::TI_PLAIN_MAP_LARGE:
            case TypeKind::TI_STRONGLY_CONNECTED_COMPONENT:
            case TypeKind::EK_MINIMAL:
            case TypeKind::EK_COMPLETE:
            case TypeKind::EK_BOTH:
                assert(0);
        }

        TypeObjectFactory::get_instance()->add_type_identifier(descriptor.get_name(), &identifier);
    }
}

void DynamicTypeBuilderFactory::build_type_object(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete,
        bool force) const
{
    const TypeObject* obj2 = (force)
        ? nullptr
        : TypeObjectFactory::get_instance()->get_type_object(descriptor.get_name(), complete);

    if (obj2 != nullptr)
    {
        object = *obj2;
        return;
    }

    // Create the TypeObject
    switch (descriptor.kind_)
    {
        // Basic types
        case TypeKind::TK_NONE:
        case TypeKind::TK_BOOLEAN:
        case TypeKind::TK_BYTE:
        case TypeKind::TK_INT16:
        case TypeKind::TK_INT32:
        case TypeKind::TK_INT64:
        case TypeKind::TK_UINT16:
        case TypeKind::TK_UINT32:
        case TypeKind::TK_UINT64:
        case TypeKind::TK_FLOAT32:
        case TypeKind::TK_FLOAT64:
        case TypeKind::TK_FLOAT128:
        case TypeKind::TK_CHAR8:
        case TypeKind::TK_CHAR16:
        {
            break;
        }
        // String TKs
        case TypeKind::TK_STRING8:
        {
            build_string8_type_code(descriptor);
            break;
        }
        case TypeKind::TK_STRING16:
        {
            build_string16_type_code(descriptor);
            break;
        }
        // Collection TKs
        case TypeKind::TK_SEQUENCE:
        {
            build_sequence_type_code(descriptor, object, complete);
            break;
        }
        case TypeKind::TK_ARRAY:
        {
            build_array_type_code(descriptor, object, complete);
            break;
        }
        case TypeKind::TK_MAP:
        {
            build_map_type_code(descriptor, object, complete);
            break;
        }

        // Constructed/Named types
        case TypeKind::TK_ALIAS:
        {
            build_alias_type_code(descriptor, object, complete);
        }
        break;
        // Enumerated TKs
        case TypeKind::TK_ENUM:
        {
            build_enum_type_code(descriptor, object, complete);
        }
        break;
        case TypeKind::TK_BITMASK:
        {
            build_bitmask_type_code(descriptor, object, complete);
        }
        break;
        // Structured TKs
        case TypeKind::TK_ANNOTATION:
        {
            build_annotation_type_code(descriptor, object, complete);
        }
        break;
        case TypeKind::TK_STRUCTURE:
        {
            build_struct_type_code(descriptor, object, complete);
        }
        break;
        case TypeKind::TK_UNION:
        {
            build_union_type_code(descriptor, object, complete);
        }
        break;
        case TypeKind::TK_BITSET:
        {
            build_bitset_type_code(descriptor, object, complete);
        }
        break;
        // TODO:BARRO handle this specific cases
        case TypeKind::TI_STRING8_SMALL:
        case TypeKind::TI_STRING8_LARGE:
        case TypeKind::TI_STRING16_SMALL:
        case TypeKind::TI_STRING16_LARGE:
        case TypeKind::TI_PLAIN_SEQUENCE_LARGE:
        case TypeKind::TI_PLAIN_SEQUENCE_SMALL:
        case TypeKind::TI_PLAIN_ARRAY_LARGE:
        case TypeKind::TI_PLAIN_ARRAY_SMALL:
        case TypeKind::TI_PLAIN_MAP_SMALL:
        case TypeKind::TI_PLAIN_MAP_LARGE:
        case TypeKind::TI_STRONGLY_CONNECTED_COMPONENT:
        case TypeKind::EK_MINIMAL:
        case TypeKind::EK_COMPLETE:
        case TypeKind::EK_BOTH:
            assert(0);
    }
}

void DynamicTypeBuilderFactory::build_string8_type_code(
        const TypeDescriptor& descriptor) const
{
    const TypeIdentifier* identifier =
            TypeObjectFactory::get_instance()->get_string_identifier(
        descriptor.get_bounds(),
        false);

    TypeObjectFactory::get_instance()->add_type_identifier(descriptor.get_name(), identifier);
}

void DynamicTypeBuilderFactory::build_string16_type_code(
        const TypeDescriptor& descriptor) const
{
    const TypeIdentifier* identifier =
            TypeObjectFactory::get_instance()->get_string_identifier(
        descriptor.get_bounds(),
        true);

    TypeObjectFactory::get_instance()->add_type_identifier(descriptor.get_name(), identifier);
}

void DynamicTypeBuilderFactory::build_sequence_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_SEQUENCE);
        object.complete().sequence_type().collection_flag().IS_FINAL(false);
        object.complete().sequence_type().collection_flag().IS_APPENDABLE(false);
        object.complete().sequence_type().collection_flag().IS_MUTABLE(false);
        object.complete().sequence_type().collection_flag().IS_NESTED(false);
        object.complete().sequence_type().collection_flag().IS_AUTOID_HASH(false);

        // Apply annotations
        apply_type_annotations(object.complete().sequence_type().header().detail().ann_custom(), descriptor);

        object.complete().sequence_type().header().detail().type_name(descriptor.get_name());
        object.complete().sequence_type().header().common().bound(descriptor.get_bounds());
        object.complete().sequence_type().element().common().element_flags().TRY_CONSTRUCT1(false);
        object.complete().sequence_type().element().common().element_flags().TRY_CONSTRUCT2(false);
        object.complete().sequence_type().element().common().element_flags().IS_EXTERNAL(false);
        object.complete().sequence_type().element().common().element_flags().IS_OPTIONAL(false);
        object.complete().sequence_type().element().common().element_flags().IS_MUST_UNDERSTAND(false);
        object.complete().sequence_type().element().common().element_flags().IS_KEY(false);
        object.complete().sequence_type().element().common().element_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_element_type(), obj, complete);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_element_type()->get_name());

        object.complete().sequence_type().element().common().type(ident);

        const TypeIdentifier* identifier =
                TypeObjectFactory::get_instance()->get_sequence_identifier(
            descriptor.get_element_type()->get_name(),
            descriptor.get_bounds(),
            true);

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_MINIMAL);
        object.minimal()._d(TypeKind::TK_SEQUENCE);
        object.minimal().sequence_type().collection_flag().IS_FINAL(false);
        object.minimal().sequence_type().collection_flag().IS_APPENDABLE(false);
        object.minimal().sequence_type().collection_flag().IS_MUTABLE(false);
        object.minimal().sequence_type().collection_flag().IS_NESTED(false);
        object.minimal().sequence_type().collection_flag().IS_AUTOID_HASH(false);

        // Apply annotations
        object.minimal().sequence_type().header().common().bound(descriptor.get_bounds());
        object.minimal().sequence_type().element().common().element_flags().TRY_CONSTRUCT1(false);
        object.minimal().sequence_type().element().common().element_flags().TRY_CONSTRUCT2(false);
        object.minimal().sequence_type().element().common().element_flags().IS_EXTERNAL(false);
        object.minimal().sequence_type().element().common().element_flags().IS_OPTIONAL(false);
        object.minimal().sequence_type().element().common().element_flags().IS_MUST_UNDERSTAND(false);
        object.minimal().sequence_type().element().common().element_flags().IS_KEY(false);
        object.minimal().sequence_type().element().common().element_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_element_type(), obj);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_element_type()->get_name());

        object.minimal().sequence_type().element().common().type(ident);

        const TypeIdentifier* identifier =
                TypeObjectFactory::get_instance()->get_sequence_identifier(
            descriptor.get_element_type()->get_name(),
            descriptor.get_bounds(),
            false);

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_array_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_ARRAY);
        object.complete().array_type().collection_flag().IS_FINAL(false);
        object.complete().array_type().collection_flag().IS_APPENDABLE(false);
        object.complete().array_type().collection_flag().IS_MUTABLE(false);
        object.complete().array_type().collection_flag().IS_NESTED(false);
        object.complete().array_type().collection_flag().IS_AUTOID_HASH(false);

        // Apply annotations
        apply_type_annotations(object.complete().array_type().header().detail().ann_custom(), descriptor);

        object.complete().array_type().header().detail().type_name(descriptor.get_name());
        for (uint32_t i = 0; i < descriptor.get_bounds_size(); ++i)
        {
            object.complete().array_type().header().common().bound_seq().push_back(descriptor.get_bounds(i));
        }
        object.complete().array_type().element().common().element_flags().TRY_CONSTRUCT1(false);
        object.complete().array_type().element().common().element_flags().TRY_CONSTRUCT2(false);
        object.complete().array_type().element().common().element_flags().IS_EXTERNAL(false);
        object.complete().array_type().element().common().element_flags().IS_OPTIONAL(false);
        object.complete().array_type().element().common().element_flags().IS_MUST_UNDERSTAND(false);
        object.complete().array_type().element().common().element_flags().IS_KEY(false);
        object.complete().array_type().element().common().element_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_element_type(), obj, complete);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_element_type()->get_name());

        object.complete().array_type().element().common().type(ident);

        const TypeIdentifier* identifier =
                TypeObjectFactory::get_instance()->get_array_identifier(
            descriptor.get_element_type()->get_name(),
            object.complete().array_type().header().common().bound_seq(),
            true);

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_MINIMAL);
        object.minimal()._d(TypeKind::TK_ARRAY);
        object.minimal().array_type().collection_flag().IS_FINAL(false);
        object.minimal().array_type().collection_flag().IS_APPENDABLE(false);
        object.minimal().array_type().collection_flag().IS_MUTABLE(false);
        object.minimal().array_type().collection_flag().IS_NESTED(false);
        object.minimal().array_type().collection_flag().IS_AUTOID_HASH(false);

        // Apply annotations
        for (uint32_t i = 0; i < descriptor.get_bounds_size(); ++i)
        {
            object.minimal().array_type().header().common().bound_seq().push_back(descriptor.get_bounds(i));
        }
        object.minimal().array_type().element().common().element_flags().TRY_CONSTRUCT1(false);
        object.minimal().array_type().element().common().element_flags().TRY_CONSTRUCT2(false);
        object.minimal().array_type().element().common().element_flags().IS_EXTERNAL(false);
        object.minimal().array_type().element().common().element_flags().IS_OPTIONAL(false);
        object.minimal().array_type().element().common().element_flags().IS_MUST_UNDERSTAND(false);
        object.minimal().array_type().element().common().element_flags().IS_KEY(false);
        object.minimal().array_type().element().common().element_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_element_type(), obj);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_element_type()->get_name());

        object.minimal().array_type().element().common().type(ident);

        const TypeIdentifier* identifier =
                TypeObjectFactory::get_instance()->get_array_identifier(
            descriptor.get_element_type()->get_name(),
            object.minimal().array_type().header().common().bound_seq(),
            false);

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_map_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_MAP);
        object.complete().map_type().collection_flag().IS_FINAL(false);
        object.complete().map_type().collection_flag().IS_APPENDABLE(false);
        object.complete().map_type().collection_flag().IS_MUTABLE(false);
        object.complete().map_type().collection_flag().IS_NESTED(false);
        object.complete().map_type().collection_flag().IS_AUTOID_HASH(false);

        // Apply annotations
        apply_type_annotations(object.complete().map_type().header().detail().ann_custom(), descriptor);

        object.complete().map_type().header().detail().type_name(descriptor.get_name());
        object.complete().map_type().header().common().bound(descriptor.get_bounds());
        object.complete().map_type().element().common().element_flags().TRY_CONSTRUCT1(false);
        object.complete().map_type().element().common().element_flags().TRY_CONSTRUCT2(false);
        object.complete().map_type().element().common().element_flags().IS_EXTERNAL(false);
        object.complete().map_type().element().common().element_flags().IS_OPTIONAL(false);
        object.complete().map_type().element().common().element_flags().IS_MUST_UNDERSTAND(false);
        object.complete().map_type().element().common().element_flags().IS_KEY(false);
        object.complete().map_type().element().common().element_flags().IS_DEFAULT(false);
        object.complete().map_type().key().common().element_flags().TRY_CONSTRUCT1(false);
        object.complete().map_type().key().common().element_flags().TRY_CONSTRUCT2(false);
        object.complete().map_type().key().common().element_flags().IS_EXTERNAL(false);
        object.complete().map_type().key().common().element_flags().IS_OPTIONAL(false);
        object.complete().map_type().key().common().element_flags().IS_MUST_UNDERSTAND(false);
        object.complete().map_type().key().common().element_flags().IS_KEY(false);
        object.complete().map_type().key().common().element_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_element_type(), obj, complete);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_element_type()->get_name());

        build_type_object(*descriptor.get_key_element_type(), obj, complete);
        TypeIdentifier ident_key = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_key_element_type()->get_name());

        object.complete().map_type().element().common().type(ident);
        object.complete().map_type().key().common().type(ident_key);

        const TypeIdentifier* identifier =
                TypeObjectFactory::get_instance()->get_map_identifier(
            descriptor.get_key_element_type()->get_name(),
            descriptor.get_element_type()->get_name(),
            descriptor.get_bounds(),
            true);

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_MINIMAL);
        object.minimal()._d(TypeKind::TK_MAP);
        object.minimal().map_type().collection_flag().IS_FINAL(false);
        object.minimal().map_type().collection_flag().IS_APPENDABLE(false);
        object.minimal().map_type().collection_flag().IS_MUTABLE(false);
        object.minimal().map_type().collection_flag().IS_NESTED(false);
        object.minimal().map_type().collection_flag().IS_AUTOID_HASH(false);

        // Apply annotations
        object.minimal().map_type().header().common().bound(descriptor.get_bounds());
        object.minimal().map_type().element().common().element_flags().TRY_CONSTRUCT1(false);
        object.minimal().map_type().element().common().element_flags().TRY_CONSTRUCT2(false);
        object.minimal().map_type().element().common().element_flags().IS_EXTERNAL(false);
        object.minimal().map_type().element().common().element_flags().IS_OPTIONAL(false);
        object.minimal().map_type().element().common().element_flags().IS_MUST_UNDERSTAND(false);
        object.minimal().map_type().element().common().element_flags().IS_KEY(false);
        object.minimal().map_type().element().common().element_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_element_type(), obj);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_element_type()->get_name());

        build_type_object(*descriptor.get_key_element_type(), obj, complete);
        TypeIdentifier ident_key = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_key_element_type()->get_name());

        object.minimal().map_type().element().common().type(ident);
        object.minimal().map_type().key().common().type(ident_key);

        const TypeIdentifier* identifier =
                TypeObjectFactory::get_instance()->get_map_identifier(
            descriptor.get_key_element_type()->get_name(),
            descriptor.get_element_type()->get_name(),
            descriptor.get_bounds(),
            false);

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_alias_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_ALIAS);
        object.complete().alias_type().alias_flags().IS_FINAL(false);
        object.complete().alias_type().alias_flags().IS_APPENDABLE(false);
        object.complete().alias_type().alias_flags().IS_MUTABLE(false);
        object.complete().alias_type().alias_flags().IS_NESTED(false);
        object.complete().alias_type().alias_flags().IS_AUTOID_HASH(false);

        // Apply annotations
        apply_type_annotations(object.complete().alias_type().header().detail().ann_custom(), descriptor);

        object.complete().alias_type().header().detail().type_name(descriptor.get_name());
        object.complete().alias_type().body().common().related_flags().TRY_CONSTRUCT1(false);
        object.complete().alias_type().body().common().related_flags().TRY_CONSTRUCT2(false);
        object.complete().alias_type().body().common().related_flags().IS_EXTERNAL(false);
        object.complete().alias_type().body().common().related_flags().IS_OPTIONAL(false);
        object.complete().alias_type().body().common().related_flags().IS_MUST_UNDERSTAND(false);
        object.complete().alias_type().body().common().related_flags().IS_KEY(false);
        object.complete().alias_type().body().common().related_flags().IS_DEFAULT(false);

        //TypeIdentifier ident;
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_base_type(), obj, complete);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_base_type()->get_name());

        object.complete().alias_type().body().common().related_type(ident);

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
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
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        // Add our alias
        TypeObjectFactory::get_instance()->add_alias(descriptor.get_name(), descriptor.get_base_type()->get_name());

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_MINIMAL);
        object.minimal()._d(TypeKind::TK_ALIAS);
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
        //build_type_identifier(*descriptor.get_base_type(), ident);
        TypeObject obj;
        build_type_object(*descriptor.get_base_type(), obj);
        TypeIdentifier ident = *TypeObjectFactory::get_instance()->get_type_identifier(
            descriptor.get_base_type()->get_name());

        object.minimal().alias_type().body().common().related_type(ident);

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_MINIMAL);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
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
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        // Add our alias
        TypeObjectFactory::get_instance()->add_alias(descriptor.get_name(), descriptor.get_base_type()->get_name());

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_enum_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_ENUM);
        object.complete().enumerated_type().header().common().bit_bound(descriptor.annotation_get_bit_bound());
        object.complete().enumerated_type().header().detail().type_name(descriptor.get_name());

        // Apply annotations
        apply_type_annotations(object.complete().enumerated_type().header().detail().ann_custom(), descriptor);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            CompleteEnumeratedLiteral mel;
            mel.common().flags().IS_DEFAULT(member.annotation_is_default_literal());
            mel.common().value(member.get_index());
            mel.detail().name(member.get_name());

            // Apply member annotations
            TypeDescriptor member_type_descriptor;
            member.get_type()->get_descriptor(member_type_descriptor);
            apply_type_annotations(mel.detail().ann_custom(), member_type_descriptor);

            object.complete().enumerated_type().literal_seq().emplace_back(mel);
        }

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
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
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_MINIMAL);
        object.minimal()._d(TypeKind::TK_ENUM);
        object.minimal().enumerated_type().header().common().bit_bound(32); // TODO fixed by IDL, isn't?

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            MinimalEnumeratedLiteral mel;
            mel.common().flags().IS_DEFAULT(member.annotation_is_default_literal());
            mel.common().value(member.get_index());
            MD5 hash(member.get_name());
            for (int i = 0; i < 4; ++i)
            {
                mel.detail().name_hash()[i] = hash.digest[i];
            }
            object.minimal().enumerated_type().literal_seq().emplace_back(mel);
        }

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_MINIMAL);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
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
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_struct_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_STRUCTURE);

        object.complete().struct_type().struct_flags().IS_FINAL(descriptor.annotation_is_final());
        object.complete().struct_type().struct_flags().IS_APPENDABLE(descriptor.annotation_is_appendable());
        object.complete().struct_type().struct_flags().IS_MUTABLE(descriptor.annotation_is_mutable());
        object.complete().struct_type().struct_flags().IS_NESTED(descriptor.annotation_is_nested());
        object.complete().struct_type().struct_flags().IS_AUTOID_HASH(false);

        // Apply annotations
        apply_type_annotations(object.complete().struct_type().header().detail().ann_custom(), descriptor);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            CompleteStructMember msm;
            msm.common().member_id(member.get_index());
            msm.common().member_flags().TRY_CONSTRUCT1(false);
            msm.common().member_flags().TRY_CONSTRUCT2(false);
            msm.common().member_flags().IS_EXTERNAL(false);
            msm.common().member_flags().IS_OPTIONAL(member.annotation_is_optional());
            msm.common().member_flags().IS_MUST_UNDERSTAND(member.annotation_is_must_understand());
            msm.common().member_flags().IS_KEY(member.annotation_is_key());
            msm.common().member_flags().IS_DEFAULT(false);

            // Apply member annotations
            auto member_type = member.get_type();
            TypeDescriptor member_type_descriptor;
            member_type->get_descriptor(member_type_descriptor);
            apply_type_annotations(msm.detail().ann_custom(), member_type_descriptor);

            TypeObject memObj;
            build_type_object(*member_type, memObj);
            const TypeIdentifier* typeId =
                    TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(member_type->get_name());
            if (typeId == nullptr)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Member " << member.get_name() << " of struct "
                                                        << descriptor.get_name() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }

            msm.detail().name(member.get_name());
            object.complete().struct_type().member_seq().emplace_back(msm);
        }

        object.complete().struct_type().header().detail().type_name(descriptor.get_name());
        //object.complete().struct_type().header().detail().ann_builtin()...
        //object.complete().struct_type().header().detail().ann_custom()...

        if (descriptor.get_base_type().get() != nullptr)
        {
            TypeIdentifier parent;
            build_type_identifier(*descriptor.get_base_type(), parent);
            object.complete().struct_type().header().base_type(parent);
        }
        //object.complete().struct_type().header().base_type().equivalence_hash()[0..13];

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    CompleteStructType::getCdrSerializedSize(object.complete().struct_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteStructMember& st : object.complete().struct_type().member_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_MINIMAL);
        object.minimal()._d(TypeKind::TK_STRUCTURE);

        object.minimal().struct_type().struct_flags().IS_FINAL(descriptor.annotation_is_final());
        object.minimal().struct_type().struct_flags().IS_APPENDABLE(descriptor.annotation_is_appendable());
        object.minimal().struct_type().struct_flags().IS_MUTABLE(descriptor.annotation_is_mutable());
        object.minimal().struct_type().struct_flags().IS_NESTED(descriptor.annotation_is_nested());
        object.minimal().struct_type().struct_flags().IS_AUTOID_HASH(false);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            MinimalStructMember msm;
            msm.common().member_id(member.get_index());
            msm.common().member_flags().TRY_CONSTRUCT1(false);
            msm.common().member_flags().TRY_CONSTRUCT2(false);
            msm.common().member_flags().IS_EXTERNAL(false);
            msm.common().member_flags().IS_OPTIONAL(member.annotation_is_optional());
            msm.common().member_flags().IS_MUST_UNDERSTAND(member.annotation_is_must_understand());
            msm.common().member_flags().IS_KEY(member.annotation_is_key());
            msm.common().member_flags().IS_DEFAULT(false);
            //TypeIdentifier memIdent;
            //build_type_identifier(*member.get_type(), memIdent);

            TypeObject memObj;
            build_type_object(*member.get_type(), memObj, false);
            const TypeIdentifier* typeId =
                    TypeObjectFactory::get_instance()->get_type_identifier(member.get_type()->get_name());
            if (typeId == nullptr)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Member " << member.get_name()
                                                        << " of struct " << descriptor.get_name() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }

            MD5 hash(member.get_name());
            for (int i = 0; i < 4; ++i)
            {
                msm.detail().name_hash()[i] = hash.digest[i];
            }
            object.minimal().struct_type().member_seq().emplace_back(msm);
        }

        if (descriptor.get_base_type().get() != nullptr)
        {
            TypeIdentifier parent;
            build_type_identifier(*descriptor.get_base_type(), parent, false);
            object.minimal().struct_type().header().base_type(parent);
        }

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_MINIMAL);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    MinimalStructType::getCdrSerializedSize(object.minimal().struct_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (MinimalStructMember& st : object.minimal().struct_type().member_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_union_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_UNION);

        object.complete().union_type().union_flags().IS_FINAL(descriptor.annotation_is_final());
        object.complete().union_type().union_flags().IS_APPENDABLE(descriptor.annotation_is_appendable());
        object.complete().union_type().union_flags().IS_MUTABLE(descriptor.annotation_is_mutable());
        object.complete().union_type().union_flags().IS_NESTED(descriptor.annotation_is_nested());
        object.complete().union_type().union_flags().IS_AUTOID_HASH(false);

        object.complete().union_type().discriminator().common().member_flags().TRY_CONSTRUCT1(false);
        object.complete().union_type().discriminator().common().member_flags().TRY_CONSTRUCT2(false);
        object.complete().union_type().discriminator().common().member_flags().IS_EXTERNAL(false);
        object.complete().union_type().discriminator().common().member_flags().IS_OPTIONAL(false);
        object.complete().union_type().discriminator().common().member_flags().IS_MUST_UNDERSTAND(false);
        object.complete().union_type().discriminator().common().member_flags().IS_KEY(
            descriptor.discriminator_type_->annotation_is_key());
        object.complete().union_type().discriminator().common().member_flags().IS_DEFAULT(false);

        // Apply annotations
        apply_type_annotations(object.complete().struct_type().header().detail().ann_custom(), descriptor);

        TypeObject discObj;
        build_type_object(*descriptor.discriminator_type_, discObj);
        TypeIdentifier discIdent =
                *TypeObjectFactory::get_instance()->get_type_identifier(descriptor.discriminator_type_->get_name());
        object.complete().union_type().discriminator().common().type_id(discIdent);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            CompleteUnionMember mum;
            mum.common().member_id(member.get_index());
            mum.common().member_flags().TRY_CONSTRUCT1(false);
            mum.common().member_flags().TRY_CONSTRUCT2(false);
            mum.common().member_flags().IS_EXTERNAL(false);
            mum.common().member_flags().IS_OPTIONAL(false);
            mum.common().member_flags().IS_MUST_UNDERSTAND(false);
            mum.common().member_flags().IS_KEY(false);
            mum.common().member_flags().IS_DEFAULT(member.is_default_union_value());

            // Apply member annotations
            TypeDescriptor member_type_descriptor;
            member.get_type()->get_descriptor(member_type_descriptor);
            apply_type_annotations(mum.detail().ann_custom(), member_type_descriptor);

            TypeObject memObj;
            build_type_object(*member.get_type(), memObj);
            const TypeIdentifier* typeId =
                    TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(member.get_type()->get_name());
            if (typeId == nullptr)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Member " << member.get_name()
                                                        << " of union " << descriptor.get_name() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                mum.common().type_id(memIdent);
            }

            for (uint64_t lab : member.get_union_labels())
            {
                mum.common().label_seq().emplace_back(static_cast<uint32_t>(lab));
            }
            mum.detail().name(member.get_name());
            object.complete().union_type().member_seq().emplace_back(mum);
        }

        object.complete().union_type().header().detail().type_name(descriptor.get_name());

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_MINIMAL);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
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
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_MINIMAL);
        object.minimal()._d(TypeKind::TK_UNION);

        object.minimal().union_type().union_flags().IS_FINAL(descriptor.annotation_is_final());
        object.minimal().union_type().union_flags().IS_APPENDABLE(descriptor.annotation_is_appendable());
        object.minimal().union_type().union_flags().IS_MUTABLE(descriptor.annotation_is_mutable());
        object.minimal().union_type().union_flags().IS_NESTED(descriptor.annotation_is_nested());
        object.minimal().union_type().union_flags().IS_AUTOID_HASH(false);

        object.minimal().union_type().discriminator().common().member_flags().TRY_CONSTRUCT1(false);
        object.minimal().union_type().discriminator().common().member_flags().TRY_CONSTRUCT2(false);
        object.minimal().union_type().discriminator().common().member_flags().IS_EXTERNAL(false);
        object.minimal().union_type().discriminator().common().member_flags().IS_OPTIONAL(false);
        object.minimal().union_type().discriminator().common().member_flags().IS_MUST_UNDERSTAND(false);
        object.minimal().union_type().discriminator().common().member_flags().IS_KEY(
            descriptor.discriminator_type_->annotation_is_key());
        object.minimal().union_type().discriminator().common().member_flags().IS_DEFAULT(false);

        TypeObject discObj;
        build_type_object(*descriptor.discriminator_type_, discObj);
        TypeIdentifier discIdent =
                *TypeObjectFactory::get_instance()->get_type_identifier(descriptor.discriminator_type_->get_name());
        object.minimal().union_type().discriminator().common().type_id(discIdent);
        //*TypeObjectFactory::get_instance().get_type_identifier(descriptor.discriminator_type_->get_name()));

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            MinimalUnionMember mum;
            mum.common().member_id(member.get_index());
            mum.common().member_flags().TRY_CONSTRUCT1(false);
            mum.common().member_flags().TRY_CONSTRUCT2(false);
            mum.common().member_flags().IS_EXTERNAL(false);
            mum.common().member_flags().IS_OPTIONAL(false);
            mum.common().member_flags().IS_MUST_UNDERSTAND(false);
            mum.common().member_flags().IS_KEY(false);
            mum.common().member_flags().IS_DEFAULT(member.is_default_union_value());

            //TypeIdentifier memIdent;
            //build_type_identifier(*member.get_type(), memIdent);

            TypeObject memObj;
            build_type_object(*member.get_type(), memObj);
            const TypeIdentifier* typeId =
                    TypeObjectFactory::get_instance()->get_type_identifier(member.get_type()->get_name());
            if (typeId == nullptr)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Member " << member.get_name()
                                                        << " of union " << descriptor.get_name() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                mum.common().type_id(memIdent);
            }

            for (uint64_t lab : member.get_union_labels())
            {
                mum.common().label_seq().emplace_back(static_cast<uint32_t>(lab));
            }
            MD5 hash(member.get_name());
            for (int i = 0; i < 4; ++i)
            {
                mum.detail().name_hash()[i] = hash.digest[i];
            }
            object.minimal().union_type().member_seq().emplace_back(mum);
        }

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_MINIMAL);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
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
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_bitset_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_BITSET);

        object.complete().bitset_type().bitset_flags().IS_FINAL(false);
        object.complete().bitset_type().bitset_flags().IS_APPENDABLE(false);
        object.complete().bitset_type().bitset_flags().IS_MUTABLE(false);
        object.complete().bitset_type().bitset_flags().IS_NESTED(false);
        object.complete().bitset_type().bitset_flags().IS_AUTOID_HASH(false);

        // Apply annotations
        apply_type_annotations(object.complete().bitset_type().header().detail().ann_custom(), descriptor);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            CompleteBitfield msm;
            msm.common().position(member.annotation_get_position()); // Position stored as annotation
            // Bitcount stored as bit_bound annotation
            msm.common().bitcount(static_cast<octet>(member.annotation_get_bit_bound()));
            msm.common().holder_type(member.get_type()->get_kind());
            msm.detail().name(member.get_name());

            // Apply member annotations
            TypeDescriptor member_type_descriptor;
            member.get_type()->get_descriptor(member_type_descriptor);
            apply_type_annotations(msm.detail().ann_custom(), member_type_descriptor);

            object.complete().bitset_type().field_seq().emplace_back(msm);
        }

        object.complete().bitset_type().header().detail().type_name(descriptor.get_name());
        //object.complete().bitset_type().header().detail().ann_builtin()...
        //object.complete().bitset_type().header().detail().ann_custom()...

        if (descriptor.get_base_type().get() != nullptr)
        {
            TypeIdentifier parent;
            build_type_identifier(*descriptor.get_base_type(), parent);
            object.complete().bitset_type().header().base_type(parent);
        }
        //object.complete().bitset_type().header().base_type().equivalence_hash()[0..13];

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    CompleteBitsetType::getCdrSerializedSize(object.complete().bitset_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteBitfield& st : object.complete().bitset_type().field_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_COMPLETE);
        object.minimal()._d(TypeKind::TK_BITSET);

        object.minimal().bitset_type().bitset_flags().IS_FINAL(false);
        object.minimal().bitset_type().bitset_flags().IS_APPENDABLE(false);
        object.minimal().bitset_type().bitset_flags().IS_MUTABLE(false);
        object.minimal().bitset_type().bitset_flags().IS_NESTED(false);
        object.minimal().bitset_type().bitset_flags().IS_AUTOID_HASH(false);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            MinimalBitfield msm;
            msm.common().position(member.annotation_get_position()); // Position stored as annotation
            // Bitcount stored as bit_bound annotation
            msm.common().bitcount(static_cast<octet>(member.annotation_get_bit_bound()));
            msm.common().holder_type(member.get_type()->get_kind());
            MD5 parent_bitfield_hash(member.get_name());
            for (int i = 0; i < 4; ++i)
            {
                msm.name_hash()[i] = parent_bitfield_hash.digest[i];
            }
            object.minimal().bitset_type().field_seq().emplace_back(msm);
        }

        //object.minimal().bitset_type().header().detail().ann_builtin()...
        //object.minimal().bitset_type().header().detail().ann_custom()...

        if (descriptor.get_base_type().get() != nullptr)
        {
            TypeIdentifier parent;
            build_type_identifier(*descriptor.get_base_type(), parent);
            object.minimal().bitset_type().header().base_type(parent);
        }
        //object.minimal().bitset_type().header().base_type().equivalence_hash()[0..13];

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    MinimalBitsetType::getCdrSerializedSize(object.minimal().bitset_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (MinimalBitfield& st : object.minimal().bitset_type().field_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_bitmask_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_BITMASK);

        object.complete().bitmask_type().bitmask_flags().IS_FINAL(false);
        object.complete().bitmask_type().bitmask_flags().IS_APPENDABLE(false);
        object.complete().bitmask_type().bitmask_flags().IS_MUTABLE(false);
        object.complete().bitmask_type().bitmask_flags().IS_NESTED(false);
        object.complete().bitmask_type().bitmask_flags().IS_AUTOID_HASH(false);

        // Apply annotations
        apply_type_annotations(object.complete().bitmask_type().header().detail().ann_custom(), descriptor);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            CompleteBitflag msm;
            msm.common().position(member.annotation_get_position()); // Position stored as annotation
            msm.detail().name(member.get_name());

            // Apply member annotations
            TypeDescriptor member_type_descriptor;
            member.get_type()->get_descriptor(member_type_descriptor);
            apply_type_annotations(msm.detail().ann_custom(), member_type_descriptor);

            object.complete().bitmask_type().flag_seq().emplace_back(msm);
        }

        object.complete().bitmask_type().header().detail().type_name(descriptor.get_name());

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    CompleteBitmaskType::getCdrSerializedSize(object.complete().bitmask_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteBitflag& st : object.complete().bitmask_type().flag_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_COMPLETE);
        object.minimal()._d(TypeKind::TK_BITMASK);

        object.minimal().bitmask_type().bitmask_flags().IS_FINAL(false);
        object.minimal().bitmask_type().bitmask_flags().IS_APPENDABLE(false);
        object.minimal().bitmask_type().bitmask_flags().IS_MUTABLE(false);
        object.minimal().bitmask_type().bitmask_flags().IS_NESTED(false);
        object.minimal().bitmask_type().bitmask_flags().IS_AUTOID_HASH(false);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            MinimalBitflag msm;
            msm.common().position(member.annotation_get_position()); // Position stored as annotation
            MD5 parent_bitfield_hash(member.get_name());
            for (int i = 0; i < 4; ++i)
            {
                msm.detail().name_hash()[i] = parent_bitfield_hash.digest[i];
            }
            object.minimal().bitmask_type().flag_seq().emplace_back(msm);
        }

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    MinimalBitmaskType::getCdrSerializedSize(object.minimal().bitmask_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (MinimalBitflag& st : object.minimal().bitmask_type().flag_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::build_annotation_type_code(
        const TypeDescriptor& descriptor,
        TypeObject& object,
        bool complete) const
{
    if (complete)
    {
        object._d(TypeKind::EK_COMPLETE);
        object.complete()._d(TypeKind::TK_ANNOTATION);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            CompleteAnnotationParameter msm;
            msm.name(member.get_name());

            if (!member.get_default_value().empty())
            {
                AnnotationParameterValue apv;
                set_annotation_default_value(apv, member);
                msm.default_value(apv);
            }

            TypeObject memObj;
            build_type_object(*member.get_type(), memObj);
            const TypeIdentifier* typeId =
                    TypeObjectFactory::get_instance()->get_type_identifier(member.get_type()->get_name());
            if (typeId == nullptr)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Member " << member.get_name()
                                                        << " of annotation " << descriptor.get_name() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }

            object.complete().annotation_type().member_seq().emplace_back(msm);
        }

        object.complete().annotation_type().header().annotation_name(descriptor.get_name());

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    CompleteAnnotationType::getCdrSerializedSize(object.complete().annotation_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (CompleteAnnotationParameter& st : object.complete().annotation_type().member_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
    else
    {
        object._d(TypeKind::EK_COMPLETE);
        object.minimal()._d(TypeKind::TK_ANNOTATION);

        for (const DynamicTypeMember& member : descriptor.get_all_members())
        {
            MinimalAnnotationParameter msm;
            msm.name(member.get_name());

            if (!member.get_default_value().empty())
            {
                AnnotationParameterValue apv;
                set_annotation_default_value(apv, member);
                msm.default_value(apv);
            }

            TypeObject memObj;
            build_type_object(*member.get_type(), memObj);
            const TypeIdentifier* typeId =
                    TypeObjectFactory::get_instance()->get_type_identifier(member.get_type()->get_name());
            if (typeId == nullptr)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Member " << member.get_name()
                                                        << " of annotation " << descriptor.get_name() << " failed.");
            }
            else
            {
                TypeIdentifier memIdent = *typeId;
                msm.common().member_type_id(memIdent);
            }

            object.minimal().annotation_type().member_seq().emplace_back(msm);
        }

        TypeIdentifier identifier;
        identifier._d(TypeKind::EK_COMPLETE);

        eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                    MinimalAnnotationType::getCdrSerializedSize(object.minimal().annotation_type()) + 4));
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload.encapsulation = CDR_LE;
        // Serialize encapsulation

        for (MinimalAnnotationParameter& st : object.minimal().annotation_type().member_seq())
        {
            ser << st;
        }
        payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        MD5 objectHash;
        objectHash.update((char*)payload.data, payload.length);
        objectHash.finalize();
        for (int i = 0; i < 14; ++i)
        {
            identifier.equivalence_hash()[i] = objectHash.digest[i];
        }

        TypeObjectFactory::get_instance()->add_type_object(descriptor.get_name(), &identifier, &object);
    }
}

void DynamicTypeBuilderFactory::set_annotation_default_value(
        AnnotationParameterValue& apv,
        const MemberDescriptor& member) const
{
    switch (member.get_kind())
    {
        case TypeKind::TK_BOOLEAN:
        {
            std::string value = member.get_default_value();
            std::transform(value.begin(), value.end(), value.begin(),
                    [](unsigned char c)
                    {
                        return static_cast<char>(std::tolower(c));
                    });
            apv.boolean_value(value.compare("0") != 0 || value.compare(CONST_TRUE) == 0);
        }
        break;
        case TypeKind::TK_BYTE:
        {
            apv.byte_value(static_cast<uint8_t>(std::stoul(member.get_default_value())));
        }
        break;
        case TypeKind::TK_INT16:
        {
            apv.int16_value(static_cast<int16_t>(std::stoi(member.get_default_value())));
        }
        break;
        case TypeKind::TK_INT32:
        {
            apv.int32_value(static_cast<int32_t>(std::stoi(member.get_default_value())));
        }
        break;
        case TypeKind::TK_INT64:
        {
            apv.int64_value(static_cast<int64_t>(std::stoll(member.get_default_value())));
        }
        break;
        case TypeKind::TK_UINT16:
        {
            apv.uint_16_value(static_cast<uint16_t>(std::stoul(member.get_default_value())));
        }
        break;
        case TypeKind::TK_UINT32:
        {
            apv.uint32_value(static_cast<uint32_t>(std::stoul(member.get_default_value())));
        }
        break;
        case TypeKind::TK_UINT64:
        {
            apv.uint64_value(static_cast<uint64_t>(std::stoull(member.get_default_value())));
        }
        break;
        case TypeKind::TK_FLOAT32:
        {
            apv.float32_value(std::stof(member.get_default_value()));
        }
        break;
        case TypeKind::TK_FLOAT64:
        {
            apv.float64_value(std::stod(member.get_default_value()));
        }
        break;
        case TypeKind::TK_FLOAT128:
        {
            apv.float128_value(std::stold(member.get_default_value()));
        }
        break;
        case TypeKind::TK_CHAR8:
        {
            apv.char_value(member.get_default_value().c_str()[0]);
        }
        break;
        case TypeKind::TK_CHAR16:
        {
            apv.wchar_value(wstring_from_bytes(member.get_default_value()).c_str()[0]);
        }
        break;
        case TypeKind::TK_STRING8:
        {
            apv.string8_value(member.get_default_value());
        }
        break;
        case TypeKind::TK_STRING16:
        {
            apv.string16_value(wstring_from_bytes(member.get_default_value()));
        }
        break;
        case TypeKind::TK_ENUM:
        {
            // TODO Translate from enum value name to integer value
            apv.enumerated_value(static_cast<int32_t>(std::stoul(member.get_default_value())));
        }
        break;
        default:
            break;
    }
}

DynamicType_ptr DynamicTypeBuilderFactory::create_alias_type(
        const DynamicTypeBuilder& base_type,
        const std::string& sName)
{
    DynamicTypeBuilder_ptr builder = create_alias_builder(base_type, sName);
    return builder ? builder->build() : DynamicType_ptr{};
}

DynamicType_ptr DynamicTypeBuilderFactory::create_alias_type(
        const DynamicType& base_type,
        const std::string& sName)
{
    DynamicTypeBuilder_ptr builder = create_alias_builder(base_type, sName);
    return builder ? builder->build() : DynamicType_ptr{};
}

DynamicType_ptr DynamicTypeBuilderFactory::create_int32_type()
{
    return create_int32_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_uint32_type()
{
    return create_uint32_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_int16_type()
{
    return create_int16_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_uint16_type()
{
    return create_uint16_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_int64_type()
{
    return create_int64_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_uint64_type()
{
    return create_uint64_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_float32_type()
{
    return create_float32_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_float64_type()
{
    return create_float64_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_float128_type()
{
    return create_float128_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_char8_type()
{
    return create_char8_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_char16_type()
{
    return create_char16_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_bool_type()
{
    return create_bool_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_byte_type()
{
    return create_byte_builder()->build();
}

DynamicType_ptr DynamicTypeBuilderFactory::create_string_type(
        uint32_t bound /*= MAX_STRING_LENGTH*/) noexcept
{
    DynamicTypeBuilder_ptr builder = create_string_builder(bound);
    return builder ? builder->build() : DynamicType_ptr{};
}

DynamicType_ptr DynamicTypeBuilderFactory::create_wstring_type(
        uint32_t bound /*= MAX_STRING_LENGTH*/) noexcept
{
    DynamicTypeBuilder_ptr builder = create_wstring_builder(bound);
    return builder ? builder->build() : DynamicType_ptr{};
}

DynamicType_ptr DynamicTypeBuilderFactory::create_bitset_type(
        uint32_t bound)
{
    DynamicTypeBuilder_ptr builder = create_bitset_builder(bound);
    return builder ? builder->build() : DynamicType_ptr{};
}

void DynamicTypeBuilderFactory::apply_type_annotations(
        AppliedAnnotationSeq& annotations,
        const TypeDescriptor& descriptor) const
{
    for (const AnnotationDescriptor& annotation : descriptor.annotation_)
    {
        AppliedAnnotation ann;
        ann.annotation_typeid(
            *TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(annotation.type_->get_name()));
        std::map<std::string, std::string> values;
        annotation.get_all_value(values);
        for (auto it : values)
        {
            AppliedAnnotationParameter ann_param;
            MD5 message_hash(it.first);
            for (int i = 0; i < 4; ++i)
            {
                ann_param.paramname_hash()[i] = message_hash.digest[i];
            }
            AnnotationParameterValue param_value;
            param_value._d(annotation.type_->get_kind());
            param_value.from_string(it.second);
            ann_param.value(param_value);
            ann.param_seq().push_back(ann_param);
        }
        annotations.push_back(ann);
    }
}

namespace typekind_detail {

#define XTYPENAME(type)                                                                            \
const char* TypeKindName<TypeKind::type, char, std::char_traits<char>>::name = #type;              \
const wchar_t* TypeKindName<TypeKind::type, wchar_t, std::char_traits<wchar_t>>::name = L"" #type; \

XTYPENAME(TK_BOOLEAN)
XTYPENAME(TK_BYTE)
XTYPENAME(TK_INT16)
XTYPENAME(TK_INT32)
XTYPENAME(TK_INT64)
XTYPENAME(TK_UINT16)
XTYPENAME(TK_UINT32)
XTYPENAME(TK_UINT64)
XTYPENAME(TK_FLOAT32)
XTYPENAME(TK_FLOAT64)
XTYPENAME(TK_FLOAT128)
XTYPENAME(TK_CHAR8)
XTYPENAME(TK_CHAR16)
XTYPENAME(TK_STRING8)
XTYPENAME(TK_STRING16)
XTYPENAME(TK_ALIAS)
XTYPENAME(TK_ENUM)
XTYPENAME(TK_BITMASK)
XTYPENAME(TK_ANNOTATION)
XTYPENAME(TK_STRUCTURE)
XTYPENAME(TK_UNION)
XTYPENAME(TK_BITSET)
XTYPENAME(TK_SEQUENCE)
XTYPENAME(TK_ARRAY)
XTYPENAME(TK_MAP)
XTYPENAME(TI_STRING8_SMALL)
XTYPENAME(TI_STRING8_LARGE)
XTYPENAME(TI_STRING16_SMALL)
XTYPENAME(TI_STRING16_LARGE)
XTYPENAME(TI_PLAIN_SEQUENCE_SMALL)
XTYPENAME(TI_PLAIN_SEQUENCE_LARGE)
XTYPENAME(TI_PLAIN_ARRAY_SMALL)
XTYPENAME(TI_PLAIN_ARRAY_LARGE)
XTYPENAME(TI_PLAIN_MAP_SMALL)
XTYPENAME(TI_PLAIN_MAP_LARGE)
XTYPENAME(TI_STRONGLY_CONNECTED_COMPONENT)

#undef XTYPENAME

} // namespace typekind_detail
} // namespace types
} // namespace fastrtps
} // namespace eprosima
