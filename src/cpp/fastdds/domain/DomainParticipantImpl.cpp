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

#include "DomainParticipantImpl.hpp"
#include <fastdds/domain/DomainParticipant.hpp>
#include <fastdds/domain/DomainParticipantListener.hpp>
#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/reader/ReaderDiscoveryInfo.h>
#include <fastrtps/rtps/writer/WriterDiscoveryInfo.h>

#include <fastrtps/topic/TopicDataType.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>

#include <fastrtps/attributes/PublisherAttributes.h>
#include "../publisher/PublisherImpl.hpp"
#include <fastdds/publisher/Publisher.hpp>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include "../subscriber/SubscriberImpl.hpp"
#include <fastdds/subscriber/Subscriber.hpp>

#include <fastdds/topic/DataReader.hpp>
#include <fastdds/topic/DataWriter.hpp>

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/UDPv6Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>

#include <fastrtps/log/Log.h>

#include <chrono>

using namespace eprosima;
using namespace eprosima::fastdds;

using fastrtps::ParticipantAttributes;
using fastrtps::TopicDataType;
using fastrtps::rtps::RTPSDomain;
using fastrtps::rtps::RTPSParticipant;
using fastrtps::rtps::ParticipantDiscoveryInfo;
#if HAVE_SECURITY
using fastrtps::rtps::ParticipantAuthenticationInfo;
#endif
using fastrtps::rtps::ReaderDiscoveryInfo;
using fastrtps::rtps::ReaderProxyData;
using fastrtps::rtps::WriterDiscoveryInfo;
using fastrtps::rtps::WriterProxyData;
using fastrtps::rtps::InstanceHandle_t;
using fastrtps::rtps::GUID_t;
using fastrtps::rtps::EndpointKind_t;
using fastrtps::rtps::ResourceEvent;
using fastrtps::Log;

DomainParticipantImpl::DomainParticipantImpl(
        const ParticipantAttributes& patt,
        DomainParticipant* pspart,
        DomainParticipantListener* listen)
    : att_(patt)
    , rtps_participant_(nullptr)
    , participant_(pspart)
    , listener_(listen)
#pragma warning (disable : 4355 )
    , rtps_listener_(this)
    {
        participant_->impl_ = this;
    }

DomainParticipantImpl::~DomainParticipantImpl()
{
    {
        std::lock_guard<std::mutex> lock(mtx_pubs_);

        for (auto pub_it : publishers_)
        {
            delete_publisher(pub_it.first);
        }
        publishers_.clear();
        publishers_by_handle_.clear();
    }

    {
        std::lock_guard<std::mutex> lock(mtx_subs_);

        for (auto sub_it : subscribers_)
        {
            delete_subscriber(sub_it.first);
        }
        subscribers_.clear();
        subscribers_by_handle_.clear();
    }

    if(rtps_participant_ != nullptr)
    {
        RTPSDomain::removeRTPSParticipant(rtps_participant_);
    }

    delete participant_;
}


bool DomainParticipantImpl::delete_publisher(
        Publisher* pub)
{
    std::lock_guard<std::mutex> lock(mtx_pubs_);
    auto pit = publishers_.find(pub);
    if (pit != publishers_.end())
    {
        if (pub->get_instance_handle() == pit->second->get_instance_handle())
        {
            publishers_by_handle_.erase(publishers_by_handle_.find(pub->get_instance_handle()));
            delete pit->second;
            publishers_.erase(pit);
            return true;
        }
    }

    return false;
}

bool DomainParticipantImpl::delete_subscriber(
        Subscriber* sub)
{
    std::lock_guard<std::mutex> lock(mtx_subs_);
    auto sit = subscribers_.find(sub);
    if (sit != subscribers_.end())
    {
        if (sub->get_instance_handle() == sit->second->get_instance_handle())
        {
            subscribers_by_handle_.erase(subscribers_by_handle_.find(sub->get_instance_handle()));
            delete sit->second;
            subscribers_.erase(sit);
            return true;
        }
    }

    return false;
}

const InstanceHandle_t& DomainParticipantImpl::get_instance_handle() const
{
    return static_cast<const InstanceHandle_t&>(rtps_participant_->getGuid());
}

const GUID_t& DomainParticipantImpl::guid() const
{
    return rtps_participant_->getGuid();
}

Publisher* DomainParticipantImpl::create_publisher(
        const PublisherQos& qos,
        const fastrtps::PublisherAttributes& att,
        PublisherListener* listen)
{
    if(att_.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol)
    {
        if(att.getUserDefinedID() <= 0)
        {
            logError(PARTICIPANT,"Static EDP requires user defined Id");
            return nullptr;
        }
    }

    if(!att.unicastLocatorList.isValid())
    {
        logError(PARTICIPANT,"Unicast Locator List for Publisher contains invalid Locator");
        return nullptr;
    }

    if(!att.multicastLocatorList.isValid())
    {
        logError(PARTICIPANT," Multicast Locator List for Publisher contains invalid Locator");
        return nullptr;
    }

    if(!att.remoteLocatorList.isValid())
    {
        logError(PARTICIPANT,"Remote Locator List for Publisher contains invalid Locator");
        return nullptr;
    }

    if(!att.qos.checkQos() || !att.topic.checkQos())
    {
        return nullptr;
    }

    //TODO CONSTRUIR LA IMPLEMENTACION DENTRO DEL OBJETO DEL USUARIO.
    PublisherImpl* pubimpl = new PublisherImpl(this, qos, att, listen);
    Publisher* pub = new Publisher(pubimpl);
    pubimpl->user_publisher_ = pub;
    pubimpl->rtps_participant_ = rtps_participant_;

    // Create InstanceHandle for the new publisher
    GUID_t pub_guid = guid();
    do
    {
        pub_guid.entityId = fastrtps::rtps::c_EntityId_Unknown;
        rtps_participant_->get_new_entity_id(pub_guid.entityId);
    } while (exists_entity_id(pub_guid.entityId));

    InstanceHandle_t pub_handle(pub_guid);
    pubimpl->handle_ = pub_handle;

    //SAVE THE PUBLISHER INTO MAPS
    std::lock_guard<std::mutex> lock(mtx_pubs_);
    publishers_by_handle_[pub_handle] = pub;
    publishers_[pub] = pubimpl;

    return pub;
}

Subscriber* DomainParticipantImpl::get_builtin_subscriber()
{
    logError(PARTICIPANT, "Not implemented.");
    return nullptr;
}

bool DomainParticipantImpl::ignore_participant(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    (void)handle;
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::ignore_topic(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    (void)handle;
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::ignore_publication(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    (void)handle;
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::ignore_subscription(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    (void)handle;
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

uint8_t DomainParticipantImpl::get_domain_id() const
{
    return static_cast<uint8_t>(att_.rtps.builtin.domainId);
}

bool DomainParticipantImpl::delete_contained_entities()
{
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::assert_liveliness()
{
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::set_default_publisher_qos(
        const fastdds::PublisherQos& qos)
{
    if (qos.checkQos())
    {
        default_pub_qos_.setQos(qos, false);
        return true;
    }
    return false;
}

const fastdds::PublisherQos& DomainParticipantImpl::get_default_publisher_qos() const
{
    return default_pub_qos_;
}

bool DomainParticipantImpl::set_default_subscriber_qos(
        const fastdds::SubscriberQos& qos)
{
    if (qos.checkQos())
    {
        default_sub_qos_.setQos(qos, false);
        return true;
    }
    return false;
}

const fastdds::SubscriberQos& DomainParticipantImpl::get_default_subscriber_qos() const
{
    return default_sub_qos_;
}

bool DomainParticipantImpl::get_discovered_participants(
        std::vector<fastrtps::rtps::InstanceHandle_t>& participant_handles) const
{
    (void)participant_handles;
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::get_discovered_topics(
        std::vector<fastrtps::rtps::InstanceHandle_t>& topic_handles) const
{
    (void)topic_handles;
    logError(PARTICIPANT, "Not implemented.");
    return false;
}

bool DomainParticipantImpl::contains_entity(
        const fastrtps::rtps::InstanceHandle_t& handle,
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

    if (recursive)
    {
        // Look into publishers
        std::vector<DataWriter*> writers;
        {
            std::lock_guard<std::mutex> lock(mtx_pubs_);
            for (auto pit : publishers_)
            {
                pit.second->get_datawriters(writers);
            }
        }

        for (DataWriter* writer : writers)
        {
            InstanceHandle_t h(writer->guid());
            if (h == handle)
            {
                return true;
            }
        }

        // Look into subscribers
        std::vector<DataReader*> readers;
        {
            std::lock_guard<std::mutex> lock(mtx_pubs_);
            for (auto pit : subscribers_)
            {
                pit.second->get_datareaders(readers);
            }
        }

        for (DataReader* reader : readers)
        {
            InstanceHandle_t h(reader->guid());
            if (h == handle)
            {
                return true;
            }
        }
    }

    return false;
}

bool DomainParticipantImpl::get_current_time(
        fastrtps::Time_t& current_time) const
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

    current_time.seconds = static_cast<int32_t>(seconds.count());
    current_time.nanosec = static_cast<uint32_t>(nanos.count());

    return true;
}

const DomainParticipant* DomainParticipantImpl::get_participant() const
{
    return participant_;
}

DomainParticipant* DomainParticipantImpl::get_participant()
{
    return participant_;
}

std::vector<std::string> DomainParticipantImpl::getParticipantNames() const
{
    return rtps_participant_->getParticipantNames();
}

Subscriber* DomainParticipantImpl::create_subscriber(
        const SubscriberQos& qos,
        const fastrtps::SubscriberAttributes& att,
        SubscriberListener* listen)
{
    if(att_.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol)
    {
        if(att.getUserDefinedID() <= 0)
        {
            logError(PARTICIPANT,"Static EDP requires user defined Id");
            return nullptr;
        }
    }

    if(!att.unicastLocatorList.isValid())
    {
        logError(PARTICIPANT,"Unicast Locator List for Subscriber contains invalid Locator");
        return nullptr;
    }

    if(!att.multicastLocatorList.isValid())
    {
        logError(PARTICIPANT," Multicast Locator List for Subscriber contains invalid Locator");
        return nullptr;
    }

    if(!att.remoteLocatorList.isValid())
    {
        logError(PARTICIPANT,"Output Locator List for Subscriber contains invalid Locator");
        return nullptr;
    }

    if(!att.qos.checkQos() || !att.topic.checkQos())
    {
        return nullptr;
    }

    SubscriberImpl* subimpl = new SubscriberImpl(this, qos, att, listen);
    Subscriber* sub = new Subscriber(subimpl);
    subimpl->user_subscriber_ = sub;
    subimpl->rtps_participant_ = this->rtps_participant_;

    // Create InstanceHandle for the new subscriber
    GUID_t sub_guid = guid();
    do
    {
        sub_guid.entityId = fastrtps::rtps::c_EntityId_Unknown;
        rtps_participant_->get_new_entity_id(sub_guid.entityId);
    } while (exists_entity_id(sub_guid.entityId));

    InstanceHandle_t sub_handle(sub_guid);
    subimpl->handle_ = sub_handle;

    //SAVE THE PUBLISHER INTO MAPS
    std::lock_guard<std::mutex> lock(mtx_subs_);
    subscribers_by_handle_[sub_handle] = sub;
    subscribers_[sub] = subimpl;

    return sub;
}


TopicDataType* DomainParticipantImpl::find_type(
        const std::string& type_name) const
{
    std::lock_guard<std::mutex> lock(mtx_types_);

    auto type_it = types_.find(type_name);

    if (type_it != types_.end())
    {
        return type_it->second;
    }

    return nullptr;
}

bool DomainParticipantImpl::register_type(
        TopicDataType* type)
{
    TopicDataType* t = find_type(type->getName());

    if (t != nullptr)
    {
        if (t == type)
        {
            return true;
        }

        logError(PARTICIPANT, "Another type with the same name '" << type->getName() << "' is already registered.");
    }

    if (strlen(type->getName()) <= 0)
    {
        logError(PARTICIPANT, "Registered Type must have a name");
        return false;
    }

    logInfo(PARTICIPANT, "Type " << type->getName() << " registered.");
    std::lock_guard<std::mutex> lock(mtx_types_);
    types_[type->getName()] = type;
    return true;
}

bool DomainParticipantImpl::unregister_type(
        const char* type_name)
{
    TopicDataType* t = find_type(type_name);

    if (t == nullptr)
    {
        return true; // Not registered, so unregistering complete.
    }

    {
        // Check is any subscriber is using the type
        std::lock_guard<std::mutex> lock(mtx_subs_);

        for (auto sit : subscribers_)
        {
            std::vector<DataReader*> readers;
            sit.second->get_datareaders(readers);
            for (DataReader* reader : readers)
            {
                if (reader->get_topic().getTopicDataType() == type_name)
                {
                    return false; // Is in use
                }
            }
        }
    }

    {
        // Check is any publisher is using the type
        std::lock_guard<std::mutex> lock(mtx_pubs_);

        for (auto pit : publishers_)
        {
            std::vector<DataWriter*> writers;
            pit.second->get_datawriters(writers);
            for (DataWriter* writer : writers)
            {
                if (writer->get_topic().getTopicDataType() == type_name)
                {
                    return false; // Is in use
                }
            }
        }
    }

    types_.erase(type_name);

    return true;
}

void DomainParticipantImpl::MyRTPSParticipantListener::onParticipantDiscovery(
        RTPSParticipant*,
        ParticipantDiscoveryInfo&& info)
{
    if (participant_->listener_ != nullptr)
    {
        participant_->listener_->onParticipantDiscovery(participant_->participant_, std::move(info));
    }
}

#if HAVE_SECURITY
void DomainParticipantImpl::MyRTPSParticipantListener::onParticipantAuthentication(
        RTPSParticipant*,
        ParticipantAuthenticationInfo&& info)
{
    if (participant_->listener_ != nullptr)
    {
        participant_->listener_->onParticipantAuthentication(participant_->participant_, std::move(info));
    }
}
#endif

void DomainParticipantImpl::MyRTPSParticipantListener::onReaderDiscovery(
        RTPSParticipant*,
        ReaderDiscoveryInfo&& info)
{
    if (participant_->listener_ != nullptr)
    {
        participant_->listener_->onSubscriberDiscovery(participant_->participant_, std::move(info));
    }
}

void DomainParticipantImpl::MyRTPSParticipantListener::onWriterDiscovery(
        RTPSParticipant*,
        WriterDiscoveryInfo&& info)
{
    if (participant_->listener_ != nullptr)
    {
        participant_->listener_->onPublisherDiscovery(participant_->participant_, std::move(info));
    }
}

bool DomainParticipantImpl::newRemoteEndpointDiscovered(
        const GUID_t& partguid,
        uint16_t endpointId,
        EndpointKind_t kind)
{
    if (kind == fastrtps::rtps::WRITER)
    {
        return rtps_participant_->newRemoteWriterDiscovered(partguid, static_cast<int16_t>(endpointId));
    }
    else
    {
        return rtps_participant_->newRemoteReaderDiscovered(partguid, static_cast<int16_t>(endpointId));
    }
}

bool DomainParticipantImpl::get_remote_writer_info(
        const GUID_t& writerGuid,
        WriterProxyData& returnedInfo)
{
    return rtps_participant_->get_remote_writer_info(writerGuid, returnedInfo);
}

bool DomainParticipantImpl::get_remote_reader_info(
        const GUID_t& readerGuid,
        ReaderProxyData& returnedInfo)
{
    return rtps_participant_->get_remote_reader_info(readerGuid, returnedInfo);
}

ResourceEvent& DomainParticipantImpl::get_resource_event() const
{
    return rtps_participant_->get_resource_event();
}

bool DomainParticipantImpl::exists_entity_id(
        const fastrtps::rtps::EntityId_t& entity_id) const
{
    GUID_t g = guid();
    g.entityId = entity_id;
    InstanceHandle_t instance(g);

    return contains_entity(instance, false);
}
