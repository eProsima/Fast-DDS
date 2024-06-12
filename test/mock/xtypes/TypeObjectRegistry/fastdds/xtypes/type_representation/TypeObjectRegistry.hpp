// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file
 * This file contains the required classes to keep a TypeObject/TypeIdentifier registry.
 */

#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_

#include <unordered_set>

#include <gmock/gmock.h>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

namespace std {
template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentifier>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentifier& k) const
    {
        // The collection only has direct hash TypeIdentifiers so the EquivalenceHash can be used.
        return (static_cast<size_t>(k.equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.equivalence_hash()[2]));
    }

};

} // std

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class TypeObjectRegistry : public ITypeObjectRegistry
{
public:

    MOCK_METHOD(const TypeIdentifier, get_complementary_type_identifier, (
                const TypeIdentifier& /*type_id*/));

    MOCK_METHOD(ReturnCode_t, get_type_dependencies, (
                const TypeIdentifierSeq& /*type_identifiers*/,
                std::unordered_set<TypeIdentfierWithSize>& /*type_dependencies*/));

    MOCK_METHOD(ReturnCode_t, get_type_identifiers, (
                const std::string& /*type_name*/,
                TypeIdentifierPair & /*type_identifiers*/), (override));

    MOCK_METHOD(ReturnCode_t, get_type_object, (
                const TypeIdentifier& /*type_identifier*/,
                TypeObject & /*type_object*/), (override));

    MOCK_METHOD(ReturnCode_t, get_type_objects, (
                const std::string& /*type_name*/,
                TypeObjectPair & /*type_objects*/), (override));

    MOCK_METHOD(ReturnCode_t, register_type_identifier, (
                const std::string& /*type_name*/,
                TypeIdentifierPair & /*type_identifier*/), (override));

    MOCK_METHOD(ReturnCode_t, register_type_object, (
                const std::string& /*type_name*/,
                const CompleteTypeObject& /*complete_type_object*/,
                TypeIdentifierPair & /*type_ids*/), (override));

    MOCK_METHOD(ReturnCode_t, register_type_object, (
                const TypeObject& /*type_object*/,
                TypeIdentifierPair & /*type_identifier*/), (override));

    MOCK_METHOD(ReturnCode_t, register_type_object, (
                const TypeObject& /*type_object*/,
                TypeIdentifierPair& /*type_identifier*/,
                bool /*build_minimal*/));

    MOCK_METHOD(ReturnCode_t, register_typeobject_w_dynamic_type, (
                const DynamicType::_ref_type& /*dynamic_type*/,
                TypeIdentifierPair & /*type_ids*/), (override));

    MOCK_METHOD(bool, is_type_identifier_known, (
                const TypeIdentfierWithSize& /*type_identifier_with_size*/));

    MOCK_METHOD(ReturnCode_t, get_type_information, (
                const TypeIdentifierPair& /*type_ids*/,
                TypeInformation& /*type_information*/,
                bool /*with_dependencies*/), (override));

};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
