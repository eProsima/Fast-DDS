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

#include <gtest/gtest.h>

#include <fastdds/dds/builtin/typelookup/TypeLookupManager.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/domain/DomainParticipantImpl.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastrtps::rtps;

class DomainParticipantTest : public DomainParticipant
{
public:

    const DomainParticipantImpl* get_impl() const
    {
        return impl_;
    }

};

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
}

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

/**
 * Test that checks the TypeLookup Service builtin endpoints configuration when the client configuration is enabled.
 */
TEST(TypeLookupServiceTests, typelookup_service_client_endpoints_configuration)
{
    // Create DomainParticipant enabling the TypeLookup Service client configuration
    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().builtin.typelookup_config.use_client = true;
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, participant_qos);
    ASSERT_NE(nullptr, participant);
    // Get TypeLookup Service Manager
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    ASSERT_NE(nullptr, typelookup_manager);
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
    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().builtin.typelookup_config.use_server = true;
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, participant_qos);
    ASSERT_NE(nullptr, participant);
    // Get TypeLookup Service Manager
    builtin::TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    ASSERT_NE(nullptr, typelookup_manager);
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

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
