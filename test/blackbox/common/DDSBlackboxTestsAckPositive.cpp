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

#include <atomic>
#include <condition_variable>
#include <gmock/gmock-matchers.h>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/core/policy/ParameterSerializer.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/types/TypesBase.h>

#include "BlackboxTests.hpp"
#include "../api/dds-pim/CustomPayloadPool.hpp"
#include "../api/dds-pim/PubSubReader.hpp"
#include "../api/dds-pim/PubSubWriter.hpp"
#include "../api/dds-pim/PubSubWriterReader.hpp"
#include "../types/FixedSized.h"
#include "../types/FixedSizedPubSubTypes.h"
#include "../types/HelloWorldPubSubTypes.h"


TEST(AcknackQos, DDSEnableUpdatabilityOfPositiveAcksPeriodDDSLayer)
{
    // This test checks the behaviour of disabling positive ACKs.
    // It also checks that only the positive ACKs 
    // period is updatable on run time through set_qos. 

    PubSubWriter<HelloWorldPubSubType> publisher(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> subscriber(TEST_TOPIC_NAME);

    // Configure datapublisher_qos
    publisher.keep_duration({1, 0});
    publisher.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    publisher.durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS);
    publisher.init();

    ASSERT_TRUE(publisher.isInitialized());

    // Configure datasubscriber_qos
    subscriber.keep_duration({1, 0});
    subscriber.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    subscriber.init();

    ASSERT_TRUE(subscriber.isInitialized());

    // Check correct initialitation
    eprosima::fastdds::dds::DataWriterQos get_att = publisher.get_qos();
    EXPECT_TRUE(get_att.reliable_writer_qos().disable_positive_acks.enabled);
    EXPECT_EQ(get_att.reliable_writer_qos().disable_positive_acks.duration, eprosima::fastrtps::Duration_t({1,0}));

    // Wait for discovery.
    publisher.wait_discovery();
    subscriber.wait_discovery();

    auto data = default_helloworld_data_generator();

    subscriber.startReception(data);
    // Send data
    publisher.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    subscriber.block_for_all();
    // Wait for all acked msgs
    EXPECT_TRUE(publisher.waitForAllAcked(std::chrono::milliseconds(1200)));

    // Wait to disable timer because no new messages are sent
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    // Send a new message to check that timer is restarted correctly
    data = default_helloworld_data_generator(1);
    subscriber.startReception(data);
    publisher.send(data);
    ASSERT_TRUE(data.empty());
    subscriber.block_for_all();
    EXPECT_TRUE(publisher.waitForAllAcked(std::chrono::milliseconds(1200)));

    // Update attributes on DDS layer
    eprosima::fastdds::dds::DataWriterQos w_att = publisher.get_qos();
    w_att.reliable_writer_qos().disable_positive_acks.enabled = true; 
    w_att.reliable_writer_qos().disable_positive_acks.duration = eprosima::fastrtps::Duration_t({2,0});

    EXPECT_TRUE(publisher.set_qos(w_att));

    // Check that period has been changed in DataWriterQos
    get_att = publisher.get_qos();
    EXPECT_TRUE(get_att.reliable_writer_qos().disable_positive_acks.enabled);
    EXPECT_EQ(get_att.reliable_writer_qos().disable_positive_acks.duration, eprosima::fastrtps::Duration_t({2,0}));

    data = default_helloworld_data_generator();

    subscriber.startReception(data);
    // Send data
    publisher.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    subscriber.block_for_all();
    // Check that period has been correctly updated
    EXPECT_FALSE(publisher.waitForAllAcked(std::chrono::milliseconds(1200)));
    EXPECT_TRUE(publisher.waitForAllAcked(std::chrono::milliseconds(1200)));

    // Try to disable positive_acks
    w_att.reliable_writer_qos().disable_positive_acks.enabled = false; 
    
    // Check that is not possible to change disable_positive_acks on run time
    EXPECT_FALSE(publisher.set_qos(w_att));    
}