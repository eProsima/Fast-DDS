// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <condition_variable>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "types/StringTestPubSubTypes.h"
#include "types/FixedSizedPubSubTypes.h"

using namespace eprosima::fastdds::dds;

constexpr const int TEST_DOMAIN = 13;
constexpr const int MESSAGES_TO_SEND = 2;
constexpr const char* TEST_TOPIC_NAME = "some_random_topic_name";

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

template <typename PubSubType>
void execute_test(bool intraprocess, bool reliable_transient, bool datasharing, bool keep_all)
{
    // Wait for reader to have notified the data
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::atomic<int> received(0);
    int messages_to_send = MESSAGES_TO_SEND;

    // Disable intraprocess
    if (!intraprocess)
    {
        eprosima::fastrtps::LibrarySettingsAttributes att;
        att.intraprocess_delivery = eprosima::fastrtps::IntraprocessDeliveryType::INTRAPROCESS_OFF;
        eprosima::fastrtps::xmlparser::XMLProfileManager::library_settings(att);
    }

    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Create participant
    DomainParticipant* participant_pub = factory->create_participant(TEST_DOMAIN, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_pub);
    DomainParticipant* participant_sub = factory->create_participant(TEST_DOMAIN, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_sub);

    // Register type
    TypeSupport type_support;
    type_support.reset(new PubSubType());
    type_support.register_type(participant_pub);
    type_support.register_type(participant_sub);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant_sub->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant_pub->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic_pub = participant_pub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    Topic* topic_sub = participant_sub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic_pub);
    ASSERT_NE(nullptr, topic_sub);

    // Set QoS
    DataWriterQos dw_qos;
    DataReaderQos dr_qos;

    if (!datasharing)
    {
        dw_qos.data_sharing().off();
        dr_qos.data_sharing().off();
    }

    if (reliable_transient)
    {
        dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

        dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    }

    if (keep_all)
    {
        dw_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
        dr_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    }
    else
    {
        dw_qos.history().depth = messages_to_send * 3;
        dr_qos.history().depth = messages_to_send * 3;
    }


    struct CustomReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        CustomReaderListener(std::condition_variable& cv, std::mutex& cv_mutex, std::atomic<int>& received)
            : cv_(cv)
            , cv_mutex_(cv_mutex)
            , received_(received)
        {}

        void on_data_available(
            DataReader* reader) override
        {
            {
                std::unique_lock<std::mutex> lock(cv_mutex_);
                received_++;
            }
            std::cout << "DEBUG: Notifying data " << received_ << std::endl;
            cv_.notify_all();
            ASSERT_EQ(received_, reader->get_unread_count());
        }

        std::condition_variable& cv_;
        std::mutex& cv_mutex_;
        std::atomic<int>& received_;
    };
    CustomReaderListener listener(cv, cv_mutex, received);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic_pub, dw_qos);
    ASSERT_NE(nullptr, writer);

    // Create reader
    DataReader* reader = subscriber->create_datareader(topic_sub, dr_qos, &listener);
    ASSERT_NE(nullptr, reader);

    // Give time to match correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create data and send
    for (int i=0; i<messages_to_send; i++)
    {
        auto data = type_support->createData();
        std::cout << "DEBUG: Sending data " << i << std::endl;
        writer->write(data);
        type_support->deleteData(data);
    }

    // Wait to reader notifying the data
    {
        std::unique_lock<std::mutex> lock(cv_mutex);
        cv.wait(lock, [&received, messages_to_send, reader](){ return received.load() >= messages_to_send; });
    }

    // Check that has been received and can be read correctly
    ASSERT_EQ(received, reader->get_unread_count());
    for (int i=0; i<messages_to_send; i++)
    {
        auto data_received = type_support->createData();
        eprosima::fastdds::dds::SampleInfo info;
        std::cout << "DEBUG: Reading data " << i << std::endl;
        auto result = reader->take_next_sample(data_received, &info);
        ASSERT_EQ(result, ReturnCode_t::RETCODE_OK);
        type_support->deleteData(data_received);
    }

    // Return to 0 receives
    received.store(0);
    // Resend data
    for (int i=0; i<messages_to_send; i++)
    {
        auto data = type_support->createData();
        std::cout << "DEBUG: Sending data " << i << std::endl;
        writer->write(data);
        type_support->deleteData(data);
    }

    // Wait to reader notifying the data
    {
        std::unique_lock<std::mutex> lock(cv_mutex);
        cv.wait(lock, [&received, messages_to_send, reader](){ return received.load() >= messages_to_send; });
    }

    // Remove writer
    publisher->delete_datawriter(writer);

    // Wait for it to be removed consistently
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Check that has been received and can be read correctly
    ASSERT_EQ(received, reader->get_unread_count());
    for (int i=0; i<messages_to_send; i++)
    {
        auto data_received = type_support->createData();
        eprosima::fastdds::dds::SampleInfo info;
        std::cout << "DEBUG: Reading data " << i << std::endl;
        auto result = reader->take_next_sample(data_received, &info);
        ASSERT_EQ(result, ReturnCode_t::RETCODE_OK);
        type_support->deleteData(data_received);
    }

    // Close everything correctly
    subscriber->delete_datareader(reader);
    // publisher->delete_datawriter(writer);

    participant_sub->delete_subscriber(subscriber);
    participant_pub->delete_publisher(publisher);

    participant_sub->delete_topic(topic_sub);
    participant_pub->delete_topic(topic_pub);

    factory->delete_participant(participant_sub);
    factory->delete_participant(participant_pub);
}

TEST(basic_dds_tests, test_fixed_intraprocess_reliable_datasharing)
{
    std::cout << "DEBUG: fixed_intraprocess_reliable_datasharing" << std::endl;
    execute_test<FixedSizedPubSubType>(
        true,  // intraprocess
        true,  // reliable
        true,  // datasharing
        false  // keep all
    );
}

TEST(basic_dds_tests, test_string_intraprocess_reliable_datasharing)
{
    std::cout << "DEBUG: string_intraprocess_reliable_datasharing" << std::endl;
    execute_test<StringTestPubSubType>(
        true,  // intraprocess
        true,  // reliable
        true,  // datasharing
        false  // keep all
    );
}

TEST(basic_dds_tests, test_fixed_intraprocess_reliable)
{
    std::cout << "DEBUG: fixed_intraprocess_reliable" << std::endl;
    execute_test<FixedSizedPubSubType>(
        true,  // intraprocess
        true,  // reliable
        false,  // datasharing
        false  // keep all
    );
}

TEST(basic_dds_tests, test_string_intraprocess_reliable)
{
    std::cout << "DEBUG: string_intraprocess_reliable" << std::endl;
    execute_test<StringTestPubSubType>(
        true,  // intraprocess
        true,  // reliable
        false,  // datasharing
        false  // keep all
    );
}

TEST(basic_dds_tests, test_fixed_reliable)
{
    std::cout << "DEBUG: fixed_reliable" << std::endl;
    execute_test<FixedSizedPubSubType>(
        false,  // intraprocess
        true,  // reliable
        false,  // datasharing
        false  // keep all
    );
}

TEST(basic_dds_tests, test_strings_reliable)
{
    std::cout << "DEBUG: strings_reliable" << std::endl;
    execute_test<StringTestPubSubType>(
        false,  // intraprocess
        true,  // reliable
        false,  // datasharing
        false  // keep all
    );
}

TEST(basic_dds_tests, test_fixed_datasharing)
{
    std::cout << "DEBUG: fixed" << std::endl;
    execute_test<FixedSizedPubSubType>(
        false,  // intraprocess
        false,  // reliable
        true,  // datasharing
        false  // keep all
    );
}

TEST(basic_dds_tests, test_string_datasharing)
{
    std::cout << "DEBUG: string" << std::endl;
    execute_test<StringTestPubSubType>(
        false,  // intraprocess
        false,  // reliable
        true,  // datasharing
        false  // keep all
    );
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
