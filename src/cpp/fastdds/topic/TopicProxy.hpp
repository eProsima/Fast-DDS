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

/*
 * TopicProxy.hpp
 */

#ifndef _FASTDDS_TOPICPROXY_HPP_
#define _FASTDDS_TOPICPROXY_HPP_

#include <memory>
#include <string>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/topic/TopicDescriptionImpl.hpp>
#include <fastdds/topic/TopicImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;

class TopicProxy : public TopicDescriptionImpl
{
public:

    TopicProxy(
            const std::string& topic_name,
            const std::string& type_name,
            const StatusMask& mask,
            TopicImpl* impl) noexcept
        : impl_(impl)
        , user_topic_(new Topic(topic_name, type_name, this, mask))
    {
    }

    const TopicQos& get_qos() const
    {
        return impl_->get_qos();
    }

    ReturnCode_t set_qos(
            const TopicQos& qos)
    {
        return impl_->set_qos(qos);
    }

    const TopicListener* get_listener() const
    {
        return impl_->get_listener();
    }

    void set_listener(
            TopicListener* listener,
            const StatusMask& mask)
    {
        impl_->set_listener(listener, mask);
    }

    DomainParticipant* get_participant() const
    {
        return impl_->get_participant();
    }

    const TypeSupport& get_type() const
    {
        return impl_->get_type();
    }

    TopicListener* get_listener_for(
            const StatusMask& status)
    {
        return impl_->get_listener_for(status, user_topic_.get());
    }

    Topic* get_topic() const
    {
        return user_topic_.get();
    }

    const std::string& get_rtps_topic_name() const override
    {
        return user_topic_->get_name();
    }

private:

    TopicImpl* impl_ = nullptr;
    std::unique_ptr<Topic> user_topic_;
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif /* _FASTDDS_TOPICPROXY_HPP_ */
