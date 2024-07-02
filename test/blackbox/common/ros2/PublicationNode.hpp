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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__PUBLICATIONNODE_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__PUBLICATIONNODE_HPP

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "./Context.hpp"
#include "./DataWriterHolder.hpp"
#include "./Node.hpp"
#include "./TopicHolder.hpp"

namespace eprosima {
namespace testing {
namespace ros2 {

using namespace eprosima::fastdds::dds;

struct PublicationListener : public DataWriterListener
{
};

struct PublicationNode : public Node
{
    PublicationNode(
            const std::shared_ptr<Context>& context,
            const std::string& node_name,
            const std::string& topic_name,
            const TypeSupport& type_support)
        : Node(context, node_name)
        , topic_name_(topic_name)
        , type_support_(type_support)
        , qos_(context->publisher()->get_default_datawriter_qos())
    {
    }

    ~PublicationNode() override
    {
        stop();
    }

    void set_qos(
            const DataWriterQos& qos)
    {
        qos_ = qos;
    }

    void publish(
            void* data)
    {
        std::lock_guard<std::mutex> _(mutex_);

        if (!started_)
        {
            return;
        }

        if (!publication_)
        {
            std::cout << "Publication not created yet" << std::endl;
            return;
        }

        auto writer = publication_->writer();
        ASSERT_NE(nullptr, writer);
        writer->write(data);
    }

protected:

    void do_start() override
    {
        context_->create_topic(topic_holder_, topic_name_, type_support_.get_type_name(), type_support_);
        context_->create_publication(publication_, topic_holder_, qos_, listener_.get());
        ASSERT_NE(nullptr, publication_);
    }

    void do_stop() override
    {
        context_->delete_publication(publication_);
        publication_.reset();
        topic_holder_.reset();
    }

    std::string topic_name_{};
    TypeSupport type_support_{};
    DataWriterQos qos_{};
    std::shared_ptr<TopicHolder> topic_holder_{};
    std::shared_ptr<PublicationListener> listener_ = std::make_shared<PublicationListener>();
    std::shared_ptr<DataWriterHolder> publication_{};
};

}  // namespace ros2
}  // namespace testing
}  // namespace eprosima

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__PUBLICATIONNODE_HPP
