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

#include <fastdds/dds/xtypes/type_representation/TypeObjectRegistry.hpp>

#include <string>

#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

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
    type_ids.type_identifier1(get_type_identifier(minimal_entry.type_object_,
        minimal_entry.type_object_serialized_size_));
    type_ids.type_identifier2(get_type_identifier(complete_entry.type_object_,
        complete_entry.type_object_serialized_size_));
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
        const TypeObjectPair& type_objects)
{
    static_cast<void>(type_name);
    static_cast<void>(type_objects);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_identifiers(
        const std::string& type_name,
        const TypeIdentifierPair& type_identifiers)
{
    static_cast<void>(type_name);
    static_cast<void>(type_identifiers);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const TypeIdentifier& type_identifier,
        const TypeObject& type_object)
{
    static_cast<void>(type_identifier);
    static_cast<void>(type_object);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_object(
        const TypeIdentifier& type_identifier,
        TypeObjectPair& type_objects)
{
    static_cast<void>(type_identifier);
    static_cast<void>(type_objects);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_information(
        const std::string& type_name,
        TypeInformation& type_information)
{
    static_cast<void>(type_name);
    static_cast<void>(type_information);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::are_types_compatible(
        const TypeIdentifierPair& type_identifiers,
        const TypeConsistencyEnforcementQosPolicy& type_consistency_qos)
{
    static_cast<void>(type_identifiers);
    static_cast<void>(type_consistency_qos);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_dependencies(
        const TypeIdentifierSeq& type_identifiers,
        std::unordered_set<TypeIdentfierWithSize> type_dependencies)
{
    static_cast<void>(type_identifiers);
    static_cast<void>(type_dependencies);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::is_type_identifier_known(
        const TypeIdentifier& type_identifier)
{
    static_cast<void>(type_identifier);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

bool TypeObjectRegistry::is_builtin_annotation(
        const TypeIdentifier& type_identifier)
{
    static_cast<void>(type_identifier);
    return false;
}

const TypeIdentifier TypeObjectRegistry::get_type_identifier(
        const TypeObject& type_object,
        uint32_t& type_object_serialized_size)
{
    static_cast<void>(type_object);
    static_cast<void>(type_object_serialized_size);
    return TypeIdentifier();
}

const TypeObject TypeObjectRegistry::build_minimal_from_complete_type_object(
        const CompleteTypeObject& complete_type_object)
{
    static_cast<void>(complete_type_object);
    return TypeObject();
}

} // xtypes
} // dds
} // fastdds
} // eprosima
