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
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

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
    return set_listener(listener, std::chrono::seconds::max());
}

ReturnCode_t DomainParticipant::set_listener(
        DomainParticipantListener* listener,
        const std::chrono::seconds timeout)
{
    return set_listener(listener, StatusMask::all(), timeout);
}

ReturnCode_t DomainParticipant::set_listener(
        DomainParticipantListener* listener,
        const StatusMask& mask)
{
    return set_listener(listener, mask, std::chrono::seconds::max());
}

ReturnCode_t DomainParticipant::set_listener(
        DomainParticipantListener* listener,
        const StatusMask& mask,
        const std::chrono::seconds timeout)
{
    ReturnCode_t ret_val = impl_->set_listener(listener, timeout);
    if (ret_val == RETCODE_OK)
    {
        status_mask_ = mask;
    }

    return ret_val;
}

ReturnCode_t DomainParticipant::enable()
{
    if (enable_)
    {
        return RETCODE_OK;
    }

    enable_ = true;
    ReturnCode_t ret_code = impl_->enable();
    enable_ = RETCODE_OK == ret_code;
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
        const Publisher* publisher)
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
        const Subscriber* subscriber)
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
        const Topic* topic)
{
    return impl_->delete_topic(topic);
}

ContentFilteredTopic* DomainParticipant::create_contentfilteredtopic(
        const std::string& name,
        Topic* related_topic,
        const std::string& filter_expression,
        const std::vector<std::string>& expression_parameters)
{
    return impl_->create_contentfilteredtopic(name, related_topic, filter_expression, expression_parameters,
                   FASTDDS_SQLFILTER_NAME);
}

ContentFilteredTopic* DomainParticipant::create_contentfilteredtopic(
        const std::string& name,
        Topic* related_topic,
        const std::string& filter_expression,
        const std::vector<std::string>& expression_parameters,
        const char* filter_class_name)
{
    return impl_->create_contentfilteredtopic(name, related_topic, filter_expression, expression_parameters,
                   filter_class_name);
}

ReturnCode_t DomainParticipant::delete_contentfilteredtopic(
        const ContentFilteredTopic* a_contentfilteredtopic)
{
    return impl_->delete_contentfilteredtopic(a_contentfilteredtopic);
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
    EPROSIMA_LOG_WARNING(DOMAIN_PARTICIPANT, "create_multitopic method not implemented");
    return nullptr;
}

ReturnCode_t DomainParticipant::delete_multitopic(
        const MultiTopic* a_multitopic)
{
    static_cast<void> (a_multitopic);
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::register_content_filter_factory(
        const char* filter_class_name,
        IContentFilterFactory* const filter_factory)
{
    return impl_->register_content_filter_factory(filter_class_name, filter_factory);
}

IContentFilterFactory* DomainParticipant::lookup_content_filter_factory(
        const char* filter_class_name)
{
    return impl_->lookup_content_filter_factory(filter_class_name);
}

ReturnCode_t DomainParticipant::unregister_content_filter_factory(
        const char* filter_class_name)
{
    return impl_->unregister_content_filter_factory(filter_class_name);
}

Topic* DomainParticipant::find_topic(
        const std::string& topic_name,
        const fastdds::dds::Duration_t& timeout)
{
    return impl_->find_topic(topic_name, timeout);
}

rpc::Service* DomainParticipant::create_service(
        const std::string& service_name,
        const std::string& service_type_name)
{
    // NOTE: According to the RPC Standard annotation, service_name must be <interface_name>_<Service_name>
    // Where <Service_name> is "Service" by default.
    // The Service topics will be service_name + "_Request" and service_name + "_Reply"

    return impl_->create_service(service_name, service_type_name);
}

rpc::Service* DomainParticipant::find_service(
        const std::string& service_name) const
{
    return impl_->find_service(service_name);
}

ReturnCode_t DomainParticipant::delete_service(
        const rpc::Service* service)
{
    return impl_->delete_service(service);
}

rpc::Requester* DomainParticipant::create_service_requester(
        rpc::Service* service,
        const RequesterQos& requester_qos)
{
    return impl_->create_service_requester(service, requester_qos);
}

ReturnCode_t DomainParticipant::delete_service_requester(
        const std::string& service_name,
        rpc::Requester* requester)
{
    return impl_->delete_service_requester(service_name, requester);
}

rpc::Replier* DomainParticipant::create_service_replier(
        rpc::Service* service,
        const ReplierQos& replier_qos)
{
    return impl_->create_service_replier(service, replier_qos);
}

ReturnCode_t DomainParticipant::delete_service_replier(
        const std::string& service_name,
        rpc::Replier* replier)
{
    return impl_->delete_service_replier(service_name, replier);
}

TopicDescription* DomainParticipant::lookup_topicdescription(
        const std::string& topic_name) const
{
    return impl_->lookup_topicdescription(topic_name);
}

const Subscriber* DomainParticipant::get_builtin_subscriber() const
{
    EPROSIMA_LOG_WARNING(DOMAIN_PARTICIPANT, "get_builtin_subscriber method not implemented");
    return nullptr;
}

ReturnCode_t DomainParticipant::ignore_participant(
        const InstanceHandle_t& handle)
{
    return impl_->ignore_participant(handle);
}

ReturnCode_t DomainParticipant::ignore_topic(
        const InstanceHandle_t& handle)
{
    static_cast<void> (handle);
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::ignore_publication(
        const InstanceHandle_t& handle)
{
    static_cast<void> (handle);
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::ignore_subscription(
        const InstanceHandle_t& handle)
{
    static_cast<void> (handle);
    return RETCODE_UNSUPPORTED;
}

DomainId_t DomainParticipant::get_domain_id() const
{
    return impl_->get_domain_id();
}

ReturnCode_t DomainParticipant::delete_contained_entities()
{
    return impl_->delete_contained_entities();
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
    return RETCODE_OK;
}

ReturnCode_t DomainParticipant::get_publisher_qos_from_profile(
        const std::string& profile_name,
        PublisherQos& qos) const
{
    return impl_->get_publisher_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::get_publisher_qos_from_xml(
        const std::string& xml,
        PublisherQos& qos) const
{
    return impl_->get_publisher_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_publisher_qos_from_xml(
        const std::string& xml,
        PublisherQos& qos,
        const std::string& profile_name) const
{
    return impl_->get_publisher_qos_from_xml(xml, qos, profile_name);
}

ReturnCode_t DomainParticipant::get_default_publisher_qos_from_xml(
        const std::string& xml,
        PublisherQos& qos) const
{
    return impl_->get_default_publisher_qos_from_xml(xml, qos);
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
    return RETCODE_OK;
}

ReturnCode_t DomainParticipant::get_subscriber_qos_from_profile(
        const std::string& profile_name,
        SubscriberQos& qos) const
{
    return impl_->get_subscriber_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::get_subscriber_qos_from_xml(
        const std::string& xml,
        SubscriberQos& qos) const
{
    return impl_->get_subscriber_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_subscriber_qos_from_xml(
        const std::string& xml,
        SubscriberQos& qos,
        const std::string& profile_name) const
{
    return impl_->get_subscriber_qos_from_xml(xml, qos, profile_name);
}

ReturnCode_t DomainParticipant::get_default_subscriber_qos_from_xml(
        const std::string& xml,
        SubscriberQos& qos) const
{
    return impl_->get_default_subscriber_qos_from_xml(xml, qos);
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
    return RETCODE_OK;
}

ReturnCode_t DomainParticipant::get_topic_qos_from_profile(
        const std::string& profile_name,
        TopicQos& qos) const
{
    return impl_->get_topic_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::get_topic_qos_from_profile(
        const std::string& profile_name,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type) const
{
    return impl_->get_topic_qos_from_profile(profile_name, qos, topic_name, topic_data_type);
}

ReturnCode_t DomainParticipant::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos) const
{
    return impl_->get_topic_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type) const
{
    return impl_->get_topic_qos_from_xml(xml, qos, topic_name, topic_data_type);
}

ReturnCode_t DomainParticipant::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        const std::string& profile_name) const
{
    return impl_->get_topic_qos_from_xml(xml, qos, profile_name);
}

ReturnCode_t DomainParticipant::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type,
        const std::string& profile_name) const
{
    return impl_->get_topic_qos_from_xml(xml, qos, topic_name, topic_data_type, profile_name);
}

ReturnCode_t DomainParticipant::get_default_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos) const
{
    return impl_->get_default_topic_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_default_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type) const
{
    return impl_->get_default_topic_qos_from_xml(xml, qos, topic_name, topic_data_type);
}

ReturnCode_t DomainParticipant::get_requester_qos_from_profile(
        const std::string& profile_name,
        RequesterQos& qos) const
{
    return impl_->get_requester_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::get_requester_qos_from_xml(
        const std::string& xml,
        RequesterQos& qos) const
{
    return impl_->get_requester_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_requester_qos_from_xml(
        const std::string& xml,
        RequesterQos& qos,
        const std::string& profile_name) const
{
    return impl_->get_requester_qos_from_xml(xml, qos, profile_name);
}

ReturnCode_t DomainParticipant::get_default_requester_qos_from_xml(
        const std::string& xml,
        RequesterQos& qos) const
{
    return impl_->get_default_requester_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_replier_qos_from_profile(
        const std::string& profile_name,
        ReplierQos& qos) const
{
    return impl_->get_replier_qos_from_profile(profile_name, qos);
}

ReturnCode_t DomainParticipant::get_replier_qos_from_xml(
        const std::string& xml,
        ReplierQos& qos) const
{
    return impl_->get_replier_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_replier_qos_from_xml(
        const std::string& xml,
        ReplierQos& qos,
        const std::string& profile_name) const
{
    return impl_->get_replier_qos_from_xml(xml, qos, profile_name);
}

ReturnCode_t DomainParticipant::get_default_replier_qos_from_xml(
        const std::string& xml,
        ReplierQos& qos) const
{
    return impl_->get_default_replier_qos_from_xml(xml, qos);
}

ReturnCode_t DomainParticipant::get_discovered_participants(
        std::vector<InstanceHandle_t>& participant_handles) const
{
    static_cast<void> (participant_handles);
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::get_discovered_participant_data(
        ParticipantBuiltinTopicData& participant_data,
        const InstanceHandle_t& participant_handle) const
{
    static_cast<void> (participant_data);
    static_cast<void> (participant_handle);
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::get_discovered_topics(
        std::vector<InstanceHandle_t>& topic_handles) const
{
    static_cast<void> (topic_handles);
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::get_discovered_topic_data(
        builtin::TopicBuiltinTopicData& topic_data,
        const InstanceHandle_t& topic_handle) const
{
    static_cast<void> (topic_data);
    static_cast<void> (topic_handle);
    return RETCODE_UNSUPPORTED;
}

bool DomainParticipant::contains_entity(
        const InstanceHandle_t& a_handle,
        bool recursive /* = true */) const
{
    return impl_->contains_entity(a_handle, recursive);
}

ReturnCode_t DomainParticipant::get_current_time(
        fastdds::dds::Time_t& current_time) const
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

ReturnCode_t DomainParticipant::register_service_type(
        rpc::ServiceTypeSupport service_type,
        const std::string& service_type_name)
{
    return impl_->register_service_type(service_type, service_type_name);
}

ReturnCode_t DomainParticipant::unregister_service_type(
        const std::string& service_name)
{
    return impl_->unregister_service_type(service_name);
}

rpc::ServiceTypeSupport DomainParticipant::find_service_type(
        const std::string& service_type_name) const
{
    return impl_->find_service_type(service_type_name);
}

const InstanceHandle_t& DomainParticipant::get_instance_handle() const
{
    return impl_->get_instance_handle();
}

const fastdds::rtps::GUID_t& DomainParticipant::guid() const
{
    return impl_->guid();
}

std::vector<std::string> DomainParticipant::get_participant_names() const
{
    return impl_->get_participant_names();
}

bool DomainParticipant::new_remote_endpoint_discovered(
        const fastdds::rtps::GUID_t& partguid,
        uint16_t userId,
        fastdds::rtps::EndpointKind_t kind)
{
    return impl_->new_remote_endpoint_discovered(partguid, userId, kind);
}

bool DomainParticipant::has_active_entities()
{
    return impl_->has_active_entities();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
