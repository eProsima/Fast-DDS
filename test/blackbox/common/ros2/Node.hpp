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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__NODE_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__NODE_HPP

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "./Context.hpp"
#include "./DataReaderHolder.hpp"
#include "./DataWriterHolder.hpp"
#include "./TopicHolder.hpp"
#include "../../types/HelloWorldPubSubTypes.hpp"
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

namespace eprosima {
namespace testing {
namespace ros2 {

using namespace eprosima::fastdds::dds;

class Node
{
    using BuiltinPubSubType = HelloWorldPubSubType;

public:

    Node(
            const std::shared_ptr<Context>& context,
            const std::string& node_name)
        : context_(context)
        , node_name_(node_name)
    {
    }

    virtual ~Node() = default;

    void start()
    {
        std::lock_guard<std::mutex> _(mutex_);

        if (started_)
        {
            return;
        }

        started_ = true;
        create_builtin();
        do_start();
    }

    void stop()
    {
        std::lock_guard<std::mutex> _(mutex_);

        if (!started_)
        {
            return;
        }

        started_ = false;
        do_stop();
        destroy_builtin();
    }

protected:

    std::shared_ptr<Context> context_{};
    std::string node_name_{};

    mutable std::mutex mutex_{};
    bool started_ = false;

    virtual void do_start() = 0;

    virtual void do_stop() = 0;

private:

    enum class EndpointKind
    {
        PUB_ONLY,
        SUB_ONLY,
        PUB_AND_SUB
    };

    void create_builtin()
    {
        create_rosout_pub();
        // "rt/rosout", "rcl_interfaces::msg::dds_::Log_", KEEP_LAST(1000), RELIABLE, TRANSIENT_LOCAL, WRITER_ONLY
        create_parameters_server();
        create_type_description_server();
    }

    void create_rosout_pub()
    {
        create_builtin_topic("rt/rosout", "rcl_interfaces::msg::dds_::Log_",
                RELIABLE_RELIABILITY_QOS, TRANSIENT_LOCAL_DURABILITY_QOS, EndpointKind::PUB_ONLY);
    }

    void create_parameters_server()
    {
        create_service("get_parameters", "rcl_interfaces::srv::dds_::GetParameters");
        create_service("get_parameter_types", "rcl_interfaces::srv::dds_::GetParameterTypes");
        create_service("set_parameters", "rcl_interfaces::srv::dds_::SetParameters");
        create_service("set_parameters_atomically", "rcl_interfaces::srv::dds_::SetParametersAtomically");
        create_service("describe_parameters", "rcl_interfaces::srv::dds_::DescribeParameters");
        create_service("list_parameters", "rcl_interfaces::srv::dds_::ListParameters");
        create_builtin_topic("rt/parameter_events", "rcl_interfaces::msg::dds_::ParameterEvent_",
                RELIABLE_RELIABILITY_QOS, VOLATILE_DURABILITY_QOS, EndpointKind::PUB_AND_SUB);
    }

    void create_type_description_server()
    {
        create_service("get_type_description", "type_description_interfaces::srv::dds_::GetTypeDescription");
    }

    void create_service(
            const std::string& service_name,
            const std::string& service_type)
    {
        create_builtin_topic("rq/" + node_name_ + "/" + service_name + "Request", service_type + "_Request_",
                RELIABLE_RELIABILITY_QOS, VOLATILE_DURABILITY_QOS, EndpointKind::SUB_ONLY);
        create_builtin_topic("rr/" + node_name_ + "/" + service_name + "Reply", service_type + "_Response_",
                RELIABLE_RELIABILITY_QOS, VOLATILE_DURABILITY_QOS, EndpointKind::PUB_ONLY);
    }

    void create_builtin_topic(
            const std::string& topic_name,
            const std::string& type_name,
            ReliabilityQosPolicyKind reliability,
            DurabilityQosPolicyKind durability,
            EndpointKind kind)
    {
        TypeSupport type_support(new BuiltinPubSubType());
        std::shared_ptr<TopicHolder> topic_holder;
        context_->create_topic(topic_holder, topic_name, type_name, type_support);
        if (kind == EndpointKind::PUB_ONLY || kind == EndpointKind::PUB_AND_SUB)
        {
            DataWriterQos qos = context_->publisher()->get_default_datawriter_qos();
            qos.reliability().kind = reliability;
            qos.durability().kind = durability;
            qos.history().kind = KEEP_LAST_HISTORY_QOS;
            qos.history().depth = 1000;
            std::shared_ptr<DataWriterHolder> writer;
            context_->create_publication(writer, topic_holder, qos, nullptr, true);
            builtin_writers_.push_back(writer);
        }
        if (kind == EndpointKind::SUB_ONLY || kind == EndpointKind::PUB_AND_SUB)
        {
            DataReaderQos qos = context_->subscriber()->get_default_datareader_qos();
            qos.reliability().kind = reliability;
            qos.durability().kind = durability;
            qos.history().kind = KEEP_LAST_HISTORY_QOS;
            qos.history().depth = 1000;
            std::shared_ptr<DataReaderHolder> reader;
            context_->create_subscription(reader, topic_holder, qos, nullptr, true);
            builtin_readers_.push_back(reader);
        }
        builtin_topics_.push_back(topic_holder);
    }

    void destroy_builtin()
    {
        for (auto& reader : builtin_readers_)
        {
            context_->delete_subscription(reader);
        }
        builtin_readers_.clear();

        for (auto& writer : builtin_writers_)
        {
            context_->delete_publication(writer);
        }
        builtin_writers_.clear();

        builtin_topics_.clear();
    }

    std::vector<std::shared_ptr<DataReaderHolder>> builtin_readers_;
    std::vector<std::shared_ptr<DataWriterHolder>> builtin_writers_;
    std::vector<std::shared_ptr<TopicHolder>> builtin_topics_;
};

}  // namespace ros2
}  // namespace testing
}  // namespace eprosima

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__NODE_HPP
