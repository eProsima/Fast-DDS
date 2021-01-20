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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

using namespace eprosima;
using namespace eprosima::fastdds::dds;

DomainParticipant::DomainParticipant(
        const StatusMask& mask)
    : Entity(mask)
    , impl_(nullptr)
{
}

DomainParticipant::~DomainParticipant()
{
    if (impl_)
    {
        DomainParticipantFactory::get_instance()->participant_has_been_deleted(impl_);
        impl_->participant_ = nullptr;
        delete impl_;
        impl_ = nullptr;
    }
}

ReturnCode_t DomainParticipant::get_qos(
        DomainParticipantQos& qos) const
{
    return impl_->get_qos(qos);
}

const DomainParticipantQos& DomainParticipant::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t DomainParticipant::set_qos(
        const DomainParticipantQos& qos) const
{
    return impl_->set_qos(qos);
}

const DomainParticipantListener* DomainParticipant::get_listener() const
{
    return impl_->get_listener();
}

ReturnCode_t DomainParticipant::set_listener(
        DomainParticipantListener* listener)
{
    return set_listener(listener, StatusMask::all());
}

ReturnCode_t DomainParticipant::set_listener(
        DomainParticipantListener* listener,
        const StatusMask& mask)
{
    ReturnCode_t ret_val = impl_->set_listener(listener);
    if (ret_val == ReturnCode_t::RETCODE_OK)
    {
        status_mask_ = mask;
    }

    return ret_val;
}

ReturnCode_t DomainParticipant::enable()
{
    if (enable_)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    enable_ = true;
    ReturnCode_t ret_code = impl_->enable();
    enable_ = !!ret_code;
    return ret_code;
}

Publisher* DomainParticipant::create_publisher(
        const PublisherQos& qos,
        PublisherListener* listener,
        const StatusMask& mask)
{
    return impl_->create_publisher(qos, listener, mask);
}

Publisher* DomainParticipant::create_publisher_with_profile(
        const std::string& profile_name,
        PublisherListener* listener,
        const StatusMask& mask)
{
    return impl_->create_publisher_with_profile(profile_name, listener, mask);
}

ReturnCode_t DomainParticipant::delete_publisher(
        Publisher* publisher)
{
    return impl_->delete_publisher(publisher);
}

Subscriber* DomainParticipant::create_subscriber(
        const SubscriberQos& qos,
        SubscriberListener* listener,
        const StatusMask& mask)
{
    return impl_->create_subscriber(qos, listener, mask);
}

Subscriber* DomainParticipant::create_subscriber_with_profile(
        const std::string& profile_name,
        SubscriberListener* listener,
        const StatusMask& mask)
{
    return impl_->create_subscriber_with_profile(profile_name, listener, mask);
}

ReturnCode_t DomainParticipant::delete_subscriber(
        Subscriber* subscriber)
{
    return impl_->delete_subscriber(subscriber);
}

Topic* DomainParticipant::create_topic(
        const std::string& topic_name,
        const std::string& type_name,
        const TopicQos& qos,
        TopicListener* listener,
        const StatusMask& mask)
{
    return impl_->create_topic(topic_name, type_name, qos, listener, mask);
}

Topic* DomainParticipant::create_topic_with_profile(
        const std::string& topic_name,
        const std::string& type_name,
        const std::string& profile_name,
        TopicListener* listener,
        const StatusMask& mask)
{
    return impl_->create_topic_with_profile(topic_name, type_name, profile_name, listener, mask);
}

ReturnCode_t DomainParticipant::delete_topic(
        Topic* topic)
{
    return impl_->delete_topic(topic);
}

ContentFilteredTopic* DomainParticipant::create_contentfilteredtopic(
        const std::string& name,
        const Topic* related_topic,
        const std::string& filter_expression,
        const std::vector<std::string>& expression_parameters)
{
    static_cast<void> (name);
    static_cast<void> (related_topic);
    static_cast<void> (filter_expression);
    static_cast<void> (expression_parameters);
    return nullptr;
}

ReturnCode_t DomainParticipant::delete_contentfilteredtopic(
        const ContentFilteredTopic* a_contentfilteredtopic)
{
    static_cast<void> (a_contentfilteredtopic);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

MultiTopic* DomainParticipant::create_multitopic(
        const std::string& name,
        const std::string& type_name,
        const std::string& subscription_expression,
        const std::vector<std::string>& expression_parameters)
{
    static_cast<void> (name);
    static_cast<void> (type_name);
    static_cast<void> (subscription_expression);
    static_cast<void> (expression_parameters);
    return nullptr;
}

ReturnCode_t DomainParticipant::delete_multitopic(
        const MultiTopic* a_multitopic)
{
    static_cast<void> (a_multitopic);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

Topic* DomainParticipant::find_topic(
        const std::string& topic_name,
        const fastrtps::Duration_t& timeout)
{
    static_cast<void> (topic_name);
    static_cast<void> (timeout);
    return nullptr;
}

TopicDescription* DomainParticipant::lookup_topicdescription(
        const std::string& topic_name) const
{
    return impl_->lookup_topicdescription(topic_name);
}

const Subscriber* DomainParticipant::get_builtin_subscriber() const
{
    return nullptr;
}

ReturnCode_t DomainParticipant::ignore_participant(
        const InstanceHandle_t& handle)
{
    static_cast<void> (handle);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::ignore_topic(
        const InstanceHandle_t& handle)
{
    static_cast<void> (handle);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::ignore_publictaion(
        const InstanceHandle_t& handle)
{
    static_cast<void> (handle);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::ignore_subscription(
        const InstanceHandle_t& handle)
{
    static_cast<void> (handle);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

DomainId_t DomainParticipant::get_domain_id() const
{
    return impl_->get_domain_id();
}

ReturnCode_t DomainParticipant::delete_contained_entities()
{
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::assert_liveliness()
{
    return impl_->assert_liveliness();
}

ReturnCode_t DomainParticipant::set_default_publisher_qos(
        const PublisherQos& qos)
{
    return impl_->set_default_publisher_qos(qos);
}

const PublisherQos& DomainParticipant::get_default_publisher_qos() const
{
    return impl_->get_default_publisher_qos();
}

ReturnCode_t DomainParticipant::get_default_publisher_qos(
        PublisherQos& qos) const
{
    qos = impl_->get_default_publisher_qos();
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DomainParticipant::get_publisher_qos_from_profile(
        const std::string& profile_name,
        PublisherQos& qos) const
{
    return impl_->get_publisher_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::set_default_subscriber_qos(
        const SubscriberQos& qos)
{
    return impl_->set_default_subscriber_qos(qos);
}

const SubscriberQos& DomainParticipant::get_default_subscriber_qos() const
{
    return impl_->get_default_subscriber_qos();
}

ReturnCode_t DomainParticipant::get_default_subscriber_qos(
        SubscriberQos& qos) const
{
    qos = impl_->get_default_subscriber_qos();
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DomainParticipant::get_subscriber_qos_from_profile(
        const std::string& profile_name,
        SubscriberQos& qos) const
{
    return impl_->get_subscriber_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::set_default_topic_qos(
        const TopicQos& qos)
{
    return impl_->set_default_topic_qos(qos);
}

const TopicQos& DomainParticipant::get_default_topic_qos() const
{
    return impl_->get_default_topic_qos();
}

ReturnCode_t DomainParticipant::get_default_topic_qos(
        TopicQos& qos) const
{
    qos = impl_->get_default_topic_qos();
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DomainParticipant::get_topic_qos_from_profile(
        const std::string& profile_name,
        TopicQos& qos) const
{
    return impl_->get_topic_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::get_discovered_participants(
        std::vector<InstanceHandle_t>& participant_handles) const
{
    static_cast<void> (participant_handles);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::get_discovered_participant_data(
        ParticipantBuiltinTopicData& participant_data,
        const InstanceHandle_t& participant_handle) const
{
    static_cast<void> (participant_data);
    static_cast<void> (participant_handle);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::get_discovered_topics(
        std::vector<InstanceHandle_t>& topic_handles) const
{
    static_cast<void> (topic_handles);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::get_discovered_topic_data(
        TopicBuiltinTopicData& topic_data,
        const InstanceHandle_t& topic_handle) const
{
    static_cast<void> (topic_data);
    static_cast<void> (topic_handle);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

bool DomainParticipant::contains_entity(
        const InstanceHandle_t& a_handle,
        bool recursive /* = true */) const
{
    return impl_->contains_entity(a_handle, recursive);
}

ReturnCode_t DomainParticipant::get_current_time(
        fastrtps::Time_t& current_time) const
{
    return impl_->get_current_time(current_time);
}

ReturnCode_t DomainParticipant::register_type(
        TypeSupport type,
        const std::string& type_name)
{
    return impl_->register_type(type, type_name);
}

ReturnCode_t DomainParticipant::register_type(
        TypeSupport type)
{
    return impl_->register_type(type, type.get_type_name());
}

ReturnCode_t DomainParticipant::unregister_type(
        const std::string& typeName)
{
    return impl_->unregister_type(typeName);
}

TypeSupport DomainParticipant::find_type(
        const std::string& type_name) const
{
    return impl_->find_type(type_name);
}

const InstanceHandle_t& DomainParticipant::get_instance_handle() const
{
    return impl_->get_instance_handle();
}

const fastrtps::rtps::GUID_t& DomainParticipant::guid() const
{
    return impl_->guid();
}

std::vector<std::string> DomainParticipant::get_participant_names() const
{
    return impl_->get_participant_names();
}

bool DomainParticipant::new_remote_endpoint_discovered(
        const fastrtps::rtps::GUID_t& partguid,
        uint16_t userId,
        fastrtps::rtps::EndpointKind_t kind)
{
    return impl_->new_remote_endpoint_discovered(partguid, userId, kind);
}

fastrtps::rtps::ResourceEvent& DomainParticipant::get_resource_event() const
{
    return impl_->get_resource_event();
}

fastrtps::rtps::SampleIdentity DomainParticipant::get_type_dependencies(
        const fastrtps::types::TypeIdentifierSeq& in) const
{
    return impl_->get_type_dependencies(in);
}

fastrtps::rtps::SampleIdentity DomainParticipant::get_types(
        const fastrtps::types::TypeIdentifierSeq& in) const
{
    return impl_->get_types(in);
}

ReturnCode_t DomainParticipant::register_remote_type(
        const fastrtps::types::TypeInformation& type_information,
        const std::string& type_name,
        std::function<void(const std::string& name, const fastrtps::types::DynamicType_ptr type)>& callback)
{
    return impl_->register_remote_type(type_information, type_name, callback);
}

bool DomainParticipant::has_active_entities()
{
    return impl_->has_active_entities();
}
