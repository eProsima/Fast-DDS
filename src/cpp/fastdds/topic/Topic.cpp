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
    : TopicDescription(const_cast<DomainParticipant*>(dp), topic_name.c_str(), type_name.c_str())
    , listener_(listener)
    , qos_(qos)
    , mask_(mask)
    , participant_(const_cast<DomainParticipant*>(dp))
{
    topic_att_.topicName = topic_name;
    topic_att_.topicDataType = type_name;
    topic_att_.topicKind = qos.topic_kind;
    topic_att_.historyQos = qos.history;
    topic_att_.resourceLimitsQos = qos.resource_limits;
}

Topic::Topic(
        const DomainParticipant* dp,
        fastrtps::TopicAttributes att,
        TopicListener* listener,
        const ::dds::core::status::StatusMask& mask)
    : TopicDescription(const_cast<DomainParticipant*>(dp), att.getTopicName().c_str(), att.getTopicDataType().c_str())
    , listener_(listener)
    , mask_(mask)
    , topic_att_(att)
    , participant_(const_cast<DomainParticipant*>(dp))
{
    TopicQos qos;
    qos.history = att.historyQos;
    qos.resource_limits = att.resourceLimitsQos;
    qos.topic_kind = att.topicKind;
    qos.auto_fill_type_information = att.auto_fill_type_information;
    qos.auto_fill_type_object = att.auto_fill_type_object;
    qos_ = qos;
}

fastrtps::TopicAttributes Topic::get_topic_attributes() const
{
    return topic_att_;
}

ReturnCode_t Topic::get_qos(
        TopicQos& qos) const
{
    qos = qos_;
    return ReturnCode_t::RETCODE_OK;
}

const TopicQos& Topic::get_qos() const
{
    return qos_;
}

ReturnCode_t Topic::set_qos(
        const TopicQos& qos)
{
    if (qos.checkQos())
    {
        qos_ = qos;
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
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
    return participant_;
}

std::vector<DataWriter*>* Topic::get_writers() const
{
    return const_cast<std::vector<DataWriter*>*>(&writers_);
}

std::vector<DataReader*>* Topic::get_readers() const
{
    return const_cast<std::vector<DataReader*>*>(&readers_);
}

ReturnCode_t Topic::set_instance_handle(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    handle_ = handle;
    return ReturnCode_t::RETCODE_OK;
}

fastrtps::rtps::InstanceHandle_t Topic::get_instance_handle() const
{
    return handle_;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
