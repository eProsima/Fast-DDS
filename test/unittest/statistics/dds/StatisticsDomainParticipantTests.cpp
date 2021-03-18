// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/types/TypesBase.h>
#include <statistics/types/typesPubSubTypes.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

/*
 * This test checks eprosima::fastdds::statistics::dds::DomainParticipant narrow methods.
 * 1. Create a eprosima::fastdds::dds::DomainParticipant
 * 2. Check if the -DFASTDDS_STATISTICS flag has been set and if not check that nullptr is returned even with a valid
 * DomainParticipant.
 * 3. Use both narrow methods to obtain the pointer to the children class
 * 4. Call both narrow methods with an invalid pointer and check that it returns nullptr
 * 5. Delete DDS entities
 */
TEST(StatisticsDomainParticipantTests, NarrowDomainParticipantTest)
{
    eprosima::fastdds::dds::DomainParticipant * participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
            create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    const eprosima::fastdds::dds::DomainParticipant * const_participant = participant;

#ifndef FASTDDS_STATISTICS
    eprosima::fastdds::statistics::dds::DomainParticipant * statistics_participant =
            eprosima::fastdds::statistics::dds::DomainParticipant::narrow(participant);
    EXPECT_EQ(statistics_participant, nullptr);

    const eprosima::fastdds::statistics::dds::DomainParticipant * const_statistics_participant =
            eprosima::fastdds::statistics::dds::DomainParticipant::narrow(const_participant);
    EXPECT_EQ(const_statistics_participant, nullptr);

#else   
    logError(STATISTICS_DOMAINPARTICIPANT_TEST, "This test is going to fail because API is not yet implemented.")

    eprosima::fastdds::statistics::dds::DomainParticipant * statistics_participant =
            eprosima::fastdds::statistics::dds::DomainParticipant::narrow(participant);
    EXPECT_NE(statistics_participant, nullptr);

    const eprosima::fastdds::statistics::dds::DomainParticipant * const_statistics_participant =
            eprosima::fastdds::statistics::dds::DomainParticipant::narrow(const_participant);
    EXPECT_NE(const_statistics_participant, nullptr);

    eprosima::fastdds::dds::DomainParticipant* null_participant = nullptr;
    statistics_participant = eprosima::fastdds::statistics::dds::DomainParticipant::narrow(null_participant);
    EXPECT_EQ(statistics_participant, nullptr);

    const_participant = nullptr;
    const_statistics_participant = eprosima::fastdds::statistics::dds::DomainParticipant::narrow(const_participant);
    EXPECT_EQ(const_statistics_participant, nullptr);

#endif // FASTDDS_STATISTICS
    ASSERT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant), 
            eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks both eprosima::fastdds::statistics::dds::DomainParticipant enable_statistics_datawriter() and
 * disable_statistics_datawriter() methods.
 * 1. If the FASTDDS_STATISTICS compilation flag is not set, check that they return RETCODE_UNSUPPORTED.
 * 2. Create a eprosima::fastdds::dds::DomainParticipant and narrow to the children class.
 * 3. Enable every single statistics datawriter successfully returning RETCODE_OK.
 * 4. Check that the corresponding topic has been created and the type has been registered.
 * 5. Enable and already enabled statistics DataWriter and check that it returns RETCODE_OK.
 * 6. Call the method with an invalid topic name and check that returns RETCODE_BAD_PARAMETER.
 * 7. Disable one statistics DataWriter and check that it is successful.
 * 8. Enable the previous statistics DataWriter with an inconsistent QoS and check that it returns
 * RETCODE_INCONSISTENT_POLICY.
 * The case where the create_datawriter fails returning RETCODE_ERROR is not checked because it only passes the error
 * upstream.
 * 9. Try to delete the DomainParticipant and check that fails with RETCODE_PRECONDITION_NOT_MET because there are still
 * statistics DataWriters enabled.
 * 10. Try to disable an already disabled statistics DataWriter and check that returns RETCODE_ERROR.
 * 11. Check that if an invalid topic name is provided to the disable_statistics_datawriter method, it returns
 * RETCODE_BAD_PARAMETER.
 * 12. Delete DDS entities.
 * The case where the delete_datawriter fails returning RETCODE_ERROR is not checked because it only passes the error
 * upstream.
 */
TEST(StatisticsDomainParticipantTests, EnableDisableStatisticsDataWriterTest)
{
eprosima::fastdds::dds::DataWriterQos inconsistent_qos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
inconsistent_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
inconsistent_qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;

#ifndef FASTDDS_STATISTICS
    // 1. Compilation flag not set
    eprosima::fastdds::statistics::dds::DomainParticipant statistics_participant;
    eprosima::fastrtps::types::ReturnCode_t ret = statistics_participant.enable_statistics_datawriter(
            HISTORY_LATENCY_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED);

    ret = statistics_participant.enable_statistics_datawriter(
            "INVALID_TOPIC", inconsistent_qos);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED);

    ret = statistics_participant.disable_statistics_datawriter(HISTORY_LATENCY_TOPIC);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED);

    ret = statistics_participant.disable_statistics_datawriter("INVALID_TOPIC");
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED);
#else
    logError(STATISTICS_DOMAINPARTICIPANT_TEST, "This test is going to fail because API is not yet implemented.")

    // 2. Create DomainParticipant and narrow to eprosima::fastdds::statistics::dds::DomainParticipant
    eprosima::fastdds::dds::DomainParticipant * participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
            create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    eprosima::fastdds::statistics::dds::DomainParticipant * statistics_participant =
            eprosima::fastdds::statistics::dds::DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 3 & 4 Enable statistics DataWriters and check that the corresponding topic has been created successfully.
    // Check also that the type has been correctly registered.
    eprosima::fastdds::dds::TypeSupport history_latency_type(
            new eprosima::fastdds::statistics::WriterReaderDataPubSubType);
    eprosima::fastdds::dds::TypeSupport network_latency_type(
            new eprosima::fastdds::statistics::Locator2LocatorDataPubSubType);
    eprosima::fastdds::dds::TypeSupport throughput_type(
            new eprosima::fastdds::statistics::EntityDataPubSubType);
    eprosima::fastdds::dds::TypeSupport rtps_traffic_type(
            new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType);
    eprosima::fastdds::dds::TypeSupport count_type(
            new eprosima::fastdds::statistics::EntityCountPubSubType);
    eprosima::fastdds::dds::TypeSupport discovery_type(
            new eprosima::fastdds::statistics::DiscoveryTimePubSubType);
    eprosima::fastdds::dds::TypeSupport sample_identity_count_type(
            new eprosima::fastdds::statistics::SampleIdentityCountPubSubType);
    eprosima::fastdds::dds::TypeSupport physical_data_type(
            new eprosima::fastdds::statistics::PhysicalDataPubSubType);

    eprosima::fastrtps::types::ReturnCode_t ret = statistics_participant->enable_statistics_datawriter(
            HISTORY_LATENCY_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(history_latency_type, statistics_participant->find_type(history_latency_type.get_type_name()));

    ret = statistics_participant->enable_statistics_datawriter(
            NETWORK_LATENCY_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(network_latency_type, statistics_participant->find_type(network_latency_type.get_type_name()));

    ret = statistics_participant->enable_statistics_datawriter(
            PUBLICATION_THROUGHPUT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_EQ(throughput_type, statistics_participant->find_type(throughput_type.get_type_name()));

    ret = statistics_participant->enable_statistics_datawriter(
            SUBSCRIPTION_THROUGHPUT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    // This topic uses the throughput_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            RTPS_SENT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    EXPECT_EQ(rtps_traffic_type, statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    ret = statistics_participant->enable_statistics_datawriter(
            RTPS_LOST_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    // This topic uses the rtps_traffic_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            RESENT_DATAS_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    EXPECT_EQ(count_type, statistics_participant->find_type(count_type.get_type_name()));

    ret = statistics_participant->enable_statistics_datawriter(
            HEARTBEAT_COUNT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    // This topic uses the count_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            ACKNACK_COUNT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    // This topic uses the count_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            NACKFRAG_COUNT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    // This topic uses the count_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            GAP_COUNT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    // This topic uses the count_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            DATA_COUNT_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    // This topic uses the count_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            PDP_PACKETS_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    // This topic uses the count_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            EDP_PACKETS_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    // This topic uses the count_type that has been checked previously.

    ret = statistics_participant->enable_statistics_datawriter(
            DISCOVERY_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_EQ(discovery_type, statistics_participant->find_type(discovery_type.get_type_name()));

    ret = statistics_participant->enable_statistics_datawriter(
            SAMPLE_DATAS_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(sample_identity_count_type, statistics_participant->find_type(
            sample_identity_count_type.get_type_name()));

    ret = statistics_participant->enable_statistics_datawriter(
            PHYSICAL_DATA_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(physical_data_type, statistics_participant->find_type(physical_data_type.get_type_name()));

    // 5. Enable an enabled statistics DataWriter
    ret = statistics_participant->enable_statistics_datawriter(
            SAMPLE_DATAS_TOPIC, STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);

    // 6. Invalid topic name
    ret = statistics_participant->enable_statistics_datawriter(
            "INVALID_TOPIC", STATISTICS_DATAWRITER_QOS);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_BAD_PARAMETER);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription("INVALID_TOPIC"));

    // 7. Disable statistics DataWriter
    ret = statistics_participant->disable_statistics_datawriter(HISTORY_LATENCY_TOPIC);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            history_latency_type.get_type_name()));

    // 8. Enable previous statistics DataWriter with an inconsistent QoS
    ret = statistics_participant->enable_statistics_datawriter(
            HISTORY_LATENCY_TOPIC, inconsistent_qos);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_INCONSISTENT_POLICY);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            history_latency_type.get_type_name()));

    // RETCODE_BAD_PARAMETER error has precedence over RETCODE_INCONSISTENT_POLICY
    ret = statistics_participant->enable_statistics_datawriter(
            "INVALID_TOPIC", inconsistent_qos);
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_BAD_PARAMETER);

    // 9. Try to delete DomainParticipant and check that it fails because statistics DataWriters are still enabled
    EXPECT_TRUE(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
            delete_participant(statistics_participant) == 
            eprosima::fastrtps::types::ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    // 10. Disable already disabled DataWriter
    ret = statistics_participant->disable_statistics_datawriter(HISTORY_LATENCY_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);

    // 11. Disable invalid topic name
    ret = statistics_participant->disable_statistics_datawriter("INVALID_TOPIC");
    EXPECT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_BAD_PARAMETER);

    // 12. Remove DDS entities
    ret = statistics_participant->disable_statistics_datawriter(NETWORK_LATENCY_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            network_latency_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(PUBLICATION_THROUGHPUT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            throughput_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(SUBSCRIPTION_THROUGHPUT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            throughput_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(RTPS_SENT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            rtps_traffic_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(RTPS_LOST_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            rtps_traffic_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(RESENT_DATAS_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(ACKNACK_COUNT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(NACKFRAG_COUNT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(GAP_COUNT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(DATA_COUNT_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(PDP_PACKETS_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    // The type is being used by another topic yet
    EXPECT_NE(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(EDP_PACKETS_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(DISCOVERY_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            discovery_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(SAMPLE_DATAS_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            sample_identity_count_type.get_type_name()));

    ret = statistics_participant->disable_statistics_datawriter(PHYSICAL_DATA_TOPIC);
    ASSERT_EQ(ret, eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(eprosima::fastdds::dds::TypeSupport(nullptr), statistics_participant->find_type(
            physical_data_type.get_type_name()));

    ASSERT_TRUE(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
            delete_participant(statistics_participant) == 
            eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::KillThread();
    return ret;
}
