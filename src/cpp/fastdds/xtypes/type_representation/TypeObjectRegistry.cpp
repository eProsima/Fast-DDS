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

#include <cassert>
#include <algorithm>
#include <exception>
#include <mutex>
#include <string>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>
#include <fastcdr/xcdr/external.hpp>
#include <fastcdr/xcdr/optional.hpp>

#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/VerbatimTextDescriptor.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>
#include <fastdds/utils/md5.hpp>

#include <fastdds/xtypes/dynamic_types/AnnotationDescriptorImpl.hpp>
#include <fastdds/xtypes/dynamic_types/DynamicTypeImpl.hpp>
#include <fastdds/xtypes/dynamic_types/TypeDescriptorImpl.hpp>
#include <fastdds/xtypes/dynamic_types/TypeValueConverter.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

constexpr const int32_t NO_DEPENDENCIES = -1;

bool TypeRegistryEntry::operator !=(
        const TypeRegistryEntry& entry)
{
    return this->type_object != entry.type_object ||
           this->type_object_serialized_size != entry.type_object_serialized_size ||
           this->complementary_type_id != entry.complementary_type_id;
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const std::string& type_name,
        const CompleteTypeObject& complete_type_object,
        TypeIdentifierPair& type_ids)
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
        EPROSIMA_LOG_ERROR(
            XTYPES_TYPE_REPRESENTATION,
            "Inconsistent CompleteTypeObject: " << exception.what());
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#endif // !defined(NDEBUG)
    TypeRegistryEntry complete_entry;
    TypeRegistryEntry minimal_entry;
    complete_entry.type_object.complete(complete_type_object);
    minimal_entry.type_object = build_minimal_from_complete_type_object(complete_type_object);
    type_ids.type_identifier1(
        calculate_type_identifier(
            minimal_entry.type_object,
            minimal_entry.type_object_serialized_size));
    type_ids.type_identifier2(
        calculate_type_identifier(
            complete_entry.type_object,
            complete_entry.type_object_serialized_size));
    complete_entry.complementary_type_id = type_ids.type_identifier1();
    minimal_entry.complementary_type_id = type_ids.type_identifier2();

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    auto type_ids_result {local_type_identifiers_.insert({type_name, type_ids})};

    if (type_ids_result.second)
    {
        auto min_entry_result {type_registry_entries_.insert(
                                   {type_ids.type_identifier1(), minimal_entry})};
        if (!min_entry_result.second)
        {
            EPROSIMA_LOG_INFO(
                XTYPES_TYPE_REPRESENTATION,
                "Type " << type_name << " already registered his EK_MINIMAL remotely.");
        }
        auto max_entry_result {type_registry_entries_.insert(
                                   {type_ids.type_identifier2(), complete_entry})};
        if (!max_entry_result.second)
        {
            EPROSIMA_LOG_INFO(
                XTYPES_TYPE_REPRESENTATION,
                "Type " << type_name << " already registered his EK_COMPLETE remotely.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(
            XTYPES_TYPE_REPRESENTATION,
            "Type " << type_name << " already registered locally.");
        return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::register_type_identifier(
        const std::string& type_name,
        TypeIdentifierPair& type_identifier)
{
    // Preconditions
    if (TypeObjectUtils::is_direct_hash_type_identifier(type_identifier.type_identifier1()) ||
            type_name.empty())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#if !defined(NDEBUG)
    try
    {
        TypeObjectUtils::type_identifier_consistency(type_identifier.type_identifier1());
    }
    catch (eprosima::fastdds::dds::xtypes::InvalidArgumentError& exception)
    {
        EPROSIMA_LOG_ERROR(
            XTYPES_TYPE_REPRESENTATION,
            "Inconsistent TypeIdentifier: " << exception.what());
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#endif // !defined(NDEBUG)

    type_identifier.type_identifier2().no_value({});

    switch (type_identifier.type_identifier1()._d()){
        case TI_PLAIN_SEQUENCE_SMALL:
            if (EK_BOTH != type_identifier.type_identifier1().seq_sdefn().header().equiv_kind())
            {
                type_identifier.type_identifier2(type_identifier.type_identifier1());
                type_identifier.type_identifier1().seq_sdefn().header().equiv_kind(EK_MINIMAL);
                type_identifier.type_identifier1().seq_sdefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().seq_sdefn().element_identifier())));
            }
            break;
        case TI_PLAIN_SEQUENCE_LARGE:
            if (EK_BOTH != type_identifier.type_identifier1().seq_ldefn().header().equiv_kind())
            {
                type_identifier.type_identifier2(type_identifier.type_identifier1());
                type_identifier.type_identifier1().seq_ldefn().header().equiv_kind(EK_MINIMAL);
                type_identifier.type_identifier1().seq_ldefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().seq_ldefn().element_identifier())));
            }
            break;
        case TI_PLAIN_ARRAY_SMALL:
            if (EK_BOTH != type_identifier.type_identifier1().array_sdefn().header().equiv_kind())
            {
                type_identifier.type_identifier2(type_identifier.type_identifier1());
                type_identifier.type_identifier1().array_sdefn().header().equiv_kind(EK_MINIMAL);
                type_identifier.type_identifier1().array_sdefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().array_sdefn().element_identifier())));
            }
            break;
        case TI_PLAIN_ARRAY_LARGE:
            if (EK_BOTH != type_identifier.type_identifier1().array_ldefn().header().equiv_kind())
            {
                type_identifier.type_identifier2(type_identifier.type_identifier1());
                type_identifier.type_identifier1().array_ldefn().header().equiv_kind(EK_MINIMAL);
                type_identifier.type_identifier1().array_ldefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().array_ldefn().element_identifier())));
            }
            break;
        case TI_PLAIN_MAP_SMALL:
            if (EK_BOTH != type_identifier.type_identifier1().map_sdefn().header().equiv_kind())
            {
                type_identifier.type_identifier2(type_identifier.type_identifier1());
                type_identifier.type_identifier1().map_sdefn().header().equiv_kind(EK_MINIMAL);
                type_identifier.type_identifier1().map_sdefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().map_sdefn().element_identifier())));
            }
            if (TypeObjectUtils::is_direct_hash_type_identifier(
                        *type_identifier.type_identifier1().map_sdefn().
                                key_identifier()))
            {
                if (TK_NONE == type_identifier.type_identifier2()._d())
                {
                    type_identifier.type_identifier2(type_identifier.type_identifier1());
                }
                type_identifier.type_identifier1().map_sdefn().key_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().map_sdefn().key_identifier())));
            }
            break;
        case TI_PLAIN_MAP_LARGE:
            if (EK_BOTH != type_identifier.type_identifier1().map_ldefn().header().equiv_kind())
            {
                type_identifier.type_identifier2(type_identifier.type_identifier1());
                type_identifier.type_identifier1().map_ldefn().header().equiv_kind(EK_MINIMAL);
                type_identifier.type_identifier1().map_ldefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().map_ldefn().element_identifier())));
            }
            if (TypeObjectUtils::is_direct_hash_type_identifier(
                        *type_identifier.type_identifier1().map_ldefn().
                                key_identifier()))
            {
                if (TK_NONE == type_identifier.type_identifier2()._d())
                {
                    type_identifier.type_identifier2(type_identifier.type_identifier1());
                }
                type_identifier.type_identifier1().map_ldefn().key_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_identifier.type_identifier2().map_ldefn().key_identifier())));
            }
            break;
        default:
            break;
    }

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    auto result {local_type_identifiers_.insert({type_name, type_identifier})};
    if (!result.second)
    {
        if (local_type_identifiers_[type_name] != type_identifier)
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
    ReturnCode_t ret_code {get_type_identifiers(type_name, type_ids)};
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
                    type_registry_entries_.at(type_ids.type_identifier1()).type_object;
            type_objects.complete_type_object =
                    type_registry_entries_.at(type_ids.type_identifier2()).type_object;
        }
        else
        {
            type_objects.complete_type_object =
                    type_registry_entries_.at(type_ids.type_identifier1()).type_object;
            type_objects.minimal_type_object =
                    type_registry_entries_.at(type_ids.type_identifier2()).type_object;
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
        type_object = type_registry_entries_.at(type_identifier).type_object;
    }
    catch (std::exception&)
    {
        return eprosima::fastdds::dds::RETCODE_NO_DATA;
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::get_type_information(
        const TypeIdentifierPair& type_ids,
        TypeInformation& type_information,
        bool with_dependencies)
{
    if (TK_NONE == type_ids.type_identifier1()._d())
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    if (!TypeObjectUtils::is_direct_hash_type_identifier(type_ids.type_identifier1()) ||
            (TK_NONE != type_ids.type_identifier2()._d() &&
            !TypeObjectUtils::is_direct_hash_type_identifier(type_ids.type_identifier2())))
    {
        return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
    }

    {
        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        if (type_registry_entries_.end() == type_registry_entries_.find(type_ids.type_identifier1()) ||
                (TK_NONE != type_ids.type_identifier2()._d() &&
                type_registry_entries_.end() == type_registry_entries_.find(type_ids.type_identifier2())))
        {
            return RETCODE_NO_DATA;
        }
    }

    if (EK_COMPLETE == type_ids.type_identifier1()._d())
    {
        type_information.complete().typeid_with_size().type_id(type_ids.type_identifier1());
        type_information.minimal().typeid_with_size().type_id(type_ids.type_identifier2());

        if (with_dependencies)
        {
            std::unordered_set<TypeIdentfierWithSize> type_dependencies;
            if (RETCODE_OK == get_type_dependencies_impl(
                        {type_ids.type_identifier1()},
                        type_dependencies))
            {
                type_information.complete().dependent_typeid_count(
                    static_cast<int32_t>(type_dependencies.
                            size()));
                for (auto& dependency : type_dependencies)
                {
                    type_information.complete().dependent_typeids().emplace_back(std::move(dependency));
                }
                type_dependencies.clear();
            }
            else
            {
                EPROSIMA_LOG_ERROR(
                    XTYPES_TYPE_REPRESENTATION,
                    "Error retrieving complete type dependenciest.");
            }

            if (TK_NONE != type_ids.type_identifier2()._d())
            {
                if (RETCODE_OK == get_type_dependencies_impl(
                            {type_ids.type_identifier2()},
                            type_dependencies))
                {
                    type_information.minimal().dependent_typeid_count(
                        static_cast<int32_t>(type_dependencies.
                                size()));
                    for (auto& dependency : type_dependencies)
                    {
                        type_information.minimal().dependent_typeids().emplace_back(std::move(dependency));
                    }
                    type_dependencies.clear();
                }
                else
                {
                    EPROSIMA_LOG_ERROR(
                        XTYPES_TYPE_REPRESENTATION,
                        "Error retrieving minimal type dependenciest.");
                }
            }
            else
            {
                type_information.minimal().dependent_typeid_count(NO_DEPENDENCIES);
            }
        }
        else
        {
            type_information.complete().dependent_typeid_count(NO_DEPENDENCIES);
            type_information.minimal().dependent_typeid_count(NO_DEPENDENCIES);
        }

        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        type_information.complete().typeid_with_size().typeobject_serialized_size(
            type_registry_entries_.at(
                type_ids.type_identifier1()).type_object_serialized_size);
        if (TK_NONE != type_ids.type_identifier2()._d())
        {
            type_information.minimal().typeid_with_size().typeobject_serialized_size(
                type_registry_entries_.at(
                    type_ids.type_identifier2()).type_object_serialized_size);
        }
    }
    else
    {
        type_information.minimal().typeid_with_size().type_id(type_ids.type_identifier1());
        type_information.complete().typeid_with_size().type_id(type_ids.type_identifier2());

        if (with_dependencies)
        {
            std::unordered_set<TypeIdentfierWithSize> type_dependencies;
            if (RETCODE_OK == get_type_dependencies_impl(
                        {type_ids.type_identifier1()},
                        type_dependencies))
            {
                type_information.minimal().dependent_typeid_count(
                    static_cast<int32_t>(type_dependencies.
                            size()));
                for (auto& dependency : type_dependencies)
                {
                    type_information.minimal().dependent_typeids().emplace_back(std::move(dependency));
                }
                type_dependencies.clear();
            }
            else
            {
                EPROSIMA_LOG_ERROR(
                    XTYPES_TYPE_REPRESENTATION,
                    "Error retrieving minimal type dependenciest.");
            }

            if (TK_NONE != type_ids.type_identifier2()._d())
            {
                if (RETCODE_OK == get_type_dependencies_impl(
                            {type_ids.type_identifier2()},
                            type_dependencies))
                {
                    type_information.complete().dependent_typeid_count(
                        static_cast<int32_t>(type_dependencies.
                                size()));
                    for (auto& dependency : type_dependencies)
                    {
                        type_information.complete().dependent_typeids().emplace_back(std::move(dependency));
                    }
                    type_dependencies.clear();
                }
                else
                {
                    EPROSIMA_LOG_ERROR(
                        XTYPES_TYPE_REPRESENTATION,
                        "Error retrieving complete type dependenciest.");
                }
            }
            else
            {
                type_information.minimal().dependent_typeid_count(NO_DEPENDENCIES);
            }
        }
        else
        {
            type_information.minimal().dependent_typeid_count(NO_DEPENDENCIES);
            type_information.complete().dependent_typeid_count(NO_DEPENDENCIES);
        }

        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        type_information.minimal().typeid_with_size().typeobject_serialized_size(
            type_registry_entries_.at(
                type_ids.type_identifier1()).type_object_serialized_size);
        if (TK_NONE != type_ids.type_identifier2()._d())
        {
            type_information.complete().typeid_with_size().typeobject_serialized_size(
                type_registry_entries_.at(
                    type_ids.type_identifier2()).type_object_serialized_size);
        }
    }
    return RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::get_type_dependencies(
        const TypeIdentifierSeq& type_identifiers,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    for (const TypeIdentifier& type_id : type_identifiers)
    {
        if (!TypeObjectUtils::is_direct_hash_type_identifier(type_id))
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
    }
    return get_type_dependencies_impl(type_identifiers, type_dependencies);
}

bool TypeObjectRegistry::is_type_identifier_known(
        const TypeIdentfierWithSize& type_identifier_with_size)
{
    if (TypeObjectUtils::is_direct_hash_type_identifier(type_identifier_with_size.type_id()))
    {
        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        // Check TypeIdentifier is known
        auto it {type_registry_entries_.find(type_identifier_with_size.type_id())};
        if (it != type_registry_entries_.end())
        {
            // Check typeobject_serialized_size is the same
            if (it->second.type_object_serialized_size ==
                    type_identifier_with_size.typeobject_serialized_size())
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
        if (it.second.type_identifier1() == type_identifier ||
                it.second.type_identifier2() == type_identifier)
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
    eprosima::fastdds::rtps::SerializedPayload_t payload(static_cast<uint32_t>(
                calculator.calculate_serialized_size(type_object, current_alignment)));
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data),
            payload.max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
            eprosima::fastcdr::CdrVersion::XCDRv2);
    ser << type_object;
    type_object_serialized_size = static_cast<uint32_t>(ser.get_serialized_data_length());
    EquivalenceHash equivalence_hash;
    MD5 type_object_hash;
    type_object_hash.update(reinterpret_cast<char*>(payload.data), type_object_serialized_size);
    type_object_hash.finalize();
    for (size_t i {0}; i < equivalence_hash.size(); i++)
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
        const TypeObject& type_object,
        TypeIdentifierPair& type_ids,
        bool build_minimal)
{
    uint32_t type_object_serialized_size {0};
    TypeIdentifier type_identifier {calculate_type_identifier(
                                        type_object,
                                        type_object_serialized_size)};

    if (TK_NONE == type_ids.type_identifier1()._d())
    {
        if (build_minimal && EK_COMPLETE == type_object._d())
        {
            type_ids.type_identifier2(type_identifier);
        }
        else
        {
            type_ids.type_identifier1(type_identifier);
        }
    }
    else if (type_ids.type_identifier1()._d() != type_object._d() ||
            type_identifier != type_ids.type_identifier1())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
    else if (build_minimal && EK_COMPLETE == type_object._d())
    {
        type_ids.type_identifier2(type_identifier);
    }

    TypeRegistryEntry complete_entry;

    if (build_minimal && EK_COMPLETE == type_object._d())
    {
        TypeRegistryEntry minimal_entry;
        minimal_entry.type_object = build_minimal_from_complete_type_object(type_object.complete());
        type_ids.type_identifier1(
            calculate_type_identifier(
                minimal_entry.type_object,
                minimal_entry.type_object_serialized_size));
        minimal_entry.complementary_type_id = type_ids.type_identifier2();
        complete_entry.complementary_type_id = type_ids.type_identifier1();

        std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
        type_registry_entries_.insert({type_ids.type_identifier1(), minimal_entry});
    }

    complete_entry.type_object = type_object;
    complete_entry.type_object_serialized_size = type_object_serialized_size;

    std::lock_guard<std::mutex> data_guard(type_object_registry_mutex_);
    if (!type_registry_entries_.insert({type_identifier, complete_entry}).second)
    {
        if (build_minimal && EK_COMPLETE == type_object._d())
        {
            auto it = type_registry_entries_.find(type_identifier);
            assert(type_registry_entries_.end() != it);

            if (TK_NONE == it->second.complementary_type_id._d())
            {
                it->second.complementary_type_id = type_ids.type_identifier1();
            }
            else if (type_ids.type_identifier1() != it->second.complementary_type_id)
            {
                EPROSIMA_LOG_WARNING(
                    XTYPES_TYPE_REPRESENTATION,
                    "Registering an already registered complete type object but with different minimal type identifier");
            }
        }
    }
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
    ReturnCode_t ret_code {eprosima::fastdds::dds::RETCODE_OK};
    TypeIdentifierSeq dependent_type_ids;
    TypeIdentfierWithSize type_id_size;
    switch (type_object._d()){
        case EK_MINIMAL:
            switch (type_object.minimal()._d()){
                case TK_ALIAS:
                    ret_code = get_alias_dependencies(type_object.minimal().alias_type(), type_dependencies);
                    break;
                case TK_ANNOTATION:
                    ret_code = get_annotation_dependencies(
                        type_object.minimal().annotation_type(), type_dependencies);
                    break;
                case TK_STRUCTURE:
                    ret_code = get_structure_dependencies(
                        type_object.minimal().struct_type(), type_dependencies);
                    break;
                case TK_UNION:
                    ret_code = get_union_dependencies(type_object.minimal().union_type(), type_dependencies);
                    break;
                case TK_SEQUENCE:
                    ret_code = get_sequence_array_dependencies(
                        type_object.minimal().sequence_type(),
                        type_dependencies);
                    break;
                case TK_ARRAY:
                    ret_code = get_sequence_array_dependencies(
                        type_object.minimal().array_type(), type_dependencies);
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
            switch (type_object.complete()._d()){
                case TK_ALIAS:
                    ret_code = get_alias_dependencies(type_object.complete().alias_type(), type_dependencies);

                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().alias_type().header().detail().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().alias_type().header().detail().ann_custom().value(),
                            type_dependencies);
                    }

                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().alias_type().body().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().alias_type().body().ann_custom().value(),
                            type_dependencies);
                    }
                    break;
                case TK_ANNOTATION:
                    ret_code = get_annotation_dependencies(
                        type_object.complete().annotation_type(), type_dependencies);
                    break;
                case TK_STRUCTURE:
                    ret_code = get_structure_dependencies(
                        type_object.complete().struct_type(), type_dependencies);
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().struct_type().header().detail().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().struct_type().header().detail().ann_custom().value(),
                            type_dependencies);
                    }
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK)
                    {
                        for (CompleteStructMember member : type_object.complete().struct_type().member_seq())
                        {
                            if (member.detail().ann_custom().has_value())
                            {
                                ret_code = get_custom_annotations_dependencies(
                                    member.detail().ann_custom().value(), type_dependencies);
                                if (ret_code != eprosima::fastdds::dds::RETCODE_OK)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case TK_UNION:
                    ret_code = get_union_dependencies(type_object.complete().union_type(), type_dependencies);

                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().union_type().header().detail().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().union_type().header().detail().ann_custom().value(),
                            type_dependencies);
                    }

                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().union_type().discriminator().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().union_type().discriminator().ann_custom().value(),
                            type_dependencies);
                    }
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK)
                    {
                        for (CompleteUnionMember member : type_object.complete().union_type().member_seq())
                        {
                            if (member.detail().ann_custom().has_value())
                            {
                                ret_code = get_custom_annotations_dependencies(
                                    member.detail().ann_custom().value(), type_dependencies);
                                if (ret_code != eprosima::fastdds::dds::RETCODE_OK)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case TK_SEQUENCE:
                    ret_code = get_sequence_array_dependencies(
                        type_object.complete().sequence_type(),
                        type_dependencies);
                    //TODO Collection annotations are not currently supported, so their dependencies are ignored.
                    break;
                case TK_ARRAY:
                    ret_code = get_sequence_array_dependencies(
                        type_object.complete().array_type(), type_dependencies);
                    //TODO Collection annotations are not currently supported, so their dependencies are ignored.
                    break;
                case TK_MAP:
                    ret_code = get_map_dependencies(type_object.complete().map_type(), type_dependencies);
                    //TODO Collection annotations are not currently supported, so their dependencies are ignored.
                    break;
                case TK_BITSET:
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().bitset_type().header().detail().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().bitset_type().header().detail().ann_custom().value(),
                            type_dependencies);
                    }
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK)
                    {
                        for (CompleteBitfield member : type_object.complete().bitset_type().field_seq())
                        {
                            if (member.detail().ann_custom().has_value())
                            {
                                ret_code = get_custom_annotations_dependencies(
                                    member.detail().ann_custom().value(), type_dependencies);
                                if (ret_code != eprosima::fastdds::dds::RETCODE_OK)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case TK_ENUM:
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().enumerated_type().header().detail().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().enumerated_type().header().detail().ann_custom().value(),
                            type_dependencies);
                    }
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK)
                    {
                        for (CompleteEnumeratedLiteral member :
                                type_object.complete().enumerated_type().literal_seq())
                        {
                            if (member.detail().ann_custom().has_value())
                            {
                                ret_code = get_custom_annotations_dependencies(
                                    member.detail().ann_custom().value(), type_dependencies);
                                if (ret_code != eprosima::fastdds::dds::RETCODE_OK)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case TK_BITMASK:
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK &&
                            type_object.complete().bitmask_type().header().detail().ann_custom().has_value())
                    {
                        ret_code = get_custom_annotations_dependencies(
                            type_object.complete().bitmask_type().header().detail().ann_custom().value(),
                            type_dependencies);
                    }
                    if (ret_code == eprosima::fastdds::dds::RETCODE_OK)
                    {
                        for (CompleteBitflag member : type_object.complete().bitmask_type().flag_seq())
                        {
                            if (member.detail().ann_custom().has_value())
                            {
                                ret_code = get_custom_annotations_dependencies(
                                    member.detail().ann_custom().value(), type_dependencies);
                                if (ret_code != eprosima::fastdds::dds::RETCODE_OK)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    break;
            }
            break;
    }
    return ret_code;
}

const TypeIdentifier TypeObjectRegistry::get_complementary_type_identifier(
        const TypeIdentifier& type_id)
{
    if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
    {
        std::unique_lock<std::mutex> lock(type_object_registry_mutex_);
        auto it = type_registry_entries_.find(type_id);
        if (type_registry_entries_.end() != it)
        {
            if (TK_NONE != it->second.complementary_type_id._d())
            {
                return it->second.complementary_type_id;
            }
            else if (EK_COMPLETE == type_id._d()) // From EK_COMPLETE its EK_MINIMAL complementary can be built.
            {
                TypeRegistryEntry minimal_entry;
                CompleteTypeObject complete_type_object = it->second.type_object.complete();
                lock.unlock();
                minimal_entry.type_object = build_minimal_from_complete_type_object(complete_type_object);
                minimal_entry.complementary_type_id = type_id;
                TypeIdentifier minimal_type_id = calculate_type_identifier(
                    minimal_entry.type_object,
                    minimal_entry.type_object_serialized_size);

                lock.lock();
                auto min_entry_result {type_registry_entries_.insert(
                                           {minimal_type_id, minimal_entry})};
                if (!min_entry_result.second)
                {
                    EPROSIMA_LOG_INFO(
                        XTYPES_TYPE_REPRESENTATION,
                        "Minimal type identifier already registered his EK_MINIMAL remotely.");
                }
                it = type_registry_entries_.find(type_id);
                assert(type_registry_entries_.end() != it);
                it->second.complementary_type_id = minimal_type_id;

                return minimal_type_id;
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(
                XTYPES_TYPE_REPRESENTATION,
                "Complete type identifier was not registered previously.");

        }
    }
    return type_id;
}

ReturnCode_t TypeObjectRegistry::get_type_dependencies_impl(
        const TypeIdentifierSeq& type_identifiers,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    ReturnCode_t ret_code {eprosima::fastdds::dds::RETCODE_OK};
    for (const TypeIdentifier& type_id : type_identifiers)
    {
        if (TypeObjectUtils::is_fully_descriptive_type_identifier(type_id))
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
        else if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
        {
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
        else if (TypeObjectUtils::is_indirect_hash_type_identifier(type_id))
        {
            switch (type_id._d()){
                case TI_PLAIN_SEQUENCE_SMALL:
                    get_indirect_hash_collection_dependencies(type_id.seq_sdefn(), type_dependencies);
                    break;
                case TI_PLAIN_SEQUENCE_LARGE:
                    get_indirect_hash_collection_dependencies(type_id.seq_ldefn(), type_dependencies);
                    break;
                case TI_PLAIN_ARRAY_SMALL:
                    get_indirect_hash_collection_dependencies(type_id.array_sdefn(), type_dependencies);
                    break;
                case TI_PLAIN_ARRAY_LARGE:
                    get_indirect_hash_collection_dependencies(type_id.array_ldefn(), type_dependencies);
                    break;
                case TI_PLAIN_MAP_SMALL:
                    get_indirect_hash_map_dependencies(type_id.map_sdefn(), type_dependencies);
                    break;
                case TI_PLAIN_MAP_LARGE:
                    get_indirect_hash_map_dependencies(type_id.map_ldefn(), type_dependencies);
                    break;
                default:
                    return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
            }
        }
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
        type_id_size.typeobject_serialized_size(
            type_registry_entries_.at(
                type_id).type_object_serialized_size);
    }
    type_dependencies.insert(type_id_size);
}

ReturnCode_t TypeObjectRegistry::get_custom_annotations_dependencies(
        const AppliedAnnotationSeq& custom_annotation_seq,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    TypeIdentifierSeq type_ids;
    for (auto ann : custom_annotation_seq)
    {
        TypeIdentifier type_id = ann.annotation_typeid();
        if (TypeObjectUtils::is_direct_hash_type_identifier(type_id))
        {
            add_dependency(type_id, type_dependencies);
            type_ids.push_back(type_id);
        }
        else if (TypeObjectUtils::is_indirect_hash_type_identifier(type_id))
        {
            type_ids.push_back(type_id);
        }
    }
    if (!type_ids.empty())
    {
        return get_type_dependencies(type_ids, type_dependencies);
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

bool TypeObjectRegistry::is_builtin_annotation_name(
        const std::string& name)
{
    if (name == id_annotation_name || name == autoid_annotation_name ||
            name == optional_annotation_name ||
            name == position_annotation_name || name == value_annotation_name ||
            name == extensibility_annotation_name ||
            name == final_annotation_name || name == appendable_annotation_name ||
            name == mutable_annotation_name ||
            name == key_annotation_name || name == must_understand_annotation_name ||
            name == default_literal_annotation_name || name == default_annotation_name ||
            name == range_annotation_name ||
            name == min_annotation_name || name == max_annotation_name || name == unit_annotation_name ||
            name == bit_bound_annotation_name || name == external_annotation_name ||
            name == nested_annotation_name ||
            name == verbatim_annotation_name || name == service_annotation_name ||
            name == oneway_annotation_name ||
            name == ami_annotation_name || name == hashid_annotation_name ||
            name == default_nested_annotation_name ||
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
    switch (complete_type_object._d()){
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
    minimal_alias_type.body().common().related_type(
        minimal_from_complete_type_identifier(
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
    for (const CompleteAnnotationParameter& complete_annotation_parameter :
            complete_annotation_type.member_seq())
    {
        MinimalAnnotationParameter minimal_annotation_parameter;
        minimal_annotation_parameter.common(complete_annotation_parameter.common());
        minimal_annotation_parameter.common().member_type_id(
            minimal_from_complete_type_identifier(
                complete_annotation_parameter.common().member_type_id()));
        minimal_annotation_parameter.name_hash(
            TypeObjectUtils::name_hash(
                complete_annotation_parameter.name().c_str()));
        minimal_annotation_parameter.default_value(complete_annotation_parameter.default_value());
        auto it {minimal_annotation_parameter_sequence.begin()};
        for (; it != minimal_annotation_parameter_sequence.end(); ++it)
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
    minimal_struct_type.header().base_type(
        minimal_from_complete_type_identifier(
            complete_struct_type.header().base_type()));
    // header().detail: empty. Available for future extension.
    MinimalStructMemberSeq minimal_struct_member_sequence;
    for (const CompleteStructMember& complete_struct_member : complete_struct_type.member_seq())
    {
        MinimalStructMember minimal_struct_member;
        minimal_struct_member.common(complete_struct_member.common());
        minimal_struct_member.common().member_type_id(
            minimal_from_complete_type_identifier(
                complete_struct_member.common().member_type_id()));
        minimal_struct_member.detail().name_hash(
            TypeObjectUtils::name_hash(
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
    minimal_union_type.discriminator().common().type_id(
        minimal_from_complete_type_identifier(
            complete_union_type.discriminator().common().type_id()));
    MinimalUnionMemberSeq minimal_union_member_sequence;
    for (const CompleteUnionMember& complete_union_member : complete_union_type.member_seq())
    {
        MinimalUnionMember minimal_union_member;
        minimal_union_member.common(complete_union_member.common());
        minimal_union_member.common().type_id(
            minimal_from_complete_type_identifier(
                minimal_union_member.common().type_id()));
        minimal_union_member.detail().name_hash(
            TypeObjectUtils::name_hash(
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
        minimal_bitfield.name_hash(
            TypeObjectUtils::name_hash(
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
    minimal_sequence_type.element().common().type(
        minimal_from_complete_type_identifier(
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
    minimal_array_type.element().common().type(
        minimal_from_complete_type_identifier(
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
    minimal_map_type.key().common().type(
        minimal_from_complete_type_identifier(
            complete_map_type.key().common().type()));
    minimal_map_type.element().common(complete_map_type.element().common());
    minimal_map_type.element().common().type(
        minimal_from_complete_type_identifier(
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
    for (const CompleteEnumeratedLiteral& complete_enumerated_literal :
            complete_enumerated_type.literal_seq())
    {
        MinimalEnumeratedLiteral minimal_enumerated_literal;
        minimal_enumerated_literal.common(complete_enumerated_literal.common());
        minimal_enumerated_literal.detail().name_hash(
            TypeObjectUtils::name_hash(
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
        minimal_bitflag.detail().name_hash(
            TypeObjectUtils::name_hash(
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
    switch (type_id._d()){
        case EK_COMPLETE:
        {
            return get_complementary_type_identifier(type_id);
        }
        break;
        case TI_PLAIN_SEQUENCE_SMALL:
            if (type_id.seq_sdefn().header().equiv_kind() == EK_COMPLETE)
            {
                TypeIdentifier ret_type_id;
                ret_type_id = type_id;
                ret_type_id.seq_sdefn().header().equiv_kind(EK_MINIMAL);
                ret_type_id.seq_sdefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_id.seq_sdefn().element_identifier())));
                return ret_type_id;
            }
            break;
        case TI_PLAIN_SEQUENCE_LARGE:
            if (type_id.seq_ldefn().header().equiv_kind() == EK_COMPLETE)
            {
                TypeIdentifier ret_type_id;
                ret_type_id = type_id;
                ret_type_id.seq_ldefn().header().equiv_kind(EK_MINIMAL);
                ret_type_id.seq_ldefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_id.seq_ldefn().element_identifier())));
                return ret_type_id;
            }
            break;
        case TI_PLAIN_ARRAY_SMALL:
            if (type_id.array_sdefn().header().equiv_kind() == EK_COMPLETE)
            {
                TypeIdentifier ret_type_id;
                ret_type_id = type_id;
                ret_type_id.array_sdefn().header().equiv_kind(EK_MINIMAL);
                ret_type_id.array_sdefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_id.array_sdefn().element_identifier())));
                return ret_type_id;
            }
            break;
        case TI_PLAIN_ARRAY_LARGE:
            if (type_id.array_ldefn().header().equiv_kind() == EK_COMPLETE)
            {
                TypeIdentifier ret_type_id;
                ret_type_id = type_id;
                ret_type_id.array_ldefn().header().equiv_kind(EK_MINIMAL);
                ret_type_id.array_ldefn().element_identifier(
                    new TypeIdentifier(
                        get_complementary_type_identifier(
                            *type_id.array_ldefn().element_identifier())));
                return ret_type_id;
            }
            break;
        case TI_PLAIN_MAP_SMALL:
            if (type_id.map_sdefn().header().equiv_kind() == EK_COMPLETE ||
                    type_id.map_sdefn().key_identifier()->_d() == EK_COMPLETE)
            {
                TypeIdentifier ret_type_id;
                ret_type_id = type_id;

                if (type_id.map_sdefn().header().equiv_kind() == EK_COMPLETE)
                {
                    ret_type_id.map_sdefn().header().equiv_kind(EK_MINIMAL);
                    ret_type_id.map_sdefn().element_identifier(
                        new TypeIdentifier(
                            get_complementary_type_identifier(
                                *type_id.map_sdefn().element_identifier())));
                }
                if (type_id.map_sdefn().key_identifier()->_d() == EK_COMPLETE)
                {
                    ret_type_id.map_sdefn().key_identifier(
                        new TypeIdentifier(
                            get_complementary_type_identifier(
                                *type_id.map_sdefn().key_identifier())));
                }
                return ret_type_id;
            }
            break;
        case TI_PLAIN_MAP_LARGE:
            if (type_id.map_ldefn().header().equiv_kind() == EK_COMPLETE ||
                    type_id.map_ldefn().key_identifier()->_d() == EK_COMPLETE)
            {
                TypeIdentifier ret_type_id;
                ret_type_id = type_id;

                if (type_id.map_ldefn().header().equiv_kind() == EK_COMPLETE)
                {
                    ret_type_id.map_ldefn().header().equiv_kind(EK_MINIMAL);
                    ret_type_id.map_ldefn().element_identifier(
                        new TypeIdentifier(
                            get_complementary_type_identifier(
                                *type_id.map_ldefn().element_identifier())));
                }
                if (type_id.map_ldefn().key_identifier()->_d() == EK_COMPLETE)
                {
                    ret_type_id.map_ldefn().key_identifier(
                        new TypeIdentifier(
                            get_complementary_type_identifier(
                                *type_id.map_ldefn().key_identifier())));
                }
                return ret_type_id;
            }
            break;
        default:
            break;
    }

    return type_id;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_dynamic_type(
        const DynamicType::_ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {eprosima::fastdds::dds::RETCODE_OK};
    traits<DynamicTypeImpl>::ref_type dynamic_type_impl {traits<DynamicType>::narrow<DynamicTypeImpl>(
                                                             dynamic_type)};
    type_ids.type_identifier1().no_value({});
    type_ids.type_identifier2().no_value({});

    switch (dynamic_type_impl->get_kind()){
        case eprosima::fastdds::dds::TK_ALIAS:
            ret_code = register_typeobject_w_alias_dynamic_type(dynamic_type_impl, type_ids);
            break;
        case eprosima::fastdds::dds::TK_ANNOTATION:
            ret_code = register_typeobject_w_annotation_dynamic_type(dynamic_type_impl, type_ids);
            break;
        case eprosima::fastdds::dds::TK_STRUCTURE:
            ret_code = register_typeobject_w_struct_dynamic_type(dynamic_type_impl, type_ids);
            break;
        case eprosima::fastdds::dds::TK_UNION:
            ret_code = register_typeobject_w_union_dynamic_type(dynamic_type_impl, type_ids);
            break;
        case eprosima::fastdds::dds::TK_BITSET:
            ret_code = register_typeobject_w_bitset_dynamic_type(dynamic_type_impl, type_ids);
            break;
        case eprosima::fastdds::dds::TK_SEQUENCE:
        {
            const TypeDescriptorImpl& type_descriptor {dynamic_type_impl->get_descriptor()};
            if (0 == dynamic_type_impl->get_annotation_count() &&
                    0 == dynamic_type_impl->get_verbatim_text_count() &&
                    0 == type_descriptor.element_type()->get_annotation_count())
            {
                ret_code = typeidentifier_w_sequence_dynamic_type(dynamic_type_impl, type_ids);
            }
            else
            {
                ret_code = register_typeobject_w_sequence_dynamic_type(dynamic_type_impl, type_ids);
            }
            break;
        }
        case eprosima::fastdds::dds::TK_ARRAY:
        {
            const TypeDescriptorImpl& type_descriptor {dynamic_type_impl->get_descriptor()};
            if (0 == dynamic_type_impl->get_annotation_count() &&
                    0 == dynamic_type_impl->get_verbatim_text_count() &&
                    0 == type_descriptor.element_type()->get_annotation_count())
            {
                ret_code = typeidentifier_w_array_dynamic_type(
                    dynamic_type_impl,
                    type_ids.type_identifier1());
            }
            else
            {
                ret_code = register_typeobject_w_array_dynamic_type(dynamic_type_impl, type_ids);
            }
            break;
        }
        case eprosima::fastdds::dds::TK_MAP:
        {
            const TypeDescriptorImpl& type_descriptor {dynamic_type_impl->get_descriptor()};
            if (0 == dynamic_type_impl->get_annotation_count() &&
                    0 == dynamic_type_impl->get_verbatim_text_count() &&
                    0 == type_descriptor.element_type()->get_annotation_count() &&
                    0 == type_descriptor.key_element_type()->get_annotation_count())
            {
                ret_code = typeidentifier_w_map_dynamic_type(
                    dynamic_type_impl,
                    type_ids.type_identifier1());
            }
            else
            {
                ret_code = register_typeobject_w_map_dynamic_type(dynamic_type_impl, type_ids);
            }
            break;
        }
        case eprosima::fastdds::dds::TK_ENUM:
            ret_code = register_typeobject_w_enum_dynamic_type(dynamic_type_impl, type_ids);
            break;
        case eprosima::fastdds::dds::TK_BITMASK:
            ret_code = register_typeobject_w_bitmask_dynamic_type(dynamic_type_impl, type_ids);
            break;
        case eprosima::fastdds::dds::TK_BOOLEAN:
            type_ids.type_identifier1()._d(TK_BOOLEAN);
            break;
        case eprosima::fastdds::dds::TK_BYTE:
            type_ids.type_identifier1()._d(TK_BYTE);
            break;
        case eprosima::fastdds::dds::TK_INT16:
            type_ids.type_identifier1()._d(TK_INT16);
            break;
        case eprosima::fastdds::dds::TK_INT32:
            type_ids.type_identifier1()._d(TK_INT32);
            break;
        case eprosima::fastdds::dds::TK_INT64:
            type_ids.type_identifier1()._d(TK_INT64);
            break;
        case eprosima::fastdds::dds::TK_UINT16:
            type_ids.type_identifier1()._d(TK_UINT16);
            break;
        case eprosima::fastdds::dds::TK_UINT32:
            type_ids.type_identifier1()._d(TK_UINT32);
            break;
        case eprosima::fastdds::dds::TK_UINT64:
            type_ids.type_identifier1()._d(TK_UINT64);
            break;
        case eprosima::fastdds::dds::TK_FLOAT32:
            type_ids.type_identifier1()._d(TK_FLOAT32);
            break;
        case eprosima::fastdds::dds::TK_FLOAT64:
            type_ids.type_identifier1()._d(TK_FLOAT64);
            break;
        case eprosima::fastdds::dds::TK_FLOAT128:
            type_ids.type_identifier1()._d(TK_FLOAT128);
            break;
        case eprosima::fastdds::dds::TK_INT8:
            type_ids.type_identifier1()._d(TK_INT8);
            break;
        case eprosima::fastdds::dds::TK_UINT8:
            type_ids.type_identifier1()._d(TK_UINT8);
            break;
        case eprosima::fastdds::dds::TK_CHAR8:
            type_ids.type_identifier1()._d(TK_CHAR8);
            break;
        case eprosima::fastdds::dds::TK_CHAR16:
            type_ids.type_identifier1()._d(TK_CHAR16);
            break;
        case eprosima::fastdds::dds::TK_STRING8:
            typeidentifier_w_string_dynamic_type(dynamic_type_impl, type_ids.type_identifier1());
            break;
        case eprosima::fastdds::dds::TK_STRING16:
            typeidentifier_w_wstring_dynamic_type(dynamic_type_impl, type_ids.type_identifier1());
            break;
            // DynamicType consistency is ensure by DynamicTypeBuilder.
    }
    assert(RETCODE_OK == ret_code);
    return ret_code;


}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_alias_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    CompleteTypeDetail detail;
    complete_type_detail(dynamic_type, detail);
    CompleteAliasHeader header = TypeObjectUtils::build_complete_alias_header(detail);

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    TypeIdentifierPair alias_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.base_type(), alias_type_ids);

    bool ec {false};
    CommonAliasBody common {TypeObjectUtils::build_common_alias_body(
                                0, TypeObjectUtils::retrieve_complete_type_identifier(
                                    alias_type_ids,
                                    ec))};
    CompleteAliasBody body {TypeObjectUtils::build_complete_alias_body(
                                common,
                                eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                                eprosima::fastcdr::optional<AppliedAnnotationSeq>())};
    CompleteAliasType alias_type = TypeObjectUtils::build_complete_alias_type(0, header, body);
    CompleteTypeObject complete_typeobject;
    complete_typeobject.alias_type(alias_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_annotation_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    CompleteAnnotationHeader header {TypeObjectUtils::build_complete_annotation_header(
                                         dynamic_type->get_name())};

    auto& parameters {dynamic_type->get_all_members_by_index()};
    CompleteAnnotationParameterSeq member_seq;
    for (auto& member : parameters)
    {
        MemberDescriptorImpl& member_descriptor {member->get_descriptor()};
        TypeIdentifierPair parameter_type_ids;
        register_typeobject_w_dynamic_type(member_descriptor.type(), parameter_type_ids);
        bool ec {false};
        CommonAnnotationParameter common {TypeObjectUtils::build_common_annotation_parameter(
                                              0, TypeObjectUtils::retrieve_complete_type_identifier(
                                                  parameter_type_ids,
                                                  ec))};

        AnnotationParameterValue default_value;
        set_annotation_parameter_value(
            member_descriptor.type(), member_descriptor.default_value(),
            default_value);

        CompleteAnnotationParameter param {TypeObjectUtils::build_complete_annotation_parameter(
                                               common,
                                               member_descriptor.name(), default_value)};
        TypeObjectUtils::add_complete_annotation_parameter(member_seq, param);
    }

    CompleteAnnotationType annotation_type {TypeObjectUtils::build_complete_annotation_type(
                                                0, header,
                                                member_seq)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.annotation_type(annotation_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_struct_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    StructTypeFlag struct_flags {TypeObjectUtils::build_struct_type_flag(
                                     extensibility_kind(type_descriptor.extensibility_kind()),
                                     type_descriptor.is_nested(), false)};

    CompleteTypeDetail detail;
    complete_type_detail(dynamic_type, detail);

    TypeIdentifierPair base_type_ids;
    if (type_descriptor.base_type())
    {
        register_typeobject_w_dynamic_type(type_descriptor.base_type(), base_type_ids);
    }

    bool ec {false};
    CompleteStructHeader header {TypeObjectUtils::build_complete_struct_header(
                                     TypeObjectUtils::retrieve_complete_type_identifier(
                                         base_type_ids,
                                         ec), detail)};

    auto& struct_members {dynamic_type->get_all_members_by_index()};
    CompleteStructMemberSeq member_seq;
    uint32_t initial_index {dynamic_type->get_index_own_members()};
    assert(initial_index <= struct_members.size());
    for (auto member {struct_members.begin() + initial_index}; member != struct_members.end();
            ++member)
    {
        MemberDescriptorImpl& member_descriptor {(*member)->get_descriptor()};
        StructMemberFlag member_flags = TypeObjectUtils::build_struct_member_flag(
            try_construct_kind(member_descriptor.try_construct_kind()),
            member_descriptor.is_optional(),
            member_descriptor.is_must_understand(), member_descriptor.is_key(),
            member_descriptor.is_shared());
        TypeIdentifierPair member_type_ids;
        register_typeobject_w_dynamic_type(member_descriptor.type(), member_type_ids);
        CommonStructMember common {TypeObjectUtils::build_common_struct_member(
                                       member_descriptor.id(),
                                       member_flags,
                                       TypeObjectUtils::retrieve_complete_type_identifier(member_type_ids, ec))};

        CompleteMemberDetail member_detail;
        complete_member_detail(*member, member_detail);
        CompleteStructMember struct_member {TypeObjectUtils::build_complete_struct_member(
                                                common,
                                                member_detail)};
        TypeObjectUtils::add_complete_struct_member(member_seq, struct_member);
    }

    CompleteStructType struct_type {TypeObjectUtils::build_complete_struct_type(
                                        struct_flags, header,
                                        member_seq)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.struct_type(struct_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_union_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    UnionTypeFlag union_flags {TypeObjectUtils::build_union_type_flag(
                                   extensibility_kind(type_descriptor.extensibility_kind()),
                                   type_descriptor.is_nested(), false)};

    CompleteTypeDetail detail;
    complete_type_detail(dynamic_type, detail);
    CompleteUnionHeader header {TypeObjectUtils::build_complete_union_header(detail)};

    // Union discriminator is described using a TypeDescriptor and not a MemberDescriptor (!)
    UnionDiscriminatorFlag discriminator_flags {TypeObjectUtils::build_union_discriminator_flag(
                                                    TryConstructFailAction::DISCARD, false)};
    TypeIdentifierPair discriminator_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.discriminator_type(), discriminator_type_ids);
    bool ec {false};
    CommonDiscriminatorMember common_discriminator {TypeObjectUtils::build_common_discriminator_member(
                                                        discriminator_flags, TypeObjectUtils::retrieve_complete_type_identifier(
                                                            discriminator_type_ids,
                                                            ec))};

    eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> ann_builtin;
    apply_verbatim_annotation(type_descriptor.discriminator_type(), ann_builtin);
    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom;
    apply_custom_annotations(
        traits<DynamicType>::narrow<DynamicTypeImpl>(
            type_descriptor.discriminator_type())->get_annotations(), ann_custom);
    CompleteDiscriminatorMember discriminator {TypeObjectUtils::build_complete_discriminator_member(
                                                   common_discriminator, ann_builtin, ann_custom)};

    auto& union_members {dynamic_type->get_all_members_by_index()};
    CompleteUnionMemberSeq member_seq;
    for (auto& member : union_members)
    {
        // Member ID 0 is the discriminator
        if (0 != member->get_id())
        {
            MemberDescriptorImpl& member_descriptor {member->get_descriptor()};
            UnionMemberFlag member_flags {TypeObjectUtils::build_union_member_flag(
                                              try_construct_kind(member_descriptor.try_construct_kind()),
                                              member_descriptor.is_default_label(),
                                              member_descriptor.is_shared())};
            TypeIdentifierPair member_type_ids;
            register_typeobject_w_dynamic_type(member_descriptor.type(), member_type_ids);
            UnionCaseLabelSeq labels;
            for (int32_t label : member_descriptor.label())
            {
                TypeObjectUtils::add_union_case_label(labels, label);
            }
            CommonUnionMember common {TypeObjectUtils::build_common_union_member(
                                          member_descriptor.id(), member_flags,
                                          TypeObjectUtils::retrieve_complete_type_identifier(
                                              member_type_ids,
                                              ec), labels)};

            CompleteMemberDetail member_detail;
            complete_member_detail(member, member_detail);
            CompleteUnionMember union_member {TypeObjectUtils::build_complete_union_member(
                                                  common,
                                                  member_detail)};
            TypeObjectUtils::add_complete_union_member(member_seq, union_member);
        }
    }

    CompleteUnionType union_type {TypeObjectUtils::build_complete_union_type(
                                      union_flags, header, discriminator,
                                      member_seq)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.union_type(union_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_bitset_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    CompleteTypeDetail detail;
    complete_type_detail(dynamic_type, detail);
    CompleteBitsetHeader header {TypeObjectUtils::build_complete_bitset_header(detail)};

    auto& bitfields {dynamic_type->get_all_members_by_index()};
    CompleteBitfieldSeq field_seq;
    for (auto& bitfield : bitfields)
    {
        MemberDescriptorImpl& member_descriptor {bitfield->get_descriptor()};
        CommonBitfield common {TypeObjectUtils::build_common_bitfield(
                                   static_cast<uint16_t>(member_descriptor.id()), 0,
                                   static_cast<uint8_t>(type_descriptor.bound().at(member_descriptor.index())),
                                   type_kind(member_descriptor.type()->get_kind()))};
        CompleteMemberDetail member_detail;
        complete_member_detail(bitfield, member_detail);
        CompleteBitfield bitfield_member {TypeObjectUtils::build_complete_bitfield(
                                              common,
                                              member_detail)};
        TypeObjectUtils::add_complete_bitfield(field_seq, bitfield_member);
    }

    CompleteBitsetType bitset_type {TypeObjectUtils::build_complete_bitset_type(
                                        0, header,
                                        field_seq)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.bitset_type(bitset_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_sequence_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    CommonCollectionHeader common {TypeObjectUtils::build_common_collection_header(
                                       (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ?
                                       0 : type_descriptor.bound().front())};

    eprosima::fastcdr::optional<CompleteTypeDetail> detail;
    complete_type_detail(dynamic_type, detail.value());

    CompleteCollectionHeader header {TypeObjectUtils::build_complete_collection_header(
                                         common,
                                         detail)};

    TypeIdentifierPair element_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.element_type(), element_type_ids);
    // CollectionElementFlags are not applicable (!)
    bool ec {false};
    CommonCollectionElement common_element {TypeObjectUtils::build_common_collection_element(
                                                0, TypeObjectUtils::retrieve_complete_type_identifier(
                                                    element_type_ids,
                                                    ec))};

    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom;
    apply_custom_annotations(
        traits<DynamicType>::narrow<DynamicTypeImpl>(
            type_descriptor.element_type())->get_annotations(), ann_custom);

    CompleteElementDetail detail_element {TypeObjectUtils::build_complete_element_detail(
                                              eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                                              ann_custom)};
    CompleteCollectionElement element {TypeObjectUtils::build_complete_collection_element(
                                           common_element,
                                           detail_element)};
    CompleteSequenceType sequence_type {TypeObjectUtils::build_complete_sequence_type(
                                            0, header,
                                            element)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.sequence_type(sequence_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_array_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    CommonArrayHeader common {TypeObjectUtils::build_common_array_header(type_descriptor.bound())};

    CompleteTypeDetail detail;
    complete_type_detail(dynamic_type, detail);

    CompleteArrayHeader header {TypeObjectUtils::build_complete_array_header(common, detail)};

    TypeIdentifierPair element_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.element_type(), element_type_ids);
    // CollectionElementFlags are not applicable (!)
    bool ec {false};
    CommonCollectionElement common_element {TypeObjectUtils::build_common_collection_element(
                                                0, TypeObjectUtils::retrieve_complete_type_identifier(
                                                    element_type_ids,
                                                    ec))};

    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom;
    apply_custom_annotations(
        traits<DynamicType>::narrow<DynamicTypeImpl>(
            type_descriptor.element_type())->get_annotations(), ann_custom);
    CompleteElementDetail detail_element {TypeObjectUtils::build_complete_element_detail(
                                              eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                                              ann_custom)};
    CompleteCollectionElement element {TypeObjectUtils::build_complete_collection_element(
                                           common_element,
                                           detail_element)};
    CompleteArrayType array_type {TypeObjectUtils::build_complete_array_type(0, header, element)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.array_type(array_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_map_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    CommonCollectionHeader common {TypeObjectUtils::build_common_collection_header(
                                       (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ?
                                       0 : type_descriptor.bound().front())};

    eprosima::fastcdr::optional<CompleteTypeDetail> detail;
    complete_type_detail(dynamic_type, detail.value());

    CompleteCollectionHeader header {TypeObjectUtils::build_complete_collection_header(
                                         common,
                                         detail)};

    TypeIdentifierPair element_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.element_type(), element_type_ids);
    // CollectionElementFlags are not applicable (!)
    bool ec {false};
    CommonCollectionElement common_element {TypeObjectUtils::build_common_collection_element(
                                                0, TypeObjectUtils::retrieve_complete_type_identifier(
                                                    element_type_ids,
                                                    ec))};

    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom;
    apply_custom_annotations(
        traits<DynamicType>::narrow<DynamicTypeImpl>(
            type_descriptor.element_type())->get_annotations(), ann_custom);
    CompleteElementDetail detail_element {TypeObjectUtils::build_complete_element_detail(
                                              eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                                              ann_custom)};
    CompleteCollectionElement element {TypeObjectUtils::build_complete_collection_element(
                                           common_element,
                                           detail_element)};

    TypeIdentifierPair key_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.key_element_type(), key_type_ids);
    CommonCollectionElement common_key {TypeObjectUtils::build_common_collection_element(
                                            0, TypeObjectUtils::retrieve_complete_type_identifier(
                                                key_type_ids,
                                                ec))};
    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_key;
    apply_custom_annotations(
        traits<DynamicType>::narrow<DynamicTypeImpl>(
            type_descriptor.key_element_type())->get_annotations(), ann_custom_key);
    CompleteElementDetail detail_key {TypeObjectUtils::build_complete_element_detail(
                                          eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>(),
                                          ann_custom_key)};
    CompleteCollectionElement key {TypeObjectUtils::build_complete_collection_element(
                                       common_key,
                                       detail_key)};

    CompleteMapType map_type {TypeObjectUtils::build_complete_map_type(0, header, key, element)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.map_type(map_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_enum_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    auto& literals {dynamic_type->get_all_members_by_index()};
    assert(0 < literals.size());
    MemberDescriptorImpl& first_member_descriptor {literals.at(0)->get_descriptor()};
    BitBound bound {32};
    switch (first_member_descriptor.type()->get_kind()){
        case TK_BOOLEAN:
            bound = 1;
            break;
        case TK_INT8:
        case TK_UINT8:
            bound = 8;
            break;
        case TK_INT16:
        case TK_UINT16:
            bound = 16;
            break;
    }

    CommonEnumeratedHeader common {TypeObjectUtils::build_common_enumerated_header(bound)};
    CompleteTypeDetail detail;
    complete_type_detail(dynamic_type, detail);
    CompleteEnumeratedHeader header {TypeObjectUtils::build_complete_enumerated_header(
                                         common,
                                         detail)};

    // Enum members cannot be accessed using get_all_members because the Member Id does not apply to enum literals.
    CompleteEnumeratedLiteralSeq literal_seq;
    for (auto& literal : literals)
    {
        MemberDescriptorImpl& member_descriptor {literal->get_descriptor()};
        EnumeratedLiteralFlag flags {TypeObjectUtils::build_enumerated_literal_flag(
                                         member_descriptor.is_default_label())};
        // Literal value might be automatically assigned or taken from default_value (@value annotation)
        CommonEnumeratedLiteral common_literal {TypeObjectUtils::build_common_enumerated_literal(
                                                    member_descriptor.default_value().empty() ? member_descriptor.index() :
                                                    std::stol(member_descriptor.default_value()), flags)};
        CompleteMemberDetail member_detail;
        complete_member_detail(literal, member_detail);
        CompleteEnumeratedLiteral literal_member {TypeObjectUtils::build_complete_enumerated_literal(
                                                      common_literal,
                                                      member_detail)};
        TypeObjectUtils::add_complete_enumerated_literal(literal_seq, literal_member);
    }

    CompleteEnumeratedType enumerated_type {TypeObjectUtils::build_complete_enumerated_type(
                                                0, header,
                                                literal_seq)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.enumerated_type(enumerated_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_bitmask_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    CommonEnumeratedHeader common {TypeObjectUtils::build_common_enumerated_header(
                                       static_cast<BitBound>(type_descriptor.bound().front()), true)};
    CompleteTypeDetail detail;
    complete_type_detail(dynamic_type, detail);
    CompleteEnumeratedHeader header {TypeObjectUtils::build_complete_enumerated_header(
                                         common, detail,
                                         true)};

    auto& bitflags {dynamic_type->get_all_members_by_index()};
    CompleteBitflagSeq flag_seq;
    for (auto& bitflag : bitflags)
    {
        MemberDescriptorImpl& member_descriptor {bitflag->get_descriptor()};
        CommonBitflag common_bitflag {TypeObjectUtils::build_common_bitflag(
                                          static_cast<uint16_t>(member_descriptor.id()), 0)};
        CompleteMemberDetail member_detail;
        complete_member_detail(bitflag, member_detail);
        CompleteBitflag bitflag_member {TypeObjectUtils::build_complete_bitflag(
                                            common_bitflag,
                                            member_detail)};
        TypeObjectUtils::add_complete_bitflag(flag_seq, bitflag_member);
    }
    CompleteBitmaskType bitmask_type {TypeObjectUtils::build_complete_bitmask_type(
                                          0, header,
                                          flag_seq)};
    CompleteTypeObject complete_typeobject;
    complete_typeobject.bitmask_type(bitmask_type);
    TypeObject typeobject;
    typeobject.complete(complete_typeobject);
    ret_code = register_type_object(typeobject, type_ids, true);
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::typeidentifier_w_sequence_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    TypeIdentifierPair element_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.element_type(), element_type_ids);

    bool ec {false};
    const TypeIdentifier& element_type_id {
        TK_NONE == element_type_ids.type_identifier2()._d() ?
        element_type_ids.type_identifier1() :
        TypeObjectUtils::retrieve_complete_type_identifier(element_type_ids, ec)};
    EquivalenceKind equiv_kind {equivalence_kind(element_type_id)};
    assert(
        (TK_NONE == element_type_ids.type_identifier2()._d() && EK_BOTH == equiv_kind) ||
        EK_COMPLETE == equiv_kind);

    // CollectionElementFlags cannot be applied because element_type is a DynamicType and the applicable annotations are
    // contained in MemberDescriptor (accessible through DynamicTypeMember). XTypes inconsistency (!)
    PlainCollectionHeader header {TypeObjectUtils::build_plain_collection_header(equiv_kind, 0)};

    eprosima::fastcdr::external<TypeIdentifier> external_element_type_id =
            eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(element_type_id));
    if ((static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ||
            256 > type_descriptor.bound().front())
    {
        SBound bound = (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ?
                0 : static_cast<SBound>(type_descriptor.bound().front());
        PlainSequenceSElemDefn seq_defn {TypeObjectUtils::build_plain_sequence_s_elem_defn(
                                             header, bound,
                                             external_element_type_id)};
        type_ids.type_identifier1().seq_sdefn(seq_defn);
    }
    else
    {
        PlainSequenceLElemDefn seq_defn {TypeObjectUtils::build_plain_sequence_l_elem_defn(
                                             header,
                                             type_descriptor.bound().front(), external_element_type_id)};
        type_ids.type_identifier1().seq_ldefn(seq_defn);
    }

    if (EK_BOTH != equiv_kind)
    {
        const TypeIdentifier& element_type_id_minimal {TypeObjectUtils::
                                                               retrieve_minimal_type_identifier(
                                                           element_type_ids, ec)};
        EquivalenceKind equiv_kind_minimal {equivalence_kind(element_type_id_minimal)};

        // CollectionElementFlags cannot be applied because element_type is a DynamicType and the applicable annotations are
        // contained in MemberDescriptor (accessible through DynamicTypeMember). XTypes inconsistency (!)
        PlainCollectionHeader header_minimal {TypeObjectUtils::build_plain_collection_header(
                                                  equiv_kind_minimal, 0)};

        eprosima::fastcdr::external<TypeIdentifier> external_element_type_id_minimal =
                eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(element_type_id_minimal));
        if ((static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ||
                256 > type_descriptor.bound().front())
        {
            SBound bound = (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ?
                    0 : static_cast<SBound>(type_descriptor.bound().front());
            PlainSequenceSElemDefn seq_defn {TypeObjectUtils::build_plain_sequence_s_elem_defn(
                                                 header_minimal, bound,
                                                 external_element_type_id_minimal)};
            type_ids.type_identifier2().seq_sdefn(seq_defn);
        }
        else
        {
            PlainSequenceLElemDefn seq_defn {TypeObjectUtils::build_plain_sequence_l_elem_defn(
                                                 header_minimal,
                                                 type_descriptor.bound().front(),
                                                 external_element_type_id_minimal)};
            type_ids.type_identifier2().seq_ldefn(seq_defn);
        }
    }
    else
    {
        ExtendedTypeDefn reset_type_id;
        type_ids.type_identifier2().no_value({});
    }

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::typeidentifier_w_array_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifier& type_id)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    TypeIdentifierPair element_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.element_type(), element_type_ids);

    bool ec {false};
    const TypeIdentifier& element_type_id {TypeObjectUtils::retrieve_complete_type_identifier(
                                               element_type_ids, ec)};
    EquivalenceKind equiv_kind {equivalence_kind(element_type_id)};
    PlainCollectionHeader header {TypeObjectUtils::build_plain_collection_header(equiv_kind, 0)};

    eprosima::fastcdr::external<TypeIdentifier> external_element_type_id =
            eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(element_type_id));
    bool is_large_array {false};
    for (auto bound : type_descriptor.bound())
    {
        if (bound > 255)
        {
            is_large_array = true;
            break;
        }
    }
    if (!is_large_array)
    {
        SBoundSeq bounds;
        for (auto bound : type_descriptor.bound())
        {
            bounds.push_back(static_cast<SBound>(bound));
        }
        PlainArraySElemDefn array_defn {TypeObjectUtils::build_plain_array_s_elem_defn(
                                            header,
                                            bounds, external_element_type_id)};
        type_id.array_sdefn(array_defn);
    }
    else
    {
        PlainArrayLElemDefn array_defn {TypeObjectUtils::build_plain_array_l_elem_defn(
                                            header,
                                            type_descriptor.bound(), external_element_type_id)};
        type_id.array_ldefn(array_defn);
    }

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::typeidentifier_w_map_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifier& type_id)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    TypeIdentifierPair element_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.element_type(), element_type_ids);
    TypeIdentifierPair key_type_ids;
    register_typeobject_w_dynamic_type(type_descriptor.key_element_type(), key_type_ids);

    bool ec {false};
    const TypeIdentifier& element_type_id {TypeObjectUtils::retrieve_complete_type_identifier(
                                               element_type_ids, ec)};
    const TypeIdentifier& key_type_id {TypeObjectUtils::retrieve_complete_type_identifier(
                                           key_type_ids, ec)};
    EquivalenceKind equiv_kind {equivalence_kind(element_type_id, key_type_id)};
    PlainCollectionHeader header {TypeObjectUtils::build_plain_collection_header(equiv_kind, 0)};
    eprosima::fastcdr::external<TypeIdentifier> external_element_type_id =
            eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(element_type_id));
    eprosima::fastcdr::external<TypeIdentifier> external_key_type_id =
            eprosima::fastcdr::external<TypeIdentifier>(new TypeIdentifier(key_type_id));
    if ((static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ||
            256 > type_descriptor.bound().front())
    {
        SBound bound = (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ?
                0 : static_cast<SBound>(type_descriptor.bound().front());
        PlainMapSTypeDefn map_defn {TypeObjectUtils::build_plain_map_s_type_defn(
                                        header, bound,
                                        external_element_type_id, 0, external_key_type_id)};
        type_id.map_sdefn(map_defn);
    }
    else
    {
        PlainMapLTypeDefn map_defn {TypeObjectUtils::build_plain_map_l_type_defn(
                                        header,
                                        type_descriptor.bound().front(), external_element_type_id, 0,
                                        external_key_type_id)};
        type_id.map_ldefn(map_defn);
    }

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::typeidentifier_w_string_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifier& type_id)
{
    ReturnCode_t ret_code {RETCODE_OK};

    const TypeDescriptorImpl& type_descriptor {dynamic_type->get_descriptor()};

    if ((static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ||
            256 > type_descriptor.bound().front())
    {
        SBound bound = (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_descriptor.bound().front()) ?
                0 : static_cast<SBound>(type_descriptor.bound().front());
        StringSTypeDefn string_defn {TypeObjectUtils::build_string_s_type_defn(bound)};
        type_id.string_sdefn(string_defn);
    }
    else
    {
        StringLTypeDefn string_defn {TypeObjectUtils::build_string_l_type_defn(
                                         type_descriptor.bound().front())};
        type_id.string_ldefn(string_defn);
    }

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::typeidentifier_w_wstring_dynamic_type(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        TypeIdentifier& type_id)
{
    ReturnCode_t ret_code {typeidentifier_w_string_dynamic_type(dynamic_type, type_id)};
    switch (type_id._d()){
        case TI_STRING8_SMALL:
            type_id._d(TI_STRING16_SMALL);
            break;
        case TI_STRING8_LARGE:
            type_id._d(TI_STRING16_LARGE);
            break;
    }
    assert(RETCODE_OK == ret_code);

    return ret_code;
}

ReturnCode_t TypeObjectRegistry::apply_custom_annotations(
        const std::vector<AnnotationDescriptorImpl>& annotations,
        eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom)
{
    ReturnCode_t ret_code {RETCODE_OK};
    AppliedAnnotationSeq tmp_ann_custom;
    for (auto& annotation_descriptor : annotations)
    {
        TypeIdentifierPair annotation_typeids;
        register_typeobject_w_annotation_dynamic_type(
            traits<DynamicType>::narrow<DynamicTypeImpl>(
                annotation_descriptor.type()), annotation_typeids);

        Parameters parameter_seq;
        annotation_descriptor.get_all_value(parameter_seq); // Always returns RETCODE_OK

        eprosima::fastcdr::optional<AppliedAnnotationParameterSeq> param_seq;
        AppliedAnnotationParameterSeq tmp_param_seq;
        for (auto param = parameter_seq.begin(); param != parameter_seq.end(); ++param)
        {
            NameHash paramname_hash {TypeObjectUtils::name_hash(param->first.to_string())};

            AnnotationParameterValue param_value;
            DynamicTypeMember::_ref_type param_member;
            // DynamicTypeBuilder::apply_annotation checks annotation consistency.
            ret_code = annotation_descriptor.type()->get_member_by_name(param_member, param->first);
            assert(RETCODE_OK == ret_code);
            MemberDescriptor::_ref_type param_descriptor {traits<MemberDescriptor>::make_shared()};
            param_member->get_descriptor(param_descriptor);
            set_annotation_parameter_value(
                param_descriptor->type(), param->second.to_string(),
                param_value);
            AppliedAnnotationParameter parameter {TypeObjectUtils::build_applied_annotation_parameter(
                                                      paramname_hash, param_value)};
            TypeObjectUtils::add_applied_annotation_parameter(tmp_param_seq, parameter);
        }
        if (!tmp_param_seq.empty())
        {
            param_seq = tmp_param_seq;
        }
        bool ec {false};
        AppliedAnnotation applied_annotation {TypeObjectUtils::build_applied_annotation(
                                                  TypeObjectUtils::retrieve_complete_type_identifier(
                                                      annotation_typeids,
                                                      ec),
                                                  param_seq)};
        TypeObjectUtils::add_applied_annotation(tmp_ann_custom, applied_annotation);
    }
    if (!tmp_ann_custom.empty())
    {
        ann_custom = tmp_ann_custom;
    }
    return ret_code;
}

ReturnCode_t TypeObjectRegistry::apply_verbatim_annotation(
        const DynamicType::_ref_type& dynamic_type,
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& ann_builtin)
{
    if (0 != dynamic_type->get_verbatim_text_count())
    {
        VerbatimTextDescriptor::_ref_type verbatim_descriptor {traits<VerbatimTextDescriptor>::
                                                               make_shared()};
        dynamic_type->get_verbatim_text(verbatim_descriptor, 0);

        // TypeObject only allows defining one @verbatim comment
        AppliedVerbatimAnnotation verbatim {TypeObjectUtils::build_applied_verbatim_annotation(
                                                placement_kind(
                                                    verbatim_descriptor->placement()), "c++",
                                                verbatim_descriptor->text())};
        ann_builtin = TypeObjectUtils::build_applied_builtin_type_annotations(
            verbatim);
    }
    return RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::set_annotation_parameter_value(
        const DynamicType::_ref_type& dynamic_type,
        const std::string& value,
        AnnotationParameterValue& param_value)
{
    switch (dynamic_type->get_kind()){
        case TK_BOOLEAN:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<bool>(!value.empty() ?
                TypeValueConverter::sto(value) : false));
            break;
        case TK_BYTE:
            param_value = TypeObjectUtils::build_annotation_parameter_value_byte(
                static_cast<uint8_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_INT8:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<int8_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_UINT8:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<uint8_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_INT16:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<int16_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_UINT16:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<uint16_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_INT32:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<int32_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_UINT32:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<uint32_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_INT64:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<int64_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_UINT64:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<uint64_t>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_FLOAT32:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<float>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_FLOAT64:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<double>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_FLOAT128:
            param_value = TypeObjectUtils::build_annotation_parameter_value(
                static_cast<long double>(!value.empty() ?
                TypeValueConverter::sto(value) : 0));
            break;
        case TK_CHAR8:
            param_value = TypeObjectUtils::build_annotation_parameter_value(!value.empty() ? value : 0);
            break;
        case TK_ENUM:
        {
            DynamicTypeMember::_ref_type member;
            dynamic_type->get_member_by_index(member, 0);
            param_value = TypeObjectUtils::build_annotation_parameter_value_enum(
                static_cast<int32_t>(!value.empty() ?
                std::stol(value) : member->get_id()));
            break;
        }
        case TK_STRING8:
            param_value = TypeObjectUtils::build_annotation_parameter_value(!value.empty() ? value : "");
            break;
            // Wide char and wide string annotation parameter values not yet supported.
    }
    return RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::complete_type_detail(
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        CompleteTypeDetail& detail)
{
    eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> ann_builtin;
    apply_verbatim_annotation(dynamic_type, ann_builtin);

    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom;
    apply_custom_annotations(dynamic_type->get_annotations(), ann_custom);

    detail = TypeObjectUtils::build_complete_type_detail(
        ann_builtin, ann_custom,
        dynamic_type->get_name());

    return RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::complete_member_detail(
        const traits<DynamicTypeMemberImpl>::ref_type& member,
        CompleteMemberDetail& member_detail)
{
    // @unit, @max, @min, @range & @hashid builtin annotations are not applied with dynamic language binding
    eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin; // Empty

    eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom;
    apply_custom_annotations(member->get_annotations(), ann_custom);

    member_detail = TypeObjectUtils::build_complete_member_detail(
        member->get_descriptor().name(),
        member_ann_builtin, ann_custom);
    return RETCODE_OK;
}

ExtensibilityKind TypeObjectRegistry::extensibility_kind(
        eprosima::fastdds::dds::ExtensibilityKind extensibility_kind) const
{
    ExtensibilityKind ret_extensibility_kind {ExtensibilityKind::APPENDABLE};
    switch (extensibility_kind){
        case eprosima::fastdds::dds::ExtensibilityKind::FINAL:
            ret_extensibility_kind = ExtensibilityKind::FINAL;
            break;
        case eprosima::fastdds::dds::ExtensibilityKind::APPENDABLE:
            ret_extensibility_kind = ExtensibilityKind::APPENDABLE;
            break;
        case eprosima::fastdds::dds::ExtensibilityKind::MUTABLE:
            ret_extensibility_kind = ExtensibilityKind::MUTABLE;
            break;
    }
    return ret_extensibility_kind;
}

TryConstructFailAction TypeObjectRegistry::try_construct_kind(
        eprosima::fastdds::dds::TryConstructKind try_construct_kind) const
{
    TryConstructFailAction ret_try_construct_kind {TryConstructFailAction::DISCARD};
    switch (try_construct_kind){
        case eprosima::fastdds::dds::TryConstructKind::DISCARD:
            ret_try_construct_kind = TryConstructFailAction::DISCARD;
            break;
        case eprosima::fastdds::dds::TryConstructKind::TRIM:
            ret_try_construct_kind = TryConstructFailAction::TRIM;
            break;
        case eprosima::fastdds::dds::TryConstructKind::USE_DEFAULT:
            ret_try_construct_kind = TryConstructFailAction::USE_DEFAULT;
            break;
    }
    return ret_try_construct_kind;
}

TypeKind TypeObjectRegistry::type_kind(
        eprosima::fastdds::dds::TypeKind type_kind) const
{
    TypeKind ret_type_kind {TK_NONE};
    switch (type_kind){
        case eprosima::fastdds::dds::TK_NONE:
            ret_type_kind = TK_NONE;
            break;
        case eprosima::fastdds::dds::TK_BOOLEAN:
            ret_type_kind = TK_BOOLEAN;
            break;
        case eprosima::fastdds::dds::TK_BYTE:
            ret_type_kind = TK_BYTE;
            break;
        case eprosima::fastdds::dds::TK_INT16:
            ret_type_kind = TK_INT16;
            break;
        case eprosima::fastdds::dds::TK_INT32:
            ret_type_kind = TK_INT32;
            break;
        case eprosima::fastdds::dds::TK_INT64:
            ret_type_kind = TK_INT64;
            break;
        case eprosima::fastdds::dds::TK_UINT16:
            ret_type_kind = TK_UINT16;
            break;
        case eprosima::fastdds::dds::TK_UINT32:
            ret_type_kind = TK_UINT32;
            break;
        case eprosima::fastdds::dds::TK_UINT64:
            ret_type_kind = TK_UINT64;
            break;
        case eprosima::fastdds::dds::TK_FLOAT32:
            ret_type_kind = TK_FLOAT32;
            break;
        case eprosima::fastdds::dds::TK_FLOAT64:
            ret_type_kind = TK_FLOAT64;
            break;
        case eprosima::fastdds::dds::TK_FLOAT128:
            ret_type_kind = TK_FLOAT128;
            break;
        case eprosima::fastdds::dds::TK_INT8:
            ret_type_kind = TK_INT8;
            break;
        case eprosima::fastdds::dds::TK_UINT8:
            ret_type_kind = TK_UINT8;
            break;
        case eprosima::fastdds::dds::TK_CHAR8:
            ret_type_kind = TK_CHAR8;
            break;
        case eprosima::fastdds::dds::TK_CHAR16:
            ret_type_kind = TK_CHAR16;
            break;
        case eprosima::fastdds::dds::TK_STRING8:
            ret_type_kind = TK_STRING8;
            break;
        case eprosima::fastdds::dds::TK_STRING16:
            ret_type_kind = TK_STRING16;
            break;
        case eprosima::fastdds::dds::TK_ALIAS:
            ret_type_kind = TK_ALIAS;
            break;
        case eprosima::fastdds::dds::TK_ENUM:
            ret_type_kind = TK_ENUM;
            break;
        case eprosima::fastdds::dds::TK_BITMASK:
            ret_type_kind = TK_BITMASK;
            break;
        case eprosima::fastdds::dds::TK_ANNOTATION:
            ret_type_kind = TK_ANNOTATION;
            break;
        case eprosima::fastdds::dds::TK_STRUCTURE:
            ret_type_kind = TK_STRUCTURE;
            break;
        case eprosima::fastdds::dds::TK_UNION:
            ret_type_kind = TK_UNION;
            break;
        case eprosima::fastdds::dds::TK_BITSET:
            ret_type_kind = TK_BITSET;
            break;
        case eprosima::fastdds::dds::TK_SEQUENCE:
            ret_type_kind = TK_SEQUENCE;
            break;
        case eprosima::fastdds::dds::TK_ARRAY:
            ret_type_kind = TK_ARRAY;
            break;
        case eprosima::fastdds::dds::TK_MAP:
            ret_type_kind = TK_MAP;
            break;
    }
    return ret_type_kind;
}

EquivalenceKind TypeObjectRegistry::equivalence_kind(
        const TypeIdentifier& element_type_id)
{
    EquivalenceKind equiv_kind {EK_BOTH};
    switch (element_type_id._d()){
        case TI_PLAIN_SEQUENCE_SMALL:
            equiv_kind = element_type_id.seq_sdefn().header().equiv_kind();
            break;
        case TI_PLAIN_SEQUENCE_LARGE:
            equiv_kind = element_type_id.seq_ldefn().header().equiv_kind();
            break;
        case TI_PLAIN_ARRAY_SMALL:
            equiv_kind = element_type_id.array_sdefn().header().equiv_kind();
            break;
        case TI_PLAIN_ARRAY_LARGE:
            equiv_kind = element_type_id.array_ldefn().header().equiv_kind();
            break;
        case TI_PLAIN_MAP_SMALL:
            equiv_kind = element_type_id.map_sdefn().header().equiv_kind();
            break;
        case TI_PLAIN_MAP_LARGE:
            equiv_kind = element_type_id.map_ldefn().header().equiv_kind();
            break;
        case EK_MINIMAL:
            equiv_kind = EK_MINIMAL;
            break;
        case EK_COMPLETE:
            equiv_kind = EK_COMPLETE;
            break;
    }
    return equiv_kind;
}

EquivalenceKind TypeObjectRegistry::equivalence_kind(
        const TypeIdentifier& element_type_id,
        const TypeIdentifier& key_type_id)
{
    EquivalenceKind equiv_kind {equivalence_kind(element_type_id)};
    if (EK_BOTH == equiv_kind)
    {
        equiv_kind = equivalence_kind(key_type_id);
    }
    return equiv_kind;
}

PlacementKind TypeObjectRegistry::placement_kind(
        const std::string& placement_kind) const
{
    PlacementKind ret_placement_kind {PlacementKind::BEFORE_DECLARATION};

    std::string lower_case_placement_kind;
    // XTypes v1.3 Section 7.2.2.4.8.2 [The placement] shall be interpreted in a case-insensitive manner.
    std::transform(
        placement_kind.begin(), placement_kind.end(), lower_case_placement_kind.begin(),
        [](char c)
        {
            return static_cast<char>(std::tolower(c));
        });
    if (lower_case_placement_kind == begin_declaration_file_str)
    {
        ret_placement_kind = PlacementKind::BEGIN_FILE;
    }
    else if (lower_case_placement_kind == begin_declaration_str)
    {
        ret_placement_kind = PlacementKind::BEGIN_DECLARATION;
    }
    else if (lower_case_placement_kind == end_declaration_str)
    {
        ret_placement_kind = PlacementKind::END_DECLARATION;
    }
    else if (lower_case_placement_kind == after_declaration_str)
    {
        ret_placement_kind = PlacementKind::AFTER_DECLARATION;
    }
    else if (lower_case_placement_kind == end_declaration_file_str)
    {
        ret_placement_kind = PlacementKind::END_FILE;
    }
    else if (lower_case_placement_kind != before_declaration_str)
    {
        EPROSIMA_LOG_WARNING(
            XTYPES_TYPE_REPRESENTATION,
            "Verbatim placement kind not recognized: using BEFORE_DECLARATION (default value)");
    }
    return ret_placement_kind;
}

} // xtypes
} // dds
} // fastdds
} // eprosima
