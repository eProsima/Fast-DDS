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
 * @file DomainParticipantImpl.cpp
 *
 */

#include <fastdds/domain/DomainParticipantImpl.hpp>

#include <algorithm>
#include <chrono>
#include <string>

#include <asio.hpp>

#include <fastdds/core/policy/QosPolicyUtils.hpp>
#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <fastdds/core/policy/QosPolicyUtils.hpp>
#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/topic/ContentFilteredTopicImpl.hpp>
#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/topic/TopicProxy.hpp>
#include <fastdds/topic/TopicProxyFactory.hpp>
#include <fastdds/utils/QosConverters.hpp>
#include <fastdds/utils/TypePropagation.hpp>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/RTPSDomainImpl.hpp>
#include <utils/SystemInfo.hpp>
#include <xmlparser/attributes/PublisherAttributes.hpp>
#include <xmlparser/attributes/ReplierAttributes.hpp>
#include <xmlparser/attributes/RequesterAttributes.hpp>
#include <xmlparser/attributes/SubscriberAttributes.hpp>
#include <xmlparser/attributes/TopicAttributes.hpp>
#include <xmlparser/XMLProfileManager.h>

#include "../rpc/ReplierImpl.hpp"
#include "../rpc/RequesterImpl.hpp"
#include "../rpc/ServiceImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

using xmlparser::XMLProfileManager;
using xmlparser::XMLP_ret;
using rtps::RTPSDomain;
using rtps::RTPSDomainImpl;
using rtps::RTPSParticipant;
using xmlparser::XMLP_ret;
using xmlparser::XMLProfileManager;
#if HAVE_SECURITY
using rtps::ParticipantAuthenticationInfo;
#endif // if HAVE_SECURITY
using rtps::EndpointKind_t;
using rtps::ReaderDiscoveryStatus;
using rtps::ResourceEvent;
using rtps::WriterDiscoveryStatus;

DomainParticipantImpl::DomainParticipantImpl(
        DomainParticipant* dp,
        DomainId_t did,
        const DomainParticipantQos& qos,
        DomainParticipantListener* listen)
    : domain_id_(did)
    , next_instance_id_(0)
    , qos_(qos)
    , rtps_participant_(nullptr)
    , participant_(dp)
    , listener_(listen)
    , default_pub_qos_(PUBLISHER_QOS_DEFAULT)
    , default_sub_qos_(SUBSCRIBER_QOS_DEFAULT)
    , default_topic_qos_(TOPIC_QOS_DEFAULT)
    , id_counter_(0)
#pragma warning (disable : 4355 )
    , rtps_listener_(this)
{
    participant_->impl_ = this;

    xmlparser::PublisherAttributes pub_attr;
    XMLProfileManager::getDefaultPublisherAttributes(pub_attr);
    utils::set_qos_from_attributes(default_pub_qos_, pub_attr);

    xmlparser::SubscriberAttributes sub_attr;
    XMLProfileManager::getDefaultSubscriberAttributes(sub_attr);
    utils::set_qos_from_attributes(default_sub_qos_, sub_attr);

    xmlparser::TopicAttributes top_attr;
    XMLProfileManager::getDefaultTopicAttributes(top_attr);
    utils::set_qos_from_attributes(default_topic_qos_, top_attr);

    // Pre calculate participant id and generated guid
    participant_id_ = qos_.wire_protocol().participant_id;
    if (!eprosima::fastdds::rtps::RTPSDomainImpl::create_participant_guid(participant_id_, guid_))
    {
        EPROSIMA_LOG_ERROR(DOMAIN_PARTICIPANT, "Error generating GUID for participant");
    }

    /* Fill physical data properties if they are found and empty */
    std::string* property_value = fastdds::rtps::PropertyPolicyHelper::find_property(
        qos_.properties(), parameter_policy_physical_data_host);
    if (nullptr != property_value && property_value->empty())
    {
        if (SystemInfo::instance().machine_id().size() > 0)
        {
            property_value->assign(SystemInfo::instance().machine_id().to_string());
        }
        else
        {
            property_value->assign(asio::ip::host_name() + ":" + std::to_string(utils::default_domain_id()));
        }
    }

    property_value = fastdds::rtps::PropertyPolicyHelper::find_property(
        qos_.properties(), parameter_policy_physical_data_user);
    if (nullptr != property_value && property_value->empty())
    {
        std::string username = "unknown";
        if (RETCODE_OK == SystemInfo::get_username(username))
        {
            property_value->assign(username);
        }
    }

    property_value = fastdds::rtps::PropertyPolicyHelper::find_property(
        qos_.properties(), parameter_policy_physical_data_process);
    if (nullptr != property_value && property_value->empty())
    {
        property_value->assign(std::to_string(SystemInfo::instance().process_id()));
    }
}

void DomainParticipantImpl::disable()
{
    DomainParticipant* participant = get_participant();
    if (participant)
    {
        participant->set_listener(nullptr);
    }

    // The function to disable the DomainParticipantImpl is called from
    // DomainParticipantFactory::delete_participant() and DomainParticipantFactory destructor.
    auto rtps_participant = get_rtps_participant();
    if (rtps_participant != nullptr)
    {
        rtps_participant->set_listener(nullptr);

        {
            std::lock_guard<std::mutex> lock(mtx_pubs_);
            for (auto pub_it = publishers_.begin(); pub_it != publishers_.end(); ++pub_it)
            {
                pub_it->second->disable();
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtx_subs_);
            for (auto sub_it = subscribers_.begin(); sub_it != subscribers_.end(); ++sub_it)
            {
                sub_it->second->disable();
            }
        }
    }
}

DomainParticipantImpl::~DomainParticipantImpl()
{
    {
        std::lock_guard<std::mutex> lock(mtx_services_);
        for (auto service_it = services_.begin(); service_it != services_.end(); ++service_it)
        {
            delete service_it->second;
        }
        services_.clear();
    }

    services_publisher_ = {};
    services_subscriber_ = {};

    {
        std::lock_guard<std::mutex> lock(mtx_pubs_);
        for (auto pub_it = publishers_.begin(); pub_it != publishers_.end(); ++pub_it)
        {
            delete pub_it->second;
        }
        publishers_.clear();
        publishers_by_handle_.clear();
    }

    {
        std::lock_guard<std::mutex> lock(mtx_subs_);

        for (auto sub_it = subscribers_.begin(); sub_it != subscribers_.end(); ++sub_it)
        {
            delete sub_it->second;
        }
        subscribers_.clear();
        subscribers_by_handle_.clear();
    }

    {
        std::lock_guard<std::mutex> lock(mtx_topics_);

        filtered_topics_.clear();

        for (auto topic_it = topics_.begin(); topic_it != topics_.end(); ++topic_it)
        {
            delete topic_it->second;
        }
        topics_.clear();
        topics_by_handle_.clear();
    }

    auto rtps_participant = get_rtps_participant();
    if (rtps_participant != nullptr)
    {
        RTPSDomain::removeRTPSParticipant(rtps_participant);
    }

    {
        std::lock_guard<std::mutex> lock(mtx_types_);
        types_.clear();
    }

    std::lock_guard<std::mutex> _(mtx_gs_);

    // Assert no callbacks are being executed.
    // Note that this should never occur since reception and events threads joined when removing rtps_participant.
    assert(!(rtps_listener_.callback_counter_ > 0));

    if (participant_)
    {
        participant_->impl_ = nullptr;
        delete participant_;
        participant_ = nullptr;
    }
}

ReturnCode_t DomainParticipantImpl::enable()
{
    // Should not have been previously enabled
    assert(get_rtps_participant() == nullptr);
    // Should not have failed assigning the GUID
    assert (guid_ != fastdds::rtps::GUID_t::unknown());

    auto qos_check = check_qos(qos_);

    if (RETCODE_OK != qos_check)
    {
        return qos_check;
    }

    fastdds::rtps::RTPSParticipantAttributes rtps_attr;
    utils::set_attributes_from_qos(rtps_attr, qos_);
    rtps_attr.participantID = participant_id_;

    RTPSParticipant* part = RTPSDomain::createParticipant(
        domain_id_,
        false,
        rtps_attr,
        &rtps_listener_);

    if (part == nullptr)
    {
        EPROSIMA_LOG_ERROR(DOMAIN_PARTICIPANT, "Problem creating RTPSParticipant");
        return RETCODE_ERROR;
    }

    guid_ = part->getGuid();

    {
        std::lock_guard<std::mutex> _(mtx_gs_);

        rtps_participant_ = part;

        part->set_check_type_function(
            [this](const std::string& type_name) -> bool
            {
                return find_type(type_name).get() != nullptr;
            });
    }

    if (qos_.entity_factory().autoenable_created_entities)
    {
        // Enable topics first
        {
            std::lock_guard<std::mutex> lock(mtx_topics_);

            for (auto topic : topics_)
            {
                topic.second->enable_topic();
            }
        }

        // Enable publishers
        {
            std::lock_guard<std::mutex> lock(mtx_pubs_);
            for (auto pub : publishers_)
            {
                pub.second->rtps_participant_ = part;
                pub.second->user_publisher_->enable();
            }
        }

        // Enable subscribers
        {
            std::lock_guard<std::mutex> lock(mtx_subs_);

            for (auto sub : subscribers_)
            {
                sub.second->rtps_participant_ = part;
                sub.second->user_subscriber_->enable();
            }
        }
    }

    part->enable();

    return RETCODE_OK;
}

ReturnCode_t DomainParticipantImpl::set_qos(
        const DomainParticipantQos& qos)
{
    bool enabled = false;
    bool qos_should_be_updated = false;
    fastdds::rtps::RTPSParticipantAttributes patt;
    fastdds::rtps::RTPSParticipant* rtps_participant = nullptr;

    {
        std::lock_guard<std::mutex> _(mtx_gs_);

        rtps_participant = rtps_participant_;
        enabled = rtps_participant != nullptr;
        const DomainParticipantQos& qos_to_set = (&qos == &PARTICIPANT_QOS_DEFAULT) ?
                DomainParticipantFactory::get_instance()->get_default_participant_qos() : qos;

        if (&qos != &PARTICIPANT_QOS_DEFAULT)
        {
            ReturnCode_t ret_val = check_qos(qos_to_set);
            if (RETCODE_OK != ret_val)
            {
                return ret_val;
            }
        }

        if (enabled && !can_qos_be_updated(qos_, qos_to_set))
        {
            return RETCODE_IMMUTABLE_POLICY;
        }

        qos_should_be_updated = set_qos(qos_, qos_to_set, !enabled);
        if (enabled)
        {
            if (qos_should_be_updated)
            {
                // Notify the participant that there is a QoS update
                utils::set_attributes_from_qos(patt, qos_);
            }
            else
            {
                // Trigger update of network interfaces by calling update_attributes with current attributes
                patt = rtps_participant->get_attributes();
            }
        }
    }

    if (enabled)
    {
        rtps_participant->update_attributes(patt);
    }

    return RETCODE_OK;
}

ReturnCode_t DomainParticipantImpl::get_qos(
        DomainParticipantQos& qos) const
{
    std::lock_guard<std::mutex> _(mtx_gs_);
    qos = qos_;
    return RETCODE_OK;
}

const DomainParticipantQos& DomainParticipantImpl::get_qos() const
{
    std::lock_guard<std::mutex> _(mtx_gs_);
    return qos_;
}

ReturnCode_t DomainParticipantImpl::delete_publisher(
        const Publisher* pub)
{
    if (get_participant() != pub->get_participant())
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }
    std::lock_guard<std::mutex> lock(mtx_pubs_);
    auto pit = publishers_.find(const_cast<Publisher*>(pub));

    if (pit != publishers_.end())
    {
        assert(pub->get_instance_handle() == pit->second->get_instance_handle()
                && "The publisher instance handle does not match the publisher implementation instance handle");
        if (pub->has_datawriters())
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
        pit->second->set_listener(nullptr);
        publishers_by_handle_.erase(publishers_by_handle_.find(pit->second->get_instance_handle()));
        delete pit->second;
        publishers_.erase(pit);
        return RETCODE_OK;
    }

    return RETCODE_ERROR;
}

ReturnCode_t DomainParticipantImpl::delete_subscriber(
        const Subscriber* sub)
{
    if (get_participant() != sub->get_participant())
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }
    std::lock_guard<std::mutex> lock(mtx_subs_);
    auto sit = subscribers_.find(const_cast<Subscriber*>(sub));

    if (sit != subscribers_.end())
    {
        assert(sub->get_instance_handle() == sit->second->get_instance_handle()
                && "The subscriber instance handle does not match the subscriber implementation instance handle");
        if (sub->has_datareaders())
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
        sit->second->set_listener(nullptr);
        subscribers_by_handle_.erase(subscribers_by_handle_.find(sit->second->get_instance_handle()));
        delete sit->second;
        subscribers_.erase(sit);
        return RETCODE_OK;
    }

    return RETCODE_ERROR;
}

Topic* DomainParticipantImpl::find_topic(
        const std::string& topic_name,
        const fastdds::dds::Duration_t& timeout)
{
    auto find_fn = [this, &topic_name]()
            {
                return topics_.count(topic_name) > 0;
            };

    std::unique_lock<std::mutex> lock(mtx_topics_);
    if (timeout.is_infinite())
    {
        cond_topics_.wait(lock, find_fn);
    }
    else
    {
        auto duration = std::chrono::seconds(timeout.seconds) + std::chrono::nanoseconds(timeout.nanosec);
        if (!cond_topics_.wait_for(lock, duration, find_fn))
        {
            return nullptr;
        }
    }

    Topic* ret_val = topics_[topic_name]->create_topic()->get_topic();

    InstanceHandle_t topic_handle;
    create_instance_handle(topic_handle);
    ret_val->set_instance_handle(topic_handle);
    topics_by_handle_[topic_handle] = ret_val;

    return ret_val;
}

void DomainParticipantImpl::set_topic_listener(
        const TopicProxyFactory* factory,
        TopicImpl* impl,
        TopicListener* listener,
        const StatusMask& mask)
{
    std::lock_guard<std::mutex> lock(mtx_topics_);
    impl->set_listener(listener);
    factory->for_each([mask](const std::unique_ptr<TopicProxy>& proxy)
            {
                proxy->get_topic()->status_mask_ = mask;
            });
}

ReturnCode_t DomainParticipantImpl::delete_topic(
        const Topic* topic)
{
    if (topic == nullptr)
    {
        return RETCODE_BAD_PARAMETER;
    }

    std::lock_guard<std::mutex> lock(mtx_topics_);
    auto handle_it = std::find_if(topics_by_handle_.begin(), topics_by_handle_.end(),
                    [topic](const decltype(topics_by_handle_)::value_type& item)
                    {
                        return item.second == topic;
                    });
    if (handle_it != topics_by_handle_.end())
    {
        auto it = topics_.find(topic->get_name());
        assert(it != topics_.end() && "Topic found by handle but factory not found");
        InstanceHandle_t handle = topic->get_instance_handle();

        TopicProxy* proxy = dynamic_cast<TopicProxy*>(topic->get_impl());
        assert(nullptr != proxy);
        auto ret_code = it->second->delete_topic(proxy);
        if (RETCODE_OK == ret_code)
        {
            topics_by_handle_.erase(handle);

            if (it->second->can_be_deleted())
            {
                auto factory = it->second;
                topics_.erase(it);
                delete factory;
            }
        }
        return ret_code;
    }

    return RETCODE_PRECONDITION_NOT_MET;
}

ContentFilteredTopic* DomainParticipantImpl::create_contentfilteredtopic(
        const std::string& name,
        Topic* related_topic,
        const std::string& filter_expression,
        const std::vector<std::string>& expression_parameters,
        const char* filter_class_name)
{
    if ((nullptr == related_topic) || (nullptr == filter_class_name))
    {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mtx_topics_);

    // Check there is no Topic with the same name
    if ((topics_.find(name) != topics_.end()) ||
            (filtered_topics_.find(name) != filtered_topics_.end()))
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Topic with name : " << name << " already exists");
        return nullptr;
    }

    if (related_topic->get_participant() != get_participant())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Creating ContentFilteredTopic with name " << name <<
                ": related_topic not from this participant");
        return nullptr;
    }

    IContentFilterFactory* filter_factory = find_content_filter_factory(filter_class_name);
    if (nullptr == filter_factory)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Could not find factory for filter class " << filter_class_name);
        return nullptr;
    }

    if (expression_parameters.size() > qos_.allocation().content_filter.expression_parameters.maximum)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Number of expression parameters exceeds maximum allocation limit: "
                << expression_parameters.size() << " > "
                << qos_.allocation().content_filter.expression_parameters.maximum);
        return nullptr;
    }

    if (expression_parameters.size() > 100)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Number of expression parameters exceeds maximum protocol limit: "
                << expression_parameters.size() << " > 100");
        return nullptr;
    }

    TopicProxy* topic_impl = dynamic_cast<TopicProxy*>(related_topic->get_impl());
    assert(nullptr != topic_impl);
    const TypeSupport& type = topic_impl->get_type();
    LoanableSequence<const char*>::size_type n_params;
    n_params = static_cast<LoanableSequence<const char*>::size_type>(expression_parameters.size());
    LoanableSequence<const char*> filter_parameters(n_params);
    filter_parameters.length(n_params);
    while (n_params > 0)
    {
        n_params--;
        filter_parameters[n_params] = expression_parameters[n_params].c_str();
    }

    // Tell filter factory to compile the expression
    IContentFilter* filter_instance = nullptr;
    if (RETCODE_OK !=
            filter_factory->create_content_filter(filter_class_name, related_topic->get_type_name().c_str(),
            type.get(), filter_expression.c_str(), filter_parameters, filter_instance))
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Could not create filter of class " << filter_class_name << " for expression \"" <<
                filter_expression);
        return nullptr;
    }

    ContentFilteredTopic* topic;
    topic = new ContentFilteredTopic(name, related_topic, filter_expression, expression_parameters);
    ContentFilteredTopicImpl* content_topic_impl = static_cast<ContentFilteredTopicImpl*>(topic->get_impl());
    content_topic_impl->filter_property.filter_class_name = filter_class_name;
    content_topic_impl->filter_factory = filter_factory;
    content_topic_impl->filter_instance = filter_instance;
    content_topic_impl->update_signature();

    // Save the topic into the map
    filtered_topics_.emplace(std::make_pair(name, std::unique_ptr<ContentFilteredTopic>(topic)));

    return topic;
}

ReturnCode_t DomainParticipantImpl::delete_contentfilteredtopic(
        const ContentFilteredTopic* topic)
{
    if (topic == nullptr)
    {
        return RETCODE_BAD_PARAMETER;
    }

    std::lock_guard<std::mutex> lock(mtx_topics_);
    auto it = filtered_topics_.find(topic->get_name());

    if (it != filtered_topics_.end())
    {
        if (it->second->get_impl()->is_referenced())
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
        filtered_topics_.erase(it);
        return RETCODE_OK;
    }

    return RETCODE_PRECONDITION_NOT_MET;
}

ReturnCode_t DomainParticipantImpl::register_content_filter_factory(
        const char* filter_class_name,
        IContentFilterFactory* const filter_factory)
{
    if (nullptr == filter_factory || nullptr == filter_class_name || strlen(filter_class_name) > 255)
    {
        return RETCODE_BAD_PARAMETER;
    }

    std::lock_guard<std::mutex> lock(mtx_topics_);
    auto it = filter_factories_.find(filter_class_name);
    if ((it != filter_factories_.end()) || (0 == strcmp(filter_class_name, FASTDDS_SQLFILTER_NAME)))
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    filter_factories_[filter_class_name] = filter_factory;
    return RETCODE_OK;
}

IContentFilterFactory* DomainParticipantImpl::lookup_content_filter_factory(
        const char* filter_class_name)
{
    if (nullptr == filter_class_name)
    {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mtx_topics_);
    auto it = filter_factories_.find(filter_class_name);
    if ((it == filter_factories_.end()) || (it->first == FASTDDS_SQLFILTER_NAME))
    {
        return nullptr;
    }
    return it->second;
}

ReturnCode_t DomainParticipantImpl::unregister_content_filter_factory(
        const char* filter_class_name)
{
    if (nullptr == filter_class_name)
    {
        return RETCODE_BAD_PARAMETER;
    }

    std::lock_guard<std::mutex> lock(mtx_topics_);
    auto it = filter_factories_.find(filter_class_name);
    if ((it == filter_factories_.end()) || (it->first == FASTDDS_SQLFILTER_NAME))
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    for (auto& topic : filtered_topics_)
    {
        if (topic.second->impl_->filter_property.filter_class_name == filter_class_name)
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }

    for (auto& pub : publishers_)
    {
        for (auto& topic : pub.second->writers_)
        {
            for (auto& wr : topic.second)
            {
                wr->filter_is_being_removed(filter_class_name);
            }
        }
    }

    filter_factories_.erase(it);

    return RETCODE_OK;
}

IContentFilterFactory* DomainParticipantImpl::find_content_filter_factory(
        const char* filter_class_name)
{
    auto it = filter_factories_.find(filter_class_name);
    if (it != filter_factories_.end())
    {
        return it->second;
    }

    if (0 != strcmp(filter_class_name, FASTDDS_SQLFILTER_NAME))
    {
        return nullptr;
    }

    return &dds_sql_filter_factory_;
}

const InstanceHandle_t& DomainParticipantImpl::get_instance_handle() const
{
    return static_cast<const InstanceHandle_t&>(guid_);
}

const fastdds::rtps::GUID_t& DomainParticipantImpl::guid() const
{
    return guid_;
}

Publisher* DomainParticipantImpl::create_publisher(
        const PublisherQos& qos,
        PublisherListener* listener,
        const StatusMask& mask)
{
    return create_publisher(qos, nullptr, listener, mask);
}

Publisher* DomainParticipantImpl::create_publisher(
        const PublisherQos& qos,
        PublisherImpl** impl,
        PublisherListener* listener,
        const StatusMask& mask)
{
    if (RETCODE_OK != PublisherImpl::check_qos(qos))
    {
        // The PublisherImpl::check_qos() function is not yet implemented and always returns RETCODE_OK.
        // It will be implemented in future releases of Fast DDS.
        // EPROSIMA_LOG_ERROR(PARTICIPANT, "PublisherQos inconsistent or not supported");
        // return nullptr;
    }

    //TODO CONSTRUIR LA IMPLEMENTACION DENTRO DEL OBJETO DEL USUARIO.
    PublisherImpl* pubimpl = create_publisher_impl(qos, listener);
    Publisher* pub = new Publisher(pubimpl, mask);
    pubimpl->user_publisher_ = pub;
    pubimpl->rtps_participant_ = get_rtps_participant();
    bool enabled = get_rtps_participant() != nullptr;

    // Create InstanceHandle for the new publisher
    InstanceHandle_t pub_handle;
    create_instance_handle(pub_handle);
    pubimpl->handle_ = pub_handle;

    //SAVE THE PUBLISHER INTO MAPS
    std::lock_guard<std::mutex> lock(mtx_pubs_);
    publishers_by_handle_[pub_handle] = pub;
    publishers_[pub] = pubimpl;

    // Enable publisher if appropriate
    if (enabled && qos_.entity_factory().autoenable_created_entities)
    {
        ReturnCode_t ret_publisher_enable = pub->enable();
        assert(RETCODE_OK == ret_publisher_enable);
        (void)ret_publisher_enable;
    }

    if (impl)
    {
        *impl = pubimpl;
    }

    return pub;
}

Publisher* DomainParticipantImpl::create_publisher_with_profile(
        const std::string& profile_name,
        PublisherListener* listener,
        const StatusMask& mask)
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    xmlparser::PublisherAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(profile_name, attr))
    {
        PublisherQos qos = default_pub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return create_publisher(qos, listener, mask);
    }

    return nullptr;
}

PublisherImpl* DomainParticipantImpl::create_publisher_impl(
        const PublisherQos& qos,
        PublisherListener* listener)
{
    return new PublisherImpl(this, qos, listener);
}

/* TODO
   Subscriber* DomainParticipantImpl::get_builtin_subscriber()
   {
    EPROSIMA_LOG_ERROR(PARTICIPANT, "Not implemented.");
    return nullptr;
   }
 */

ReturnCode_t DomainParticipantImpl::ignore_participant(
        const InstanceHandle_t& handle)
{
    return (nullptr == rtps_participant_) ? RETCODE_NOT_ENABLED :
           rtps_participant_->ignore_participant(iHandle2GUID(handle).guidPrefix) ? RETCODE_OK :
           RETCODE_BAD_PARAMETER;
}

/* TODO
   bool DomainParticipantImpl::ignore_topic(
        const InstanceHandle_t& handle)
   {
    (void)handle;
    EPROSIMA_LOG_ERROR(PARTICIPANT, "Not implemented.");
    return false;
   }
 */

bool DomainParticipantImpl::ignore_publication(
        const InstanceHandle_t& handle)
{
    static_cast<void>(handle);
    EPROSIMA_LOG_ERROR(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::ignore_subscription(
        const InstanceHandle_t& handle)
{
    static_cast<void>(handle);
    EPROSIMA_LOG_ERROR(PARTICIPANT, "Not implemented.");
    return false;
}

DomainId_t DomainParticipantImpl::get_domain_id() const
{
    return domain_id_;
}

ReturnCode_t DomainParticipantImpl::delete_contained_entities()
{
    bool can_be_deleted = true;

    std::lock_guard<std::mutex> lock_subscribers(mtx_subs_);

    for (auto subscriber : subscribers_)
    {
        can_be_deleted = subscriber.second->can_be_deleted();
        if (!can_be_deleted)
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }

    std::lock_guard<std::mutex> lock_publishers(mtx_pubs_);



    for (auto publisher : publishers_)
    {
        can_be_deleted = publisher.second->can_be_deleted();
        if (!can_be_deleted)
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }

    ReturnCode_t ret_code = RETCODE_OK;

    for (auto& subscriber : subscribers_)
    {
        ret_code = subscriber.first->delete_contained_entities();
        if (RETCODE_OK != ret_code)
        {
            return RETCODE_ERROR;
        }
    }

    auto it_subs = subscribers_.begin();
    while (it_subs != subscribers_.end())
    {
        it_subs->second->set_listener(nullptr);
        subscribers_by_handle_.erase(it_subs->second->get_subscriber()->get_instance_handle());
        delete it_subs->second;
        it_subs = subscribers_.erase(it_subs);
    }

    for (auto& publisher : publishers_)
    {
        ret_code = publisher.first->delete_contained_entities();
        if (RETCODE_OK != ret_code)
        {
            return RETCODE_ERROR;
        }
    }

    auto it_pubs = publishers_.begin();
    while (it_pubs != publishers_.end())
    {
        it_pubs->second->set_listener(nullptr);
        publishers_by_handle_.erase(it_pubs->second->get_publisher()->get_instance_handle());
        delete it_pubs->second;
        it_pubs = publishers_.erase(it_pubs);
    }

    services_publisher_ = {};
    services_subscriber_ = {};

    std::lock_guard<std::mutex> lock_topics(mtx_topics_);

    filtered_topics_.clear();
    topics_by_handle_.clear();

    auto it_topics = topics_.begin();
    while (it_topics != topics_.end())
    {
        delete it_topics->second;
        it_topics = topics_.erase(it_topics);
    }

    return RETCODE_OK;
}

ReturnCode_t DomainParticipantImpl::assert_liveliness()
{
    fastdds::rtps::RTPSParticipant* rtps_participant = get_rtps_participant();
    if (rtps_participant == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    if (rtps_participant->wlp() != nullptr)
    {
        if (rtps_participant->wlp()->assert_liveliness_manual_by_participant())
        {
            return RETCODE_OK;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Invalid WLP, cannot assert liveliness of participant");
    }
    return RETCODE_ERROR;
}

ReturnCode_t DomainParticipantImpl::set_default_publisher_qos(
        const PublisherQos& qos)
{
    if (&qos == &PUBLISHER_QOS_DEFAULT)
    {
        reset_default_publisher_qos();
        return RETCODE_OK;
    }

    ReturnCode_t ret_val = PublisherImpl::check_qos(qos);
    if (RETCODE_OK != ret_val)
    {
        // The PublisherImpl::check_qos() function is not yet implemented and always returns RETCODE_OK.
        // It will be implemented in future releases of Fast DDS.
        // return ret_val;
    }
    PublisherImpl::set_qos(default_pub_qos_, qos, true);
    return RETCODE_OK;
}

void DomainParticipantImpl::reset_default_publisher_qos()
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    PublisherImpl::set_qos(default_pub_qos_, PUBLISHER_QOS_DEFAULT, true);
    xmlparser::PublisherAttributes attr;
    XMLProfileManager::getDefaultPublisherAttributes(attr);
    utils::set_qos_from_attributes(default_pub_qos_, attr);
}

const PublisherQos& DomainParticipantImpl::get_default_publisher_qos() const
{
    return default_pub_qos_;
}

ReturnCode_t DomainParticipantImpl::get_publisher_qos_from_profile(
        const std::string& profile_name,
        PublisherQos& qos) const
{
    xmlparser::PublisherAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(profile_name, attr))
    {
        qos = default_pub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_publisher_qos_from_xml(
        const std::string& xml,
        PublisherQos& qos) const
{
    xmlparser::PublisherAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_publisher_attributes_from_xml(xml, attr, false))
    {
        qos = default_pub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_publisher_qos_from_xml(
        const std::string& xml,
        PublisherQos& qos,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(DOMAIN_PARTICIPANT, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    xmlparser::PublisherAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_publisher_attributes_from_xml(xml, attr, true, profile_name))
    {
        qos = default_pub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_default_publisher_qos_from_xml(
        const std::string& xml,
        PublisherQos& qos) const
{
    xmlparser::PublisherAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_publisher_attributes_from_xml(xml, attr, true))
    {
        qos = default_pub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::set_default_subscriber_qos(
        const SubscriberQos& qos)
{
    if (&qos == &SUBSCRIBER_QOS_DEFAULT)
    {
        reset_default_subscriber_qos();
        return RETCODE_OK;
    }
    ReturnCode_t check_result = SubscriberImpl::check_qos(qos);
    if (RETCODE_OK != check_result)
    {
        // The SubscriberImpl::check_qos() function is not yet implemented and always returns RETCODE_OK.
        // It will be implemented in future releases of Fast DDS.
        // return check_result;
    }
    SubscriberImpl::set_qos(default_sub_qos_, qos, true);
    return RETCODE_OK;
}

void DomainParticipantImpl::reset_default_subscriber_qos()
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    SubscriberImpl::set_qos(default_sub_qos_, SUBSCRIBER_QOS_DEFAULT, true);
    xmlparser::SubscriberAttributes attr;
    XMLProfileManager::getDefaultSubscriberAttributes(attr);
    utils::set_qos_from_attributes(default_sub_qos_, attr);
}

const SubscriberQos& DomainParticipantImpl::get_default_subscriber_qos() const
{
    return default_sub_qos_;
}

ReturnCode_t DomainParticipantImpl::get_subscriber_qos_from_profile(
        const std::string& profile_name,
        SubscriberQos& qos) const
{
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(profile_name, attr))
    {
        qos = default_sub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_subscriber_qos_from_xml(
        const std::string& xml,
        SubscriberQos& qos) const
{
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_subscriber_attributes_from_xml(xml, attr, false))
    {
        qos = default_sub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_subscriber_qos_from_xml(
        const std::string& xml,
        SubscriberQos& qos,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(DOMAIN_PARTICIPANT, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_subscriber_attributes_from_xml(xml, attr, true, profile_name))
    {
        qos = default_sub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_default_subscriber_qos_from_xml(
        const std::string& xml,
        SubscriberQos& qos) const
{
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_subscriber_attributes_from_xml(xml, attr, true))
    {
        qos = default_sub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::set_default_topic_qos(
        const TopicQos& qos)
{
    if (&qos == &TOPIC_QOS_DEFAULT)
    {
        reset_default_topic_qos();
        return RETCODE_OK;
    }

    ReturnCode_t ret_val = TopicImpl::check_qos(qos);
    if (RETCODE_OK != ret_val)
    {
        return ret_val;
    }

    TopicImpl::set_qos(default_topic_qos_, qos, true);
    return RETCODE_OK;
}

void DomainParticipantImpl::reset_default_topic_qos()
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    TopicImpl::set_qos(default_topic_qos_, TOPIC_QOS_DEFAULT, true);
    xmlparser::TopicAttributes attr;
    XMLProfileManager::getDefaultTopicAttributes(attr);
    utils::set_qos_from_attributes(default_topic_qos_, attr);
}

const TopicQos& DomainParticipantImpl::get_default_topic_qos() const
{
    return default_topic_qos_;
}

ReturnCode_t DomainParticipantImpl::get_topic_qos_from_profile(
        const std::string& profile_name,
        TopicQos& qos) const
{
    std::string _topic_name, _topic_data_type;
    return get_topic_qos_from_profile(profile_name, qos, _topic_name, _topic_data_type);
}

ReturnCode_t DomainParticipantImpl::get_topic_qos_from_profile(
        const std::string& profile_name,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type) const
{
    xmlparser::TopicAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(profile_name, attr))
    {
        qos = default_topic_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.getTopicName();
        topic_data_type = attr.getTopicDataType();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos) const
{
    std::string _topic_name, _topic_data_type;
    return get_topic_qos_from_xml(xml, qos, _topic_name, _topic_data_type);
}

ReturnCode_t DomainParticipantImpl::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type) const
{
    xmlparser::TopicAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_topic_attributes_from_xml(xml, attr, false))
    {
        qos = default_topic_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.getTopicName();
        topic_data_type = attr.getTopicDataType();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        const std::string& profile_name) const
{
    std::string _topic_name, _topic_data_type;
    return get_topic_qos_from_xml(xml, qos, _topic_name, _topic_data_type, profile_name);
}

ReturnCode_t DomainParticipantImpl::get_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(DOMAIN_PARTICIPANT, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    xmlparser::TopicAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_topic_attributes_from_xml(xml, attr, true, profile_name))
    {
        qos = default_topic_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.getTopicName();
        topic_data_type = attr.getTopicDataType();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_default_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos) const
{
    std::string _topic_name, _topic_data_type;
    return get_default_topic_qos_from_xml(xml, qos, _topic_name, _topic_data_type);
}

ReturnCode_t DomainParticipantImpl::get_default_topic_qos_from_xml(
        const std::string& xml,
        TopicQos& qos,
        std::string& topic_name,
        std::string& topic_data_type) const
{
    xmlparser::TopicAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_topic_attributes_from_xml(xml, attr, true))
    {
        qos = default_topic_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.getTopicName();
        topic_data_type = attr.getTopicDataType();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_replier_qos_from_profile(
        const std::string& profile_name,
        ReplierQos& qos) const
{
    xmlparser::ReplierAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillReplierAttributes(profile_name, attr))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_replier_qos_from_xml(
        const std::string& xml,
        ReplierQos& qos) const
{
    xmlparser::ReplierAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_replier_attributes_from_xml(xml, attr, false))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_replier_qos_from_xml(
        const std::string& xml,
        ReplierQos& qos,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(DOMAIN_PARTICIPANT, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    xmlparser::ReplierAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_replier_attributes_from_xml(xml, attr, true, profile_name))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_default_replier_qos_from_xml(
        const std::string& xml,
        ReplierQos& qos) const
{
    xmlparser::ReplierAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_replier_attributes_from_xml(xml, attr, true))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_requester_qos_from_profile(
        const std::string& profile_name,
        RequesterQos& qos) const
{
    xmlparser::RequesterAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillRequesterAttributes(profile_name, attr))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_requester_qos_from_xml(
        const std::string& xml,
        RequesterQos& qos) const
{
    xmlparser::RequesterAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_requester_attributes_from_xml(xml, attr, false))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_requester_qos_from_xml(
        const std::string& xml,
        RequesterQos& qos,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(DOMAIN_PARTICIPANT, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    xmlparser::RequesterAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_requester_attributes_from_xml(xml, attr, true, profile_name))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantImpl::get_default_requester_qos_from_xml(
        const std::string& xml,
        RequesterQos& qos) const
{
    xmlparser::RequesterAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_requester_attributes_from_xml(xml, attr, true))
    {
        utils::set_qos_from_attributes(qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

/* TODO
   bool DomainParticipantImpl::get_discovered_participants(
        std::vector<InstanceHandle_t>& participant_handles) const
   {
    (void)participant_handles;
    EPROSIMA_LOG_ERROR(PARTICIPANT, "Not implemented.");
    return false;
   }
 */

/* TODO
   bool DomainParticipantImpl::get_discovered_topics(
        std::vector<InstanceHandle_t>& topic_handles) const
   {
    (void)topic_handles;
    EPROSIMA_LOG_ERROR(PARTICIPANT, "Not implemented.");
    return false;
   }
 */

bool DomainParticipantImpl::contains_entity(
        const InstanceHandle_t& handle,
        bool recursive) const
{
    // Look for publishers
    {
        std::lock_guard<std::mutex> lock(mtx_pubs_);
        if (publishers_by_handle_.find(handle) != publishers_by_handle_.end())
        {
            return true;
        }
    }

    // Look for subscribers
    {
        std::lock_guard<std::mutex> lock(mtx_subs_);
        if (subscribers_by_handle_.find(handle) != subscribers_by_handle_.end())
        {
            return true;
        }
    }

    // Look for topics
    {
        std::lock_guard<std::mutex> lock(mtx_topics_);
        if (topics_by_handle_.find(handle) != topics_by_handle_.end())
        {
            return true;
        }
    }

    if (recursive)
    {
        // Look into publishers
        {
            std::lock_guard<std::mutex> lock(mtx_pubs_);
            for (auto pit : publishers_)
            {
                if (pit.second->contains_entity(handle))
                {
                    return true;
                }
            }
        }

        // Look into subscribers
        {
            std::lock_guard<std::mutex> lock(mtx_subs_);
            for (auto sit : subscribers_)
            {
                if (sit.second->contains_entity(handle))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

ReturnCode_t DomainParticipantImpl::get_current_time(
        fastdds::dds::Time_t& current_time) const
{
    fastdds::dds::Time_t::now(current_time);

    return RETCODE_OK;
}

std::vector<std::string> DomainParticipantImpl::get_participant_names() const
{
    std::lock_guard<std::mutex> _(mtx_gs_);
    return rtps_participant_ == nullptr ?
           std::vector<std::string> {}
           :
           rtps_participant_->getParticipantNames();
}

Subscriber* DomainParticipantImpl::create_subscriber(
        const SubscriberQos& qos,
        SubscriberListener* listener,
        const StatusMask& mask)
{
    if (RETCODE_OK != SubscriberImpl::check_qos(qos))
    {
        // The SubscriberImpl::check_qos() function is not yet implemented and always returns RETCODE_OK.
        // It will be implemented in future releases of Fast DDS.
        // EPROSIMA_LOG_ERROR(PARTICIPANT, "SubscriberQos inconsistent or not supported");
        // return nullptr;
    }

    //TODO CONSTRUIR LA IMPLEMENTACION DENTRO DEL OBJETO DEL USUARIO.
    SubscriberImpl* subimpl = create_subscriber_impl(qos, listener);
    Subscriber* sub = new Subscriber(subimpl, mask);
    subimpl->user_subscriber_ = sub;
    subimpl->rtps_participant_ = get_rtps_participant();

    // Create InstanceHandle for the new subscriber
    InstanceHandle_t sub_handle;
    bool enabled = get_rtps_participant() != nullptr;

    // Create InstanceHandle for the new subscriber
    create_instance_handle(sub_handle);
    subimpl->handle_ = sub_handle;

    //SAVE THE PUBLISHER INTO MAPS
    std::lock_guard<std::mutex> lock(mtx_subs_);
    subscribers_by_handle_[sub_handle] = sub;
    subscribers_[sub] = subimpl;

    // Enable subscriber if appropriate
    if (enabled && qos_.entity_factory().autoenable_created_entities)
    {
        ReturnCode_t ret_subscriber_enable = sub->enable();
        assert(RETCODE_OK == ret_subscriber_enable);
        (void)ret_subscriber_enable;
    }

    return sub;
}

Subscriber* DomainParticipantImpl::create_subscriber_with_profile(
        const std::string& profile_name,
        SubscriberListener* listener,
        const StatusMask& mask)
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(profile_name, attr))
    {
        SubscriberQos qos = default_sub_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return create_subscriber(qos, listener, mask);
    }

    return nullptr;
}

SubscriberImpl* DomainParticipantImpl::create_subscriber_impl(
        const SubscriberQos& qos,
        SubscriberListener* listener)
{
    return new SubscriberImpl(this, qos, listener);
}

Topic* DomainParticipantImpl::create_topic(
        const std::string& topic_name,
        const std::string& type_name,
        const TopicQos& qos,
        TopicListener* listener,
        const StatusMask& mask)
{
    //Look for the correct type registration
    TypeSupport type_support = find_type(type_name);
    if (type_support.empty())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Type : " << type_name << " Not Registered");
        return nullptr;
    }

    if (RETCODE_OK != TopicImpl::check_qos_including_resource_limits(qos, type_support))
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "TopicQos inconsistent or not supported");
        return nullptr;
    }

    bool enabled = get_rtps_participant() != nullptr;

    std::lock_guard<std::mutex> lock(mtx_topics_);

    // Check there is no Topic with the same name
    if ((topics_.find(topic_name) != topics_.end()) ||
            (filtered_topics_.find(topic_name) != filtered_topics_.end()))
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Topic with name : " << topic_name << " already exists");
        return nullptr;
    }

    InstanceHandle_t topic_handle;
    create_instance_handle(topic_handle);

    TopicProxyFactory* factory = new TopicProxyFactory(this, topic_name, type_name, mask, type_support, qos, listener);
    TopicProxy* proxy = factory->create_topic();
    Topic* topic = proxy->get_topic();
    topic->set_instance_handle(topic_handle);

    //SAVE THE TOPIC INTO MAPS
    topics_by_handle_[topic_handle] = topic;
    topics_[topic_name] = factory;

    // Enable topic if appropriate
    if (enabled && qos_.entity_factory().autoenable_created_entities)
    {
        ReturnCode_t ret_topic_enable = topic->enable();
        assert(RETCODE_OK == ret_topic_enable);
        (void)ret_topic_enable;
    }

    cond_topics_.notify_all();

    return topic;
}

Topic* DomainParticipantImpl::create_topic_with_profile(
        const std::string& topic_name,
        const std::string& type_name,
        const std::string& profile_name,
        TopicListener* listener,
        const StatusMask& mask)
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    xmlparser::TopicAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(profile_name, attr))
    {
        TopicQos qos = default_topic_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return create_topic(topic_name, type_name, qos, listener, mask);
    }

    return nullptr;
}

TopicDescription* DomainParticipantImpl::lookup_topicdescription(
        const std::string& topic_name) const
{
    std::lock_guard<std::mutex> lock(mtx_topics_);

    auto it = topics_.find(topic_name);
    if (it != topics_.end())
    {
        return it->second->get_topic()->get_topic();
    }

    auto filtered_it = filtered_topics_.find(topic_name);
    if (filtered_it != filtered_topics_.end())
    {
        return filtered_it->second.get();
    }

    return nullptr;
}

const TypeSupport DomainParticipantImpl::find_type(
        const std::string& type_name) const
{
    std::lock_guard<std::mutex> lock(mtx_types_);

    auto type_it = types_.find(type_name);

    if (type_it != types_.end())
    {
        return type_it->second;
    }

    return TypeSupport(nullptr);
}

ReturnCode_t DomainParticipantImpl::register_type(
        const TypeSupport type,
        const std::string& type_name)
{
    if (type_name.size() <= 0)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Registered Type must have a name");
        return RETCODE_BAD_PARAMETER;
    }

    /*
     * The type object registration sets the TypeIdentifiers in the type support's underlying TopicDataType.
     * This means that we need need to trigger the registration of the type object representation
     * (idempotent operation) before finding the type in the registry.
     * Otherwise, registering two TypeSupport instances with the same underlying TopicDataType will fail upon
     * the second registration, as the TypeIdentifiers of the retrieved type from the registry would not be equal
     * to those of the incoming type support.
     */
    type.get()->register_type_object_representation();

    TypeSupport t = find_type(type_name);

    if (!t.empty())
    {
        if (t == type)
        {
            return RETCODE_OK;
        }

        EPROSIMA_LOG_ERROR(PARTICIPANT, "Another type with the same name '" << type_name << "' is already registered.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    EPROSIMA_LOG_INFO(PARTICIPANT, "Type " << type_name << " registered.");
    std::lock_guard<std::mutex> lock(mtx_types_);
    types_.insert(std::make_pair(type_name, type));

    return RETCODE_OK;
}

ReturnCode_t DomainParticipantImpl::unregister_type(
        const std::string& type_name)
{
    if (type_name.size() <= 0)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Registered Type must have a name");
        return RETCODE_BAD_PARAMETER;
    }

    TypeSupport t = find_type(type_name);

    if (t.empty())
    {
        return RETCODE_OK; // Not registered, so unregistering complete.
    }

    {
        // Check is any subscriber is using the type
        std::lock_guard<std::mutex> lock(mtx_subs_);

        for (auto sit : subscribers_)
        {
            if (sit.second->type_in_use(type_name))
            {
                EPROSIMA_LOG_WARNING(PARTICIPANT, "Type '" << type_name << "' is in use");
                return RETCODE_PRECONDITION_NOT_MET;
            }
        }
    }

    {
        // Check is any publisher is using the type
        std::lock_guard<std::mutex> lock(mtx_pubs_);

        for (auto pit : publishers_)
        {
            if (pit.second->type_in_use(type_name))
            {
                EPROSIMA_LOG_WARNING(PARTICIPANT, "Type '" << type_name << "' is in use");
                return RETCODE_PRECONDITION_NOT_MET;
            }
        }
    }

    std::lock_guard<std::mutex> lock(mtx_types_);
    types_.erase(type_name);

    return RETCODE_OK;
}

const rpc::ServiceTypeSupport DomainParticipantImpl::find_service_type(
        const std::string& service_type_name) const
{
    std::lock_guard<std::mutex> lock(mtx_service_types_);

    auto service_type_it = service_types_.find(service_type_name);

    if (service_type_it != service_types_.end())
    {
        return service_type_it->second;
    }

    return rpc::ServiceTypeSupport(TypeSupport(nullptr), TypeSupport(nullptr));
}

ReturnCode_t DomainParticipantImpl::register_service_type(
        rpc::ServiceTypeSupport service_type,
        const std::string& service_type_name)
{
    if (service_type_name.size() <= 0)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Registered Service Type must have a name");
        return RETCODE_BAD_PARAMETER;
    }

    if (service_type.empty_types())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Trying to register a Service Type with empty request/reply types");
        return RETCODE_BAD_PARAMETER;
    }

    // Check if the service type is already registered
    rpc::ServiceTypeSupport t = find_service_type(service_type_name);

    if (!t.empty_types())
    {
        if (t == service_type)
        {
            return RETCODE_OK;
        }

        EPROSIMA_LOG_ERROR(PARTICIPANT, "Another service type with the same name '" << service_type_name <<
                "' is already registered.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    // Register request/reply types
    ReturnCode_t ret_code;
    ret_code = register_type(service_type.request_type(), service_type_name + "_Request");

    if (RETCODE_OK != ret_code)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Error registering Request Type for Service Type " << service_type_name);
        return ret_code;
    }

    ret_code = register_type(service_type.reply_type(), service_type_name + "_Reply");

    if (RETCODE_OK != ret_code)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Error registering Request Type for Service Type " << service_type_name);
        return ret_code;
    }

    EPROSIMA_LOG_INFO(PARTICIPANT, "Service Type " << service_type_name << " registered.");
    std::lock_guard<std::mutex> lock(mtx_service_types_);
    service_types_.insert(std::make_pair(service_type_name, service_type));

    return RETCODE_OK;
}

ReturnCode_t DomainParticipantImpl::unregister_service_type(
        const std::string& service_type_name)
{
    if (service_type_name.size() <= 0)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Registered Service Type must have a name");
        return RETCODE_BAD_PARAMETER;
    }

    rpc::ServiceTypeSupport t = find_service_type(service_type_name);
    if (t.empty_types())
    {
        return RETCODE_OK; // Not registered, so unregistering complete.
    }

    // Iterate over all active services and check if any of them is using the service type
    {
        std::lock_guard<std::mutex> lock(mtx_services_);
        for (auto service_it : services_)
        {
            if (service_it.second->service_type_in_use(service_type_name))
            {
                EPROSIMA_LOG_ERROR(PARTICIPANT, "Service Type " << service_type_name << " is in use by service " <<
                        service_it.first);
                return RETCODE_PRECONDITION_NOT_MET;
            }
        }
    }

    // Unregister request/reply types
    ReturnCode_t ret_code;
    ret_code = unregister_type(service_type_name + "_Request");

    if (RETCODE_OK != ret_code)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Error unregistering Request Type for Service Type " << service_type_name);
        return ret_code;
    }

    ret_code = unregister_type(service_type_name + "_Reply");

    if (RETCODE_OK != ret_code)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Error unregistering Reply Type for Service Type " << service_type_name);

        // Request topic type was unregistered, so register it again to avoid leaving the participant in an inconsistent state
        register_type(t.request_type(), service_type_name + "_Request");

        return ret_code;
    }

    {
        std::lock_guard<std::mutex> lock(mtx_service_types_);
        service_types_.erase(service_type_name);
    }

    return RETCODE_OK;
}

rpc::Service* DomainParticipantImpl::create_service(
        const std::string& service_name,
        const std::string& service_type_name)
{
    if (service_name.empty())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service name cannot be empty.");
        return nullptr;
    }

    if (service_type_name.empty())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service type name cannot be empty.");
        return nullptr;
    }

    // Check if the service has already been created
    {
        std::lock_guard<std::mutex> lock(mtx_services_);
        auto it = services_.find(service_name);

        if (it != services_.end())
        {
            EPROSIMA_LOG_ERROR(PARTICIPANT, "Service with name '" << service_name << "' already exists.");
            return nullptr;
        }
    }

    // Check if the request/reply content filter factory is registered. If not, register it.
    IContentFilterFactory* factory = find_content_filter_factory(rpc::RequestReplyContentFilterFactory::FILTER_NAME);

    if (!factory)
    {
        factory = &req_rep_filter_factory_;
        ReturnCode_t ret_code =
                register_content_filter_factory(rpc::RequestReplyContentFilterFactory::FILTER_NAME, factory);

        if (RETCODE_OK != ret_code)
        {
            EPROSIMA_LOG_ERROR(PARTICIPANT, "Error registering Request/Reply Content Filter Factory.");
            return nullptr;
        }
    }

    // Before creating the service, create the publisher and subscriber
    // that it will use for DDS endpoints creation, if it is necessary
    if (!services_publisher_.second)
    {
        Publisher* pub = create_publisher(PUBLISHER_QOS_DEFAULT);
        if (!pub)
        {
            EPROSIMA_LOG_ERROR(PARTICIPANT, "Error creating Services publisher.");
            return nullptr;
        }
        services_publisher_ = std::make_pair(pub, publishers_[pub]);
    }

    if (!services_subscriber_.second)
    {
        Subscriber* sub = create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        if (!sub)
        {
            EPROSIMA_LOG_ERROR(PARTICIPANT, "Error creating Services subscriber.");
            return nullptr;
        }
        services_subscriber_ = std::make_pair(sub, subscribers_[sub]);
    }
    // Create and store the service (Internally, this will create the required DDS Request/Reply Topics))
    rpc::ServiceImpl* service(nullptr);

    try
    {
        service = new rpc::ServiceImpl(
            service_name, service_type_name, this, services_publisher_.second, services_subscriber_.second);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Error creating service: " << e.what());
        return nullptr;
    }

    // Try to enable the created service
    if (RETCODE_OK != service->enable())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Error enabling service.");
        delete service;
        return nullptr;
    }

    {
        // Register the service in the participant
        std::lock_guard<std::mutex> lock(mtx_services_);
        services_[service_name] = service;
    }

    return service;
}

rpc::Service* DomainParticipantImpl::find_service(
        const std::string& service_name) const
{
    std::lock_guard<std::mutex> lock(mtx_services_);
    auto it = services_.find(service_name);

    if (it != services_.end())
    {
        return it->second;
    }
    return nullptr;
}

ReturnCode_t DomainParticipantImpl::delete_service(
        const rpc::Service* service)
{
    if (!service)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service is nullptr.");
        return RETCODE_BAD_PARAMETER;
    }

    const rpc::ServiceImpl* service_impl = dynamic_cast<const rpc::ServiceImpl*>(service);
    assert(service_impl != nullptr);

    std::lock_guard<std::mutex> lock(mtx_services_);
    auto it = services_.find(service->get_service_name());

    if (it != services_.end())
    {
        if (service_impl != it->second)
        {
            EPROSIMA_LOG_ERROR(PARTICIPANT, "Service mismatch.");
            return RETCODE_PRECONDITION_NOT_MET;
        }

        // Check that the service is disabled
        if (it->second->is_enabled())
        {
            EPROSIMA_LOG_INFO(PARTICIPANT, "Trying to delete an enabled service.");
            ReturnCode_t retcode = it->second->close();
            if (RETCODE_OK != retcode)
            {
                EPROSIMA_LOG_ERROR(PARTICIPANT, "Error closing service: " << retcode);
                return retcode;
            }
        }

        delete it->second;
        services_.erase(it);
        return RETCODE_OK;
    }

    // The service was not found in this participant
    EPROSIMA_LOG_ERROR(PARTICIPANT, "Service with name '" << service->get_service_name() << "' not found.");
    return RETCODE_PRECONDITION_NOT_MET;
}

rpc::Requester* DomainParticipantImpl::create_service_requester(
        rpc::Service* service,
        const RequesterQos& qos)
{
    // Check if the service is valid and registered in participant
    if (!service)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service is nullptr.");
        return nullptr;
    }

    const rpc::ServiceImpl* service_impl = dynamic_cast<const rpc::ServiceImpl*>(service);
    assert(service_impl != nullptr);

    std::lock_guard<std::mutex> lock(mtx_services_);
    auto it = services_.find(service->get_service_name());
    if (it == services_.end())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service with name '" << service->get_service_name() << "' not found.");
        return nullptr;
    }

    // Make sure that both services are the same
    if (it->second != service_impl)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service mismatch.");
        return nullptr;
    }

    // Create the requester and register it in the service
    return it->second->create_requester(qos);
}

ReturnCode_t DomainParticipantImpl::delete_service_requester(
        const std::string& service_name,
        rpc::Requester* requester)
{

    rpc::RequesterImpl* requester_impl = dynamic_cast<rpc::RequesterImpl*>(requester);
    assert(requester_impl != nullptr);

    // Get registered service implementation
    std::lock_guard<std::mutex> lock(mtx_services_);
    auto it = services_.find(service_name);

    if (it == services_.end())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service with name '" << service_name << "' not registered.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return it->second->remove_requester(requester_impl);
}

rpc::Replier* DomainParticipantImpl::create_service_replier(
        rpc::Service* service,
        const ReplierQos& qos)
{
    // Check if the service is valid and registered in participant
    if (!service)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service is nullptr.");
        return nullptr;
    }

    const rpc::ServiceImpl* service_impl = dynamic_cast<const rpc::ServiceImpl*>(service);
    assert(service_impl != nullptr);

    std::lock_guard<std::mutex> lock(mtx_services_);
    auto it = services_.find(service->get_service_name());
    if (it == services_.end())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service with name '" << service->get_service_name() << "' not found.");
        return nullptr;
    }

    // Make sure that both services are the same
    if (it->second != service_impl)
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service mismatch.");
        return nullptr;
    }

    // Create the replier and register it in the service
    return it->second->create_replier(qos);
}

ReturnCode_t DomainParticipantImpl::delete_service_replier(
        const std::string& service_name,
        rpc::Replier* replier)
{
    rpc::ReplierImpl* replier_impl = dynamic_cast<rpc::ReplierImpl*>(replier);
    assert(replier_impl != nullptr);

    // Get registered service implementation
    std::lock_guard<std::mutex> lock(mtx_services_);
    auto it = services_.find(service_name);

    if (it == services_.end())
    {
        EPROSIMA_LOG_ERROR(PARTICIPANT, "Service with name '" << service_name << "' not registered.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return it->second->remove_replier(replier_impl);
}

void DomainParticipantImpl::MyRTPSParticipantListener::on_participant_discovery(
        RTPSParticipant*,
        eprosima::fastdds::rtps::ParticipantDiscoveryStatus reason,
        const ParticipantBuiltinTopicData& info,
        bool& should_be_ignored)
{
    should_be_ignored = false;
    Sentry sentinel(this);
    if (sentinel)
    {
        participant_->listener_->on_participant_discovery(participant_->participant_, reason, std::move(info),
                should_be_ignored);
    }
}

#if HAVE_SECURITY
void DomainParticipantImpl::MyRTPSParticipantListener::onParticipantAuthentication(
        RTPSParticipant*,
        ParticipantAuthenticationInfo&& info)
{
    Sentry sentinel(this);
    if (sentinel)
    {
        participant_->listener_->onParticipantAuthentication(participant_->participant_, std::move(info));
    }
}

#endif // if HAVE_SECURITY

void DomainParticipantImpl::MyRTPSParticipantListener::on_reader_discovery(
        RTPSParticipant*,
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info,
        bool& should_be_ignored)
{
    should_be_ignored = false;

    Sentry sentinel(this);
    if (sentinel)
    {
        DomainParticipantListener* listener = participant_->listener_;
        if (nullptr != listener)
        {
            listener->on_data_reader_discovery(participant_->participant_, reason, info, should_be_ignored);
        }
    }
}

void DomainParticipantImpl::MyRTPSParticipantListener::on_writer_discovery(
        RTPSParticipant*,
        WriterDiscoveryStatus reason,
        const PublicationBuiltinTopicData& info,
        bool& should_be_ignored)
{
    should_be_ignored = false;

    Sentry sentinel(this);
    if (sentinel)
    {
        DomainParticipantListener* listener = participant_->listener_;
        if (nullptr != listener)
        {
            listener->on_data_writer_discovery(participant_->participant_, reason, info, should_be_ignored);
        }
    }
}

bool DomainParticipantImpl::new_remote_endpoint_discovered(
        const fastdds::rtps::GUID_t& partguid,
        uint16_t endpointId,
        EndpointKind_t kind)
{
    if (get_rtps_participant() != nullptr)
    {
        if (kind == fastdds::rtps::WRITER)
        {
            return get_rtps_participant()->newRemoteWriterDiscovered(partguid, static_cast<int16_t>(endpointId));
        }
        else
        {
            return get_rtps_participant()->newRemoteReaderDiscovered(partguid, static_cast<int16_t>(endpointId));
        }
    }

    return false;
}

ResourceEvent& DomainParticipantImpl::get_resource_event() const
{
    assert(nullptr != get_rtps_participant());
    return get_rtps_participant()->get_resource_event();
}

ReturnCode_t DomainParticipantImpl::register_dynamic_type(
        DynamicType::_ref_type dyn_type)
{
    TypeSupport type(new DynamicPubSubType(dyn_type));
    return get_participant()->register_type(type);
}

bool DomainParticipantImpl::has_active_entities()
{
    if (!publishers_.empty())
    {
        return true;
    }
    if (!subscribers_.empty())
    {
        return true;
    }
    if (!topics_.empty())
    {
        return true;
    }
    return false;
}

bool DomainParticipantImpl::set_qos(
        DomainParticipantQos& to,
        const DomainParticipantQos& from,
        bool first_time)
{
    bool qos_should_be_updated = false;

    if (!(to.entity_factory() == from.entity_factory()))
    {
        to.entity_factory() = from.entity_factory();
    }
    if (!(to.user_data() == from.user_data()))
    {
        to.user_data() = from.user_data();
        to.user_data().hasChanged = true;
        if (!first_time)
        {
            qos_should_be_updated = true;
        }
    }
    if (first_time && !(to.allocation() == from.allocation()))
    {
        to.allocation() = from.allocation();
    }
    if (first_time && (to.properties() != from.properties()))
    {
        to.properties() = from.properties();
    }
    if (!(to.wire_protocol() == from.wire_protocol()))
    {
        to.wire_protocol() = from.wire_protocol();
        to.wire_protocol().hasChanged = true;
        if (!first_time)
        {
            qos_should_be_updated = true;
        }
    }
    if (first_time && !(to.transport() == from.transport()))
    {
        to.transport() = from.transport();
    }
    if (first_time && to.name() != from.name())
    {
        to.name() = from.name();
    }

    return qos_should_be_updated;
}

ReturnCode_t DomainParticipantImpl::check_qos(
        const DomainParticipantQos& qos)
{
    ReturnCode_t ret_val = RETCODE_OK;

    if (qos.allocation().data_limits.max_user_data != 0 &&
            qos.allocation().data_limits.max_user_data <= qos.user_data().getValue().size())
    {
        ret_val = RETCODE_INCONSISTENT_POLICY;
    }

    if (RETCODE_OK == ret_val)
    {
        // Check participant's type propagation policy
        using utils::to_type_propagation;
        using utils::TypePropagation;

        auto type_propagation = to_type_propagation(qos.properties());

        if (TypePropagation::TYPEPROPAGATION_UNKNOWN == type_propagation)
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "Invalid value for property " << parameter_policy_type_propagation);
            return RETCODE_INCONSISTENT_POLICY;
        }
    }

    // Check participant's wire protocol (builtin flow controller) configuration
    if (RETCODE_OK == ret_val)
    {
        const std::string& builtin_flow_controller_name = qos.wire_protocol().builtin.flow_controller_name;

        if (!builtin_flow_controller_name.empty())
        {
            // Get the list of flow controllers
            auto flow_controllers = qos.flow_controllers();

            // Check if any flow controller matches the builtin flow controller name
            bool found = std::any_of(flow_controllers.begin(), flow_controllers.end(),
                            [&builtin_flow_controller_name](const std::shared_ptr<fastdds::rtps::
                                    FlowControllerDescriptor>& fc)
                            {
                                return fc && fc->name == builtin_flow_controller_name;
                            });

            if (!found)
            {
                EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "Flow controller name not found in flow controllers list");
                return RETCODE_INCONSISTENT_POLICY;
            }
        }
    }


    return ret_val;
}

bool DomainParticipantImpl::can_qos_be_updated(
        const DomainParticipantQos& to,
        const DomainParticipantQos& from)
{
    bool updatable = true;
    if (!(to.allocation() == from.allocation()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "ParticipantResourceLimitsQos cannot be changed after the participant is enabled");
    }
    if ((to.properties() != from.properties()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "PropertyPolicyQos cannot be changed after the participant is enabled");
    }
    if (!(to.wire_protocol() == from.wire_protocol()))
    {
        // Check that the only modification was in wire_protocol().discovery_config.m_DiscoveryServers
        if ((to.wire_protocol().builtin.discovery_config.m_DiscoveryServers ==
                from.wire_protocol().builtin.discovery_config.m_DiscoveryServers) ||
                (!(to.wire_protocol().builtin.discovery_config.m_DiscoveryServers ==
                from.wire_protocol().builtin.discovery_config.m_DiscoveryServers) &&
                (!(to.wire_protocol().prefix == from.wire_protocol().prefix) ||
                !(to.wire_protocol().participant_id == from.wire_protocol().participant_id) ||
                !(to.wire_protocol().port == from.wire_protocol().port) ||
                !(to.wire_protocol().default_unicast_locator_list ==
                from.wire_protocol().default_unicast_locator_list) ||
                !(to.wire_protocol().default_multicast_locator_list ==
                from.wire_protocol().default_multicast_locator_list) ||
                !(to.wire_protocol().default_external_unicast_locators ==
                from.wire_protocol().default_external_unicast_locators) ||
                !(to.wire_protocol().ignore_non_matching_locators ==
                from.wire_protocol().ignore_non_matching_locators) ||
                !(to.wire_protocol().builtin.use_WriterLivelinessProtocol ==
                from.wire_protocol().builtin.use_WriterLivelinessProtocol) ||
                !(to.wire_protocol().builtin.network_configuration ==
                from.wire_protocol().builtin.network_configuration) ||
                !(to.wire_protocol().builtin.metatrafficUnicastLocatorList ==
                from.wire_protocol().builtin.metatrafficUnicastLocatorList) ||
                !(to.wire_protocol().builtin.metatrafficMulticastLocatorList ==
                from.wire_protocol().builtin.metatrafficMulticastLocatorList) ||
                !(to.wire_protocol().builtin.metatraffic_external_unicast_locators ==
                from.wire_protocol().builtin.metatraffic_external_unicast_locators) ||
                !(to.wire_protocol().builtin.initialPeersList == from.wire_protocol().builtin.initialPeersList) ||
                !(to.wire_protocol().builtin.readerHistoryMemoryPolicy ==
                from.wire_protocol().builtin.readerHistoryMemoryPolicy) ||
                !(to.wire_protocol().builtin.readerPayloadSize == from.wire_protocol().builtin.readerPayloadSize) ||
                !(to.wire_protocol().builtin.writerHistoryMemoryPolicy ==
                from.wire_protocol().builtin.writerHistoryMemoryPolicy) ||
                !(to.wire_protocol().builtin.writerPayloadSize == from.wire_protocol().builtin.writerPayloadSize) ||
                !(to.wire_protocol().builtin.mutation_tries == from.wire_protocol().builtin.mutation_tries) ||
                !(to.wire_protocol().builtin.flow_controller_name ==
                from.wire_protocol().builtin.flow_controller_name) ||
                !(to.wire_protocol().builtin.avoid_builtin_multicast ==
                from.wire_protocol().builtin.avoid_builtin_multicast) ||
                !(to.wire_protocol().builtin.discovery_config.discoveryProtocol ==
                from.wire_protocol().builtin.discovery_config.discoveryProtocol) ||
                !(to.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol ==
                from.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol) ||
                !(to.wire_protocol().builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol ==
                from.wire_protocol().builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol) ||
                !(to.wire_protocol().builtin.discovery_config.discoveryServer_client_syncperiod ==
                from.wire_protocol().builtin.discovery_config.discoveryServer_client_syncperiod) ||
                !(to.wire_protocol().builtin.discovery_config.m_PDPfactory ==
                from.wire_protocol().builtin.discovery_config.m_PDPfactory) ||
                !(to.wire_protocol().builtin.discovery_config.leaseDuration ==
                from.wire_protocol().builtin.discovery_config.leaseDuration) ||
                !(to.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod ==
                from.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod) ||
                !(to.wire_protocol().builtin.discovery_config.initial_announcements ==
                from.wire_protocol().builtin.discovery_config.initial_announcements) ||
                !(to.wire_protocol().builtin.discovery_config.m_simpleEDP ==
                from.wire_protocol().builtin.discovery_config.m_simpleEDP) ||
                !(strcmp(to.wire_protocol().builtin.discovery_config.static_edp_xml_config(),
                from.wire_protocol().builtin.discovery_config.static_edp_xml_config()) == 0) ||
                !(to.wire_protocol().builtin.discovery_config.ignoreParticipantFlags ==
                from.wire_protocol().builtin.discovery_config.ignoreParticipantFlags))))
        {
            updatable = false;
            EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "WireProtocolConfigQos cannot be changed after the participant is enabled, "
                    << "with the exception of builtin.discovery_config.m_DiscoveryServers");
        }
    }
    if (!(to.transport() == from.transport()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "TransportConfigQos cannot be changed after the participant is enabled");
    }
    if (!(to.name() == from.name()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Participant name cannot be changed after the participant is enabled");
    }
    if (!(to.builtin_controllers_sender_thread() == from.builtin_controllers_sender_thread()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Participant builtin_controllers_sender_thread cannot be changed after the participant is enabled");
    }
    if (!(to.timed_events_thread() == from.timed_events_thread()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Participant timed_events_thread cannot be changed after the participant is enabled");
    }
    if (!(to.discovery_server_thread() == from.discovery_server_thread()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Participant discovery_server_thread cannot be changed after the participant is enabled");
    }
    if (!(to.typelookup_service_thread() == from.typelookup_service_thread()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Participant typelookup_service_thread cannot be changed after the participant is enabled");
    }
#if HAVE_SECURITY
    if (!(to.security_log_thread() == from.security_log_thread()))
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Participant security_log_thread cannot be changed after the participant is enabled");
    }
#endif // if HAVE_SECURITY
    return updatable;
}

void DomainParticipantImpl::create_instance_handle(
        InstanceHandle_t& handle)
{
    using rtps::octet;

    uint32_t id = ++next_instance_id_;
    handle = guid_;
    handle.value[15] = 0x01; // Vendor specific;
    handle.value[14] = static_cast<octet>(id & 0xFF);
    handle.value[13] = static_cast<octet>((id >> 8) & 0xFF);
    handle.value[12] = static_cast<octet>((id >> 16) & 0xFF);
}

DomainParticipantListener* DomainParticipantImpl::get_listener_for(
        const StatusMask& status)
{
    if (get_participant()->get_status_mask().is_active(status))
    {
        return get_listener();
    }
    return nullptr;
}

bool DomainParticipantImpl::fill_type_information(
        const TypeSupport& type,
        xtypes::TypeInformationParameter& type_information)
{
    using utils::to_type_propagation;
    using utils::TypePropagation;

    auto properties = qos_.properties();
    auto type_propagation = to_type_propagation(properties);
    bool should_assign_type_information =
            (TypePropagation::TYPEPROPAGATION_ENABLED == type_propagation) ||
            (TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH == type_propagation);

    if (should_assign_type_information && (xtypes::TK_NONE != type->type_identifiers().type_identifier1()._d()))
    {
        xtypes::TypeInformation type_info;

        if (RETCODE_OK ==
                fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().get_type_information(
                    type->type_identifiers(), type_info))
        {
            switch (type_propagation)
            {
                case TypePropagation::TYPEPROPAGATION_ENABLED:
                {
                    // Use both complete and minimal type information
                    type_information.type_information = type_info;
                    break;
                }
                case TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH:
                {
                    // Use minimal type information only
                    type_information.type_information.minimal() = type_info.minimal();
                    break;
                }
                default:
                    // This should never happen as other cases are protected by should_assign_type_information
                    assert(false);
                    break;
            }

            type_information.assigned(true);
            return true;
        }
    }

    return false;
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
