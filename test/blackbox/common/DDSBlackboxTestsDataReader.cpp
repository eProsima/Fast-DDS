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
#include <type_traits>

#include <gtest/gtest.h>

#include <fastdds/dds/core/StackAllocatedSequence.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>

#include "../utils/filter_helpers.hpp"
#include "BlackboxTests.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

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
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
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
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
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

auto check_qos_in_data_r =
        [](rtps::CDRMessage_t& msg, std::atomic<uint8_t>& qos_found, std::vector<uint16_t>& expected_qos_pids)
        {
            uint32_t qos_size = 0;
            uint32_t original_pos = msg.pos;
            bool is_sentinel = false;

            while (!is_sentinel)
            {
                msg.pos = original_pos + qos_size;

                uint16_t pid = eprosima::fastdds::helpers::cdr_parse_u16(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 2;
                uint16_t plength = eprosima::fastdds::helpers::cdr_parse_u16(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 2;
                bool valid = true;

                // If inline_qos submessage is found we will have an additional Sentinel
                if (pid == eprosima::fastdds::dds::PID_SENTINEL)
                {
                    // PID_SENTINEL is always considered of length 0
                    plength = 0;
                    // If the PID is not inline qos, then we need to set the sentinel
                    // to true, as it is the last PID
                    is_sentinel = true;
                }

                qos_size += (4 + plength);

                // Align to 4 byte boundary and prepare for next iteration
                qos_size = (qos_size + 3) & ~3;

                if (!valid || ((msg.pos + plength) > msg.length))
                {
                    return false;
                }
                else if (!is_sentinel)
                {
                    if (pid == eprosima::fastdds::dds::PID_DURABILITY)
                    {
                        std::cout << "Durability found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_DEADLINE)
                    {
                        std::cout << "Deadline found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_LATENCY_BUDGET)
                    {
                        std::cout << "Latency found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_LIVELINESS)
                    {
                        std::cout << "Liveliness found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_RELIABILITY)
                    {
                        std::cout << "Reliability found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_LIFESPAN)
                    {
                        std::cout << "Lifespan found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_USER_DATA)
                    {
                        std::cout << "User data found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_TIME_BASED_FILTER)
                    {
                        std::cout << "Time base filter found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_OWNERSHIP)
                    {
                        std::cout << "Ownership found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_DESTINATION_ORDER)
                    {
                        std::cout << "Destination Order found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_PRESENTATION)
                    {
                        std::cout << "Presentation found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_PARTITION)
                    {
                        std::cout << "Partition found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_TOPIC_DATA)
                    {
                        std::cout << "Topic data found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_DISABLE_POSITIVE_ACKS)
                    {
                        std::cout << "Disable positive acks found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_DATASHARING)
                    {
                        std::cout << "Data sharing found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_GROUP_DATA)
                    {
                        std::cout << "Group data found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_DATA_REPRESENTATION)
                    {
                        std::cout << "Data representation found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_HISTORY)
                    {
                        std::cout << "History found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_TYPE_CONSISTENCY_ENFORCEMENT)
                    {
                        std::cout << "Type consistency enforcement found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_RESOURCE_LIMITS)
                    {
                        std::cout << "Optional Resource limits found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_READER_DATA_LIFECYCLE)
                    {
                        std::cout << "Optional Reader data lifecycle found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_RTPS_RELIABLE_READER)
                    {
                        std::cout << "Optional RTPS reliable reader found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_RTPS_ENDPOINT)
                    {
                        std::cout << "Optional RTPS endpoint found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    else if (pid == eprosima::fastdds::dds::PID_READER_RESOURCE_LIMITS)
                    {
                        std::cout << "Optional Reader resource limits found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    // Delete the PID from the expected list if present
                    expected_qos_pids.erase(
                        std::remove(expected_qos_pids.begin(), expected_qos_pids.end(), pid),
                        expected_qos_pids.end());
                }
            }

            // Do not drop the packet in any case
            return false;
        };

TEST_P(DDSDataReader, LivelinessChangedStatusGet)
{
    static constexpr int32_t num_times = 3u;

    static const Duration_t lease_duration(60, 0);
    static const Duration_t announcement_period(0, 100 * 1000);

    // Create and start reader that will not invoke listener for liveliness_changed
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::liveliness_changed());
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Create a participant for the writers
    std::unique_ptr<PubSubParticipant<HelloWorldPubSubType>> writers;
    writers.reset(new PubSubParticipant<HelloWorldPubSubType>(num_times, 0, num_times, 0));
    writers->pub_topic_name(TEST_TOPIC_NAME)
            .pub_liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
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

    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_NO_DATA, reader.get_first_untaken_info(&info));

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
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, reader.get_first_untaken_info(&info));

    HelloWorld msg;
    eprosima::fastdds::dds::SampleInfo sinfo;

    //! Try getting a sample
    auto result = reader.take_next_sample((void*)&msg, &sinfo);

    //! Assert last operation
    ASSERT_EQ(result, eprosima::fastdds::dds::RETCODE_OK) << "Reader's unread count is: " << reader.get_unread_count();
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

    auto testTransport_1 = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    EntityId_t writer1_id;
    EntityId_t reader_id;

    testTransport_1->drop_data_messages_filter_ =
            [&writer1_id, &reader_id](eprosima::fastdds::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.3 Data Submessage
                EntityId_t readerID;
                EntityId_t writerID;
                SequenceNumber_t sn;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline quos
                readerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                writerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.low = eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);

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
        ASSERT_NE(eprosima::fastdds::dds::RETCODE_OK, reader.get_native_reader().get_first_untaken_info(
                    &info));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Now we send data from writer_2 with no drops and all samples shall be received.
    data = default_helloworld_data_generator(3);
    writer_2.send(data);
    reader.block_for_unread_count_of(3);

    // get_first_untaken_info() must return OK now
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            reader.get_native_reader().get_first_untaken_info(&info));
    eprosima::fastdds::dds::StackAllocatedSequence<HelloWorld, 1> data_values;
    eprosima::fastdds::dds::SampleInfoSeq sample_infos{1};
    // As get_first_untaken_info() returns OK, take() must return OK too
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
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
    eprosima::fastdds::LibrarySettings library_settings;
    library_settings.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_FULL;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);

    auto participant = DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230,
        DomainParticipantFactory::get_instance()->get_default_participant_qos(), nullptr,
        eprosima::fastdds::dds::StatusMask::none());

    eprosima::fastdds::dds::TypeSupport t_type{ new HelloWorldPubSubType() };
    ASSERT_TRUE(t_type.register_type( participant ) == eprosima::fastdds::dds::RETCODE_OK);

    auto topic = participant->create_topic( TEST_TOPIC_NAME, t_type.get_type_name(),
                    participant->get_default_topic_qos());

    // create publisher and writer
    auto publisher = participant->create_publisher( participant->get_default_publisher_qos());

    auto writer_qos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
    writer_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    auto writer = publisher->create_datawriter( topic, writer_qos );

    auto data = HelloWorld{};
    ASSERT_EQ(writer->write( &data ), eprosima::fastdds::dds::RETCODE_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // create a late joiner subscriber and reader
    auto subscriber = participant->create_subscriber( participant->get_default_subscriber_qos());
    auto reader_qos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    reader_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
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
    library_settings.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
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

/**
 * Utility class to set some values other than default to those Qos common to Topic and DataReader.
 *
 * This is a class instead of a free function to avoid linking with its TestsDataWriter counterpart.
 */
class TestsDataReaderQosCommonUtils
{
public:

    // Set common Qos values to both TopicQos and DataReaderQos
    template<typename T>
    static void set_common_qos(
            T& qos)
    {
        qos.durability_service().history_kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        qos.deadline().period = {0, 500000000};
        qos.latency_budget().duration = 0;
        qos.liveliness().kind = eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        qos.resource_limits().max_samples = 1000;
        qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
        // Representation is not on the same place in DataReaderQos and TopicQos
        set_representation_qos(qos);
        qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    }

private:

    // Set representation Qos (as it is not in the same place in DataReaderQos and TopicQos)
    template<typename T>
    static void set_representation_qos(
            T& qos);
};

// Specialization for DataReaderQos
template<>
void TestsDataReaderQosCommonUtils::set_representation_qos(
        eprosima::fastdds::dds::DataReaderQos& qos)
{
    qos.representation().m_value.push_back(
        eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
}

// Specialization for TopicQos
template<>
void TestsDataReaderQosCommonUtils::set_representation_qos(
        eprosima::fastdds::dds::TopicQos& qos)
{
    qos.representation().m_value.push_back(eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
}

/*
 * This test:
 *   1. Creates a Topic with custom Qos
 *   2. Updates the default DataReader Qos that are not in common with Topic Qos with non-default values
 *   3. Creates a DataReader with DATAREADER_QOS_USE_TOPIC_QOS
 *   4. Checks that the used Qos are the merge between the default ones and the Topic ones
 */
TEST(DDSDataReader, datareader_qos_use_topic_qos)
{
    using namespace eprosima::fastdds::dds;

    /* Create a topic with custom Qos */
    // Set Topic Qos different from default
    TopicQos topic_qos;
    TestsDataReaderQosCommonUtils::set_common_qos(topic_qos);

    // Create DomainParticipant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Create Topic
    TypeSupport type_support;
    type_support.reset(new HelloWorldPubSubType());
    type_support.register_type(participant, "HelloWorld");
    Topic* topic = participant->create_topic("HelloWorldTopic", "HelloWorld", topic_qos);

    /* Create a DataReader with modified default Qos using the Topic Qos */
    // Create the Subscriber
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    // Change default DataReader Qos (only those that are different from Topic Qos)
    DataReaderQos control_qos;
    control_qos.reader_data_lifecycle().autopurge_no_writer_samples_delay = {3, 0};
    control_qos.user_data().push_back(0);
    control_qos.endpoint().entity_id = 1;
    control_qos.reader_resource_limits().matched_publisher_allocation =
            ResourceLimitedContainerConfig::fixed_size_configuration(1u);
    control_qos.data_sharing().off();
    subscriber->set_default_datareader_qos(control_qos);

    // Create DataReader with DATAREADER_QOS_USE_TOPIC_QOS
    DataReader* reader = subscriber->create_datareader(topic, DATAREADER_QOS_USE_TOPIC_QOS);
    ASSERT_NE(reader, nullptr);

    /* Check that used Qos are the merge between the default ones and the Topic ones */
    // Set the topic values on the control DataReaderQos
    TestsDataReaderQosCommonUtils::set_common_qos(control_qos);

    // Get used DataReader Qos
    DataReaderQos test_qos = reader->get_qos();

    // Check that the Qos that are not in common with Topic Qos are correctly set as the default ones,
    // and that the rest of the Qos are left unmodified
    ASSERT_EQ(control_qos, test_qos);
}

bool validate_publication_builtin_topic_data(
        const eprosima::fastdds::rtps::PublicationBuiltinTopicData& pubdata,
        const eprosima::fastdds::dds::DataWriter& datawriter)
{
    bool ret = true;

    auto dw_qos = datawriter.get_qos();
    auto pub_qos = datawriter.get_publisher()->get_qos();

    eprosima::fastdds::rtps::BuiltinTopicKey_t dw_key, part_key;

    entity_id_to_builtin_topic_key(dw_key, datawriter.guid().entityId);
    guid_prefix_to_builtin_topic_key(part_key, datawriter.get_publisher()->get_participant()->guid().guidPrefix);

    ret &= (0 == memcmp(pubdata.key.value, dw_key.value, sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));
    ret &=
            (0 ==
            memcmp(pubdata.participant_key.value, part_key.value,
            sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));

    ret &= (pubdata.topic_name.to_string() == datawriter.get_topic()->get_name());
    ret &= (pubdata.type_name.to_string() == datawriter.get_topic()->get_type_name());

    // DataWriter Qos
    ret &= (pubdata.durability == dw_qos.durability());
    ret &= (pubdata.durability_service == dw_qos.durability_service());
    ret &= (pubdata.deadline == dw_qos.deadline());
    ret &= (pubdata.latency_budget == dw_qos.latency_budget());
    ret &= (pubdata.liveliness == dw_qos.liveliness());
    ret &= (pubdata.reliability == dw_qos.reliability());
    ret &= (pubdata.lifespan == dw_qos.lifespan());
    ret &= (
        (pubdata.user_data.size() == dw_qos.user_data().size()) &&
        (0 == memcmp(pubdata.user_data.data(), dw_qos.user_data().data(), pubdata.user_data.size())));
    ret &= (pubdata.ownership == dw_qos.ownership());
    ret &= (pubdata.ownership_strength == dw_qos.ownership_strength());
    ret &= (pubdata.destination_order == dw_qos.destination_order());

    // Publisher Qos
    ret &= (pubdata.presentation == pub_qos.presentation());
    ret &= (pubdata.partition.getNames() == pub_qos.partition().getNames());
    // topic_data not implemented
    // group_data too

    return ret;
}

/**
 * @test DDS-DR-API-GMPD-01
 *
 * get_matched_publication_data() must return RETCODE_BAD_PARAMETER
 * if the publication is not matched.
 */
TEST(DDSDataReader, datareader_get_matched_publication_data_bad_parameter)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_2(TEST_TOPIC_NAME);

    eprosima::fastdds::rtps::PublicationBuiltinTopicData pubdata;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    writer_1.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .init();
    writer_2.ownership_strength(10)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer_1.isInitialized());
    ASSERT_TRUE(writer_2.isInitialized());

    // Reader should not be matched with any writer
    reader.wait_discovery(std::chrono::seconds(2), 2);

    ASSERT_TRUE(!reader.is_matched());

    auto& native_reader = reader.get_native_reader();

    InstanceHandle_t w1_handle = writer_1.get_native_writer().get_instance_handle();
    ReturnCode_t ret = native_reader.get_matched_publication_data(pubdata, w1_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);

    InstanceHandle_t w2_handle = writer_2.get_native_writer().get_instance_handle();
    ret = native_reader.get_matched_publication_data(pubdata, w2_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
}

/**
 * @test DDS-DR-API-GMPD-02
 *
 * The operation must succeed when the publication is matched and correctly
 * retrieve the publication data. Parameterize the test for different transports.
 */
TEST_P(DDSDataReader, datareader_get_matched_publication_data_correctly_behaves)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_2(TEST_TOPIC_NAME);

    eprosima::fastdds::rtps::PublicationBuiltinTopicData w1_pubdata, w2_pubdata;

    reader.partition("*")
            .init();

    writer_1.partition("*")
            .init();
    writer_2.user_data({'u', 's', 'e', 'r', 'd', 'a', 't', 'a'})
            .partition("*")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer_1.isInitialized());
    ASSERT_TRUE(writer_2.isInitialized());

    // Reader must match with both writers
    reader.wait_discovery(std::chrono::seconds::zero(), 2);

    ASSERT_EQ(reader.get_matched(), 2u);

    auto& native_reader = reader.get_native_reader();

    InstanceHandle_t w1_handle = writer_1.get_native_writer().get_instance_handle();
    ReturnCode_t ret = native_reader.get_matched_publication_data(w1_pubdata, w1_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_TRUE(validate_publication_builtin_topic_data(w1_pubdata, writer_1.get_native_writer()));

    InstanceHandle_t w2_handle = writer_2.get_native_writer().get_instance_handle();
    ret = native_reader.get_matched_publication_data(w2_pubdata, w2_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_TRUE(validate_publication_builtin_topic_data(w2_pubdata, writer_2.get_native_writer()));
}

/**
 * @test DDS-DR-API-GMP-01
 *
 * get_matched_publications() must return RETCODE_OK
 * with an empty list if no DataWriters are matched.
 */
TEST(DDSDataReader, datareader_get_matched_publications_ok_empty_list)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_2(TEST_TOPIC_NAME);

    std::vector<InstanceHandle_t> pub_handles;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    writer_1.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .init();

    writer_2.ownership_strength(10)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer_1.isInitialized());
    ASSERT_TRUE(writer_2.isInitialized());

    // Reader should not be matched with any writer
    reader.wait_discovery(std::chrono::seconds(2), 2);
    ASSERT_FALSE(reader.is_matched());

    auto& native_reader = reader.get_native_reader();
    ReturnCode_t ret = native_reader.get_matched_publications(pub_handles);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(pub_handles.size(), 0u);
}

/**
 * @test DDS-DR-API-GMP-02
 *
 * get_matched_publications() must provide the correct list of matched publication handles.
 * Parameterize the test for different transports.
 */
TEST_P(DDSDataReader, datareader_get_matched_publications_correctly_behaves)
{
    const size_t num_writers = 5;

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    std::vector<std::unique_ptr<PubSubWriter<HelloWorldPubSubType>>> writers;
    std::vector<InstanceHandle_t> expected_pub_handles;
    std::vector<InstanceHandle_t> pub_handles;

    writers.reserve(num_writers);
    pub_handles.reserve(num_writers);

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    for (size_t i = 0; i < num_writers; ++i)
    {
        writers.emplace_back(new PubSubWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME));
        writers.back()->init();
        ASSERT_TRUE(writers.back()->isInitialized());
        expected_pub_handles.emplace_back(writers.back()->get_native_writer().get_instance_handle());
    }

    // Wait for discovery
    reader.wait_discovery(std::chrono::seconds::zero(), num_writers);
    ASSERT_EQ(reader.get_matched(), num_writers);

    auto& native_reader = reader.get_native_reader();
    ReturnCode_t ret = native_reader.get_matched_publications(pub_handles);

    // Check that the list of matched publication handles is correct
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(pub_handles.size(), num_writers);
    ASSERT_TRUE(std::is_permutation(pub_handles.begin(), pub_handles.end(), expected_pub_handles.begin()));

    // Remove two writers and check that the list of matched publication handles is updated
    writers.pop_back();
    writers.pop_back();
    expected_pub_handles.pop_back();
    expected_pub_handles.pop_back();

    // Wait for undiscovery
    reader.wait_writer_undiscovery(static_cast<unsigned int>(num_writers - 2));

    pub_handles.clear();
    ret = native_reader.get_matched_publications(pub_handles);
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(pub_handles.size(), static_cast<size_t>(num_writers - 2));
    ASSERT_TRUE(std::is_permutation(pub_handles.begin(), pub_handles.end(), expected_pub_handles.begin()));
}

/**
 * @test DDS-DR-API-GMP-03
 *
 * The operation must provide the correct list of matched publication handles in multiple
 * participants scenario. Parameterize the test for different transports.
 */
TEST_P(DDSDataReader, datareader_get_matched_publications_multiple_participants_correctly_behave)
{
    PubSubParticipant<HelloWorldPubSubType> part_1(1, 1, 1, 1);
    PubSubParticipant<HelloWorldPubSubType> part_2(1, 1, 1, 1);

    part_1.pub_topic_name(TEST_TOPIC_NAME);
    part_1.sub_topic_name(TEST_TOPIC_NAME + "_1");
    part_2.pub_topic_name(TEST_TOPIC_NAME + "_1");
    part_2.sub_topic_name(TEST_TOPIC_NAME);

    ASSERT_TRUE(part_1.init_participant());
    ASSERT_TRUE(part_1.init_publisher(0));
    ASSERT_TRUE(part_1.init_subscriber(0));

    ASSERT_TRUE(part_2.init_participant());
    ASSERT_TRUE(part_2.init_subscriber(0));
    ASSERT_TRUE(part_2.init_publisher(0));

    part_1.pub_wait_discovery();
    part_1.sub_wait_discovery();

    part_2.pub_wait_discovery();
    part_2.sub_wait_discovery();

    auto& reader_p1 = part_1.get_native_reader(0);
    auto& reader_p2 = part_2.get_native_reader(0);

    std::vector<InstanceHandle_t> pub_handles_p1;
    std::vector<InstanceHandle_t> pub_handles_p2;

    ReturnCode_t ret = reader_p1.get_matched_publications(pub_handles_p1);
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(pub_handles_p1.size(), 1u);
    ASSERT_EQ(pub_handles_p1[0], part_2.get_native_writer(0).get_instance_handle());

    ret = reader_p2.get_matched_publications(pub_handles_p2);
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(pub_handles_p2.size(), 1u);
    ASSERT_EQ(pub_handles_p2[0], part_1.get_native_writer(0).get_instance_handle());
}

// This tests checks that non-default QoS are correctly sent in the Data(r)
// Only half of the QoS are modified, so the other half should be the default ones and they should not
// be sent. The opposite scenario will be checked in the following test.
// QoS that should be sent:
// - DurabilityQosPolicy
// - DeadlineQosPolicy
// - LatencyBudgetQosPolicy
// - LivelinessQosPolicy
// - ReliabilityQosPolicy
// - LifespanQosPolicy
// - UserDataQosPolicy
// - TimeBasedFilterQosPolicy
// - OwnershipQosPolicy
// - DestinationOrderQosPolicy can NOT be tested, as it is not implemented yet so the default value cannot be modified
TEST_P(DDSDataReader, datareader_sends_non_default_qos_a)
{
    if (TRANSPORT != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on TRANSPORT";
        return;
    }

    std::atomic<uint8_t> qos_found { 0 };
    std::vector<uint16_t> expected_qos_pids = {
        eprosima::fastdds::dds::PID_DURABILITY,
        eprosima::fastdds::dds::PID_DEADLINE,
        eprosima::fastdds::dds::PID_LATENCY_BUDGET,
        eprosima::fastdds::dds::PID_LIVELINESS,
        eprosima::fastdds::dds::PID_RELIABILITY,
        eprosima::fastdds::dds::PID_LIFESPAN,
        eprosima::fastdds::dds::PID_USER_DATA,
        eprosima::fastdds::dds::PID_TIME_BASED_FILTER,
        eprosima::fastdds::dds::PID_OWNERSHIP,
    };
    const uint8_t expected_qos_size = static_cast<uint8_t>(expected_qos_pids.size());

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->drop_builtin_data_messages_filter_ = [&](rtps::CDRMessage_t& msg)
            {
                return check_qos_in_data_r(msg, qos_found, expected_qos_pids);
            };

    // Modify writer's QoS just to match the reader's
    eprosima::fastdds::dds::DataWriterQos dw_qos;
    dw_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    dw_qos.deadline().period = { 7, 0 };  // Lower than reader's
    dw_qos.latency_budget().duration = { 7, 0 };  // Lower than reader's
    dw_qos.liveliness().lease_duration = { 7, 0 };  // Lower than reader's
    dw_qos.liveliness().announcement_period = { 5, 0 };  // Lower than writer's lease duration
    dw_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    dw_qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
    dw_qos.data_sharing().off();

    eprosima::fastdds::dds::DataReaderQos dr_qos;
    dr_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    dr_qos.deadline().period = { 42, 0 };
    dr_qos.latency_budget().duration = { 42, 0 };
    dr_qos.liveliness().lease_duration = { 42, 0 };
    dr_qos.liveliness().announcement_period = { 39, 0 };
    dr_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    dr_qos.lifespan().duration = { 42, 0 };
    std::vector<eprosima::fastdds::rtps::octet> vec;
    eprosima::fastdds::rtps::octet val = 42;
    vec.push_back(val);
    dr_qos.user_data().data_vec(vec);
    dr_qos.time_based_filter().minimum_separation = { 42, 0 };
    dr_qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
    dr_qos.destination_order().kind = eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    dr_qos.data_sharing().off();

    writer.data_writer_qos(dw_qos);
    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .data_reader_qos(dr_qos);

    writer.init();
    reader.init();
    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    EXPECT_EQ(qos_found.load(), expected_qos_size);
    EXPECT_EQ(expected_qos_pids.size(), 0u);
}

// This tests checks that non-default QoS are correctly sent in the Data(r)
// Only half of the QoS are modified, so the other half should be the default ones and they should not
// be sent. The opposite scenario will be checked in the previous test.
// QoS that should be sent:
// - DurabilityQosPolicy (always sent)
// - PresentationQosPolicy
// - PartitionQosPolicy
// - GroupDataQosPolicy
// - DisablePositiveACKsQosPolicy
// - TypeConsistencyEnforcementQosPolicy
// - DataSharingQosPolicy
// - DataRepresentationQosPolicy
// - HistoryQosPolicy
TEST_P(DDSDataReader, datareader_sends_non_default_qos_b)
{
    if (TRANSPORT != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on TRANSPORT";
        return;
    }

    std::atomic<uint8_t> qos_found { 0 };
    std::vector<uint16_t> expected_qos_pids = {
        eprosima::fastdds::dds::PID_DURABILITY,
        eprosima::fastdds::dds::PID_PRESENTATION,
        eprosima::fastdds::dds::PID_PARTITION,
        eprosima::fastdds::dds::PID_GROUP_DATA,
        eprosima::fastdds::dds::PID_DISABLE_POSITIVE_ACKS,
        eprosima::fastdds::dds::PID_TYPE_CONSISTENCY_ENFORCEMENT,
        eprosima::fastdds::dds::PID_DATASHARING,
        eprosima::fastdds::dds::PID_DATA_REPRESENTATION,
        eprosima::fastdds::dds::PID_HISTORY
    };
    const uint8_t expected_qos_size = static_cast<uint8_t>(expected_qos_pids.size());

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->drop_builtin_data_messages_filter_ = [&](rtps::CDRMessage_t& msg)
            {
                return check_qos_in_data_r(msg, qos_found, expected_qos_pids);
            };

    // Modify writer's QoS just to match the reader's
    eprosima::fastdds::dds::PublisherQos pub_qos;
    pub_qos.partition().push_back("partition_1");
    eprosima::fastdds::dds::DataWriterQos dw_qos;
    dw_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    dw_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    dw_qos.reliable_writer_qos().disable_positive_acks.enabled = true;

    eprosima::fastdds::dds::SubscriberQos sub_qos;
    sub_qos.presentation().access_scope = eprosima::fastdds::dds::GROUP_PRESENTATION_QOS;
    sub_qos.partition().push_back("partition_1");
    std::vector<eprosima::fastdds::rtps::octet> vec;
    eprosima::fastdds::rtps::octet val = 42;
    vec.push_back(val);
    sub_qos.group_data().data_vec(vec);
    eprosima::fastdds::dds::DataReaderQos dr_qos;
    dr_qos.reliable_reader_qos().disable_positive_acks.enabled = true;
    dr_qos.reliable_reader_qos().disable_positive_acks.duration = { 42, 0 };
    dr_qos.representation().m_value.push_back(eprosima::fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    dr_qos.type_consistency().m_kind = eprosima::fastdds::dds::DISALLOW_TYPE_COERCION;
    dr_qos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    dr_qos.history().depth = 42;

#ifdef _WIN32
    dw_qos.data_sharing().on("c:\\programdata\\eprosima\\fastdds_interprocess\\");
    dr_qos.data_sharing().on("c:\\programdata\\eprosima\\fastdds_interprocess\\");
#elif __APPLE__
    dw_qos.data_sharing().on("/private/tmp/boost_interprocess/");
    dr_qos.data_sharing().on("/private/tmp/boost_interprocess/");
#elif __linux__
    dw_qos.data_sharing().on("/dev/shm");
    dr_qos.data_sharing().on("/dev/shm");
#else
    throw std::runtime_error(std::string("Platform not supported"));
#endif // ifdef _WIN32

    writer.publisher_qos(pub_qos)
            .data_writer_qos(dw_qos);
    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .subscriber_qos(sub_qos)
            .data_reader_qos(dr_qos);

    writer.init();
    reader.init();
    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    EXPECT_EQ(qos_found.load(), expected_qos_size);
    EXPECT_EQ(expected_qos_pids.size(), 0u);
}

// This tests checks that non-default optional QoS are correctly sent in the Data(r)
// QoS that should be sent:
// - ResourceLimitsQosPolicy
// - ReaderDataLifecycleQosPolicy
// - RTPSReliableReaderQos
// - RTPSEndpointQos
// - ReaderResourceLimitsQos
// a) The test is run with the property set to false, so the optional QoS are not serialized.
// b) The test is run with the property set to true, so the optional QoS are serialized.
// c) The test is run with the default QoS and the property set to true, so the optional QoS are not serialized.
TEST_P(DDSDataReader, datareader_sends_non_default_qos_optional)
{
    if (TRANSPORT != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on TRANSPORT";
        return;
    }

    std::atomic<uint8_t> qos_found { 0 };
    std::vector<uint16_t> expected_qos_pids = {
        eprosima::fastdds::dds::PID_RESOURCE_LIMITS,
        eprosima::fastdds::dds::PID_READER_DATA_LIFECYCLE,
        eprosima::fastdds::dds::PID_RTPS_RELIABLE_READER,
        eprosima::fastdds::dds::PID_RTPS_ENDPOINT,
        eprosima::fastdds::dds::PID_READER_RESOURCE_LIMITS,
    };
    const uint8_t expected_qos_size = static_cast<uint8_t>(expected_qos_pids.size());

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->drop_builtin_data_messages_filter_ = [&](rtps::CDRMessage_t& msg)
            {
                return check_qos_in_data_r(msg, qos_found, expected_qos_pids);
            };

    // Default writer's QoS
    eprosima::fastdds::dds::DataWriterQos dw_qos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
    dw_qos.data_sharing().off();

    eprosima::fastdds::dds::DataReaderQos dr_qos;
    dr_qos.resource_limits().max_samples = 1000;
    dr_qos.reader_data_lifecycle().autopurge_disposed_samples_delay = { 4, 0 };
    dr_qos.reliable_reader_qos().times.initial_acknack_delay = { 4, 0 };
    dr_qos.endpoint().entity_id = 42;
    dr_qos.reader_resource_limits().matched_publisher_allocation.initial = 1;
    dr_qos.data_sharing().off();

    writer.data_writer_qos(dw_qos);
    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .data_reader_qos(dr_qos);

    // a) Init both entities without setting the property
    writer.init();
    reader.init();
    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // No optional QoS should be sent. Only PID_DURABILITY as it is always sent
    EXPECT_EQ(qos_found.load(), 1u);
    EXPECT_EQ(expected_qos_pids.size(), expected_qos_size);

    // b) Now set the property to serialize optional QoS and re-init the reader
    reader.destroy();
    writer.wait_reader_undiscovery();
    qos_found.store(0);

    eprosima::fastdds::dds::PropertyPolicyQos properties;
    properties.properties().emplace_back("fastdds.serialize_optional_qos", "true");
    reader.property_policy(properties);

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // Check that the optional QoS are serialized + PID_DURABILITY as it is always sent
    EXPECT_EQ(qos_found.load(), expected_qos_size + 1u);
    EXPECT_EQ(expected_qos_pids.size(), 0u);

    // c) Now re-init the reader with default QoS and the property set
    reader.destroy();
    writer.wait_reader_undiscovery();
    qos_found.store(0);

    dr_qos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    dr_qos.data_sharing().off();

    reader.data_reader_qos(dr_qos)
            .init();
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // Check that no optional QoS are serialized. Only PID_DURABILITY as it is always sent
    EXPECT_EQ(qos_found.load(), 1u);
}

// This is a regression test to check the reception time used when Samples are lost and need to be resent.
TEST(DDSDataReader, reception_timestamp_for_resent_samples)
{
    using namespace eprosima::fastdds::dds;

    // A reliable Pub-Sub scenario will be created.
    // One sample will be filtered out to force the publisher to resend it.
    // The reception timestamp of the sample will be checked.

    class CustomPubSubReader : public PubSubReader<HelloWorldPubSubType>
    {
    public:

        CustomPubSubReader(
                const std::string& topic_name)
            : PubSubReader(topic_name)
        {
        }

        std::map<uint16_t, rtps::Time_t> reception_timestamps;

    private:

        void postprocess_sample(
                const type& sample,
                const SampleInfo& info) override final
        {
            if (info.valid_data)
            {
                reception_timestamps[sample.index()] = info.reception_timestamp;
                std::cout << "Sample " << sample.index() << " received at "
                          << info.reception_timestamp.seconds() << "." << info.reception_timestamp.nanosec()
                          << std::endl;
            }
        }

    };

    std::atomic<bool> filter_activated { false };
    auto block_data_msgs = [&filter_activated](CDRMessage_t& msg)
            {
                // Filter Data messages
                if (filter_activated.load(std::memory_order::memory_order_seq_cst))
                {
                    uint32_t old_pos = msg.pos;

                    SequenceNumber_t sn;

                    msg.pos += 2; // Flags
                    msg.pos += 2; // Octets to inline QoS
                    msg.pos += 4; // Reader ID
                    msg.pos += 4; // Writer ID
                    sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32(
                        (char*)&msg.buffer[msg.pos]);
                    msg.pos += 4;
                    sn.low = eprosima::fastdds::helpers::cdr_parse_u32(
                        (char*)&msg.buffer[msg.pos]);

                    // Restore buffer pos
                    msg.pos = old_pos;

                    // Filter only first Data sent with Sequence number 0-1
                    if (sn == SequenceNumber_t{0, 1})
                    {
                        std::cout << "Blocking Data msg of Sequence number 0-1." << std::endl;
                        return true;
                    }
                }
                return false;
            };

    // Declare a test transport that will block DATA msgs sent
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->drop_data_messages_filter_ = [&](CDRMessage_t& msg)
            {
                return block_data_msgs(msg);
            };

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    CustomPubSubReader reader(TEST_TOPIC_NAME);

    // The writer will use the test transport. Both reliable and history depth will be set to 5.
    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .reliability(ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
            .history_kind(KEEP_LAST_HISTORY_QOS)
            .history_depth(3)
            .init();
    reader.setup_transports(eprosima::fastdds::rtps::BuiltinTransports::UDPv4)
            .reliability(ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
            .history_kind(KEEP_LAST_HISTORY_QOS)
            .history_depth(3)
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    // Activate the filter and send first sample
    filter_activated.store(true, std::memory_order::memory_order_seq_cst);

    auto data = default_helloworld_data_generator(3);
    reader.startReception(data);

    auto samples_it = data.begin();
    writer.send_sample(*samples_it);
    // Ensure that the sample has not been received yet
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(1)), 0u);

    // Send the rest of the samples and then deactivate the filter
    ++samples_it;
    for (; samples_it != data.end(); ++samples_it)
    {
        writer.send_sample(*samples_it);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    filter_activated.store(false, std::memory_order::memory_order_seq_cst);
    // Wait for the reception of all samples
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(5)), 3u);

    // Check timestamps. reception_timestamps map is accesed by index of HelloWorld data
    ASSERT_EQ(reader.reception_timestamps.size(), 3u);
    auto reception_ts_1 = reader.reception_timestamps[1];
    auto reception_ts_2 = reader.reception_timestamps[2];
    auto reception_ts_3 = reader.reception_timestamps[3];
    EXPECT_TRUE(reception_ts_1 <= reception_ts_2);
    EXPECT_TRUE(reception_ts_2 <= reception_ts_3);
}

/* This is a regression test for redmine issue 22929.
 *
 * Considers the following scenario:
 * - A DataReader is created on keyed topic A
 * - A DataWriter is created on the same topic
 * - DataWriter writes sample 1 to instance 1
 * - DataReader takes sample 1
 * - DataWriter is deleted
 *
 * The following behavior is expected:
 * - Calling take on the DataReader returns a sample on instance 1 with
 *   valid_data = false to inform about the change in the instance state
 *   to NOT_ALIVE_NO_WRITERS
 */
TEST_P(DDSDataReader, return_sample_when_writer_disappears)
{
    namespace fdds = eprosima::fastdds::dds;

    fdds::InstanceHandle_t instance_handle{};

    // Create a DataReader on a keyed topic
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .init();
    ASSERT_TRUE(reader.isInitialized());
    fdds::DataReader& data_reader = reader.get_native_reader();

    // Create a DataWriter on the same topic
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // DataWriter writes sample 1 to instance 1
    {
        KeyedHelloWorldPubSubType::type sample;
        sample.key(1);
        sample.index(1);
        sample.message("Hello World");
        EXPECT_TRUE(writer.send_sample(sample));
    }

    // DataReader takes sample 1
    {
        EXPECT_TRUE(data_reader.wait_for_unread_message(fdds::c_TimeInfinite));
        fdds::SampleInfo info;
        KeyedHelloWorldPubSubType::type sample;
        EXPECT_EQ(data_reader.take_next_sample(&sample, &info), fdds::RETCODE_OK);
        EXPECT_TRUE(info.valid_data);
        EXPECT_EQ(info.instance_state, fdds::ALIVE_INSTANCE_STATE);
        EXPECT_EQ(sample.key(), 1);
        EXPECT_EQ(sample.index(), 1);
        EXPECT_EQ(sample.message(), "Hello World");

        // Store the instance handle for later use
        instance_handle = info.instance_handle;
    }

    // DataWriter is deleted
    writer.destroy();
    reader.wait_writer_undiscovery();

    // Verify expectations
    {
        fdds::SampleInfo info;
        KeyedHelloWorldPubSubType::type sample;

        EXPECT_TRUE(data_reader.get_status_changes().is_active(fdds::StatusMask::data_available()));
        EXPECT_EQ(data_reader.take_next_sample(&sample, &info), fdds::RETCODE_OK);
        EXPECT_FALSE(info.valid_data);
        EXPECT_EQ(info.instance_handle, instance_handle);
        EXPECT_EQ(info.instance_state, fdds::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE);
    }
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

