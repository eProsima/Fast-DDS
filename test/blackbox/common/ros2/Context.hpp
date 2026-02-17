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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__CONTEXT_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__CONTEXT_HPP

#include <cstdint>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

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

#include "./DataReaderHolder.hpp"
#include "./DataWriterHolder.hpp"
#include "./TopicHolder.hpp"
#include "../BlackboxTests.hpp"
#include "../../types/HelloWorldPubSubTypes.hpp"

namespace eprosima {
namespace testing {
namespace ros2 {

using namespace eprosima::fastdds::dds;

class Context
{
    using RosDiscoveryInfoPubSubType = HelloWorldPubSubType;

public:

    Context()
    {
        factory_ = DomainParticipantFactory::get_shared_instance();

        uint32_t domain_id = static_cast<uint32_t>(GET_PID() % 230);
        participant_ = factory_->create_participant(domain_id, PARTICIPANT_QOS_DEFAULT);
        EXPECT_NE(nullptr, participant_);

        if (nullptr != participant_)
        {
            publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
            EXPECT_NE(nullptr, publisher_);
            subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
            EXPECT_NE(nullptr, subscriber_);
        }

        // Create DataWriter for ros_discovery_info topic
        if (nullptr != publisher_ && nullptr != subscriber_)
        {
            TypeSupport type_support(new RosDiscoveryInfoPubSubType());
            std::shared_ptr<TopicHolder> topic_holder;
            create_topic(topic_holder, "ros_discovery_info", "rmw_dds_common::msg::dds_::ParticipantEntitiesInfo_",
                    type_support);
            {
                DataWriterQos qos = publisher_->get_default_datawriter_qos();
                qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
                qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
                qos.history().kind = KEEP_LAST_HISTORY_QOS;
                qos.history().depth = 1;
                create_publication(discovery_info_pub_, topic_holder, DATAWRITER_QOS_DEFAULT, nullptr, false);
            }
            {
                DataReaderQos qos = subscriber_->get_default_datareader_qos();
                qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
                qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
                qos.history().kind = KEEP_LAST_HISTORY_QOS;
                qos.history().depth = 100;
                create_subscription(discovery_info_sub_, topic_holder, DATAREADER_QOS_DEFAULT, nullptr, false);
            }
        }
    }

    ~Context()
    {
        discovery_info_sub_.reset();
        discovery_info_pub_.reset();
        topics_.clear();

        if (participant_ != nullptr)
        {
            participant_->delete_contained_entities();
            factory_->delete_participant(participant_);
        }
    }

    DomainParticipant* participant() const
    {
        return participant_;
    }

    Publisher* publisher() const
    {
        return publisher_;
    }

    Subscriber* subscriber() const
    {
        return subscriber_;
    }

    void create_topic(
            std::shared_ptr<TopicHolder>& topic_holder,
            const std::string& topic_name,
            const std::string& type_name,
            const TypeSupport& type_support)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        topic_holder = topics_[topic_name];
        if (!topic_holder)
        {
            EXPECT_EQ(RETCODE_OK, type_support.register_type(participant_, type_name));
            auto topic = participant_->create_topic("testing/" + topic_name, type_name, TOPIC_QOS_DEFAULT);
            ASSERT_NE(nullptr, topic);
            topic_holder = std::make_shared<TopicHolder>(participant_, topic);
            topics_[topic_name] = topic_holder;
        }
        else
        {
            EXPECT_EQ(type_name, topic_holder->topic()->get_type_name());
        }
    }

    void create_publication(
            std::shared_ptr<DataWriterHolder>& publication,
            const std::shared_ptr<TopicHolder>& topic_holder,
            const DataWriterQos& qos,
            DataWriterListener* listener,
            bool notify = true)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        ASSERT_NE(nullptr, publisher_);
        DataWriter* writer = publisher_->create_datawriter(topic_holder->topic(), qos, listener);
        ASSERT_NE(nullptr, writer);
        publication = std::make_shared<DataWriterHolder>(publisher_, writer);
        if (notify)
        {
            notify_publication_creation(publication);
        }
    }

    void delete_publication(
            std::shared_ptr<DataWriterHolder>& publication)
    {
        if (publication)
        {
            std::lock_guard<std::mutex> lock(mutex_);

            notify_publication_deletion(publication);
            publication.reset();
        }
    }

    void create_subscription(
            std::shared_ptr<DataReaderHolder>& subscription,
            const std::shared_ptr<TopicHolder>& topic_holder,
            const DataReaderQos& qos,
            DataReaderListener* listener,
            bool notify = true)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        ASSERT_NE(nullptr, subscriber_);
        DataReader* reader = subscriber_->create_datareader(topic_holder->topic(), qos, listener);
        ASSERT_NE(nullptr, reader);
        subscription = std::make_shared<DataReaderHolder>(subscriber_, reader);
        if (notify)
        {
            notify_subscription_creation(subscription);
        }
    }

    void delete_subscription(
            std::shared_ptr<DataReaderHolder>& subscription)
    {
        if (subscription)
        {
            std::lock_guard<std::mutex> lock(mutex_);

            notify_subscription_deletion(subscription);
            subscription.reset();
        }
    }

private:

    void notify_publication_creation(
            const std::shared_ptr<DataWriterHolder>& publication)
    {
        if (nullptr != discovery_info_pub_)
        {
            RosDiscoveryInfoPubSubType::type discovery_info;
            std::stringstream ss;
            ss << "Publication " << publication->writer()->guid() << " created on topic " <<
                publication->writer()->get_topic()->get_name();
            discovery_info.message(ss.str());
            discovery_info_pub_->writer()->write(&discovery_info);
        }
    }

    void notify_publication_deletion(
            const std::shared_ptr<DataWriterHolder>& publication)
    {
        if (nullptr != discovery_info_pub_)
        {
            RosDiscoveryInfoPubSubType::type discovery_info;
            std::stringstream ss;
            ss << "Publication " << publication->writer()->guid() << " deleted on topic " <<
                publication->writer()->get_topic()->get_name();
            discovery_info.message(ss.str());
            discovery_info_pub_->writer()->write(&discovery_info);
        }
    }

    void notify_subscription_creation(
            const std::shared_ptr<DataReaderHolder>& subscription)
    {
        if (nullptr != discovery_info_pub_)
        {
            RosDiscoveryInfoPubSubType::type discovery_info;
            std::stringstream ss;
            ss << "Subscription " << subscription->reader()->guid() << " created on topic " <<
                subscription->reader()->get_topicdescription()->get_name();
            discovery_info.message(ss.str());
            discovery_info_pub_->writer()->write(&discovery_info);
        }
    }

    void notify_subscription_deletion(
            const std::shared_ptr<DataReaderHolder>& subscription)
    {
        if (nullptr != discovery_info_pub_)
        {
            RosDiscoveryInfoPubSubType::type discovery_info;
            std::stringstream ss;
            ss << "Subscription " << subscription->reader()->guid() << " deleted on topic " <<
                subscription->reader()->get_topicdescription()->get_name();
            discovery_info.message(ss.str());
            discovery_info_pub_->writer()->write(&discovery_info);
        }
    }

    std::mutex mutex_;
    std::shared_ptr<DomainParticipantFactory> factory_{};
    DomainParticipant* participant_ = nullptr;
    Publisher* publisher_ = nullptr;
    Subscriber* subscriber_ = nullptr;
    std::map<std::string, std::shared_ptr<TopicHolder>> topics_{};
    std::shared_ptr<DataWriterHolder> discovery_info_pub_{};
    std::shared_ptr<DataReaderHolder> discovery_info_sub_{};
};

}  // namespace ros2
}  // namespace testing
}  // namespace eprosima

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__CONTEXT_HPP
