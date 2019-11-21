// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 */

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>

namespace eprosima {
using ReturnCode_t = fastrtps::types::ReturnCode_t;
namespace fastdds {
namespace dds {

Topic::Topic(
        const DomainParticipant* dp,
        const std::string& topic_name,
        const std::string& type_name,
        const TopicQos& qos,
        TopicListener* listener,
        const ::dds::core::status::StatusMask& mask)
    : TopicDescription(topic_name.c_str(), type_name.c_str())
    , listener_(listener)
    , qos_(qos)
    , mask_(mask)
{
    (void)dp;
}

ReturnCode_t Topic::get_qos(
        TopicQos& qos) const
{
    qos = qos_;
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t Topic::set_qos(
        const TopicQos& qos)
{
    // TODO Check updatable
    qos_ = qos;
    return ReturnCode_t::RETCODE_OK;
}

TopicListener* Topic::get_listener() const
{
    return listener_;
}

ReturnCode_t Topic::set_listener(
        TopicListener* a_listener,
        const ::dds::core::status::StatusMask& mask)
{
    listener_ = a_listener;
    mask_ = mask;
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t Topic::get_inconsistent_topic_status(
        InconsistentTopicStatus& status) const
{
    status = status_;
    return ReturnCode_t::RETCODE_OK;
}

DomainParticipant* Topic::get_participant() const
{
    // TODO: Retrieve participant
    return nullptr;
}

}
}
}
