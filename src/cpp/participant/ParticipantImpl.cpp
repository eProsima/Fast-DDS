// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ParticipantImpl.cpp
 *
 */

#include "ParticipantImpl.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/reader/ReaderDiscoveryInfo.h>
#include <fastrtps/rtps/writer/WriterDiscoveryInfo.h>
#include <fastrtps/participant/ParticipantListener.h>

#include <fastrtps/topic/TopicDataType.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>

#include <fastrtps/attributes/PublisherAttributes.h>
#include "../publisher/PublisherImpl.h"
#include <fastrtps/publisher/Publisher.h>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include "../subscriber/SubscriberImpl.h"
#include <fastrtps/subscriber/Subscriber.h>

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/UDPv6Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>

#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

ParticipantImpl::ParticipantImpl(
        const ParticipantAttributes& patt,
        Participant* pspart,
        ParticipantListener* listen)
    : m_att(patt)
    , mp_rtpsParticipant(nullptr)
    , mp_participant(pspart)
    , mp_listener(listen)
#pragma warning (disable : 4355 )
    , m_rtps_listener(this)
    {
        mp_participant->mp_impl = this;
    }

ParticipantImpl::~ParticipantImpl()
{
    while(m_publishers.size()>0)
    {
        this->removePublisher(m_publishers.begin()->first);
    }
    while(m_subscribers.size()>0)
    {
        this->removeSubscriber(m_subscribers.begin()->first);
    }

    if(this->mp_rtpsParticipant != nullptr)
    {
        RTPSDomain::removeRTPSParticipant(this->mp_rtpsParticipant);
    }

    delete(mp_participant);
}


bool ParticipantImpl::removePublisher(Publisher* pub)
{
    for(auto pit = this->m_publishers.begin();pit!= m_publishers.end();++pit)
    {
        if(pit->second->getGuid() == pub->getGuid())
        {
            delete(pit->second);
            m_publishers.erase(pit);
            return true;
        }
    }
    return false;
}

bool ParticipantImpl::removeSubscriber(Subscriber* sub)
{
    for(auto sit = m_subscribers.begin();sit!= m_subscribers.end();++sit)
    {
        if(sit->second->getGuid() == sub->getGuid())
        {
            delete(sit->second);
            m_subscribers.erase(sit);
            return true;
        }
    }
    return false;
}

const GUID_t& ParticipantImpl::getGuid() const
{
    return this->mp_rtpsParticipant->getGuid();
}

Publisher* ParticipantImpl::createPublisher(
        const PublisherAttributes& att,
        PublisherListener* listen)
{
    if(m_att.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol)
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

    if(!att.qos.checkQos())
    {
        return nullptr;
    }

    //TODO CONSTRUIR LA IMPLEMENTACION DENTRO DEL OBJETO DEL USUARIO.
    PublisherImpl* pubimpl = new PublisherImpl(this,att,listen);
    Publisher* pub = new Publisher(pubimpl);
    pubimpl->mp_userPublisher = pub;
    pubimpl->mp_rtpsParticipant = this->mp_rtpsParticipant;

    //SAVE THE PUBLISHER PAIR
    t_p_PublisherPair pubpair;
    pubpair.first = pub;
    pubpair.second = pubimpl;
    m_publishers.push_back(pubpair);

    return pub;
}

std::vector<std::string> ParticipantImpl::getParticipantNames() const {
    return mp_rtpsParticipant->getParticipantNames();
}

Subscriber* ParticipantImpl::createSubscriber(
        const SubscriberAttributes& att,
        SubscriberListener* listen)
{
    if(m_att.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol)
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

    if(!att.qos.checkQos())
    {
        return nullptr;
    }

    SubscriberImpl* subimpl = new SubscriberImpl(this,att,listen);
    Subscriber* sub = new Subscriber(subimpl);
    subimpl->mp_userSubscriber = sub;
    subimpl->mp_rtpsParticipant = this->mp_rtpsParticipant;

    //SAVE THE PUBLICHER PAIR
    t_p_SubscriberPair subpair;
    subpair.first = sub;
    subpair.second = subimpl;
    m_subscribers.push_back(subpair);

    return sub;
}


bool ParticipantImpl::getRegisteredType(
        const char* typeName,
        TopicDataType** type)
{
    for(std::vector<TopicDataType*>::iterator it=m_types.begin();
            it!=m_types.end();++it)
    {
        if(strcmp((*it)->getName(),typeName)==0)
        {
            *type = *it;
            return true;
        }
    }
    return false;
}

bool ParticipantImpl::registerType(TopicDataType* type)
{
    if (type->m_typeSize <= 0)
    {
        logError(PARTICIPANT, "Registered Type must have maximum byte size > 0");
        return false;
    }
    const char * name = type->getName();
    if (strlen(name) <= 0)
    {
        logError(PARTICIPANT, "Registered Type must have a name");
        return false;
    }
    for (auto ty = m_types.begin(); ty != m_types.end();++ty)
    {
        if (strcmp((*ty)->getName(), type->getName()) == 0)
        {
            logError(PARTICIPANT, "Type with the same name already exists:" << type->getName());
            return false;
        }
    }
    m_types.push_back(type);
    logInfo(PARTICIPANT, "Type " << type->getName() << " registered.");
    return true;
}

bool ParticipantImpl::unregisterType(const char* typeName)
{
    bool retValue = true;
    std::vector<TopicDataType*>::iterator typeit;

    for (typeit = m_types.begin(); typeit != m_types.end(); ++typeit)
    {
        if(strcmp((*typeit)->getName(), typeName) == 0)
        {
            break;
        }
    }

    if(typeit != m_types.end())
    {
        bool inUse = false;

        for(auto sit = m_subscribers.begin(); !inUse && sit!= m_subscribers.end(); ++sit)
        {
            if(strcmp(sit->second->getType()->getName(), typeName) == 0)
                inUse = true;
        }

        for(auto pit = m_publishers.begin(); pit!= m_publishers.end(); ++pit)
        {
            if(strcmp(pit->second->getType()->getName(), typeName) == 0)
                inUse = true;
        }

        if(!inUse)
        {
            m_types.erase(typeit);
        }
        else
        {
            retValue =  false;
        }
    }

    return retValue;
}



void ParticipantImpl::MyRTPSParticipantListener::onParticipantDiscovery(
        RTPSParticipant*,
        rtps::ParticipantDiscoveryInfo&& info)
{
    if(this->mp_participantimpl->mp_listener!=nullptr)
    {
        this->mp_participantimpl->mp_listener->onParticipantDiscovery(mp_participantimpl->mp_participant, std::move(info));
    }
}

#if HAVE_SECURITY
void ParticipantImpl::MyRTPSParticipantListener::onParticipantAuthentication(
        RTPSParticipant*,
        ParticipantAuthenticationInfo&& info)
{
    if(this->mp_participantimpl->mp_listener != nullptr)
    {
        this->mp_participantimpl->mp_listener->onParticipantAuthentication(mp_participantimpl->mp_participant, std::move(info));
    }
}
#endif

void ParticipantImpl::MyRTPSParticipantListener::onReaderDiscovery(
        RTPSParticipant*,
        rtps::ReaderDiscoveryInfo&& info)
{
    if(this->mp_participantimpl->mp_listener!=nullptr)
    {
        this->mp_participantimpl->mp_listener->onSubscriberDiscovery(mp_participantimpl->mp_participant, std::move(info));
    }
}

void ParticipantImpl::MyRTPSParticipantListener::onWriterDiscovery(
        RTPSParticipant*,
        rtps::WriterDiscoveryInfo&& info)
{
    if(this->mp_participantimpl->mp_listener!=nullptr)
    {
        this->mp_participantimpl->mp_listener->onPublisherDiscovery(mp_participantimpl->mp_participant, std::move(info));
    }
}

bool ParticipantImpl::newRemoteEndpointDiscovered(
        const GUID_t& partguid,
        uint16_t endpointId,
        EndpointKind_t kind)
{
    if (kind == WRITER)
        return this->mp_rtpsParticipant->newRemoteWriterDiscovered(partguid, endpointId);
    else
        return this->mp_rtpsParticipant->newRemoteReaderDiscovered(partguid, endpointId);
}

bool ParticipantImpl::get_remote_writer_info(
        const GUID_t& writerGuid,
        WriterProxyData& returnedInfo)
{
    return mp_rtpsParticipant->get_remote_writer_info(writerGuid, returnedInfo);
}

bool ParticipantImpl::get_remote_reader_info(
        const GUID_t& readerGuid,
        ReaderProxyData& returnedInfo)
{
    return mp_rtpsParticipant->get_remote_reader_info(readerGuid, returnedInfo);
}

ResourceEvent& ParticipantImpl::get_resource_event() const
{
    return mp_rtpsParticipant->get_resource_event();
}
