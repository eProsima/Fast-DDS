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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__SUBSCRIPTIONNODE_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__SUBSCRIPTIONNODE_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "./Context.hpp"
#include "./DataReaderHolder.hpp"
#include "./Node.hpp"
#include "./TopicHolder.hpp"

namespace eprosima {
namespace testing {
namespace ros2 {

using namespace eprosima::fastdds::dds;

struct SubscriptionListener : public DataReaderListener
{
    void on_liveliness_changed(
            DataReader* reader,
            const LivelinessChangedStatus& status) override
    {
        static_cast<void>(reader);

        if (liveliness_callback)
        {
            liveliness_callback(status);
        }
    }

    std::function<void(const LivelinessChangedStatus&)> liveliness_callback;
};

struct SubscriptionNode : public Node
{
    SubscriptionNode(
            const std::shared_ptr<Context>& context,
            const std::string& node_name,
            const std::string& topic_name,
            const TypeSupport& type_support)
        : Node(context, node_name)
        , topic_name_(topic_name)
        , type_support_(type_support)
        , qos_(context->subscriber()->get_default_datareader_qos())
    {
    }

    ~SubscriptionNode() override
    {
        stop();
    }

    void set_listener(
            const std::shared_ptr<SubscriptionListener>& listener)
    {
        listener_ = listener;
    }

    void set_qos(
            const DataReaderQos& qos)
    {
        qos_ = qos;
    }

protected:

    void do_start() override
    {
        context_->create_topic(topic_holder_, topic_name_, type_support_.get_type_name(), type_support_);
        context_->create_subscription(subscription_, topic_holder_, qos_, listener_.get());
    }

    void do_stop() override
    {
        context_->delete_subscription(subscription_);
        subscription_.reset();
        topic_holder_.reset();
    }

    std::string topic_name_{};
    TypeSupport type_support_{};
    DataReaderQos qos_{};
    std::shared_ptr<TopicHolder> topic_holder_{};
    std::shared_ptr<SubscriptionListener> listener_ = std::make_shared<SubscriptionListener>();
    std::shared_ptr<DataReaderHolder> subscription_{};
};

}  // namespace ros2
}  // namespace testing
}  // namespace eprosima

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__SUBSCRIPTIONNODE_HPP
