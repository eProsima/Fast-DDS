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

#include <gtest/gtest.h>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include "BlackboxTests.hpp"

#include "PubSubWriter.hpp"

#include "../types/HelloWorldTypeObject.h"

namespace eprosima {
namespace fastdds {
namespace dds {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

TEST(DDSContentFilter, BasicTest)
{
    registerHelloWorldTypes();

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.history_depth(10).init();

    auto participant = writer.getParticipant();
    ASSERT_NE(nullptr, participant);
    auto topic = static_cast<Topic*>(participant->lookup_topicdescription(writer.topic_name()));
    ASSERT_NE(nullptr, topic);
    auto filtered_topic = participant->create_contentfilteredtopic("filtered_topic", topic, "", {});
    ASSERT_NE(nullptr, filtered_topic);
    auto subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    DataReaderQos reader_qos = subscriber->get_default_datareader_qos();
    reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().depth = 10;
    auto reader = subscriber->create_datareader(filtered_topic, reader_qos);
    ASSERT_NE(nullptr, reader);

    auto send_data = [&](uint64_t expected_samples, const std::vector<uint16_t>& index_values)
            {
                // Send 10 samples with index 1 to 10
                auto data = default_helloworld_data_generator();
                writer.send(data);
                EXPECT_TRUE(data.empty());

                // Waiting for all samples to be acknowledged ensures the reader has processed all samples sent
                EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(5)));

                // Only the expected samples should have made its way into the history
                EXPECT_EQ(reader->get_unread_count(), expected_samples);

                // Take and check the received samples
                FASTDDS_CONST_SEQUENCE(HelloWorldSeq, HelloWorld);
                HelloWorldSeq recv_data;
                SampleInfoSeq recv_info;

                EXPECT_EQ(ReturnCode_t::RETCODE_OK, reader->take(recv_data, recv_info));
                EXPECT_EQ(recv_data.length(), expected_samples);
                for (HelloWorldSeq::size_type i = 0; i < recv_data.length(); ++i)
                {
                    EXPECT_EQ(index_values[i], recv_data[i].index());
                }
                EXPECT_EQ(ReturnCode_t::RETCODE_OK, reader->return_loan(recv_data, recv_info));
            };

    std::cout << std::endl << "TEST empty expression..." << std::endl;
    send_data(10u, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"2\", \"4\"}..." << std::endl;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
            filtered_topic->set_filter_expression("index BETWEEN %0 AND %1", { "2", "4" }));
    send_data(3u, {2, 3, 4});

    std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"6\", \"9\"}..." << std::endl;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
            filtered_topic->set_expression_parameters({ "6", "9" }));
    send_data(4u, {6, 7, 8, 9});

    std::cout << std::endl << "Test 'message match %0', {\"'HelloWorld 1.*'\"}..." << std::endl;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
            filtered_topic->set_filter_expression("message match %0", { "'HelloWorld 1.*'" }));
    send_data(2u, {1, 10});

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, subscriber->delete_datareader(reader));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(subscriber));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_contentfilteredtopic(filtered_topic));
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
