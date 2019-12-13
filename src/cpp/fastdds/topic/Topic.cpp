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
#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>

namespace eprosima {
using ReturnCode_t = fastrtps::types::ReturnCode_t;
namespace fastdds {
namespace dds {

Topic::Topic(
        TopicImpl* impl,
        const ::dds::core::status::StatusMask& /*mask*/)
    : TopicDescription(impl->get_participant(), impl->get_topic_attributes().topicName.c_str(),
            impl->get_topic_attributes().topicDataType.c_str())
    , impl_(impl)
{
}

Topic::Topic(
        DomainParticipant* dp,
        const std::string& topic_name,
        const std::string& type_name,
        const TopicQos& qos,
        TopicListener* listener,
        const ::dds::core::status::StatusMask& mask)
    : DomainEntity(mask)
    , TopicDescription(dp, topic_name.c_str(), type_name.c_str())
    , impl_(dp->create_topic(topic_name, type_name, qos, listener, mask)->impl_)
{
}

fastrtps::TopicAttributes Topic::get_topic_attributes() const
{
    return impl_->get_topic_attributes();
}

ReturnCode_t Topic::get_qos(
        TopicQos& qos) const
{
    return impl_->get_qos(qos);
}

const TopicQos& Topic::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t Topic::set_qos(
        const TopicQos& qos)
{
    return impl_->set_qos(qos);
}

TopicListener* Topic::get_listener() const
{
    return impl_->get_listener();
}

ReturnCode_t Topic::set_listener(
        TopicListener* a_listener,
        const ::dds::core::status::StatusMask& /*mask*/)
{
    return impl_->set_listener(a_listener);
}

DomainParticipant* Topic::get_participant() const
{
    return impl_->get_participant();
}

std::vector<DataWriter*>* Topic::get_writers()
{
    return impl_->get_writers();
}

std::vector<DataReader*>* Topic::get_readers()
{
    return impl_->get_readers();
}

ReturnCode_t Topic::set_instance_handle(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    DomainEntity::set_instance_handle(handle);
    return ReturnCode_t::RETCODE_OK;
}

fastrtps::rtps::GUID_t Topic::get_guid() const
{
    return fastrtps::rtps::iHandle2GUID(get_instance_handle());
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
