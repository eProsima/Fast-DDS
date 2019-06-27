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
 * @file DomainParticipant.cpp
 *
 */

#include <fastdds/domain/DomainParticipant.hpp>

#include "DomainParticipantImpl.hpp"

using namespace eprosima;
using namespace eprosima::fastdds;

DomainParticipant::DomainParticipant()
    : impl_(nullptr)
{
}

DomainParticipant::~DomainParticipant()
{
}

bool DomainParticipant::set_listener(
        DomainParticipantListener* listener)
{
    return impl_->set_listener(listener);
}

const DomainParticipantListener* DomainParticipant::get_listener() const
{
    return impl_->get_listener();
}

Publisher* DomainParticipant::create_publisher(
        const fastdds::PublisherQos& qos,
        const fastrtps::PublisherAttributes& att,
        PublisherListener* listen)
{
    return impl_->create_publisher(qos, att, listen);
}

bool DomainParticipant::delete_publisher(
        Publisher* publisher)
{
    return impl_->delete_publisher(publisher);
}

Subscriber* DomainParticipant::create_subscriber(
        const fastdds::SubscriberQos& qos,
        const fastrtps::SubscriberAttributes& att,
        SubscriberListener* listen)
{
    return impl_->create_subscriber(qos, att, listen);
}

bool DomainParticipant::delete_subscriber(
        Subscriber* subscriber)
{
    return impl_->delete_subscriber(subscriber);
}

bool DomainParticipant::register_type(
        fastrtps::TopicDataType* type)
{
    return impl_->register_type(type);
}

bool DomainParticipant::unregister_type(
        const char* typeName)
{
    return impl_->unregister_type(typeName);
}

Subscriber* DomainParticipant::get_builtin_subscriber()
{
    return impl_->get_builtin_subscriber();
}

bool DomainParticipant::ignore_participant(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    return impl_->ignore_participant(handle);
}

bool DomainParticipant::ignore_topic(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    return impl_->ignore_topic(handle);
}

bool DomainParticipant::ignore_publication(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    return impl_->ignore_publication(handle);
}

bool DomainParticipant::ignore_subscription(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    return impl_->ignore_subscription(handle);
}

uint8_t DomainParticipant::get_domain_id() const
{
    return impl_->get_domain_id();
}

bool DomainParticipant::delete_contained_entities()
{
    return impl_->delete_contained_entities();
}

bool DomainParticipant::assert_liveliness()
{
    return impl_->assert_liveliness();
}

bool DomainParticipant::set_default_publisher_qos(
        const fastdds::PublisherQos& qos)
{
    return impl_->set_default_publisher_qos(qos);
}

const fastdds::PublisherQos& DomainParticipant::get_default_publisher_qos() const
{
    return impl_->get_default_publisher_qos();
}

bool DomainParticipant::set_default_subscriber_qos(
        const fastdds::SubscriberQos& qos)
{
    return impl_->set_default_subscriber_qos(qos);
}

const fastdds::SubscriberQos& DomainParticipant::get_default_subscriber_qos() const
{
    return impl_->get_default_subscriber_qos();
}

bool DomainParticipant::get_discovered_participants(
        std::vector<fastrtps::rtps::InstanceHandle_t>& participant_handles) const
{
    return impl_->get_discovered_participants(participant_handles);
}

bool DomainParticipant::get_discovered_topics(
        std::vector<fastrtps::rtps::InstanceHandle_t>& topic_handles) const
{
    return impl_->get_discovered_topics(topic_handles);
}

bool DomainParticipant::contains_entity(
        const fastrtps::rtps::InstanceHandle_t& handle,
        bool recursive) const
{
    return impl_->contains_entity(handle, recursive);
}

bool DomainParticipant::get_current_time(
        fastrtps::Time_t& current_time) const
{
    return impl_->get_current_time(current_time);
}

const fastrtps::rtps::RTPSParticipant* DomainParticipant::rtps_participant() const
{
    return impl_->rtps_participant();
}

fastrtps::rtps::RTPSParticipant* DomainParticipant::rtps_participant()
{
    return impl_->rtps_participant();
}

fastrtps::TopicDataType* DomainParticipant::find_type(
        const std::string& type_name) const
{
    return impl_->find_type(type_name);
}

const fastrtps::rtps::InstanceHandle_t& DomainParticipant::get_instance_handle() const
{
    return impl_->get_instance_handle();
}

const fastrtps::rtps::GUID_t& DomainParticipant::guid() const
{
    return impl_->guid();
}

const fastrtps::ParticipantAttributes& DomainParticipant::get_attributes() const
{
    return impl_->get_attributes();
}

std::vector<std::string> DomainParticipant::getParticipantNames() const
{
    return impl_->getParticipantNames();
}

bool DomainParticipant::newRemoteEndpointDiscovered(
    const fastrtps::rtps::GUID_t& partguid,
    uint16_t userId,
    fastrtps::rtps::EndpointKind_t kind)
{
    return impl_->newRemoteEndpointDiscovered(partguid, userId, kind);
}

bool DomainParticipant::get_remote_writer_info(
    const fastrtps::rtps::GUID_t& writerGuid,
    fastrtps::rtps::WriterProxyData& returnedInfo)
{
    return impl_->get_remote_writer_info(writerGuid, returnedInfo);
}

bool DomainParticipant::get_remote_reader_info(
    const fastrtps::rtps::GUID_t& readerGuid,
    fastrtps::rtps::ReaderProxyData& returnedInfo)
{
    return impl_->get_remote_reader_info(readerGuid, returnedInfo);
}

fastrtps::rtps::ResourceEvent& DomainParticipant::get_resource_event() const
{
    return impl_->get_resource_event();
}
