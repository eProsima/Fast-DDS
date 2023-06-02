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

#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/builtin/typelookup/TypeLookupManager.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <fastdds/domain/DomainParticipantImpl.hpp>

#include "idl/TestPubSubTypes.h"
#include "idl/TestTypeObject.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

class DomainParticipantTest : public DomainParticipant
{
public:

    const DomainParticipantImpl* get_impl() const
    {
        return impl_;
    }

};

/**
 * Auxiliary methods
 */

//! Create a DomainParticipant with the given TypeLookup Service configuration.
void create_participant_typelookup_config(
        DomainParticipant*& participant,
        bool use_typelookup_client,
        bool use_typelookup_server)
{
    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().builtin.typelookup_config.use_client = use_typelookup_client;
    participant_qos.wire_protocol().builtin.typelookup_config.use_server = use_typelookup_server;
    participant = DomainParticipantFactory::get_instance()->create_participant(0, participant_qos);
    ASSERT_NE(nullptr, participant);
}

//! Given a DomainParticipant return the TypeLookup Service manager.
void get_typelookup_manager(
        const DomainParticipant* participant,
        builtin::TypeLookupManager*& typelookup_manager)
{
    const DomainParticipantTest* participant_test = static_cast<const DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    const DomainParticipantImpl* participant_impl = participant_test->get_impl();
    ASSERT_NE(nullptr, participant_impl);
    const RTPSParticipant* rtps_participant = participant_impl->get_rtps_participant();
    ASSERT_NE(nullptr, rtps_participant);
    typelookup_manager = rtps_participant->typelookup_manager();
    ASSERT_NE(nullptr, typelookup_manager);
}

//! Given a Typelookup Service manager return the available builtin endpoints mask
void get_available_builtin_endpoints(
        builtin::TypeLookupManager* typelookup_manager,
        BuiltinEndpointSet_t& available_builtin_endpoints)
{
    BuiltinProtocols* builtin_protocols = typelookup_manager->get_builtin_protocols();
    ASSERT_NE(nullptr, builtin_protocols);
    PDP* pdp = builtin_protocols->mp_PDP;
    ASSERT_NE(nullptr, pdp);
    ParticipantProxyData* participant_proxy = pdp->getLocalParticipantProxyData();
    ASSERT_NE(nullptr, participant_proxy);
    available_builtin_endpoints = participant_proxy->m_availableBuiltinEndpoints;
}

//! Register type and return the TypeIdentifier sequence
void register_types(
        TypeIdentifierSeq& types)
{
    registerTestTypes();
    TypeSupport basic_struct_type(new BasicStructPubSubType());
    const TypeIdentifier* type_id =
        TypeObjectFactory::get_instance()->get_type_identifier(basic_struct_type.get_type_name());
    types.push_back(*type_id);
    TypeSupport complex_struct_type(new ComplexStructPubSubType());
    type_id = TypeObjectFactory::get_instance()->get_type_identifier(complex_struct_type.get_type_name());
    types.push_back(*type_id);
}

//! Deserialize request
void deserialize_request(
        CacheChange_t* change,
        builtin::TypeLookup_Request& request)
{
    // Ignore first 4 bytes
    SerializedPayload_t payload;
    payload.max_size = change->serializedPayload.max_size - 4;
    payload.length = change->serializedPayload.length - 4;
    payload.data = change->serializedPayload.data + 4;
    builtin::TypeLookup_RequestTypeSupport request_type;
    EXPECT_TRUE(request_type.deserialize(&payload, &request));
    payload.data = nullptr;
}

/**
 * Test that checks the TypeLookup Service builtin endpoints configuration when the client configuration is enabled.
 */
TEST(TypeLookupServiceTests, typelookup_service_client_endpoints_configuration)
{
    // Create DomainParticipant enabling the TypeLookup Service client configuration
    DomainParticipant* participant;
    create_participant_typelookup_config(participant, true, false);
    // Get TypeLookup Service Manager
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    // Access client endpoints
    StatefulWriter* request_writer = typelookup_manager->get_builtin_request_writer();
    StatefulReader* reply_reader = typelookup_manager->get_builtin_reply_reader();
    ASSERT_NE(nullptr, request_writer);
    ASSERT_NE(nullptr, reply_reader);
    // Check client endpoints QoS: XTYPES v1.3 clause 7.6.3.3.3
    EXPECT_EQ(0x000300C3, request_writer->getGuid().entityId);
    EXPECT_EQ(0x000301C4, reply_reader->getGuid().entityId);
    EXPECT_EQ(ReliabilityKind_t::RELIABLE, request_writer->getAttributes().reliabilityKind);
    EXPECT_EQ(ReliabilityKind_t::RELIABLE, reply_reader->getAttributes().reliabilityKind);
    EXPECT_EQ(DurabilityKind_t::VOLATILE, request_writer->getAttributes().durabilityKind);
    EXPECT_EQ(DurabilityKind_t::VOLATILE, reply_reader->getAttributes().durabilityKind);
    // Check builtin endpoints mask: XTYPES v1.3 clause 7.6.3.3.4
    BuiltinEndpointSet_t available_builtin_endpoints;
    get_available_builtin_endpoints(typelookup_manager, available_builtin_endpoints);
    EXPECT_TRUE(available_builtin_endpoints & (1 << 12));
    EXPECT_FALSE(available_builtin_endpoints & (1 << 14));
    EXPECT_FALSE(available_builtin_endpoints & (1 << 13));
    EXPECT_TRUE(available_builtin_endpoints & (1 << 15));

    // Server endpoints
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_request_reader());
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_reply_writer());
}

/**
 * Test that checks the TypeLookup Service builtin endpoints configuration when the server configuration is enabled.
 */
TEST(TypeLookupServiceTests, typelookup_service_server_endpoints_configuration)
{
    // Create DomainParticipant enabling the TypeLookup Service server configuration
    DomainParticipant* participant;
    create_participant_typelookup_config(participant, false, true);
    // Get TypeLookup Service Manager
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    // Access server endpoints
    StatefulWriter* reply_writer = typelookup_manager->get_builtin_reply_writer();
    StatefulReader* request_reader = typelookup_manager->get_builtin_request_reader();
    ASSERT_NE(nullptr, reply_writer);
    ASSERT_NE(nullptr, request_reader);
    // Check server endpoints QoS: XTYPES v1.3 clause 7.6.3.3.3
    EXPECT_EQ(0x000301C3, reply_writer->getGuid().entityId);
    EXPECT_EQ(0x000300C4, request_reader->getGuid().entityId);
    EXPECT_EQ(ReliabilityKind_t::RELIABLE, reply_writer->getAttributes().reliabilityKind);
    EXPECT_EQ(ReliabilityKind_t::RELIABLE, request_reader->getAttributes().reliabilityKind);
    EXPECT_EQ(DurabilityKind_t::VOLATILE, reply_writer->getAttributes().durabilityKind);
    EXPECT_EQ(DurabilityKind_t::VOLATILE, request_reader->getAttributes().durabilityKind);
    // Check builtin endpoints mask: XTYPES v1.3 clause 7.6.3.3.4
    BuiltinEndpointSet_t available_builtin_endpoints;
    get_available_builtin_endpoints(typelookup_manager, available_builtin_endpoints);
    EXPECT_FALSE(available_builtin_endpoints & (1 << 12));
    EXPECT_TRUE(available_builtin_endpoints & (1 << 14));
    EXPECT_TRUE(available_builtin_endpoints & (1 << 13));
    EXPECT_FALSE(available_builtin_endpoints & (1 << 15));

    // Client endpoints
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_reply_reader());
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_request_writer());
}

/**
 * Test that checks the correct generation of the TypeLookup_getTypeDependencies_In message requesting the type
 * dependencies: XTYPES v1.3 clause 7.6.3.3.4.1.
 * Typelookup Service builtin client endpoints enabled.
 */
TEST(TypeLookupServiceTests, typelookup_service_get_type_dependencies_request_client)
{
    // Create DomainParticipant enabling the TypeLookup Service client configuration
    DomainParticipant* participant;
    create_participant_typelookup_config(participant, true, false);
    // Register types
    TypeIdentifierSeq types;
    register_types(types);
    // Call get type dependencies operation: request is generated
    SampleIdentity sample_id = participant->get_type_dependencies(types);
    // Access request writer history
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    // Request writer history should have one sample: generated request
    WriterHistory* request_writer_history = typelookup_manager->get_builtin_request_writer_history();
    ASSERT_NE(nullptr, request_writer_history);
    EXPECT_EQ(1, request_writer_history->getHistorySize());
    CacheChange_t* change;
    EXPECT_TRUE(request_writer_history->get_min_change(&change));
    ASSERT_NE(nullptr, change);
    // Analyze request
    builtin::TypeLookup_Request request;
    deserialize_request(change, request);
    EXPECT_EQ(request.header.requestId, sample_id);
    // Service instance name: XTYPES v1.3 clause 7.6.3.3.4
    std::string instance_name = "dds.builtin.TOS.";
    std::ostringstream ret;
    for (octet value : participant->guid().guidPrefix.value)
    {
        ret << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << static_cast<int>(value);
    }
    for (octet value : participant->guid().entityId.value)
    {
        ret << static_cast<int>(value);
    }
    instance_name += ret.str();
    // TODO: this check fails because current implementation is including character `|` separating the guidPrefix from
    //       the EntityId.
    EXPECT_EQ(request.header.instanceName, instance_name);
    // Check correct discriminator
    // TODO: this check fails because the hash algorithm is updated in XTYPES v1.3 in clause 7.3.1.2.1.1
    //       Fast DDS is using the XTYPES v1.2 hash which is 0x31fbaa35
    EXPECT_EQ(request.data._d(), 0x05aafb31);
    // Check TypeLookup_getTypeDependencies_In
    EXPECT_TRUE(request.data.getTypeDependencies().continuation_point.empty());
    EXPECT_EQ(request.data.getTypeDependencies().type_ids.size(), 2);
    EXPECT_EQ(request.data.getTypeDependencies().type_ids, types);

    // The TypeIdentifiers shall be only direct HASH Identifiers
    TypeIdentifier int_id;
    int_id._d() = TK_INT32;
    types.push_back(int_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_EQ(builtin::INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(types));

    // The TypeIdentifiers shall be either all MINIMAL hash TypeIdentifiers or all COMPLETE hash TypeIdentifiers.
    types.pop_back();
    TypeIdentifier complete_id;
    complete_id._d() = EK_COMPLETE;
    types.push_back(complete_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_EQ(builtin::INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(types));

    // TODO: The TypeIdentifiers shall not include identifiers for individual types in Strongly Connected Components.
    //       Instead it shall use the identifier for the whole SCC.
    //       Recursive dependencies are not supported yet.

    // Reply reader history should be empty
    ReaderHistory* reply_reader_history = typelookup_manager->get_builtin_reply_reader_history();
    ASSERT_NE(nullptr, reply_reader_history);
    EXPECT_EQ(0u, reply_reader_history->getHistorySize());
    // Server endpoints history should not have been created
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_request_reader_history());
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_reply_writer_history());
}

/**
 * Test that checks that a Typelookup Service with builtin client endpoints disabled does not send a request.
 */
TEST(TypeLookupServiceTests, typelookup_service_get_type_dependencies_request_server)
{
    // Create DomainParticipant enabling the TypeLookup Service server configuration
    DomainParticipant* participant;
    create_participant_typelookup_config(participant, false, true);
    // Register types
    TypeIdentifierSeq types;
    register_types(types);
    EXPECT_EQ(builtin::INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(types));
    // Access TypeLookup Service server histories should be empty
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    ReaderHistory* request_reader_history = typelookup_manager->get_builtin_request_reader_history();
    EXPECT_NE(nullptr, request_reader_history);
    EXPECT_EQ(0u, request_reader_history->getHistorySize());
    WriterHistory* reply_writer_history = typelookup_manager->get_builtin_reply_writer_history();
    EXPECT_NE(nullptr, reply_writer_history);
    EXPECT_EQ(0u, reply_writer_history->getHistorySize());
    // Client endpoints history should not have been created
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_reply_reader_history());
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_request_writer_history());
}

/**
 * Test that checks the correct generation of the TypeLookup_getTypes_In message requesting the TypeObjects related to
 * the given TypeIdentifiers: XTYPES v1.3 clause 7.6.3.3.4.2.
 * Typelookup Service builtin client endpoints enabled.
 */
TEST(TypeLookupServiceTests, typelookup_service_get_types_request_client)
{

    // Create DomainParticipant enabling the TypeLookup Service client configuration
    DomainParticipant* participant;
    create_participant_typelookup_config(participant, true, false);
    // Register types
    TypeIdentifierSeq types;
    register_types(types);
    // Call get type dependencies operation: request is generated
    SampleIdentity sample_id = participant->get_types(types);
    // Access request writer history
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    // Request writer history should have one sample: generated request
    WriterHistory* request_writer_history = typelookup_manager->get_builtin_request_writer_history();
    ASSERT_NE(nullptr, request_writer_history);
    EXPECT_EQ(1, request_writer_history->getHistorySize());
    CacheChange_t* change;
    EXPECT_TRUE(request_writer_history->get_min_change(&change));
    ASSERT_NE(nullptr, change);
    // Analyze request
    builtin::TypeLookup_Request request;
    deserialize_request(change, request);
    EXPECT_EQ(request.header.requestId, sample_id);
    // Service instance name: XTYPES v1.3 clause 7.6.3.3.4
    std::string instance_name = "dds.builtin.TOS.";
    std::ostringstream ret;
    for (octet value : participant->guid().guidPrefix.value)
    {
        ret << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << static_cast<int>(value);
    }
    for (octet value : participant->guid().entityId.value)
    {
        ret << static_cast<int>(value);
    }
    instance_name += ret.str();
    // TODO: this check fails because current implementation is including character `|` separating the guidPrefix from
    //       the EntityId.
    EXPECT_EQ(request.header.instanceName, instance_name);
    // Check correct discriminator
    // TODO: this check fails because the hash algorithm is updated in XTYPES v1.3 in clause 7.3.1.2.1.1
    //       Fast DDS is using the XTYPES v1.2 hash which is 0x31fbaa35
    EXPECT_EQ(request.data._d(), 0x018252d3);
    // Check TypeLookup_getTypeDependencies_In
    EXPECT_EQ(request.data.getTypes().type_ids.size(), 2);
    EXPECT_EQ(request.data.getTypes().type_ids, types);

    // The TypeIdentifiers shall be only direct HASH Identifiers
    TypeIdentifier int_id;
    int_id._d() = TK_INT32;
    types.push_back(int_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_EQ(builtin::INVALID_SAMPLE_IDENTITY, participant->get_types(types));

    // The TypeIdentifiers allows mixed MINIMAL and COMPLETE hash TypeIdentifiers.
    types.pop_back();
    TypeIdentifier complete_id;
    complete_id._d() = EK_COMPLETE;
    types.push_back(complete_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_NE(builtin::INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(types));

    // TODO: The TypeIdentifiers shall not include identifiers for individual types in Strongly Connected Components.
    //       Instead it shall use the identifier for the whole SCC.
    //       Recursive dependencies are not supported yet.

    // Reply reader history should be empty
    ReaderHistory* reply_reader_history = typelookup_manager->get_builtin_reply_reader_history();
    ASSERT_NE(nullptr, reply_reader_history);
    EXPECT_EQ(0u, reply_reader_history->getHistorySize());
    // Server endpoints history should not have been created
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_request_reader_history());
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_reply_writer_history());

}

/**
 * Test that checks that a Typelookup Service with builtin client endpoints disabled does not send a request.
 */
TEST(TypeLookupServiceTests, typelookup_service_get_types_request_server)
{
    // Create DomainParticipant enabling the TypeLookup Service server configuration
    DomainParticipant* participant;
    create_participant_typelookup_config(participant, false, true);
    // Register types
    TypeIdentifierSeq types;
    register_types(types);
    EXPECT_EQ(builtin::INVALID_SAMPLE_IDENTITY, participant->get_types(types));
    // Access TypeLookup Service server histories should be empty
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    ReaderHistory* request_reader_history = typelookup_manager->get_builtin_request_reader_history();
    EXPECT_NE(nullptr, request_reader_history);
    EXPECT_EQ(0u, request_reader_history->getHistorySize());
    WriterHistory* reply_writer_history = typelookup_manager->get_builtin_reply_writer_history();
    EXPECT_NE(nullptr, reply_writer_history);
    EXPECT_EQ(0u, reply_writer_history->getHistorySize());
    // Client endpoints history should not have been created
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_reply_reader_history());
    EXPECT_EQ(nullptr, typelookup_manager->get_builtin_request_writer_history());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
