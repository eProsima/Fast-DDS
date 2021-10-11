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

#ifndef _FASTDDS_PARTICIPANTIMPL_HPP_
#define _FASTDDS_PARTICIPANTIMPL_HPP_

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <fastdds/dds/builtin/typelookup/TypeLookupManager.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/topic/TopicImpl.hpp>

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

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
#pragma warning (disable : 4355)
        , rtps_listener_(this)
    {
        participant_->impl_ = this;

        eprosima::fastrtps::TopicAttributes top_attr;
        eprosima::fastrtps::xmlparser::XMLProfileManager::getDefaultTopicAttributes(top_attr);
        default_topic_qos_.history() = top_attr.historyQos;
        default_topic_qos_.resource_limits() = top_attr.resourceLimitsQos;
    }

    virtual ~DomainParticipantImpl()
    {
        if (rtps_participant_ != nullptr)
        {
            eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(rtps_participant_);
        }
    }

public:

    MOCK_METHOD0(delete_topic_mock, bool());

    virtual ReturnCode_t enable()
    {
        fastrtps::rtps::RTPSParticipantAttributes rtps_attr;

        rtps_participant_ = eprosima::fastrtps::rtps::RTPSDomain::createParticipant(
            domain_id_, false, rtps_attr, &rtps_listener_);

        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_qos(
            DomainParticipantQos& /*qos*/) const
    {
        return ReturnCode_t::RETCODE_OK;
    }

    const DomainParticipantQos& get_qos() const
    {
        return qos_;
    }

    ReturnCode_t set_qos(
            const DomainParticipantQos& /*qos*/)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t set_listener(
            DomainParticipantListener* /*listener*/)
    {
        return ReturnCode_t::RETCODE_OK;
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
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> lock(mtx_pubs_);
        auto pit = publishers_.find(const_cast<Publisher*>(pub));
        if (pit != publishers_.end())
        {
            if (pub->has_datawriters())
            {
                return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }
            delete pit->second;
            publishers_.erase(pit);
            return ReturnCode_t::RETCODE_OK;
        }
        return ReturnCode_t::RETCODE_ERROR;
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
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> lock(mtx_subs_);
        auto sit = subscribers_.find(const_cast<Subscriber*>(sub));
        if (sit != subscribers_.end())
        {
            if (sub->has_datareaders())
            {
                return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }
            delete sit->second;
            subscribers_.erase(sit);
            return ReturnCode_t::RETCODE_OK;
        }
        return ReturnCode_t::RETCODE_ERROR;
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
        if (!TopicImpl::check_qos(qos))
        {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(mtx_topics_);
        if (topics_.find(topic_name) != topics_.end())
        {
            return nullptr;
        }
        TopicImpl* topic_impl = new TopicImpl(this, type_support, qos, listener);
        Topic* topic = new Topic(topic_name, type_name, topic_impl, mask);
        topic_impl->user_topic_ = topic;
        topics_[topic_name] = topic_impl;
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

    ReturnCode_t delete_topic(
            const Topic* topic)
    {
        if (delete_topic_mock())
        {
            return ReturnCode_t::RETCODE_ERROR;
        }
        if (nullptr == topic)
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        if (participant_ != topic->get_participant())
        {
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> lock(mtx_topics_);
        auto it = topics_.find(topic->get_name());
        if (it != topics_.end())
        {
            if (it->second->is_referenced())
            {
                return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }
            delete it->second;
            topics_.erase(it);
            return ReturnCode_t::RETCODE_OK;
        }
        return ReturnCode_t::RETCODE_ERROR;
    }

    TopicDescription* lookup_topicdescription(
            const std::string& topic_name) const
    {
        auto it = topics_.find(topic_name);
        if (it != topics_.end())
        {
            return it->second->user_topic_;
        }
        return nullptr;
    }

    ReturnCode_t register_type(
            TypeSupport type,
            const std::string& type_name)
    {
        if (type_name.size() <= 0)
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        TypeSupport t = find_type(type_name);
        if (!t.empty())
        {
            if (t == type)
            {
                return ReturnCode_t::RETCODE_OK;
            }
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> lock(mtx_types_);
        types_.insert(std::make_pair(type_name, type));
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t unregister_type(
            const std::string& type_name)
    {
        if (type_name.size() <= 0)
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        TypeSupport t = find_type(type_name);
        if (t.empty())
        {
            return ReturnCode_t::RETCODE_OK;
        }
        {
            std::lock_guard<std::mutex> lock(mtx_subs_);
            for (auto sit : subscribers_)
            {
                if (sit.second->type_in_use(type_name))
                {
                    return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
                }
            }
        }
        {
            std::lock_guard<std::mutex> lock(mtx_pubs_);
            for (auto pit : publishers_)
            {
                if (pit.second->type_in_use(type_name))
                {
                    return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
                }
            }
        }
        std::lock_guard<std::mutex> lock(mtx_types_);
        types_.erase(type_name);
        return ReturnCode_t::RETCODE_OK;
    }

    DomainId_t get_domain_id() const
    {
        return domain_id_;
    }

    ReturnCode_t assert_liveliness()
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t set_default_publisher_qos(
            const PublisherQos& /*qos*/)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    const PublisherQos& get_default_publisher_qos() const
    {
        return default_pub_qos_;
    }

    const ReturnCode_t get_publisher_qos_from_profile(
            const std::string& /*profile_name*/,
            PublisherQos& /*qos*/) const
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t set_default_subscriber_qos(
            const SubscriberQos& /*qos*/)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    const SubscriberQos& get_default_subscriber_qos() const
    {
        return default_sub_qos_;
    }

    const ReturnCode_t get_subscriber_qos_from_profile(
            const std::string& /*profile_name*/,
            SubscriberQos& /*qos*/) const
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t set_default_topic_qos(
            const TopicQos& /*qos*/)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    const TopicQos& get_default_topic_qos() const
    {
        return default_topic_qos_;
    }

    const ReturnCode_t get_topic_qos_from_profile(
            const std::string& /*profile_name*/,
            TopicQos& /*qos*/) const
    {
        return ReturnCode_t::RETCODE_OK;
    }

    bool contains_entity(
            const InstanceHandle_t& /*handle*/,
            bool /*recursive*/) const
    {
        return true;
    }

    ReturnCode_t get_current_time(
            fastrtps::Time_t& /*current_time*/) const
    {
        return ReturnCode_t::RETCODE_OK;
    }

    DomainParticipant* get_participant() const
    {
        return participant_;
    }

    fastrtps::rtps::RTPSParticipant* rtps_participant()
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

    const fastrtps::rtps::GUID_t& guid() const
    {
        return guid_;
    }

    std::vector<std::string> get_participant_names() const
    {
        return std::vector<std::string> {};
    }

    bool new_remote_endpoint_discovered(
            const fastrtps::rtps::GUID_t& /*partguid*/,
            uint16_t /*endpointId*/,
            fastrtps::rtps::EndpointKind_t /*kind*/)
    {
        return false;
    }

    fastrtps::rtps::ResourceEvent& get_resource_event() const
    {
        return rtps_participant_->get_resource_event();
    }

    fastrtps::rtps::SampleIdentity get_type_dependencies(
            const fastrtps::types::TypeIdentifierSeq& in) const
    {
        return rtps_participant_->typelookup_manager()->get_type_dependencies(in);
    }

    fastrtps::rtps::SampleIdentity get_types(
            const fastrtps::types::TypeIdentifierSeq& in) const
    {
        return rtps_participant_->typelookup_manager()->get_types(in);
    }

    ReturnCode_t register_remote_type(
            const fastrtps::types::TypeInformation& /*type_information*/,
            const std::string& /*type_name*/,
            std::function<void(const std::string& name, const fastrtps::types::DynamicType_ptr type)>& /*callback*/)
    {
        return ReturnCode_t::RETCODE_OK;
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

    ReturnCode_t delete_contained_entities()
    {
        bool can_be_deleted = true;

        std::lock_guard<std::mutex> lock_subscribers(mtx_subs_);

        for (auto subscriber : subscribers_)
        {
            can_be_deleted = subscriber.second->can_be_deleted();
            if (!can_be_deleted)
            {
                return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }
        }

        std::lock_guard<std::mutex> lock_publishers(mtx_pubs_);



        for (auto publisher : publishers_)
        {
            can_be_deleted = publisher.second->can_be_deleted();
            if (!can_be_deleted)
            {
                return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }
        }

        ReturnCode_t ret_code = ReturnCode_t::RETCODE_OK;

        for (auto& subscriber : subscribers_)
        {
            ret_code = subscriber.first->delete_contained_entities();
            if (!ret_code)
            {
                return ReturnCode_t::RETCODE_ERROR;
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
            if (!ret_code)
            {
                return ReturnCode_t::RETCODE_ERROR;
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
            it_topics->second->set_listener(nullptr);
            delete it_topics->second;
            it_topics = topics_.erase(it_topics);
        }

        return ReturnCode_t::RETCODE_OK;
    }

    DomainParticipantListener* get_listener_for(
            const StatusMask& /*status*/)
    {
        return nullptr;
    }

    uint32_t& id_counter()
    {
        return id_counter_;
    }

protected:

    DomainId_t domain_id_;
    fastrtps::rtps::GUID_t guid_;
    DomainParticipantQos qos_;
    fastrtps::rtps::RTPSParticipant* rtps_participant_;
    DomainParticipant* participant_;
    DomainParticipantListener* listener_;
    std::map<Publisher*, PublisherImpl*> publishers_;
    mutable std::mutex mtx_pubs_;
    PublisherQos default_pub_qos_;
    std::map<Subscriber*, SubscriberImpl*> subscribers_;
    mutable std::mutex mtx_subs_;
    SubscriberQos default_sub_qos_;
    std::map<std::string, TopicImpl*> topics_;
    mutable std::mutex mtx_topics_;
    std::map<std::string, TypeSupport> types_;
    mutable std::mutex mtx_types_;
    TopicQos default_topic_qos_;
    uint32_t id_counter_ = 0;

    class MyRTPSParticipantListener : public fastrtps::rtps::RTPSParticipantListener
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
        return ReturnCode_t::RETCODE_OK;
    }

};

} // dds
} // fastdds
} // eprosima

#endif /* _FASTDDS_PARTICIPANTIMPL_HPP_ */
