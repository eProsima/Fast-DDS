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
 * @file Domain.cpp
 *
 */

#include <fastrtps/Domain.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/participant/Participant.h>
#include "participant/ParticipantImpl.h"

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>

#include <fastrtps/utils/eClock.h>

#include <fastrtps/log/Log.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::xmlparser;

namespace eprosima {
namespace fastrtps {

std::vector<Domain::t_p_Participant> Domain::m_participants;
bool Domain::default_xml_profiles_loaded = false;


Domain::Domain()
{
    // TODO Auto-generated constructor stub

}

Domain::~Domain()
{

}

void Domain::stopAll()
{
    while(m_participants.size()>0)
    {
        Domain::removeParticipant(m_participants.begin()->first);
    }
    eClock::my_sleep(100);
    Log::KillThread();
}

bool Domain::removeParticipant(Participant* part)
{
    if(part!=nullptr)
    {
        for(auto it = m_participants.begin();it!= m_participants.end();++it)
        {
            if(it->second->getGuid() == part->getGuid())
            {
                //FOUND
                delete(it->second);
                m_participants.erase(it);
                return true;
            }
        }
    }
    return false;
}

bool Domain::removePublisher(Publisher* pub)
{
    if(pub!=nullptr)
    {
        for(auto it = m_participants.begin();it!= m_participants.end();++it)
        {
            if(it->second->getGuid().guidPrefix == pub->getGuid().guidPrefix)
            {
                //FOUND
                return it->second->removePublisher(pub);
            }
        }
    }
    return false;
}

bool Domain::removeSubscriber(Subscriber* sub)
{
    if(sub!=nullptr)
    {
        for(auto it = m_participants.begin();it!= m_participants.end();++it)
        {
            if(it->second->getGuid().guidPrefix == sub->getGuid().guidPrefix)
            {
                //FOUND
                return it->second->removeSubscriber(sub);
            }
        }
    }
    return false;
}

Participant* Domain::createParticipant(const std::string &participant_profile, ParticipantListener* listen)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    ParticipantAttributes participant_att;
    if ( XMLP_ret::XML_ERROR == XMLProfileManager::fillParticipantAttributes(participant_profile, participant_att))
    {
        logError(PARTICIPANT, "Problem loading profile '" << participant_profile << "'");
        return nullptr;
    }
    return createParticipant(participant_att, listen);
}

Participant* Domain::createParticipant(ParticipantAttributes& att,ParticipantListener* listen)
{
    Participant* pubsubpar = new Participant();
    ParticipantImpl* pspartimpl = new ParticipantImpl(att,pubsubpar,listen);
    RTPSParticipant* part = RTPSDomain::createParticipant(att.rtps,&pspartimpl->m_rtps_listener);

    if(part == nullptr)
    {
        logError(PARTICIPANT,"Problem creating RTPSParticipant");
        delete pspartimpl;
        return nullptr;
    }

    pspartimpl->mp_rtpsParticipant = part;
    t_p_Participant pubsubpair;
    pubsubpair.first = pubsubpar;
    pubsubpair.second = pspartimpl;

    m_participants.push_back(pubsubpair);
    return pubsubpar;
}

void Domain::getDefaultParticipantAttributes(ParticipantAttributes& participant_attributes)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    return XMLProfileManager::getDefaultParticipantAttributes(participant_attributes);
}

Publisher* Domain::createPublisher(Participant *part, const std::string &publisher_profile, PublisherListener *listen)
{
    PublisherAttributes publisher_att;
    if ( XMLP_ret::XML_ERROR == XMLProfileManager::fillPublisherAttributes(publisher_profile, publisher_att))
    {
        logError(PUBLISHER, "Problem loading profile '" << publisher_profile << "'");
        return nullptr;
    }
    return createPublisher(part, publisher_att, listen);
}

Publisher* Domain::createPublisher(Participant *part, PublisherAttributes &att, PublisherListener *listen)
{
    for (auto it = m_participants.begin(); it != m_participants.end(); ++it)
    {
        if(it->second->getGuid() == part->getGuid())
        {
            return part->mp_impl->createPublisher(att,listen);
        }
    }
    //TODO MOSTRAR MENSAJE DE ERROR WARNING y COMPROBAR QUE EL PUNTERO QUE ME PASA NO ES NULL
    return nullptr;
}

void Domain::getDefaultPublisherAttributes(PublisherAttributes& publisher_attributes)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    return XMLProfileManager::getDefaultPublisherAttributes(publisher_attributes);
}

void Domain::getDefaultSubscriberAttributes(SubscriberAttributes& subscriber_attributes)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    return XMLProfileManager::getDefaultSubscriberAttributes(subscriber_attributes);
}

Subscriber* Domain::createSubscriber(Participant *part, const std::string &subscriber_profile, SubscriberListener *listen)
{
    SubscriberAttributes subscriber_att;
    if ( XMLP_ret::XML_ERROR == XMLProfileManager::fillSubscriberAttributes(subscriber_profile, subscriber_att))
    {
        logError(PUBLISHER, "Problem loading profile '" << subscriber_profile << "'");
        return nullptr;
    }
    return createSubscriber(part, subscriber_att, listen);
}

Subscriber* Domain::createSubscriber(Participant *part, SubscriberAttributes &att, SubscriberListener *listen)
{
    for (auto it = m_participants.begin(); it != m_participants.end(); ++it)
    {
        if(it->second->getGuid() == part->getGuid())
        {
            return part->mp_impl->createSubscriber(att,listen);
        }
    }
    return nullptr;
}

bool Domain::getRegisteredType(Participant* part, const char* typeName, TopicDataType** type)
{
    for (auto it = m_participants.begin(); it != m_participants.end();++it)
    {
        if(it->second->getGuid() == part->getGuid())
        {
            return part->mp_impl->getRegisteredType(typeName, type);
        }
    }
    return false;
}

bool Domain::registerType(Participant* part, TopicDataType* type)
{
    //TODO El registro debería hacerse de manera que no tengamos un objeto del usuario sino que tengamos un objeto TopicDataTYpe propio para que no
    //haya problemas si el usuario lo destruye antes de tiempo.
    for (auto it = m_participants.begin(); it != m_participants.end();++it)
    {
        if(it->second->getGuid() == part->getGuid())
        {
            return part->mp_impl->registerType(type);
        }
    }
    return false;
}

bool Domain::unregisterType(Participant* part, const char* typeName)
{
    //TODO El registro debería hacerse de manera que no tengamos un objeto del usuario sino que tengamos un objeto TopicDataTYpe propio para que no
    //haya problemas si el usuario lo destruye antes de tiempo.
    for (auto it = m_participants.begin(); it != m_participants.end();++it)
    {
        if(it->second->getGuid() == part->getGuid())
        {
            return part->mp_impl->unregisterType(typeName);
        }
    }
    return true;
}

bool Domain::loadXMLProfilesFile(const std::string &xml_profile_file)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    if ( XMLP_ret::XML_ERROR == XMLProfileManager::loadXMLFile(xml_profile_file))
    {
        logError(DOMAIN, "Problem loading XML file '" << xml_profile_file << "'");
        return false;
    }
    return true;
}

} /* namespace fastrtps */
} /* namespace eprosima */
