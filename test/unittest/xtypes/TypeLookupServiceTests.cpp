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

#include <atomic>
#include <bitset>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/builtin/typelookupservice/TypeLookupManager.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/domain/DomainParticipantImpl.hpp>

#include "idl/TypeLookupServiceTypesPubSubTypes.h"
#include "idl/TypeLookupServiceTypesTypeObject.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::builtin;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

/**
 * Auxiliary class to access RTPSParticipant
 */
class DomainParticipantTest : public DomainParticipant
{
public:

    const DomainParticipantImpl* get_impl() const
    {
        return impl_;
    }

};

/**
 * Auxiliary class to implement DomainParticipant discovery callbacks
 */
class ParticipantTestListener : public DomainParticipantListener
{
public:

    ParticipantTestListener()
    {
        matched_.store(0);
    }

    void on_participant_discovery(
            DomainParticipant*,
            ParticipantDiscoveryInfo&& info) override
    {
        if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
        {
            matched_++;
        }
        else if (info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT ||
                info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
        {
            matched_--;
        }
        cv_discovery_.notify_one();
    }

    void wait_discovery()
    {
        std::unique_lock<std::mutex> lock(mutex_discovery_);
        cv_discovery_.wait(lock, [&]()
                {
                    return matched_ > 0;
                });
    }

private:

    std::mutex mutex_discovery_;
    std::condition_variable cv_discovery_;
    std::atomic<unsigned int> matched_;

};

/**
 * Auxiliary class to implement the RTPSReader callbacks and get notified when a request/reply is received in the
 * service.
 */
class RTPSReaderListenerTest : public TypeLookupRequestListener
{
public:

    using TypeLookupRequestListener::TypeLookupRequestListener;

    void onNewCacheChangeAdded(
            RTPSReader* reader,
            const CacheChange_t* const change) override
    {
        cv_history_.notify_one();
        TypeLookupRequestListener::onNewCacheChangeAdded(reader, change);
    }

    void wait_request()
    {
        std::unique_lock<std::mutex> lock(mutex_history_);
        cv_history_.wait(lock);
    }

private:

    std::mutex mutex_history_;
    std::condition_variable cv_history_;

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
        TypeLookupManager*& typelookup_manager)
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
        TypeLookupManager* typelookup_manager,
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
        xtypes1_3::TypeIdentifierSeq& minimal_types,
        xtypes1_3::TypeIdentifierSeq& complete_types)
{
    registerTypeLookupServiceTypesTypes();

    TypeSupport struct_type(new InheritanceStructPubSubType());
    xtypes1_3::TypeIdentifierPair type_id_struct_type;
    ReturnCode_t rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(struct_type.get_type_name(), type_id_struct_type);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, rcode);
    minimal_types.push_back(type_id_struct_type.type_identifier1());
    complete_types.push_back(type_id_struct_type.type_identifier2());

    TypeSupport another_struct_type(new AnotherInheritanceStructPubSubType());
    xtypes1_3::TypeIdentifierPair type_id_another_struct_type;
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(another_struct_type.get_type_name(), type_id_another_struct_type);
    minimal_types.push_back(type_id_another_struct_type.type_identifier1());
    complete_types.push_back(type_id_another_struct_type.type_identifier2());
}

void check_and_add_type_identifier_and_type_object(
        const std::string& type_name,
        xtypes1_3::TypeIdentifierSeq& minimal_type_identifiers,
        xtypes1_3::TypeIdentifierSeq& complete_type_identifiers,
        std::vector<xtypes1_3::MinimalTypeObject>& minimal_type_objects,
        std::vector<xtypes1_3::CompleteTypeObject>& complete_type_objects)
{
    xtypes1_3::TypeIdentifierPair type_ids;
    ReturnCode_t rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(type_name, type_ids);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, rcode);
    //EXPECT_NE(nullptr, type_ids);
    EXPECT_EQ(EK_MINIMAL, type_ids.type_identifier1()._d());
    EXPECT_EQ(EK_COMPLETE, type_ids.type_identifier2()._d());
    minimal_type_identifiers.push_back(type_ids.type_identifier1());
    complete_type_identifiers.push_back(type_ids.type_identifier2());

    xtypes1_3::TypeObjectPair type_objs;
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(type_name, type_objs);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, rcode);
    //EXPECT_NE(nullptr, type_objs);
    EXPECT_EQ(EK_MINIMAL, type_objs.minimal_type_object._d());
    EXPECT_EQ(EK_COMPLETE, type_objs.complete_type_object._d());
    minimal_type_objects.push_back(type_objs.minimal_type_object);
    complete_type_objects.push_back(type_objs.complete_type_object);
}

// Get every type identifier
void get_every_type_identifier(
        xtypes1_3::TypeIdentifierSeq& minimal_type_identifiers,
        xtypes1_3::TypeIdentifierSeq& complete_type_identifiers,
        std::vector<xtypes1_3::MinimalTypeObject>& minimal_type_objects,
        std::vector<xtypes1_3::CompleteTypeObject>& complete_type_objects)
{
    // Non direct hash
    xtypes1_3::TypeIdentifierPair type_ids;
    ReturnCode_t rcode;
    
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("message", type_ids);
    EXPECT_NE(ReturnCode_t::RETCODE_OK, rcode);
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("basic", type_ids);
    EXPECT_NE(ReturnCode_t::RETCODE_OK, rcode);
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("index", type_ids);
    EXPECT_NE(ReturnCode_t::RETCODE_OK, rcode);
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("another_index", type_ids);
    EXPECT_NE(ReturnCode_t::RETCODE_OK, rcode);
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("number", type_ids);
    EXPECT_NE(ReturnCode_t::RETCODE_OK, rcode);
    rcode = DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("complex_map", type_ids);
    EXPECT_NE(ReturnCode_t::RETCODE_OK, rcode);

    // Direct hash
    check_and_add_type_identifier_and_type_object("BasicStruct", minimal_type_identifiers, complete_type_identifiers, minimal_type_objects, complete_type_objects);
    check_and_add_type_identifier_and_type_object("StructStruct", minimal_type_identifiers, complete_type_identifiers, minimal_type_objects, complete_type_objects);
    check_and_add_type_identifier_and_type_object("InheritanceStruct", minimal_type_identifiers, complete_type_identifiers, minimal_type_objects, complete_type_objects);
    check_and_add_type_identifier_and_type_object("AnotherBasicStruct", minimal_type_identifiers, complete_type_identifiers, minimal_type_objects, complete_type_objects);
    check_and_add_type_identifier_and_type_object("complex_sequence", minimal_type_identifiers, complete_type_identifiers, minimal_type_objects, complete_type_objects);
    check_and_add_type_identifier_and_type_object("AnotherInheritanceStruct", minimal_type_identifiers, complete_type_identifiers, minimal_type_objects, complete_type_objects);
}

//! Deserialize request
void deserialize_request(
        CacheChange_t* change,
        TypeLookup_Request& request)
{
    // Ignore first 4 bytes
    SerializedPayload_t payload;
    payload.max_size = change->serializedPayload.max_size - 4;
    payload.length = change->serializedPayload.length - 4;
    payload.data = change->serializedPayload.data + 4;
    TypeLookup_RequestPubSubType request_type;
    EXPECT_TRUE(request_type.deserialize(&payload, &request));
    payload.data = nullptr;
}

//! Deserialize reply
void deserialize_reply(
        CacheChange_t* change,
        TypeLookup_Reply& reply)
{
    // Ignore first 4 bytes
    SerializedPayload_t payload;
    payload.max_size = change->serializedPayload.max_size - 4;
    payload.length = change->serializedPayload.length - 4;
    payload.data = change->serializedPayload.data + 4;
    TypeLookup_ReplyPubSubType reply_type;
    EXPECT_TRUE(reply_type.deserialize(&payload, &reply));
    payload.data = nullptr;
}

//! Calculate Service instanceName
std::string instance_name(
        const eprosima::fastrtps::rtps::GUID_t& participant_guid)
{
    std::stringstream ss;
    ss << participant_guid;
    std::string str = ss.str();
    std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c)
            {
                return static_cast<unsigned char>(std::tolower(c));
            });
    str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
    return "dds.builtin.TOS." + str;
}

//! Check RequestHeader
void check_message_header(
        const rpc::RequestHeader& header,
        const eprosima::fastrtps::rtps::GUID_t& request_writer_guid,
        const eprosima::fastrtps::rtps::GUID_t& participant_guid)
{
    
    // Sample Identity GUID must be the builtin request writer GUID
    EXPECT_EQ(get_rtps_guid(header.requestId().writer_guid()), request_writer_guid);
    // Service instance name: XTYPES v1.3 clause 7.6.3.3.4
    std::string instance = instance_name(participant_guid);
    // TODO: this check fails because current implementation is including character `|` separating the guidPrefix from
    //       the EntityId.
    EXPECT_EQ(header.instanceName(), instance);
}

//! Check ReplyHeader
void check_message_header(
        const rpc::ReplyHeader& header,
        const eprosima::fastrtps::rtps::GUID_t& request_writer_guid,
        const eprosima::fastrtps::rtps::GUID_t& participant_guid)
{
    // Sample Identity GUID must be the builtin request writer GUID
    EXPECT_EQ(get_rtps_guid(header.relatedRequestId().writer_guid()), request_writer_guid);
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
    TypeLookupManager* typelookup_manager;
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
    TypeLookupManager* typelookup_manager;
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
    xtypes1_3::TypeIdentifierSeq minimal_types;
    xtypes1_3::TypeIdentifierSeq complete_types;
    register_types(minimal_types, complete_types);
    // Call get type dependencies operation: request is generated
    eprosima::fastrtps::rtps::SampleIdentity sample_id = participant->get_type_dependencies(complete_types);
    // Access request writer history
    TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    // Request writer history should have one sample: generated request
    WriterHistory* request_writer_history = typelookup_manager->get_builtin_request_writer_history();
    ASSERT_NE(nullptr, request_writer_history);
    EXPECT_EQ(1, request_writer_history->getHistorySize());
    CacheChange_t* change;
    EXPECT_TRUE(request_writer_history->get_min_change(&change));
    ASSERT_NE(nullptr, change);
    // Analyze request
    TypeLookup_Request request;
    deserialize_request(change, request);
    EXPECT_EQ(get_rtps_sample_identity(request.header().requestId()), sample_id);

    StatefulWriter* request_writer = typelookup_manager->get_builtin_request_writer();
    ASSERT_NE(nullptr, request_writer);
    check_message_header(request.header(), request_writer->getGuid(), participant->guid());
    // Check correct discriminator
    // TODO: this check fails because the hash algorithm is updated in XTYPES v1.3 in clause 7.3.1.2.1.1
    //       Fast DDS is using the XTYPES v1.2 hash which is 0x31fbaa35
    EXPECT_EQ(request.data()._d(), 0x05aafb31);
    // Check TypeLookup_getTypeDependencies_In
    EXPECT_TRUE(request.data().getTypeDependencies().continuation_point().empty());
    EXPECT_EQ(request.data().getTypeDependencies().type_ids().size(), 2);
    EXPECT_EQ(request.data().getTypeDependencies().type_ids(), complete_types);

    // The TypeIdentifiers shall be only direct HASH Identifiers
    xtypes1_3::TypeIdentifier int_id;
    int_id._d() = TK_INT32;
    complete_types.push_back(int_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_EQ(INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(complete_types));

    // The TypeIdentifiers shall be either all MINIMAL hash TypeIdentifiers or all COMPLETE hash TypeIdentifiers.
    complete_types.pop_back();
    xtypes1_3::TypeIdentifier complete_id;
    complete_id._d() = EK_COMPLETE;
    complete_types.push_back(complete_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_EQ(INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(complete_types));

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
    xtypes1_3::TypeIdentifierSeq minimal_types;
    xtypes1_3::TypeIdentifierSeq complete_types;
    register_types(minimal_types, complete_types);
    EXPECT_EQ(INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(complete_types));
    // Access TypeLookup Service server histories should be empty
    TypeLookupManager* typelookup_manager;
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
    xtypes1_3::TypeIdentifierSeq minimal_types;
    xtypes1_3::TypeIdentifierSeq complete_types;
    register_types(minimal_types, complete_types);
    // Call get type dependencies operation: request is generated
    eprosima::fastrtps::rtps::SampleIdentity sample_id = participant->get_types(complete_types);
    // Access request writer history
    TypeLookupManager* typelookup_manager;
    get_typelookup_manager(participant, typelookup_manager);
    // Request writer history should have one sample: generated request
    WriterHistory* request_writer_history = typelookup_manager->get_builtin_request_writer_history();
    ASSERT_NE(nullptr, request_writer_history);
    EXPECT_EQ(1, request_writer_history->getHistorySize());
    CacheChange_t* change;
    EXPECT_TRUE(request_writer_history->get_min_change(&change));
    ASSERT_NE(nullptr, change);
    // Analyze request
    TypeLookup_Request request;
    deserialize_request(change, request);
    EXPECT_EQ(get_rtps_sample_identity(request.header().requestId()), sample_id);

    StatefulWriter* request_writer = typelookup_manager->get_builtin_request_writer();
    ASSERT_NE(nullptr, request_writer);
    check_message_header(request.header(), request_writer->getGuid(), participant->guid());
    // Check correct discriminator
    // TODO: this check fails because the hash algorithm is updated in XTYPES v1.3 in clause 7.3.1.2.1.1
    //       Fast DDS is using the XTYPES v1.2 hash which is 0x31fbaa35
    EXPECT_EQ(request.data()._d(), 0x018252d3);
    // Check TypeLookup_getTypeDependencies_In
    EXPECT_EQ(request.data().getTypes().type_ids().size(), 2);
    EXPECT_EQ(request.data().getTypes().type_ids(), complete_types);

    // The TypeIdentifiers shall be only direct HASH Identifiers
    xtypes1_3::TypeIdentifier int_id;
    int_id._d() = TK_INT32;
    complete_types.push_back(int_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_EQ(INVALID_SAMPLE_IDENTITY, participant->get_types(complete_types));

    // The TypeIdentifiers allows mixed MINIMAL and COMPLETE hash TypeIdentifiers.
    complete_types.pop_back();
    xtypes1_3::TypeIdentifier complete_id;
    complete_id._d() = EK_COMPLETE;
    complete_types.push_back(complete_id);
    // TODO: This check fails because there are no sanity checks in the API implementation
    EXPECT_NE(INVALID_SAMPLE_IDENTITY, participant->get_type_dependencies(complete_types));

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
    xtypes1_3::TypeIdentifierSeq minimal_types;
    xtypes1_3::TypeIdentifierSeq complete_types;
    register_types(minimal_types, complete_types);
    EXPECT_EQ(INVALID_SAMPLE_IDENTITY, participant->get_types(complete_types));
    // Access TypeLookup Service server histories should be empty
    TypeLookupManager* typelookup_manager;
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
 * Test that checks the getTypeDependencies operation.
 */
TEST(TypeLookupServiceTests, typelookup_service_get_type_dependencies)
{
    // Create two DomainParticipants configured as client and as server
    DomainParticipant* participant_server;
    DomainParticipant* participant_client;
    create_participant_typelookup_config(participant_server, false, true);
    // Before creating the client, create listener and attach it to the Server Participant
    ParticipantTestListener server_listener;
    participant_server->set_listener(&server_listener);

    create_participant_typelookup_config(participant_client, true, false);
    ParticipantTestListener client_listener;
    participant_client->set_listener(&client_listener);

    server_listener.wait_discovery();
    client_listener.wait_discovery();

    // Access Request Writer: get GUID for matching purposes
    TypeLookupManager* typelookup_manager_client;
    get_typelookup_manager(participant_client, typelookup_manager_client);
    StatefulWriter* request_writer = typelookup_manager_client->get_builtin_request_writer();
    ASSERT_NE(nullptr, request_writer);
    eprosima::fastrtps::rtps::GUID_t request_writer_guid = request_writer->getGuid();

    // Access Request Reader
    TypeLookupManager* typelookup_manager_service;
    get_typelookup_manager(participant_server, typelookup_manager_service);
    // Add custom listener to request reader to get notified about received requests
    StatefulReader* request_reader = typelookup_manager_service->get_builtin_request_reader();
    ASSERT_NE(nullptr, request_reader);
    RTPSReaderListenerTest request_listener(&*typelookup_manager_service);
    EXPECT_TRUE(request_reader->setListener(&request_listener));
    // Access Request Reader History
    ReaderHistory* request_reader_history = typelookup_manager_service->get_builtin_request_reader_history();
    ASSERT_NE(nullptr, request_reader_history);
    // Wait for matching
    while (!request_reader->matched_writer_is_matched(request_writer_guid))
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }

    // Spawn a thread to wait for sample
    auto test = std::thread(&RTPSReaderListenerTest::wait_request, &request_listener);

    // Register types
    xtypes1_3::TypeIdentifierSeq minimal_types;
    xtypes1_3::TypeIdentifierSeq complete_types;
    register_types(minimal_types, complete_types);
    xtypes1_3::TypeIdentifierSeq minimal_type_identifiers;
    xtypes1_3::TypeIdentifierSeq complete_type_identifiers;
    std::vector<xtypes1_3::MinimalTypeObject> minimal_type_objects;
    std::vector<xtypes1_3::CompleteTypeObject> complete_type_objects;
    get_every_type_identifier(minimal_type_identifiers, complete_type_identifiers , minimal_type_objects, complete_type_objects);

    // Client call getTypeDependencies operation
    participant_client->get_type_dependencies(complete_types);

    // Wait for sample reception
    test.join();

    // Wait for reply
    // TODO: current implementation sends reply on the onNewCacheChangeAdded callback
    //       This should be refactored so another thread is notified and takes charge of preparing and sending the
    //       response.
    WriterHistory* reply_writer_history = typelookup_manager_service->get_builtin_reply_writer_history();
    ASSERT_NE(nullptr, reply_writer_history);
    while (0 == reply_writer_history->getHistorySize())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
    // Reply writer history should have one sample
    EXPECT_EQ(1, reply_writer_history->getHistorySize());
    CacheChange_t* change;
    EXPECT_TRUE(reply_writer_history->get_min_change(&change));
    ASSERT_NE(nullptr, change);
    TypeLookup_Reply reply;
    deserialize_reply(change, reply);

    // Analyze generated reply
    // TODO: wrong instance name
    check_message_header(reply.header(), request_writer->getGuid(), participant_server->guid());
    // Operation discriminator
    // TODO: wrong hash (updated in XTYPES v1.3)
    EXPECT_EQ(reply.return_value()._d(), 0x05aafb31);
    // Operation result discriminator
    EXPECT_EQ(reply.return_value().getTypeDependencies()._d(), static_cast<int32_t>(ReturnCode_t::RETCODE_OK));
    // Check TypeLookup_getTypeDependencies_Out
    // TODO: the returned information is not according to the specification.
    //       These checks are failing
    EXPECT_EQ(reply.return_value().getTypeDependencies().result().dependent_typeids().size(), 4);
    // Mask to check that the corresponding dependency is set
    // 0x01 BasicStruct
    // 0x02 StructStruct
    // 0x04 InheritanceStruct
    // 0x08 AnotherBasicStruct
    // 0x10 complex_sequence
    // 0x20 AnotherInheritanceStruct
    // Final mask should be 0x1B: 0x01 & 0x02 & 0x08 & 0x10
    std::bitset<8> type_dependency_found;
    for (auto type : reply.return_value().getTypeDependencies().result().dependent_typeids())
    {
        // The field dependent_typeids shall exclusively contain of direct HASH TypeIdentifiers that are recursive
        // dependencies from at least one of the TypeIdentifiers in the request.
        EXPECT_PRED3([](octet kind, octet complete, octet minimal)
                {
                    return kind == complete || kind == minimal;
                }, type.type_id()._d(), EK_COMPLETE, EK_MINIMAL);
        // TypeIdentifierWithSize
        // TODO: typeobject_serialized_size is not correct
        EXPECT_NE(0u, type.typeobject_serialized_size());
        for (size_t i = 0; i < minimal_type_identifiers.size(); i++)
        {
            if (EK_COMPLETE == type.type_id()._d())
            {
                if (type.type_id() == complete_type_identifiers[i])
                {
                    //  EXPECT_EQ(type.typeobject_serialized_size(), eprosima::fastcdr::calculate_serialized_size(
                    //             complete_type_objects[i]));
                    //TODO adelcampo
                    type_dependency_found.set(i);
                }
            }
            else if (EK_MINIMAL == type.type_id()._d())
            {
                if (type.type_id() == minimal_type_identifiers[i])
                {
                    // EXPECT_EQ(type.typeobject_serialized_size(), eprosima::fastcdr::calculate_serialized_size(
                    //             minimal_type_objects[i]));
                    //TODO adelcampo
                    type_dependency_found.set(i);
                }
            }
        }
    }
    EXPECT_EQ(0x1B, type_dependency_found.to_ulong());
    EXPECT_TRUE(reply.return_value().getTypeDependencies().result().continuation_point().empty());

    // Reply reception cannot be checked with current implementation because the sample is removed from the history
    // after being processed.

    // Unset listeners for a clean exit
    participant_client->set_listener(nullptr);
    participant_server->set_listener(nullptr);
}

// TODO: test that forces the usage of the continuation point mechanism (too many dependencies to send in one reply)
//       Looking at the implementation, the continuation point is only enabled if there are more than 255 type
//       dependencies (hardcoded value in TypeObjectFactory::typelookup_get_type_dependencies).
/*
   TEST(TypeLookupServiceTests, typelookup_service_get_type_dependencies_with_continuation)
   {
   }
 */

/**
 * Test that checks the reception of malformed getTypeDependencies requests.
 * Cases:
 *   1. Bad instance name
 *   2. Service GUID in instance name does not coincide with Request Writer GUID prefix
 *   3. Request Writer Entity ID is not the defined in the specification
 *   4. Some of the received TypeIdentifiers is not a direct HASH
 *   5. There is a mix between MINIMAL and COMPLETE TypeIdentifiers.
 *   6. TODO: No SCC individual TypeIdentifier should be included.
 */
/*
   TEST(TypeLookupServiceTests, typelookup_service_get_type_dependencies_malformed_request_reception)
   {
   }
 */

/**
 * Test that checks the getTypes operation receiving a request with COMPLETE TypeIdentifiers.
 */
TEST(TypeLookupServiceTests, typelookup_service_get_types_complete_request)
{
    // Create two DomainParticipants configured as client and as server
    DomainParticipant* participant_server;
    DomainParticipant* participant_client;
    create_participant_typelookup_config(participant_server, false, true);
    // Before creating the client, create listener and attach it to the Server Participant
    ParticipantTestListener server_listener;
    participant_server->set_listener(&server_listener);

    create_participant_typelookup_config(participant_client, true, false);
    ParticipantTestListener client_listener;
    participant_client->set_listener(&client_listener);

    server_listener.wait_discovery();
    client_listener.wait_discovery();

    // Access Request Writer: get GUID for matching purposes
    TypeLookupManager* typelookup_manager_client;
    get_typelookup_manager(participant_client, typelookup_manager_client);
    StatefulWriter* request_writer = typelookup_manager_client->get_builtin_request_writer();
    ASSERT_NE(nullptr, request_writer);
    eprosima::fastrtps::rtps::GUID_t request_writer_guid = request_writer->getGuid();

    // Access Request Reader
    TypeLookupManager* typelookup_manager_service;
    get_typelookup_manager(participant_server, typelookup_manager_service);
    // Add custom listener to request reader to get notified about received requests
    StatefulReader* request_reader = typelookup_manager_service->get_builtin_request_reader();
    ASSERT_NE(nullptr, request_reader);
    RTPSReaderListenerTest request_listener(&*typelookup_manager_service);
    EXPECT_TRUE(request_reader->setListener(&request_listener));
    // Access Request Reader History
    ReaderHistory* request_reader_history = typelookup_manager_service->get_builtin_request_reader_history();
    ASSERT_NE(nullptr, request_reader_history);
    // Wait for matching
    while (!request_reader->matched_writer_is_matched(request_writer_guid))
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }

    // Spawn a thread to wait for sample
    auto test = std::thread(&RTPSReaderListenerTest::wait_request, &request_listener);

    // Register types
    registerTypeLookupServiceTypesTypes();
    xtypes1_3::TypeIdentifierSeq types;

    xtypes1_3::TypeIdentifierPair typeInheritanceStruct_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("InheritanceStruct", typeInheritanceStruct_ids);
    xtypes1_3::TypeIdentifierPair typeAnotherInheritanceStruct_ids;
    DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("AnotherInheritanceStruct", typeAnotherInheritanceStruct_ids);
    types.push_back(typeInheritanceStruct_ids.type_identifier1());
    types.push_back(typeAnotherInheritanceStruct_ids.type_identifier1());

    for (auto type_id : types)
    {
        EXPECT_EQ(EK_COMPLETE, type_id._d());
    }

    xtypes1_3::TypeIdentifierSeq minimal_type_identifiers;
    xtypes1_3::TypeIdentifierSeq complete_type_identifiers;
    std::vector<xtypes1_3::MinimalTypeObject> minimal_type_objects;
    std::vector<xtypes1_3::CompleteTypeObject> complete_type_objects;
    get_every_type_identifier(minimal_type_identifiers, complete_type_identifiers , minimal_type_objects, complete_type_objects);

    // Client call getTypeDependencies operation
    participant_client->get_types(types);

    // Wait for sample reception
    test.join();

    // Wait for reply
    // TODO: current implementation sends reply on the onNewCacheChangeAdded callback
    //       This should be refactored so another thread is notified and takes charge of preparing and sending the
    //       response.
    WriterHistory* reply_writer_history = typelookup_manager_service->get_builtin_reply_writer_history();
    ASSERT_NE(nullptr, reply_writer_history);
    while (0 == reply_writer_history->getHistorySize())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
    // Reply writer history should have one sample
    EXPECT_EQ(1, reply_writer_history->getHistorySize());
    CacheChange_t* change;
    EXPECT_TRUE(reply_writer_history->get_min_change(&change));
    ASSERT_NE(nullptr, change);
    TypeLookup_Reply reply;
    deserialize_reply(change, reply);

    // Analyze generated reply
    // TODO: wrong instance name
    check_message_header(reply.header(), request_writer->getGuid(), participant_server->guid());
    // Operation discriminator
    // TODO: wrong hash (updated in XTYPES v1.3)
    EXPECT_EQ(reply.return_value()._d(), 0x018252d3);
    // Operation result discriminator
    EXPECT_EQ(reply.return_value().getType()._d(), static_cast<int32_t>(ReturnCode_t::RETCODE_OK));
    // Check TypeLookup_getTypes_Out
    // If the request had a COMPLETE TypeIdentifiers, the types shall contain COMPLETE TypeObjects
    // XTYPES v1.3 clause 7.6.3.3.4.2
    EXPECT_TRUE(reply.return_value().getType().result().complete_to_minimal().empty());
    EXPECT_EQ(types.size(), reply.return_value().getType().result().types().size());
    EXPECT_EQ(reply.return_value().getType().result().types()[0].type_identifier(), types[0]);
    EXPECT_EQ(reply.return_value().getType().result().types()[1].type_identifier(), types[1]);
    // EXPECT_EQ(reply.return_value().getType().result().types()[0].type_object(), complete_type_objects[2]);
    // EXPECT_EQ(reply.return_value().getType().result().types()[1].type_object(), complete_type_objects[5]);
    //TODO adelcampo

    // Reply reception cannot be checked with current implementation because the sample is removed from the history
    // after being processed.

    // Unset listeners for a clean exit
    participant_client->set_listener(nullptr);
    participant_server->set_listener(nullptr);
}

/**
 * Test that checks the getTypes operation receiving a request with MINIMAL TypeIdentifiers.
 */
TEST(TypeLookupServiceTests, typelookup_service_get_types_minimal_request)
{
    // Create two DomainParticipants configured as client and as server
    DomainParticipant* participant_server;
    DomainParticipant* participant_client;
    create_participant_typelookup_config(participant_server, false, true);
    // Before creating the client, create listener and attach it to the Server Participant
    ParticipantTestListener server_listener;
    participant_server->set_listener(&server_listener);

    create_participant_typelookup_config(participant_client, true, false);
    ParticipantTestListener client_listener;
    participant_client->set_listener(&client_listener);

    server_listener.wait_discovery();
    client_listener.wait_discovery();

    // Access Request Writer: get GUID for matching purposes
    TypeLookupManager* typelookup_manager_client;
    get_typelookup_manager(participant_client, typelookup_manager_client);
    StatefulWriter* request_writer = typelookup_manager_client->get_builtin_request_writer();
    ASSERT_NE(nullptr, request_writer);
    eprosima::fastrtps::rtps::GUID_t request_writer_guid = request_writer->getGuid();

    // Access Request Reader
    TypeLookupManager* typelookup_manager_service;
    get_typelookup_manager(participant_server, typelookup_manager_service);
    // Add custom listener to request reader to get notified about received requests
    StatefulReader* request_reader = typelookup_manager_service->get_builtin_request_reader();
    ASSERT_NE(nullptr, request_reader);
    RTPSReaderListenerTest request_listener(&*typelookup_manager_service);
    EXPECT_TRUE(request_reader->setListener(&request_listener));
    // Access Request Reader History
    ReaderHistory* request_reader_history = typelookup_manager_service->get_builtin_request_reader_history();
    ASSERT_NE(nullptr, request_reader_history);
    // Wait for matching
    while (!request_reader->matched_writer_is_matched(request_writer_guid))
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }

    // Spawn a thread to wait for sample
    auto test = std::thread(&RTPSReaderListenerTest::wait_request, &request_listener);

    // Register types
    xtypes1_3::TypeIdentifierSeq minimal_types;
    xtypes1_3::TypeIdentifierSeq complete_types;
    register_types(minimal_types, complete_types);
    xtypes1_3::TypeIdentifierSeq minimal_type_identifiers;
    xtypes1_3::TypeIdentifierSeq complete_type_identifiers;
    std::vector<xtypes1_3::MinimalTypeObject> minimal_type_objects;
    std::vector<xtypes1_3::CompleteTypeObject> complete_type_objects;
    get_every_type_identifier(minimal_type_identifiers, complete_type_identifiers , minimal_type_objects, complete_type_objects);

    // Client call getTypeDependencies operation
    participant_client->get_types(complete_types);

    // Wait for sample reception
    test.join();

    // Wait for reply
    // TODO: current implementation sends reply on the onNewCacheChangeAdded callback
    //       This should be refactored so another thread is notified and takes charge of preparing and sending the
    //       response.
    WriterHistory* reply_writer_history = typelookup_manager_service->get_builtin_reply_writer_history();
    ASSERT_NE(nullptr, reply_writer_history);
    while (0 == reply_writer_history->getHistorySize())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
    // Reply writer history should have one sample
    EXPECT_EQ(1, reply_writer_history->getHistorySize());
    CacheChange_t* change;
    EXPECT_TRUE(reply_writer_history->get_min_change(&change));
    ASSERT_NE(nullptr, change);
    TypeLookup_Reply reply;
    deserialize_reply(change, reply);

    // Analyze generated reply
    // TODO: wrong instance name
    check_message_header(reply.header(), request_writer->getGuid(), participant_server->guid());
    // Operation discriminator
    // TODO: wrong hash (updated in XTYPES v1.3)
    EXPECT_EQ(reply.return_value()._d(), 0x018252d3);
    // Operation result discriminator
    EXPECT_EQ(reply.return_value().getType()._d(), static_cast<int32_t>(ReturnCode_t::RETCODE_OK));
    // Check TypeLookup_getTypes_Out
    // If the request had MINIMAL TypeIdentifiers the types may contain either MINIMAL or COMPLETE TypeObjects
    // The filed complete_to_minimal shall contain the mapping from COMPLETE TypeIdentifiers to MINIMAL TypeIdentifiers
    // for any COMPLETE TypeIdentifiers that appear within COMPLETE TypeObjects that were sent in reponse to a query for
    // a MINIMAL TypeIdentifier.XTYPES v1.3 Clause 7.6.3.3.4.2
    EXPECT_EQ(complete_types.size(), reply.return_value().getType().result().types().size());
    if (reply.return_value().getType().result().complete_to_minimal().empty())
    {
        // Fast DDS seems to follow this approach
        EXPECT_EQ(reply.return_value().getType().result().types()[0].type_identifier(), complete_types[0]);
        EXPECT_EQ(reply.return_value().getType().result().types()[1].type_identifier(), complete_types[1]);
        // EXPECT_EQ(reply.return_value().getType().result().types()[0].type_object(), minimal_type_objects[2]);
        // EXPECT_EQ(reply.return_value().getType().result().types()[1].type_object(), minimal_type_objects[5]);
        //TODO adelcampo
    }
    else
    {
        EXPECT_EQ(reply.return_value().getType().result().types()[0].type_identifier(), complete_type_identifiers[2]);
        EXPECT_EQ(reply.return_value().getType().result().types()[1].type_identifier(), complete_type_identifiers[5]);
        // EXPECT_EQ(reply.return_value().getType().result().types()[0].type_object(), complete_type_objects[2]);
        // EXPECT_EQ(reply.return_value().getType().result().types()[1].type_object(), complete_type_objects[5]);
        //TODO adelcampo
        // Check complete_to_minimal correct implementation
        ASSERT_EQ(reply.return_value().getType().result().complete_to_minimal().size(), 2);
        EXPECT_EQ(reply.return_value().getType().result().complete_to_minimal()[0].type_identifier1(),
                complete_type_identifiers[2]);
        EXPECT_EQ(reply.return_value().getType().result().complete_to_minimal()[0].type_identifier2(),
                minimal_type_identifiers[2]);
        EXPECT_EQ(reply.return_value().getType().result().complete_to_minimal()[1].type_identifier1(),
                complete_type_identifiers[5]);
        EXPECT_EQ(reply.return_value().getType().result().complete_to_minimal()[1].type_identifier2(),
                minimal_type_identifiers[5]);
    }

    // TODO: If a TypeIdentifier was a SCCIdentifier, then the response shall treat the TypeObjects within the SCC
    //       atomically. Either include all in the reply or none. XTYPES v1.3 Clause 7.6.3.3.4.2

    // Reply reception cannot be checked with current implementation because the sample is removed from the history
    // after being processed.

    // Unset listeners for a clean exit
    participant_client->set_listener(nullptr);
    participant_server->set_listener(nullptr);
}

/**
 * Test that checks the reception of a malformed getTypes request.
 * Cases:
 *   1. Bad instance name
 *   2. Service GUID in instance name does not coincide with Request Writer GUID prefix
 *   3. Request Writer Entity ID is not the defined in the specification
 *   4. Some of the received TypeIdentifiers is not a direct HASH
 *   5. TODO: No SCC individual TypeIdentifier should be included.
 */
/*
   TEST(TypeLookupServiceTests, typelookup_service_get_types_malformed_request_reception)
   {
   }
 */





/**
 * get_types indirect HASH.
 */
/**
 * get_types SCC.
 */
/**
 * get_types with COMPLETE TypeIdentifiers.
 */
/**
 * get_types with COMPLETE and MINIMAL TypeIdentifiers.
 */
/**
 * get_types with only MINIMAL TypeIdentifiers, minimal_reply ON.
 */
/**
 * get_types with only MINIMAL TypeIdentifiers, minimal_reply OFF.
 */



/**
 * get_type_dependencies indirect HASH.
 */
/**
 * get_type_dependencies MINIMAL.
 */
/**
 * get_type_dependencies COMPLETE.
 */
/**
 * get_type_dependencies MINIMAL and COMPLETE.
 */
/**
 * get_type_dependencies SCC.
 */
/**
 * get_type_dependencies continuation_point.
 */
/**
 * get_type_dependencies reply wrong continuation_point.
 */









/**
 * Participant asks for N types, all known to the other participant.
 */
/**
 * Participant asks for N types, only some known to the other participant.
 */
/**
 * Participant asks for N types, none to the other participant.
 */

/**
 * Participant asks for N type dependencies, all known to the other participant.
 */
/**
 * Participant asks for N type dependencies, only some known to the other participant.
 */
/**
 * Participant asks for N type dependencies, none to the other participant.
 */



int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
