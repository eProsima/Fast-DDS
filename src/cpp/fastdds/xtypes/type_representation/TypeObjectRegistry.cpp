// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>

#include <exception>
#include <mutex>
#include <string>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>
#include <fastrtps/utils/md5.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

constexpr const int32_t NO_DEPENDENCIES = -1;

bool TypeRegistryEntry::operator !=(
        const TypeRegistryEntry& entry)
{
    return this->type_object_ != entry.type_object_ ||
           this->type_object_serialized_size_ != entry.type_object_serialized_size_;
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const std::string& type_name,
        const CompleteTypeObject& complete_type_object)
{
    if (type_name.empty())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#if !defined(NDEBUG)
    try
    {
        TypeObjectUtils::complete_type_object_consistency(complete_type_object);
    }
    catch (eprosima::fastdds::dds::xtypes::InvalidArgumentError& exception)
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Inconsistent CompleteTypeObject: " << exception.what());
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#endif // !defined(NDEBUG)
    TypeRegistryEntry complete_entry;
    TypeRegistryEntry minimal_entry;
    complete_entry.type_object_.complete(complete_type_object);
    minimal_entry.type_object_ = build_minimal_from_complete_type_object(complete_type_object);
    TypeIdentifierPair type_ids;
    type_ids.type_identifier1(calculate_type_identifier(minimal_entry.type_object_,
            minimal_entry.type_object_serialized_size_));
    type_ids.type_identifier2(calculate_type_identifier(complete_entry.type_object_,
            complete_entry.type_object_serialized_size_));

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    auto type_ids_result = local_type_identifiers_.insert({type_name, type_ids});
    auto min_entry_result = type_registry_entries_.insert({type_ids.type_identifier1(), minimal_entry});
    auto max_entry_result = type_registry_entries_.insert({type_ids.type_identifier2(), complete_entry});
    if (!type_ids_result.second || !min_entry_result.second || !max_entry_result.second)
    {
        if (local_type_identifiers_[type_name] != type_ids ||
                type_registry_entries_[type_ids.type_identifier1()] != minimal_entry ||
                type_registry_entries_[type_ids.type_identifier2()] != complete_entry)
        {
            if (type_ids_result.second)
            {
                local_type_identifiers_.erase(type_name);
            }
            if (min_entry_result.second)
            {
                type_registry_entries_.erase(type_ids.type_identifier1());
            }
            if (max_entry_result.second)
            {
                type_registry_entries_.erase(type_ids.type_identifier2());
            }
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::register_type_identifier(
        const std::string& type_name,
        const TypeIdentifier& type_identifier)
{
    // Preconditions
    if (TypeObjectUtils::is_direct_hash_type_identifier(type_identifier) || type_name.empty())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#if !defined(NDEBUG)
    try
    {
        TypeObjectUtils::type_identifier_consistency(type_identifier);
    }
    catch (eprosima::fastdds::dds::xtypes::InvalidArgumentError& exception)
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Inconsistent TypeIdentifier: " << exception.what());
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#endif // !defined(NDEBUG)
    TypeIdentifierPair type_identifiers;
    type_identifiers.type_identifier1(type_identifier);

    switch (type_identifier._d())
    {
        case TI_PLAIN_SEQUENCE_SMALL:
            if (EK_BOTH != type_identifier.seq_sdefn().header().equiv_kind())
            {
                type_identifiers.type_identifier2(type_identifier);
                type_identifiers.type_identifier1().seq_sdefn().header().equiv_kind(EK_MINIMAL);
                type_identifiers.type_identifier1().seq_sdefn().element_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().seq_sdefn().element_identifier())));
            }
            break;
        case TI_PLAIN_SEQUENCE_LARGE:
            if (EK_BOTH != type_identifier.seq_ldefn().header().equiv_kind())
            {
                type_identifiers.type_identifier2(type_identifier);
                type_identifiers.type_identifier1().seq_ldefn().header().equiv_kind(EK_MINIMAL);
                type_identifiers.type_identifier1().seq_ldefn().element_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().seq_ldefn().element_identifier())));
            }
            break;
        case TI_PLAIN_ARRAY_SMALL:
            if (EK_BOTH != type_identifier.array_sdefn().header().equiv_kind())
            {
                type_identifiers.type_identifier2(type_identifier);
                type_identifiers.type_identifier1().array_sdefn().header().equiv_kind(EK_MINIMAL);
                type_identifiers.type_identifier1().array_sdefn().element_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().array_sdefn().element_identifier())));
            }
            break;
        case TI_PLAIN_ARRAY_LARGE:
            if (EK_BOTH != type_identifier.array_ldefn().header().equiv_kind())
            {
                type_identifiers.type_identifier2(type_identifier);
                type_identifiers.type_identifier1().array_ldefn().header().equiv_kind(EK_MINIMAL);
                type_identifiers.type_identifier1().array_ldefn().element_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().array_ldefn().element_identifier())));
            }
            break;
        case TI_PLAIN_MAP_SMALL:
            if (EK_BOTH != type_identifier.map_sdefn().header().equiv_kind())
            {
                type_identifiers.type_identifier2(type_identifier);
                type_identifiers.type_identifier1().map_sdefn().header().equiv_kind(EK_MINIMAL);
                type_identifiers.type_identifier1().map_sdefn().element_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().map_sdefn().element_identifier())));
            }
            if (TypeObjectUtils::is_direct_hash_type_identifier(*type_identifier.map_sdefn().key_identifier()))
            {
                if (TK_NONE == type_identifiers.type_identifier2()._d())
                {
                    type_identifiers.type_identifier2(type_identifier);
                }
                type_identifiers.type_identifier1().map_sdefn().key_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().map_sdefn().key_identifier())));
            }
            break;
        case TI_PLAIN_MAP_LARGE:
            if (EK_BOTH != type_identifier.map_ldefn().header().equiv_kind())
            {
                type_identifiers.type_identifier2(type_identifier);
                type_identifiers.type_identifier1().map_ldefn().header().equiv_kind(EK_MINIMAL);
                type_identifiers.type_identifier1().map_ldefn().element_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().map_ldefn().element_identifier())));
            }
            if (TypeObjectUtils::is_direct_hash_type_identifier(*type_identifier.map_ldefn().key_identifier()))
            {
                if (TK_NONE == type_identifiers.type_identifier2()._d())
                {
                    type_identifiers.type_identifier2(type_identifier);
                }
                type_identifiers.type_identifier1().map_ldefn().key_identifier(new TypeIdentifier(
                            minimal_from_complete_type_identifier(
                                *type_identifiers.type_identifier2().map_ldefn().key_identifier())));
            }
            break;
        default:
            break;
    }

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    auto result = local_type_identifiers_.insert({type_name, type_identifiers});
    if (!result.second)
    {
        if (local_type_identifiers_[type_name] != type_identifiers)
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::get_type_objects(
        const std::string& type_name,
        TypeObjectPair& type_objects)
{
    if (type_name.empty())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
    TypeIdentifierPair type_ids;
    ReturnCode_t ret_code = get_type_identifiers(type_name, type_ids);
    if (eprosima::fastdds::dds::RETCODE_OK == ret_code)
    {
        if (!TypeObjectUtils::is_direct_hash_type_identifier(type_ids.type_identifier1()) ||
                !TypeObjectUtils::is_direct_hash_type_identifier(type_ids.type_identifier2()))
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }

        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        if (EK_MINIMAL == type_ids.type_identifier1()._d())
        {
            type_objects.minimal_type_object =
                    type_registry_entries_.at(type_ids.type_identifier1()).type_object_.minimal();
            type_objects.complete_type_object =
                    type_registry_entries_.at(type_ids.type_identifier2()).type_object_.complete();
        }
        else
        {
            type_objects.complete_type_object =
                    type_registry_entries_.at(type_ids.type_identifier1()).type_object_.complete();
            type_objects.minimal_type_object =
                    type_registry_entries_.at(type_ids.type_identifier2()).type_object_.minimal();
        }
    }
    return ret_code;
}

ReturnCode_t TypeObjectRegistry::get_type_identifiers(
        const std::string& type_name,
        TypeIdentifierPair& type_identifiers)
{
    if (type_name.empty())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
    try
    {
        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        type_identifiers = local_type_identifiers_.at(type_name);
    }
    catch (std::exception&)
    {
        return eprosima::fastdds::dds::RETCODE_NO_DATA;
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::get_type_object(
        const TypeIdentifier& type_identifier,
        TypeObject& type_object)
{
    if (!TypeObjectUtils::is_direct_hash_type_identifier(type_identifier))
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
    try
    {
        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        type_object = type_registry_entries_.at(type_identifier).type_object_;
    }
    catch (std::exception&)
    {
        return eprosima::fastdds::dds::RETCODE_NO_DATA;
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::get_type_information(
        const std::string& type_name,
        TypeInformation& type_information)
{
    TypeIdentifierPair type_ids;
    ReturnCode_t ret_code = get_type_identifiers(type_name, type_ids);
    if (eprosima::fastdds::dds::RETCODE_OK == ret_code)
    {
        if (!TypeObjectUtils::is_direct_hash_type_identifier(type_ids.type_identifier1()) ||
                !TypeObjectUtils::is_direct_hash_type_identifier(type_ids.type_identifier2()))
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
        if (EK_COMPLETE == type_ids.type_identifier1()._d())
        {
            type_information.complete().typeid_with_size().type_id(type_ids.type_identifier1());
            type_information.complete().dependent_typeid_count(NO_DEPENDENCIES);
            type_information.minimal().typeid_with_size().type_id(type_ids.type_identifier2());
            type_information.minimal().dependent_typeid_count(NO_DEPENDENCIES);

            std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
            type_information.complete().typeid_with_size().typeobject_serialized_size(type_registry_entries_.at(
                        type_ids.type_identifier1()).type_object_serialized_size_);
            type_information.minimal().typeid_with_size().typeobject_serialized_size(type_registry_entries_.at(
                        type_ids.type_identifier2()).type_object_serialized_size_);
        }
        else
        {
            type_information.minimal().typeid_with_size().type_id(type_ids.type_identifier1());
            type_information.minimal().dependent_typeid_count(NO_DEPENDENCIES);
            type_information.complete().typeid_with_size().type_id(type_ids.type_identifier2());
            type_information.complete().dependent_typeid_count(NO_DEPENDENCIES);

            std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
            type_information.minimal().typeid_with_size().typeobject_serialized_size(type_registry_entries_.at(
                        type_ids.type_identifier1()).type_object_serialized_size_);
            type_information.complete().typeid_with_size().typeobject_serialized_size(type_registry_entries_.at(
                        type_ids.type_identifier2()).type_object_serialized_size_);
        }
    }
    return ret_code;
}

ReturnCode_t TypeObjectRegistry::get_type_dependencies(
        const TypeIdentifierSeq& type_identifiers,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    ReturnCode_t ret_code = eprosima::fastdds::dds::RETCODE_OK;
    for (const TypeIdentifier& type_id : type_identifiers)
    {
        if (!TypeObjectUtils::is_direct_hash_type_identifier(type_id))
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
        TypeObject type_object;
        ret_code = get_type_object(type_id, type_object);
        if (eprosima::fastdds::dds::RETCODE_OK == ret_code)
        {
            ret_code = get_dependencies_from_type_object(type_object, type_dependencies);
            if (eprosima::fastdds::dds::RETCODE_OK != ret_code)
            {
                break;
            }
        }
    }
    return ret_code;
}

bool TypeObjectRegistry::is_type_identifier_known(
        const TypeIdentfierWithSize& type_identifier_with_size)
{
    if (TypeObjectUtils::is_direct_hash_type_identifier(type_identifier_with_size.type_id()))
    {
        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        // Check TypeIdentifier is known
        auto it = type_registry_entries_.find(type_identifier_with_size.type_id());
        if (it != type_registry_entries_.end())
        {
            // Check typeobject_serialized_size is the same
            if (it->second.type_object_serialized_size_ == type_identifier_with_size.typeobject_serialized_size())
            {
                return true;
            }
        }
    }

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    for (const auto& it : local_type_identifiers_)
    {
        if (it.second.type_identifier1() == type_identifier_with_size.type_id() ||
                it.second.type_identifier2() == type_identifier_with_size.type_id())
        {
            return true;
        }
    }
    return false;
}

bool TypeObjectRegistry::is_builtin_annotation(
        const TypeIdentifier& type_identifier)
{
    if (!TypeObjectUtils::is_direct_hash_type_identifier(type_identifier))
    {
        return false;
    }

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    for (const auto& it : local_type_identifiers_)
    {
        if (it.second.type_identifier1() == type_identifier || it.second.type_identifier2() == type_identifier)
        {
            return is_builtin_annotation_name(it.first);
        }
    }
    return false;
}

const TypeIdentifier TypeObjectRegistry::calculate_type_identifier(
        const TypeObject& type_object,
        uint32_t& type_object_serialized_size)
{
    TypeIdentifier type_id;
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    eprosima::fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                calculator.calculate_serialized_size(type_object, current_alignment)));
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
            eprosima::fastcdr::CdrVersion::XCDRv2);
    ser << type_object;
    type_object_serialized_size = static_cast<uint32_t>(ser.get_serialized_data_length());
    EquivalenceHash equivalence_hash;
    MD5 type_object_hash;
    type_object_hash.update(reinterpret_cast<char*>(payload.data), type_object_serialized_size);
    type_object_hash.finalize();
    for (size_t i = 0; i < equivalence_hash.size(); i++)
    {
        equivalence_hash[i] = type_object_hash.digest[i];
    }
    type_id.equivalence_hash(equivalence_hash);
    type_id._d(type_object._d());
    return type_id;
}

TypeObjectRegistry::TypeObjectRegistry()
{
    register_primitive_type_identifiers();
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const TypeIdentifier& type_identifier,
        const TypeObject& type_object)
{
    uint32_t type_object_serialized_size = 0;
    TypeObject minimal_type_object;
    if (type_identifier._d() != type_object._d() ||
            type_identifier != calculate_type_identifier(type_object, type_object_serialized_size))
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
    if (EK_COMPLETE == type_object._d())
    {
        TypeRegistryEntry entry;
        entry.type_object_ = build_minimal_from_complete_type_object(type_object.complete());
        TypeIdentifier minimal_type_id = calculate_type_identifier(entry.type_object_,
                        entry.type_object_serialized_size_);

        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        type_registry_entries_.insert({minimal_type_id, entry});
    }
    TypeRegistryEntry entry;
    entry.type_object_ = type_object;
    entry.type_object_serialized_size_ = type_object_serialized_size;

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    type_registry_entries_.insert({type_identifier, entry});
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::are_types_compatible(
        const TypeIdentifierPair& type_identifiers,
        const TypeConsistencyEnforcementQosPolicy& type_consistency_qos)
{
    static_cast<void>(type_identifiers);
    static_cast<void>(type_consistency_qos);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_dependencies_from_type_object(
        const TypeObject& type_object,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    ReturnCode_t ret_code = eprosima::fastdds::dds::RETCODE_OK;
    TypeIdentifierSeq dependent_type_ids;
    TypeIdentfierWithSize type_id_size;
    switch (type_object._d())
    {
        case EK_MINIMAL:
            switch (type_object.minimal()._d())
            {
                case TK_ALIAS:
                    ret_code = get_alias_dependencies(type_object.minimal().alias_type(), type_dependencies);
                    break;
                case TK_ANNOTATION:
                    ret_code = get_annotation_dependencies(type_object.minimal().annotation_type(), type_dependencies);
                    break;
                case TK_STRUCTURE:
                    ret_code = get_structure_dependencies(type_object.minimal().struct_type(), type_dependencies);
                    break;
                case TK_UNION:
                    ret_code = get_union_dependencies(type_object.minimal().union_type(), type_dependencies);
                    break;
                case TK_SEQUENCE:
                    ret_code = get_sequence_array_dependencies(type_object.minimal().sequence_type(),
                                    type_dependencies);
                    break;
                case TK_ARRAY:
                    ret_code = get_sequence_array_dependencies(type_object.minimal().array_type(), type_dependencies);
                    break;
                case TK_MAP:
                    ret_code = get_map_dependencies(type_object.minimal().map_type(), type_dependencies);
                    break;
                // No dependencies
                case TK_BITSET:
                case TK_ENUM:
                case TK_BITMASK:
                    break;
            }
            break;
        case EK_COMPLETE:
            switch (type_object.complete()._d())
            {
                case TK_ALIAS:
                    ret_code = get_alias_dependencies(type_object.complete().alias_type(), type_dependencies);
                    break;
                case TK_ANNOTATION:
                    ret_code = get_annotation_dependencies(type_object.complete().annotation_type(), type_dependencies);
                    break;
                case TK_STRUCTURE:
                    ret_code = get_structure_dependencies(type_object.complete().struct_type(), type_dependencies);
                    break;
                case TK_UNION:
                    ret_code = get_union_dependencies(type_object.complete().union_type(), type_dependencies);
                    break;
                case TK_SEQUENCE:
                    ret_code = get_sequence_array_dependencies(type_object.complete().sequence_type(),
                                    type_dependencies);
                    break;
                case TK_ARRAY:
                    ret_code = get_sequence_array_dependencies(type_object.complete().array_type(), type_dependencies);
                    break;
                case TK_MAP:
                    ret_code = get_map_dependencies(type_object.complete().map_type(), type_dependencies);
                    break;
                // No dependencies
                case TK_BITSET:
                case TK_ENUM:
                case TK_BITMASK:
                    break;
            }
            break;
    }
    return ret_code;
}

void TypeObjectRegistry::add_dependency(
        const TypeIdentifier& type_id,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    TypeIdentfierWithSize type_id_size;
    type_id_size.type_id(type_id);
    {
        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        type_id_size.typeobject_serialized_size(type_registry_entries_.at(type_id).type_object_serialized_size_);
    }
    type_dependencies.insert(type_id_size);
}

bool TypeObjectRegistry::is_builtin_annotation_name(
        const std::string& name)
{
    if (name == id_annotation_name || name == autoid_annotation_name || name == optional_annotation_name ||
            name == position_annotation_name || name == value_annotation_name ||
            name == extensibility_annotation_name ||
            name == final_annotation_name || name == appendable_annotation_name || name == mutable_annotation_name ||
            name == key_annotation_name || name == must_understand_annotation_name ||
            name == default_literal_annotation_name || name == default_annotation_name ||
            name == range_annotation_name ||
            name == min_annotation_name || name == max_annotation_name || name == unit_annotation_name ||
            name == bit_bound_annotation_name || name == external_annotation_name || name == nested_annotation_name ||
            name == verbatim_annotation_name || name == service_annotation_name || name == oneway_annotation_name ||
            name == ami_annotation_name || name == hashid_annotation_name || name == default_nested_annotation_name ||
            name == ignore_literal_names_annotation_name || name == try_construct_annotation_name ||
            name == non_serialized_annotation_name || name == data_representation_annotation_name ||
            name == topic_annotation_name)
    {
        return true;
    }
    return false;
}

const TypeObject TypeObjectRegistry::build_minimal_from_complete_type_object(
        const CompleteTypeObject& complete_type_object)
{
    MinimalTypeObject minimal_type_object;
    switch (complete_type_object._d())
    {
        case TK_ALIAS:
        {
            MinimalAliasType minimal_alias_type = build_minimal_from_complete_alias_type(
                complete_type_object.alias_type());
            minimal_type_object.alias_type(minimal_alias_type);
            break;
        }
        case TK_ANNOTATION:
        {
            MinimalAnnotationType minimal_annotation_type = build_minimal_from_complete_annotation_type(
                complete_type_object.annotation_type());
            minimal_type_object.annotation_type(minimal_annotation_type);
            break;
        }
        case TK_STRUCTURE:
        {
            MinimalStructType minimal_struct_type = build_minimal_from_complete_struct_type(
                complete_type_object.struct_type());
            minimal_type_object.struct_type(minimal_struct_type);
            break;
        }
        case TK_UNION:
        {
            MinimalUnionType minimal_union_type = build_minimal_from_complete_union_type(
                complete_type_object.union_type());
            minimal_type_object.union_type(minimal_union_type);
            break;
        }
        case TK_BITSET:
        {
            MinimalBitsetType minimal_bitset_type = build_minimal_from_complete_bitset_type(
                complete_type_object.bitset_type());
            minimal_type_object.bitset_type(minimal_bitset_type);
            break;
        }
        case TK_SEQUENCE:
        {
            MinimalSequenceType minimal_sequence_type = build_minimal_from_complete_sequence_type(
                complete_type_object.sequence_type());
            minimal_type_object.sequence_type(minimal_sequence_type);
            break;
        }
        case TK_ARRAY:
        {
            MinimalArrayType minimal_array_type = build_minimal_from_complete_array_type(
                complete_type_object.array_type());
            minimal_type_object.array_type(minimal_array_type);
            break;
        }
        case TK_MAP:
        {
            MinimalMapType minimal_map_type = build_minimal_from_complete_map_type(
                complete_type_object.map_type());
            minimal_type_object.map_type(minimal_map_type);
            break;
        }
        case TK_ENUM:
        {
            MinimalEnumeratedType minimal_enumerated_type = build_minimal_from_complete_enumerated_type(
                complete_type_object.enumerated_type());
            minimal_type_object.enumerated_type(minimal_enumerated_type);
            break;
        }
        case TK_BITMASK:
        {
            MinimalBitmaskType minimal_bitmask_type = build_minimal_from_complete_bitmask_type(
                complete_type_object.bitmask_type());
            minimal_type_object.bitmask_type(minimal_bitmask_type);
            break;
        }
    }
    TypeObject type_object;
    type_object.minimal(minimal_type_object);
    return type_object;
}

const MinimalAliasType TypeObjectRegistry::build_minimal_from_complete_alias_type(
        const CompleteAliasType& complete_alias_type)
{
    MinimalAliasType minimal_alias_type;
    // alias_flags: unused. No flags apply.
    // header: empty. Available for future extension.
    minimal_alias_type.body().common(complete_alias_type.body().common());
    minimal_alias_type.body().common().related_type(minimal_from_complete_type_identifier(
                complete_alias_type.body().common().related_type()));
    return minimal_alias_type;
}

const MinimalAnnotationType TypeObjectRegistry::build_minimal_from_complete_annotation_type(
        const CompleteAnnotationType& complete_annotation_type)
{
    MinimalAnnotationType minimal_annotation_type;
    // annotation_flag: unused. No flags apply.
    // header: empty. Available for future extension.
    MinimalAnnotationParameterSeq minimal_annotation_parameter_sequence;
    for (const CompleteAnnotationParameter& complete_annotation_parameter : complete_annotation_type.member_seq())
    {
        MinimalAnnotationParameter minimal_annotation_parameter;
        minimal_annotation_parameter.common(complete_annotation_parameter.common());
        minimal_annotation_parameter.common().member_type_id(minimal_from_complete_type_identifier(
                    complete_annotation_parameter.common().member_type_id()));
        minimal_annotation_parameter.name_hash(TypeObjectUtils::name_hash(
                    complete_annotation_parameter.name().c_str()));
        minimal_annotation_parameter.default_value(complete_annotation_parameter.default_value());
        auto it = minimal_annotation_parameter_sequence.begin();
        for (; it !=  minimal_annotation_parameter_sequence.end(); ++it)
        {
            if (it->name_hash() > minimal_annotation_parameter.name_hash())
            {
                break;
            }
        }
        minimal_annotation_parameter_sequence.emplace(it, minimal_annotation_parameter);
    }
    minimal_annotation_type.member_seq(minimal_annotation_parameter_sequence);
    return minimal_annotation_type;
}

const MinimalStructType TypeObjectRegistry::build_minimal_from_complete_struct_type(
        const CompleteStructType& complete_struct_type)
{
    MinimalStructType minimal_struct_type;
    minimal_struct_type.struct_flags(complete_struct_type.struct_flags());
    minimal_struct_type.header().base_type(minimal_from_complete_type_identifier(
                complete_struct_type.header().base_type()));
    // header().detail: empty. Available for future extension.
    MinimalStructMemberSeq minimal_struct_member_sequence;
    for (const CompleteStructMember& complete_struct_member : complete_struct_type.member_seq())
    {
        MinimalStructMember minimal_struct_member;
        minimal_struct_member.common(complete_struct_member.common());
        minimal_struct_member.common().member_type_id(minimal_from_complete_type_identifier(
                    complete_struct_member.common().member_type_id()));
        minimal_struct_member.detail().name_hash(TypeObjectUtils::name_hash(
                    complete_struct_member.detail().name().c_str()));
        minimal_struct_member_sequence.push_back(minimal_struct_member);
    }
    minimal_struct_type.member_seq(minimal_struct_member_sequence);
    return minimal_struct_type;
}

const MinimalUnionType TypeObjectRegistry::build_minimal_from_complete_union_type(
        const CompleteUnionType& complete_union_type)
{
    MinimalUnionType minimal_union_type;
    minimal_union_type.union_flags(complete_union_type.union_flags());
    // header: empty. Available for future extension.
    minimal_union_type.discriminator().common(complete_union_type.discriminator().common());
    minimal_union_type.discriminator().common().type_id(minimal_from_complete_type_identifier(
                complete_union_type.discriminator().common().type_id()));
    MinimalUnionMemberSeq minimal_union_member_sequence;
    for (const CompleteUnionMember& complete_union_member : complete_union_type.member_seq())
    {
        MinimalUnionMember minimal_union_member;
        minimal_union_member.common(complete_union_member.common());
        minimal_union_member.common().type_id(minimal_from_complete_type_identifier(
                    minimal_union_member.common().type_id()));
        minimal_union_member.detail().name_hash(TypeObjectUtils::name_hash(
                    complete_union_member.detail().name().c_str()));
        minimal_union_member_sequence.push_back(minimal_union_member);
    }
    minimal_union_type.member_seq(minimal_union_member_sequence);
    return minimal_union_type;
}

const MinimalBitsetType TypeObjectRegistry::build_minimal_from_complete_bitset_type(
        const CompleteBitsetType& complete_bitset_type)
{
    MinimalBitsetType minimal_bitset_type;
    // bitset_flags: unused. No flags apply.
    // header: empty. Available for future extension.
    MinimalBitfieldSeq minimal_bitfield_sequence;
    for (const CompleteBitfield& complete_bitfield : complete_bitset_type.field_seq())
    {
        MinimalBitfield minimal_bitfield;
        minimal_bitfield.common(complete_bitfield.common());
        minimal_bitfield.name_hash(TypeObjectUtils::name_hash(
                    complete_bitfield.detail().name().c_str()));
        minimal_bitfield_sequence.push_back(minimal_bitfield);
    }
    minimal_bitset_type.field_seq(minimal_bitfield_sequence);
    return minimal_bitset_type;
}

const MinimalSequenceType TypeObjectRegistry::build_minimal_from_complete_sequence_type(
        const CompleteSequenceType& complete_sequence_type)
{
    MinimalSequenceType minimal_sequence_type;
    // collection_flag: unused. No flags apply.
    minimal_sequence_type.header().common(complete_sequence_type.header().common());
    minimal_sequence_type.element().common(complete_sequence_type.element().common());
    minimal_sequence_type.element().common().type(minimal_from_complete_type_identifier(
                complete_sequence_type.element().common().type()));
    return minimal_sequence_type;
}

const MinimalArrayType TypeObjectRegistry::build_minimal_from_complete_array_type(
        const CompleteArrayType& complete_array_type)
{
    MinimalArrayType minimal_array_type;
    // collection_flag: unused. No flags apply.
    minimal_array_type.header().common(complete_array_type.header().common());
    minimal_array_type.element().common(complete_array_type.element().common());
    minimal_array_type.element().common().type(minimal_from_complete_type_identifier(
                complete_array_type.element().common().type()));
    return minimal_array_type;
}

const MinimalMapType TypeObjectRegistry::build_minimal_from_complete_map_type(
        const CompleteMapType& complete_map_type)
{
    MinimalMapType minimal_map_type;
    // collection_flag: unused. No flags apply.
    minimal_map_type.header().common(complete_map_type.header().common());
    minimal_map_type.key().common(complete_map_type.key().common());
    minimal_map_type.key().common().type(minimal_from_complete_type_identifier(
                complete_map_type.key().common().type()));
    minimal_map_type.element().common(complete_map_type.element().common());
    minimal_map_type.element().common().type(minimal_from_complete_type_identifier(
                complete_map_type.element().common().type()));
    return minimal_map_type;
}

const MinimalEnumeratedType TypeObjectRegistry::build_minimal_from_complete_enumerated_type(
        const CompleteEnumeratedType& complete_enumerated_type)
{
    MinimalEnumeratedType minimal_enumerated_type;
    // enum_flags: unused. No flags apply.
    minimal_enumerated_type.header().common(complete_enumerated_type.header().common());
    MinimalEnumeratedLiteralSeq minimal_enumerated_literal_sequence;
    for (const CompleteEnumeratedLiteral& complete_enumerated_literal : complete_enumerated_type.literal_seq())
    {
        MinimalEnumeratedLiteral minimal_enumerated_literal;
        minimal_enumerated_literal.common(complete_enumerated_literal.common());
        minimal_enumerated_literal.detail().name_hash(TypeObjectUtils::name_hash(
                    complete_enumerated_literal.detail().name().c_str()));
        minimal_enumerated_literal_sequence.push_back(minimal_enumerated_literal);
    }
    minimal_enumerated_type.literal_seq(minimal_enumerated_literal_sequence);
    return minimal_enumerated_type;
}

const MinimalBitmaskType TypeObjectRegistry::build_minimal_from_complete_bitmask_type(
        const CompleteBitmaskType& complete_bitmask_type)
{
    MinimalBitmaskType minimal_bitmask_type;
    // bitmask_flags: unused. No flags apply.
    minimal_bitmask_type.header().common(complete_bitmask_type.header().common());
    MinimalBitflagSeq minimal_bitflag_sequence;
    for (const CompleteBitflag& complete_bitflag : complete_bitmask_type.flag_seq())
    {
        MinimalBitflag minimal_bitflag;
        minimal_bitflag.common(complete_bitflag.common());
        minimal_bitflag.detail().name_hash(TypeObjectUtils::name_hash(
                    complete_bitflag.detail().name().c_str()));
        minimal_bitflag_sequence.push_back(minimal_bitflag);
    }
    minimal_bitmask_type.flag_seq(minimal_bitflag_sequence);
    return minimal_bitmask_type;
}

void TypeObjectRegistry::register_primitive_type_identifiers()
{
    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    TypeIdentifierPair type_ids;
    type_ids.type_identifier1()._d(TK_BOOLEAN);
    local_type_identifiers_.insert({boolean_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_BYTE);
    local_type_identifiers_.insert({byte_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_INT16);
    local_type_identifiers_.insert({int16_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_INT32);
    local_type_identifiers_.insert({int32_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_INT64);
    local_type_identifiers_.insert({int64_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_UINT16);
    local_type_identifiers_.insert({uint16_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_UINT32);
    local_type_identifiers_.insert({uint32_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_UINT64);
    local_type_identifiers_.insert({uint64_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_FLOAT32);
    local_type_identifiers_.insert({float32_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_FLOAT64);
    local_type_identifiers_.insert({float64_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_FLOAT128);
    local_type_identifiers_.insert({float128_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_INT8);
    local_type_identifiers_.insert({int8_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_UINT8);
    local_type_identifiers_.insert({uint8_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_CHAR8);
    local_type_identifiers_.insert({char8_type_name, type_ids});
    type_ids.type_identifier1()._d(TK_CHAR16);
    local_type_identifiers_.insert({char16_type_name, type_ids});
}

const TypeIdentifier TypeObjectRegistry::minimal_from_complete_type_identifier(
        const TypeIdentifier& type_id)
{
    if (EK_COMPLETE == type_id._d() ||
            (TI_PLAIN_SEQUENCE_SMALL == type_id._d() && type_id.seq_sdefn().header().equiv_kind() == EK_COMPLETE) ||
            (TI_PLAIN_SEQUENCE_LARGE == type_id._d() && type_id.seq_ldefn().header().equiv_kind() == EK_COMPLETE) ||
            (TI_PLAIN_ARRAY_SMALL == type_id._d() && type_id.array_sdefn().header().equiv_kind() == EK_COMPLETE) ||
            (TI_PLAIN_ARRAY_LARGE == type_id._d() && type_id.array_ldefn().header().equiv_kind() == EK_COMPLETE) ||
            (TI_PLAIN_MAP_SMALL == type_id._d() && (type_id.map_sdefn().header().equiv_kind() == EK_COMPLETE ||
            type_id.map_sdefn().key_identifier()->_d() == EK_COMPLETE)) ||
            (TI_PLAIN_MAP_LARGE == type_id._d() && (type_id.map_ldefn().header().equiv_kind() == EK_COMPLETE ||
            type_id.map_ldefn().key_identifier()->_d() == EK_COMPLETE)))
    {
        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        for (const auto& it : local_type_identifiers_)
        {
            if (it.second.type_identifier1() == type_id)
            {
                return it.second.type_identifier2();
            }
            else if (it.second.type_identifier2() == type_id)
            {
                return it.second.type_identifier1();
            }
        }
    }
    return type_id;
}

} // xtypes
} // dds
} // fastdds
} // eprosima
