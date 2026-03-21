// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file Topic.cpp
 *
 */

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/topic/TopicProxy.hpp>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

Topic::Topic(
        const std::string& topic_name,
        const std::string& type_name,
        TopicProxy* p,
        const StatusMask& mask)
    : DomainEntity(mask)
    , TopicDescription(topic_name, type_name)
    , impl_(p)
{
}

Topic::Topic(
        DomainParticipant* dp,
        const std::string& topic_name,
        const std::string& type_name,
        const TopicQos& qos,
        TopicListener* listener,
        const StatusMask& mask)
    : DomainEntity(mask)
    , TopicDescription(topic_name, type_name)
    , impl_(dp->create_topic(topic_name, type_name, qos, listener, mask)->impl_)
{
}

Topic::~Topic()
{
}

const TopicQos& Topic::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t Topic::get_qos(
        TopicQos& qos) const
{
    qos = impl_->get_qos();
    return RETCODE_OK;
}

ReturnCode_t Topic::set_qos(
        const TopicQos& qos)
{
    return impl_->set_qos(qos);
}

const TopicListener* Topic::get_listener() const
{
    return impl_->get_listener();
}

ReturnCode_t Topic::set_listener(
        TopicListener* listener,
        const StatusMask& mask)
{
    impl_->set_listener(listener, mask);
    return RETCODE_OK;
}

DomainParticipant* Topic::get_participant() const
{
    return impl_->get_participant();
}

ReturnCode_t Topic::get_inconsistent_topic_status(
        InconsistentTopicStatus& status)
{
    // TODO: return impl_->get_inconsistent_topic_status(status);
    (void)status;
    return RETCODE_UNSUPPORTED;
}

TopicDescriptionImpl* Topic::get_impl() const
{
    return impl_;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

