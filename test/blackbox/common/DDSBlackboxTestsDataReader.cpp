// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/core/StackAllocatedSequence.hpp>
#include <fastrtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "BlackboxTests.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#define INCOMPATIBLE_TEST_TOPIC_NAME std::string( \
        std::string("incompatible_") + TEST_TOPIC_NAME)


enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DDSDataReader : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

TEST_P(DDSDataReader, LivelinessChangedStatusGet)
{
    static constexpr int32_t num_times = 3u;

    static const Duration_t lease_duration(60, 0);
    static const Duration_t announcement_period(0, 100 * 1000);

    // Create and start reader that will not invoke listener for liveliness_changed
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::liveliness_changed());
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Create a participant for the writers
    std::unique_ptr<PubSubParticipant<HelloWorldPubSubType>> writers;
    writers.reset(new PubSubParticipant<HelloWorldPubSubType>(num_times, 0, num_times, 0));
    writers->pub_topic_name(TEST_TOPIC_NAME)
            .pub_liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .pub_liveliness_announcement_period(announcement_period)
            .pub_liveliness_lease_duration(lease_duration);
    ASSERT_TRUE(writers->init_participant());

    // Ensure initial status is 'there are no writers'
    eprosima::fastdds::dds::LivelinessChangedStatus status;
    status = reader.get_liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 0);
    EXPECT_EQ(status.alive_count_change, 0);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Start all writers
    for (unsigned int i = 0; i < num_times; i++)
    {
        ASSERT_TRUE(writers->init_publisher(i));
    }

    // Wait for discovery to finish
    reader.wait_discovery(std::chrono::seconds::zero(), num_times);
    writers->pub_wait_discovery();

    // Assert liveliness by sending a sample
    auto messages = default_helloworld_data_generator(1);
    reader.startReception(messages);
    writers->send_sample(messages.front(), 0);
    reader.block_for_at_least(1);

    // Check we have 'num_times' NEW alive writers
    status = reader.get_liveliness_changed_status();
    EXPECT_EQ(status.alive_count, num_times);
    EXPECT_EQ(status.alive_count_change, num_times);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Reading status again should reset count changes
    status = reader.get_liveliness_changed_status();
    EXPECT_EQ(status.alive_count, num_times);
    EXPECT_EQ(status.alive_count_change, 0);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Stop writers and wait till reader is aware
    writers.reset(nullptr);
    reader.wait_writer_undiscovery();

    // Check we have lost 'num_times' alive writers
    status = reader.get_liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 0);
    EXPECT_EQ(status.alive_count_change, -num_times);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Reading status again should reset count changes
    status = reader.get_liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 0);
    EXPECT_EQ(status.alive_count_change, 0);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

}

// Regression test of Refs #16608, Github #3203. Checks that total_unread_ variable is consistent with
// unread changes in reader's history after performing a get_first_untaken_info() on a change with no writer matched.
TEST_P(DDSDataReader, ConsistentTotalUnreadAfterGetFirstUntakenInfo)
{
    if (enable_datasharing)
    {
        //! TODO: Datasharing changes the behavior of this test. Changes are
        //! instantly removed on removePublisher() call and on the PUBListener callback
        GTEST_SKIP() << "Data-sharing removes the changes instantly changing the behavior of this test. Skipping";
    }

    //! Spawn a couple of participants writer/reader
    PubSubWriter<HelloWorldPubSubType> pubsub_writer(TEST_TOPIC_NAME);
    //! Create a reader that does nothing when new data is available. Neither take nor read it.
    PubSubReader<HelloWorldPubSubType> pubsub_reader(TEST_TOPIC_NAME, false, false, false);

    // Initialization of all the participants
    std::cout << "Initializing PubSubs for topic " << TEST_TOPIC_NAME << std::endl;

    //! Participant Writer configuration and qos
    pubsub_writer.reliability(eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS)
            .init();
    ASSERT_EQ(pubsub_writer.isInitialized(), true);

    //! Participant Reader configuration and qos
    pubsub_reader.reliability(eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS)
            .init();
    ASSERT_EQ(pubsub_reader.isInitialized(), true);

    eprosima::fastdds::dds::DataReader& reader = pubsub_reader.get_native_reader();
    eprosima::fastdds::dds::SampleInfo info;

    EXPECT_EQ(ReturnCode_t::RETCODE_NO_DATA, reader.get_first_untaken_info(&info));

    // Wait for discovery.
    pubsub_reader.wait_discovery();
    pubsub_writer.wait_discovery();

    auto data = default_helloworld_data_generator();

    pubsub_reader.startReception(data);

    pubsub_writer.send(data);
    EXPECT_TRUE(data.empty());

    pubsub_reader.block_for_unread_count_of(3);
    pubsub_writer.removePublisher();
    pubsub_reader.wait_writer_undiscovery();

    //! Try reading the first untaken info.
    //! Checks whether total_unread_ is consistent with
    //! the number of unread changes in history
    //! This API call should NOT modify the history
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, reader.get_first_untaken_info(&info));

    HelloWorld msg;
    eprosima::fastdds::dds::SampleInfo sinfo;

    //! Try getting a sample
    auto result = reader.take_next_sample((void*)&msg, &sinfo);

    //! Assert last operation
    ASSERT_EQ(result, ReturnCode_t::RETCODE_OK) << "Reader's unread count is: " << reader.get_unread_count();
}

//! Regression test for #20706
//! get_first_untaken_info() returns the first valid change of an instance, not only the first
//! cache change. This implies searching in all the cache changes of the instance.
//! In the scenario of having multiple reliable writers and one reader with history size > 1 in the same topic,
//! it can happen that get_first_untaken_info() returns OK (as it is not currently checking whether the change is in the future)
//! but take() returns NO_DATA because it is waiting for a previous SequenceNumber from the writer.
TEST(DDSDataReader, GetFirstUntakenInfoReturnsTheFirstValidChange)
{
    PubSubWriter<HelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_2(TEST_TOPIC_NAME);
    // The reader should not take nor read any sample in this test
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME, false, false, false);

    auto testTransport_1 = std::make_shared<test_UDPv4TransportDescriptor>();

    EntityId_t writer1_id;
    EntityId_t reader_id;

    testTransport_1->drop_data_messages_filter_ =
            [&writer1_id, &reader_id](eprosima::fastrtps::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.3 Data Submessage
                EntityId_t readerID;
                EntityId_t writerID;
                SequenceNumber_t sn;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline quos
                CDRMessage::readEntityId(&msg, &readerID);
                CDRMessage::readEntityId(&msg, &writerID);
                CDRMessage::readSequenceNumber(&msg, &sn);

                // restore buffer pos
                msg.pos = old_pos;

                // Loose Seqnum 1
                if (writerID == writer1_id &&
                        readerID == reader_id &&
                        (sn == SequenceNumber_t{0, 1}))
                {
                    return true;
                }

                return false;
            };

    writer_1.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport_1)
            .history_depth(3)
            .init();

    writer_2.history_depth(3)
            .init();

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_depth(3)
            .init();

    ASSERT_TRUE(writer_1.isInitialized());
    ASSERT_TRUE(writer_2.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer1_id = writer_1.datawriter_guid().entityId;
    reader_id = reader.datareader_guid().entityId;

    // Wait for discovery.
    writer_1.wait_discovery();
    writer_2.wait_discovery();
    reader.wait_discovery(std::chrono::seconds::zero(), 2);

    // Send writer_1 samples
    auto data = default_helloworld_data_generator(3);

    reader.startReception(data);
    writer_1.send(data);

    // The reader should have received samples 2,3 but not 1
    // get_first_untaken_info() should never return OK since the received changes are all in the future.
    // We try it several times in case the reader has not received the samples yet.
    eprosima::fastdds::dds::SampleInfo info;
    for (size_t i = 0; i < 3; i++)
    {
        ASSERT_NE(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, reader.get_native_reader().get_first_untaken_info(
                    &info));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Now we send data from writer_2 with no drops and all samples shall be received.
    data = default_helloworld_data_generator(3);
    writer_2.send(data);
    reader.block_for_unread_count_of(3);

    // get_first_untaken_info() must return OK now
    ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK,
            reader.get_native_reader().get_first_untaken_info(&info));
    eprosima::fastdds::dds::StackAllocatedSequence<HelloWorld*, 1> data_values;
    eprosima::fastdds::dds::SampleInfoSeq sample_infos{1};
    // As get_first_untaken_info() returns OK, take() must return OK too
    ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK,
            reader.get_native_reader().take(data_values, sample_infos));
}

//! Regression test for Issues #3822 Github #3875
//! This test needs to late join a reader in the same process.
//! Not setting this test as parametrized since it only makes sense in intraprocess.
//! Note: Without the fix, the test fails ~1/10 times, it is encouraged to launch
//! the test with --retest-until-fail 50
TEST(DDSDataReader, ConsistentReliabilityWhenIntraprocess)
{
    //! Manually set intraprocess
    LibrarySettingsAttributes library_settings;
    library_settings.intraprocess_delivery = eprosima::fastrtps::INTRAPROCESS_FULL;
    xmlparser::XMLProfileManager::library_settings(library_settings);

    auto participant = DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230,
        DomainParticipantFactory::get_instance()->get_default_participant_qos(), nullptr,
        eprosima::fastdds::dds::StatusMask::none());

    eprosima::fastdds::dds::TypeSupport t_type{ new HelloWorldPubSubType() };
    ASSERT_TRUE(t_type.register_type( participant ) == ReturnCode_t::RETCODE_OK);

    auto topic = participant->create_topic( TEST_TOPIC_NAME, t_type.get_type_name(),
                    participant->get_default_topic_qos());

    // create publisher and writer
    auto publisher = participant->create_publisher( participant->get_default_publisher_qos());

    auto writer_qos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
    writer_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    auto writer = publisher->create_datawriter( topic, writer_qos );

    auto data = HelloWorld{};
    ASSERT_TRUE(writer->write( &data ));

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // create a late joiner subscriber and reader
    auto subscriber = participant->create_subscriber( participant->get_default_subscriber_qos());
    auto reader_qos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    reader_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    auto reader = subscriber->create_datareader( topic, reader_qos );

    eprosima::fastdds::dds::SubscriptionMatchedStatus status;
    reader->get_subscription_matched_status(status);
    ASSERT_GT(status.total_count, 0);

    // wait for message
    uint64_t unread_count = 0;
    auto t0 = std::chrono::steady_clock::now();
    while (unread_count <= 0 &&
            (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - t0)).count() < 2)
    {
        unread_count = reader->get_unread_count(true);
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));
    }

    participant->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant);

    ASSERT_TRUE(unread_count > 0);

    //! Reset back to INTRAPROCESS_OFF
    library_settings.intraprocess_delivery = eprosima::fastrtps::INTRAPROCESS_OFF;
    xmlparser::XMLProfileManager::library_settings(library_settings);
}

/**
 * This is a regression test for issue https://eprosima.easyredmine.com/issues/20504.
 * It checks that a DataReader be created with default Qos and a large history depth.
 */
TEST(DDSDataReader, default_qos_large_history_depth)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.history_depth(1000).init();
    ASSERT_TRUE(reader.isInitialized());
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSDataReader,
        DDSDataReader,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<DDSDataReader::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });

