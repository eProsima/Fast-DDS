// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file TypeObjectRegistry.cpp
 *
 * Mock implementation of TypeObjectRegistry, compiled against the real header. Nothing is registered: the public
 * methods forward to the TypeObjectRegistryMock companion of each instance.
 */

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>
#include <mock/fastdds/xtypes/type_representation/TypeObjectRegistryMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

namespace {

TypeObjectRegistryMock& companion(
        const TypeObjectRegistry* registry)
{
    return mock::MockCompanion<TypeObjectRegistry>::get(registry);
}

} // namespace

bool TypeRegistryEntry::operator !=(
        const TypeRegistryEntry& entry)
{
    return this->type_object != entry.type_object ||
           this->type_object_serialized_size != entry.type_object_serialized_size ||
           this->complementary_type_id != entry.complementary_type_id;
}

TypeObjectRegistry::TypeObjectRegistry()
{
    mock::MockCompanion<TypeObjectRegistry>::attach(this);
    // Never detached, see TypeObjectRegistryMock.
    testing::Mock::AllowLeak(&companion(this));
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const std::string& type_name,
        const CompleteTypeObject& complete_type_object,
        TypeIdentifierPair& type_ids)
{
    return companion(this).register_type_object(type_name, complete_type_object, type_ids);
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const TypeObject& type_object,
        TypeIdentifierPair& type_ids,
        bool build_minimal)
{
    return companion(this).register_type_object(type_object, type_ids, build_minimal);
}

ReturnCode_t TypeObjectRegistry::register_type_identifier(
        const std::string& type_name,
        TypeIdentifierPair& type_identifier)
{
    return companion(this).register_type_identifier(type_name, type_identifier);
}

ReturnCode_t TypeObjectRegistry::get_type_objects(
        const std::string& type_name,
        TypeObjectPair& type_objects)
{
    return companion(this).get_type_objects(type_name, type_objects);
}

ReturnCode_t TypeObjectRegistry::get_type_identifiers(
        const std::string& type_name,
        TypeIdentifierPair& type_identifiers)
{
    return companion(this).get_type_identifiers(type_name, type_identifiers);
}

ReturnCode_t TypeObjectRegistry::get_type_object(
        const TypeIdentifier& type_identifier,
        TypeObject& type_object)
{
    return companion(this).get_type_object(type_identifier, type_object);
}

ReturnCode_t TypeObjectRegistry::get_complete_type_object(
        const TypeIdentifierPair& type_identifiers,
        CompleteTypeObject& type_object)
{
    return companion(this).get_complete_type_object(type_identifiers, type_object);
}

ReturnCode_t TypeObjectRegistry::get_type_information(
        const TypeIdentifierPair& type_ids,
        TypeInformation& type_information,
        bool with_dependencies)
{
    return companion(this).get_type_information(type_ids, type_information, with_dependencies);
}

ReturnCode_t TypeObjectRegistry::get_type_dependencies(
        const TypeIdentifierSeq& type_identifiers,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    return companion(this).get_type_dependencies(type_identifiers, type_dependencies);
}

bool TypeObjectRegistry::is_type_identifier_known(
        const TypeIdentfierWithSize& type_identifier_with_size)
{
    return companion(this).is_type_identifier_known(type_identifier_with_size);
}

bool TypeObjectRegistry::is_builtin_annotation(
        const TypeIdentifier&)
{
    return false;
}

const TypeIdentifier TypeObjectRegistry::calculate_type_identifier(
        const TypeObject&,
        uint32_t&)
{
    return {};
}

ReturnCode_t TypeObjectRegistry::register_typeobject_w_dynamic_type(
        const DynamicType::_ref_type& dynamic_type,
        TypeIdentifierPair& type_ids)
{
    return companion(this).register_typeobject_w_dynamic_type(dynamic_type, type_ids);
}

ReturnCode_t TypeObjectRegistry::are_types_compatible(
        const TypeIdentifierPair& type_identifiers,
        const TypeConsistencyEnforcementQosPolicy& type_consistency_qos)
{
    return companion(this).are_types_compatible(type_identifiers, type_consistency_qos);
}

ReturnCode_t TypeObjectRegistry::get_dependencies_from_type_object(
        const TypeObject& type_object,
        std::unordered_set<TypeIdentfierWithSize>& type_dependencies)
{
    return companion(this).get_dependencies_from_type_object(type_object, type_dependencies);
}

const TypeIdentifier TypeObjectRegistry::get_complementary_type_identifier(
        const TypeIdentifier& type_id)
{
    return companion(this).get_complementary_type_identifier(type_id);
}

bool TypeObjectRegistry::is_builtin_annotation_name(
        const std::string&)
{
    return false;
}

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima
