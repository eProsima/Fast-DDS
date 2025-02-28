// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_DOMAIN__DOMAINPARTICIPANTIMPL_HPP
#define FASTDDS_DOMAIN__DOMAINPARTICIPANTIMPL_HPP

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/topic/TopicProxy.hpp>
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/rpc/Service.hpp>
#include <fastdds/dds/rpc/Requester.hpp>
#include <fastdds/dds/rpc/Replier.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;
class DomainParticipantListener;
class PublisherListener;
class TopicDescription;

class DomainParticipantImpl
{
    friend class DomainParticipantFactory;
    friend class DomainParticipant;

protected:

    DomainParticipantImpl(
            DomainParticipant* dp,
            DomainId_t did,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listen = nullptr)
        : domain_id_(did)
        , qos_(qos)
        , rtps_participant_(nullptr)
        , participant_(dp)
        , listener_(listen)
        , default_pub_qos_(PUBLISHER_QOS_DEFAULT)
        , default_sub_qos_(SUBSCRIBER_QOS_DEFAULT)
        , default_topic_qos_(TOPIC_QOS_DEFAULT)
        , id_counter_(0)
#pragma warning (disable : 4355)
        , rtps_listener_(this)
    {
        participant_->impl_ = this;

        guid_.guidPrefix.value[11] = 1;
        eprosima::fastdds::xmlparser::TopicAttributes top_attr;
        eprosima::fastdds::xmlparser::XMLProfileManager::getDefaultTopicAttributes(top_attr);
        default_topic_qos_.history() = top_attr.historyQos;
        default_topic_qos_.resource_limits() = top_attr.resourceLimitsQos;
    }

    virtual ~DomainParticipantImpl()
    {
        if (rtps_participant_ != nullptr)
        {
            eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(rtps_participant_);
        }

        if (participant_)
        {
            participant_->impl_ = nullptr;
            delete participant_;
            participant_ = nullptr;
        }
    }

public:

    MOCK_METHOD0(delete_topic_mock, bool());

    virtual ReturnCode_t enable()
    {
        fastdds::rtps::RTPSParticipantAttributes rtps_attr;

        rtps_participant_ = eprosima::fastdds::rtps::RTPSDomain::createParticipant(
            domain_id_, false, rtps_attr, &rtps_listener_);

        return RETCODE_OK;
    }

    ReturnCode_t get_qos(
            DomainParticipantQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    const DomainParticipantQos& get_qos() const
    {
        return qos_;
    }

    ReturnCode_t set_qos(
            const DomainParticipantQos& /*qos*/)
    {
        return RETCODE_OK;
    }

    ReturnCode_t set_listener(
            DomainParticipantListener* /*listener*/)
    {
        return RETCODE_OK;
    }

    ReturnCode_t set_listener(
            DomainParticipantListener* /*listener*/,
            const std::chrono::seconds /*timeout*/)
    {
        return RETCODE_OK;
    }

    const DomainParticipantListener* get_listener() const
    {
        return listener_;
    }

    Publisher* create_publisher(
            const PublisherQos& qos,
            PublisherListener* listener,
            const StatusMask& mask)
    {
        return create_publisher(qos, nullptr, listener, mask);
    }

    Publisher* create_publisher(
            const PublisherQos& qos,
            PublisherImpl** impl,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        PublisherImpl* pubimpl = create_publisher_impl(qos, listener);
        Publisher* pub = new Publisher(pubimpl, mask);
        pubimpl->user_publisher_ = pub;

        std::lock_guard<std::mutex> lock(mtx_pubs_);
        publishers_[pub] = pubimpl;
        pub->enable();

        if (impl)
        {
            *impl = pubimpl;
        }

        return pub;
    }

    Publisher* create_publisher_with_profile(
            const std::string& /*profile_name*/,
            PublisherListener* listener,
            const StatusMask& mask)
    {
        return create_publisher(PUBLISHER_QOS_DEFAULT, listener, mask);
    }

    ReturnCode_t delete_publisher(
            const Publisher* pub)
    {
        if (participant_ != pub->get_participant())
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> lock(mtx_pubs_);
        auto pit = publishers_.find(const_cast<Publisher*>(pub));
        if (pit != publishers_.end())
        {
            if (pub->has_datawriters())
            {
                return RETCODE_PRECONDITION_NOT_MET;
            }
            delete pit->second;
            publishers_.erase(pit);
            return RETCODE_OK;
        }
        return RETCODE_ERROR;
    }

    Subscriber* create_subscriber(
            const SubscriberQos& qos,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        SubscriberImpl* subimpl = create_subscriber_impl(qos, listener);
        Subscriber* sub = new Subscriber(subimpl, mask);
        subimpl->user_subscriber_ = sub;

        std::lock_guard<std::mutex> lock(mtx_subs_);
        subscribers_[sub] = subimpl;
        sub->enable();
        return sub;
    }

    Subscriber* create_subscriber_with_profile(
            const std::string& /*profile_name*/,
            SubscriberListener* listener,
            const StatusMask& mask)
    {
        return create_subscriber(SUBSCRIBER_QOS_DEFAULT, listener, mask);
    }

    ReturnCode_t delete_subscriber(
            const Subscriber* sub)
    {
        if (participant_ != sub->get_participant())
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> lock(mtx_subs_);
        auto sit = subscribers_.find(const_cast<Subscriber*>(sub));
        if (sit != subscribers_.end())
        {
            if (sub->has_datareaders())
            {
                return RETCODE_PRECONDITION_NOT_MET;
            }
            delete sit->second;
            subscribers_.erase(sit);
            return RETCODE_OK;
        }
        return RETCODE_ERROR;
    }

    Topic* create_topic(
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos = TOPIC_QOS_DEFAULT,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        TypeSupport type_support = find_type(type_name);
        if (type_support.empty())
        {
            return nullptr;
        }
        if (RETCODE_OK != TopicImpl::check_qos(qos))
        {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(mtx_topics_);
        if (topics_.find(topic_name) != topics_.end())
        {
            return nullptr;
        }
        TopicImpl* topic_impl = new TopicImpl(nullptr, this, type_support, qos, listener);
        TopicProxy* proxy = new TopicProxy(topic_name, type_name, mask, topic_impl);
        Topic* topic = proxy->get_topic();
        topics_[topic_name] = proxy;
        topics_impl_[topic_name] = topic_impl;
        topic->enable();
        return topic;
    }

    Topic* create_topic_with_profile(
            const std::string& topic_name,
            const std::string& type_name,
            const std::string& /*profile_name*/,
            TopicListener* listener,
            const StatusMask& mask)
    {
        return create_topic(topic_name, type_name, TOPIC_QOS_DEFAULT, listener, mask);
    }

    Topic* find_topic(
            const std::string& /*topic_name*/,
            const fastdds::dds::Duration_t& /*timeout*/)
    {
        return nullptr;
    }

    void set_topic_listener(
            const TopicProxyFactory* /*factory*/,
            TopicImpl* /*impl*/,
            TopicListener* /*listener*/,
            const StatusMask& /*mask*/)
    {
    }

    ReturnCode_t delete_topic(
            const Topic* topic)
    {
        auto topic_name = topic->get_name();

        if (delete_topic_mock())
        {
            return RETCODE_ERROR;
        }
        if (nullptr == topic)
        {
            return RETCODE_BAD_PARAMETER;
        }
        if (participant_ != topic->get_participant())
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }

        std::lock_guard<std::mutex> lock(mtx_topics_);
        auto it = topics_.find(topic_name);
        if (it != topics_.end())
        {
            if (it->second->is_referenced())
            {
                return RETCODE_PRECONDITION_NOT_MET;
            }
            delete it->second;
            topics_.erase(it);

            // Destroy also impl, that must exist
            delete topics_impl_[topic_name];
            topics_impl_.erase(topic_name);

            return RETCODE_OK;
        }
        return RETCODE_ERROR;
    }

    MOCK_METHOD5(create_contentfilteredtopic, ContentFilteredTopic * (
                const std::string& name,
                Topic * related_topic,
                const std::string& filter_expression,
                const std::vector<std::string>& expression_parameters,
                const char* filter_class_name));

    MOCK_METHOD1(delete_contentfilteredtopic, ReturnCode_t(
                const ContentFilteredTopic * topic));

    MOCK_METHOD2(register_content_filter_factory, ReturnCode_t(
                const char* filter_class_name,
                IContentFilterFactory* const filter_factory));

    MOCK_METHOD1(lookup_content_filter_factory, IContentFilterFactory * (
                const char* filter_class_name));

    MOCK_METHOD1(unregister_content_filter_factory, ReturnCode_t (
                const char* filter_class_name));

    MOCK_METHOD1(find_content_filter_factory, IContentFilterFactory * (
                const char* filter_class_name));

    MOCK_METHOD1(ignore_participant, bool (
                const fastdds::rtps::InstanceHandle_t& handle));

    MOCK_METHOD1(find_service_type, rpc::ServiceTypeSupport(
                const std::string& service_name));

    MOCK_METHOD2(register_service_type, ReturnCode_t(
                rpc::ServiceTypeSupport service_type,
                const std::string& service_type_name));

    MOCK_METHOD1(unregister_service_type, ReturnCode_t(
                const std::string& service_name));

    MOCK_METHOD2(create_service, rpc::Service* (
                const std::string& service_name,
                const std::string& service_type_name));

    MOCK_METHOD1(find_service, rpc::Service* (
                const std::string& service_name));

    MOCK_METHOD1(delete_service, ReturnCode_t(
                const rpc::Service* service));

    MOCK_METHOD2(create_service_requester, rpc::Requester* (
                rpc::Service* service,
                const RequesterQos& requester_qos));

    MOCK_METHOD2(delete_service_requester, ReturnCode_t(
                const std::string& service_name,
                rpc::Requester* requester));

    MOCK_METHOD2(create_service_replier, rpc::Replier* (
                rpc::Service* service,
                const ReplierQos& replier_qos));

    MOCK_METHOD2(delete_service_replier, ReturnCode_t(
                const std::string& service_name,
                rpc::Replier* replier));


    TopicDescription* lookup_topicdescription(
            const std::string& topic_name) const
    {
        auto it = topics_.find(topic_name);
        if (it != topics_.end())
        {
            return it->second->get_topic();
        }
        return nullptr;
    }

    ReturnCode_t register_type(
            TypeSupport type,
            const std::string& type_name)
    {
        if (type_name.size() <= 0)
        {
            return RETCODE_BAD_PARAMETER;
        }
        TypeSupport t = find_type(type_name);
        if (!t.empty())
        {
            if (t == type)
            {
                return RETCODE_OK;
            }
            return RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> lock(mtx_types_);
        types_.insert(std::make_pair(type_name, type));
        return RETCODE_OK;
    }

    ReturnCode_t unregister_type(
            const std::string& type_name)
    {
        if (type_name.size() <= 0)
        {
            return RETCODE_BAD_PARAMETER;
        }
        TypeSupport t = find_type(type_name);
        if (t.empty())
        {
            return RETCODE_OK;
        }
        {
            std::lock_guard<std::mutex> lock(mtx_subs_);
            for (auto sit : subscribers_)
            {
                if (sit.second->type_in_use(type_name))
                {
                    return RETCODE_PRECONDITION_NOT_MET;
                }
            }
        }
        {
            std::lock_guard<std::mutex> lock(mtx_pubs_);
            for (auto pit : publishers_)
            {
                if (pit.second->type_in_use(type_name))
                {
                    return RETCODE_PRECONDITION_NOT_MET;
                }
            }
        }
        std::lock_guard<std::mutex> lock(mtx_types_);
        types_.erase(type_name);
        return RETCODE_OK;
    }

    DomainId_t get_domain_id() const
    {
        return domain_id_;
    }

    ReturnCode_t assert_liveliness()
    {
        return RETCODE_OK;
    }

    ReturnCode_t set_default_publisher_qos(
            const PublisherQos& /*qos*/)
    {
        return RETCODE_OK;
    }

    const PublisherQos& get_default_publisher_qos() const
    {
        return default_pub_qos_;
    }

    ReturnCode_t get_publisher_qos_from_profile(
            const std::string& /*profile_name*/,
            PublisherQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_publisher_qos_from_xml(
            const std::string& /*xml*/,
            PublisherQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_publisher_qos_from_xml(
            const std::string& /*xml*/,
            PublisherQos& /*qos*/,
            const std::string& /*profile_name*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_default_publisher_qos_from_xml(
            const std::string& /*xml*/,
            PublisherQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t set_default_subscriber_qos(
            const SubscriberQos& /*qos*/)
    {
        return RETCODE_OK;
    }

    const SubscriberQos& get_default_subscriber_qos() const
    {
        return default_sub_qos_;
    }

    ReturnCode_t get_subscriber_qos_from_profile(
            const std::string& /*profile_name*/,
            SubscriberQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_subscriber_qos_from_xml(
            const std::string& /*xml*/,
            SubscriberQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_subscriber_qos_from_xml(
            const std::string& /*xml*/,
            SubscriberQos& /*qos*/,
            const std::string& /*profile_name*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_default_subscriber_qos_from_xml(
            const std::string& /*xml*/,
            SubscriberQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t set_default_topic_qos(
            const TopicQos& /*qos*/)
    {
        return RETCODE_OK;
    }

    const TopicQos& get_default_topic_qos() const
    {
        return default_topic_qos_;
    }

    ReturnCode_t get_topic_qos_from_profile(
            const std::string& /*profile_name*/,
            TopicQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_topic_qos_from_profile(
            const std::string& /*profile_name*/,
            TopicQos& /*qos*/,
            std::string& /*topic_name*/,
            std::string& /*topic_data_type*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& /*xml*/,
            TopicQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& /*xml*/,
            TopicQos& /*qos*/,
            std::string& /*topic_name*/,
            std::string& /*topic_data_type*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& /*xml*/,
            TopicQos& /*qos*/,
            const std::string& /*profile_name*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& /*xml*/,
            TopicQos& /*qos*/,
            std::string& /*topic_name*/,
            std::string& /*topic_data_type*/,
            const std::string& /*profile_name*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_default_topic_qos_from_xml(
            const std::string& /*xml*/,
            TopicQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_default_topic_qos_from_xml(
            const std::string& /*xml*/,
            TopicQos& /*qos*/,
            std::string& /*topic_name*/,
            std::string& /*topic_data_type*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_replier_qos_from_profile(
            const std::string& /*profile_name*/,
            ReplierQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_replier_qos_from_xml(
            const std::string& /*xml*/,
            ReplierQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_replier_qos_from_xml(
            const std::string& /*xml*/,
            ReplierQos& /*qos*/,
            const std::string& /*profile_name*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_default_replier_qos_from_xml(
            const std::string& /*xml*/,
            ReplierQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_requester_qos_from_profile(
            const std::string& /*profile_name*/,
            RequesterQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_requester_qos_from_xml(
            const std::string& /*xml*/,
            RequesterQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_requester_qos_from_xml(
            const std::string& /*xml*/,
            RequesterQos& /*qos*/,
            const std::string& /*profile_name*/) const
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_default_requester_qos_from_xml(
            const std::string& /*xml*/,
            RequesterQos& /*qos*/) const
    {
        return RETCODE_OK;
    }

    bool contains_entity(
            const InstanceHandle_t& /*handle*/,
            bool /*recursive*/) const
    {
        return true;
    }

    ReturnCode_t get_current_time(
            fastdds::dds::Time_t& /*current_time*/) const
    {
        return RETCODE_OK;
    }

    DomainParticipant* get_participant() const
    {
        return participant_;
    }

    fastdds::rtps::RTPSParticipant* get_rtps_participant()
    {
        return rtps_participant_;
    }

    const TypeSupport find_type(
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

    const InstanceHandle_t& get_instance_handle() const
    {
        return static_cast<const InstanceHandle_t&>(guid_);
    }

    const fastdds::rtps::GUID_t& guid() const
    {
        return guid_;
    }

    std::vector<std::string> get_participant_names() const
    {
        return std::vector<std::string> {};
    }

    bool new_remote_endpoint_discovered(
            const fastdds::rtps::GUID_t& /*partguid*/,
            uint16_t /*endpointId*/,
            fastdds::rtps::EndpointKind_t /*kind*/)
    {
        return false;
    }

    fastdds::rtps::ResourceEvent& get_resource_event() const
    {
        return rtps_participant_->get_resource_event();
    }

    virtual void disable()
    {
        rtps_listener_.participant_ = nullptr;
    }

    bool has_active_entities()
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

    ReturnCode_t enable_monitor_service() const
    {
        return RETCODE_OK;
    }

    ReturnCode_t disable_monitor_service() const
    {
        return RETCODE_OK;
    }

    virtual ReturnCode_t delete_contained_entities()
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
            delete it_pubs->second;
            it_pubs = publishers_.erase(it_pubs);
        }

        std::lock_guard<std::mutex> lock_topics(mtx_topics_);

        auto it_topics = topics_.begin();

        while (it_topics != topics_.end())
        {
            delete it_topics->second;
            it_topics = topics_.erase(it_topics);
        }

        return RETCODE_OK;
    }

    DomainParticipantListener* get_listener_for(
            const StatusMask& /*status*/)
    {
        return nullptr;
    }

    std::atomic<uint32_t>& id_counter()
    {
        return id_counter_;
    }

    bool fill_type_information(
            const TypeSupport& /*type*/,
            xtypes::TypeInformationParameter& /*type_information*/)
    {
        return false;
    }

protected:

    DomainId_t domain_id_;
    fastdds::rtps::GUID_t guid_;
    DomainParticipantQos qos_;
    fastdds::rtps::RTPSParticipant* rtps_participant_;
    DomainParticipant* participant_;
    DomainParticipantListener* listener_;
    std::map<Publisher*, PublisherImpl*> publishers_;
    mutable std::mutex mtx_pubs_;
    PublisherQos default_pub_qos_;
    std::map<Subscriber*, SubscriberImpl*> subscribers_;
    mutable std::mutex mtx_subs_;
    SubscriberQos default_sub_qos_;
    std::map<std::string, TopicProxy*> topics_;
    std::map<std::string, TopicImpl*> topics_impl_;
    mutable std::mutex mtx_topics_;
    std::map<std::string, TypeSupport> types_;
    mutable std::mutex mtx_types_;
    TopicQos default_topic_qos_;
    std::atomic<uint32_t> id_counter_;

    class MyRTPSParticipantListener : public fastdds::rtps::RTPSParticipantListener
    {
    public:

        MyRTPSParticipantListener(
                DomainParticipantImpl* impl)
            : participant_(impl)
        {
        }

        virtual ~MyRTPSParticipantListener() = default;

        DomainParticipantImpl* participant_;

    }
    rtps_listener_;

    virtual PublisherImpl* create_publisher_impl(
            const PublisherQos& qos,
            PublisherListener* listener)
    {
        return new PublisherImpl(this, qos, listener);
    }

    virtual SubscriberImpl* create_subscriber_impl(
            const SubscriberQos& qos,
            SubscriberListener* listener)
    {
        return new SubscriberImpl(this, qos, listener);
    }

    static bool set_qos(
            DomainParticipantQos& /*to*/,
            const DomainParticipantQos& /*from*/,
            bool /*first_time*/)
    {
        return false;
    }

    static ReturnCode_t check_qos(
            const DomainParticipantQos& /*qos*/)
    {
        return RETCODE_OK;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DOMAIN__DOMAINPARTICIPANTIMPL_HPP
