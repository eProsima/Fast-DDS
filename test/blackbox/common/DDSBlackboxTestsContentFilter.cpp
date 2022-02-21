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

#include "BlackboxTests.hpp"

#include "PubSubWriter.hpp"

#include "../types/HelloWorldTypeObject.h"

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <gtest/gtest.h>

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

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, subscriber->delete_datareader(reader));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(subscriber));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_contentfilteredtopic(filtered_topic));
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
