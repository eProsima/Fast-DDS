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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTREGISTRYMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTREGISTRYMOCK_HPP

#include <string>
#include <unordered_set>

#include <gmock/gmock.h>

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>
#include <mock/MockCompanion.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

/**
 * Expectations of a TypeObjectRegistry created with the mock implementation, reached with mock::mock_of().
 *
 * TypeObjectRegistry has an implicit destructor, so the mock implementation cannot detach the companion when the
 * registry is destroyed: companions are never destroyed, and gmock is told they leak on purpose. Their expectations are
 * not verified automatically, so tests setting them must call testing::Mock::VerifyAndClearExpectations().
 */
class TypeObjectRegistryMock
{
public:

    MOCK_METHOD(ReturnCode_t, register_type_object, (
                const std::string& type_name,
                const CompleteTypeObject& complete_type_object,
                TypeIdentifierPair & type_ids));

    MOCK_METHOD(ReturnCode_t, register_type_object, (
                const TypeObject& type_object,
                TypeIdentifierPair & type_ids,
                bool build_minimal));

    MOCK_METHOD(ReturnCode_t, register_type_identifier, (
                const std::string& type_name,
                TypeIdentifierPair & type_identifier));

    MOCK_METHOD(ReturnCode_t, get_type_objects, (
                const std::string& type_name,
                TypeObjectPair & type_objects));

    MOCK_METHOD(ReturnCode_t, get_type_identifiers, (
                const std::string& type_name,
                TypeIdentifierPair & type_identifiers));

    MOCK_METHOD(ReturnCode_t, get_type_object, (
                const TypeIdentifier& type_identifier,
                TypeObject & type_object));

    MOCK_METHOD(ReturnCode_t, get_complete_type_object, (
                const TypeIdentifierPair& type_identifiers,
                CompleteTypeObject & type_object));

    MOCK_METHOD(ReturnCode_t, get_type_information, (
                const TypeIdentifierPair& type_ids,
                TypeInformation & type_information,
                bool with_dependencies));

    MOCK_METHOD(ReturnCode_t, get_type_dependencies, (
                const TypeIdentifierSeq& type_identifiers,
                std::unordered_set<TypeIdentfierWithSize>& type_dependencies));

    MOCK_METHOD(bool, is_type_identifier_known, (
                const TypeIdentfierWithSize& type_identifier_with_size));

    MOCK_METHOD(ReturnCode_t, register_typeobject_w_dynamic_type, (
                const DynamicType::_ref_type& dynamic_type,
                TypeIdentifierPair & type_ids));

    MOCK_METHOD(ReturnCode_t, are_types_compatible, (
                const TypeIdentifierPair& type_identifiers,
                const TypeConsistencyEnforcementQosPolicy& type_consistency_qos));

    MOCK_METHOD(ReturnCode_t, get_dependencies_from_type_object, (
                const TypeObject& type_object,
                std::unordered_set<TypeIdentfierWithSize>& type_dependencies));

    MOCK_METHOD(const TypeIdentifier, get_complementary_type_identifier, (
                const TypeIdentifier& type_id));
};

} // namespace xtypes
} // namespace dds

namespace mock {

template<>
struct MockOf<dds::xtypes::TypeObjectRegistry>
{
    using type = dds::xtypes::TypeObjectRegistryMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTREGISTRYMOCK_HPP
