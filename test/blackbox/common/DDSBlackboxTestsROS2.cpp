// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cstdint>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

#include "./ros2/Context.hpp"
#include "./ros2/DataReaderHolder.hpp"
#include "./ros2/DataWriterHolder.hpp"
#include "./ros2/Node.hpp"
#include "./ros2/PublicationNode.hpp"
#include "./ros2/SubscriptionNode.hpp"
#include "./ros2/TopicHolder.hpp"

#include "BlackboxTests.hpp"
#include "../types/HelloWorldPubSubTypes.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

namespace ros2 = eprosima::testing::ros2;

/**
 * This is a regression test for redmine issue #21189.
 *
 * It mimicks the behavior of the following ROS2 system test:
 * https://github.com/ros2/system_tests/blob/rolling/test_quality_of_service/test/test_liveliness.cpp
 */
TEST(DDS_ROS2, test_automatic_liveliness_changed)
{
    // Force intraprocess
    auto factory = DomainParticipantFactory::get_shared_instance();
    LibrarySettings old_library_settings;
    factory->get_library_settings(old_library_settings);
    {
        LibrarySettings library_settings;
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        factory->set_library_settings(library_settings);
    }

    {
        auto context = std::make_shared<ros2::Context>();

        auto topic_name = TEST_TOPIC_NAME;
        const dds::Duration_t liveliness_duration = { 1, 0 };
        const dds::Duration_t liveliness_announcement_period = { 0, 333333333 };
        LivelinessQosPolicy liveliness;
        liveliness.kind = LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
        liveliness.lease_duration = liveliness_duration;
        liveliness.announcement_period = liveliness_announcement_period;
        ReliabilityQosPolicy reliability;
        reliability.kind = RELIABLE_RELIABILITY_QOS;

        TypeSupport type_support(new HelloWorldPubSubType());
        auto pub = std::make_shared<ros2::PublicationNode>(context, topic_name + "/pub", topic_name, type_support);
        auto sub = std::make_shared<ros2::SubscriptionNode>(context, topic_name + "/sub", topic_name, type_support);

        // Configure the subscription node with a listener that will check the liveliness changed status.
        int total_number_of_liveliness_events = 0;
        auto sub_listener = std::make_shared<ros2::SubscriptionListener>();
        sub_listener->liveliness_callback = [&total_number_of_liveliness_events](
            const LivelinessChangedStatus& event) -> void
                {
                    total_number_of_liveliness_events++;

                    // strict checking for expected events
                    if (total_number_of_liveliness_events == 1)
                    {
                        // publisher came alive
                        ASSERT_EQ(1, event.alive_count);
                        ASSERT_EQ(0, event.not_alive_count);
                        ASSERT_EQ(1, event.alive_count_change);
                        ASSERT_EQ(0, event.not_alive_count_change);
                    }
                    else if (total_number_of_liveliness_events == 2)
                    {
                        // publisher died
                        ASSERT_EQ(0, event.alive_count);
                        ASSERT_EQ(0, event.not_alive_count);
                        ASSERT_EQ(-1, event.alive_count_change);
                        ASSERT_EQ(0, event.not_alive_count_change);
                    }
                };
        sub->set_listener(sub_listener);

        DataReaderQos reader_qos = context->subscriber()->get_default_datareader_qos();
        reader_qos.liveliness(liveliness);
        reader_qos.reliability(reliability);
        sub->set_qos(reader_qos);

        // Start the subscription node.
        sub->start();

        DataWriterQos writer_qos = context->publisher()->get_default_datawriter_qos();
        writer_qos.liveliness(liveliness);
        writer_qos.reliability(reliability);
        pub->set_qos(writer_qos);

        // Start the publication node.
        pub->start();

        // Wait some time and kill the publication node.
        HelloWorld hello;
        hello.message("Hello, World!");
        for (uint16_t i = 0; i < 10; ++i)
        {
            hello.index(i);
            pub->publish(&hello);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        pub->stop();

        // Wait some time and check that the liveliness changed status was triggered.
        std::this_thread::sleep_for(std::chrono::seconds(6));
        EXPECT_EQ(2, total_number_of_liveliness_events); // check expected number of liveliness events

        sub->stop();
    }

    factory->set_library_settings(old_library_settings);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
