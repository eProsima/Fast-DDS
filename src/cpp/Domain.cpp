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

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

std::vector<Domain::t_p_Participant> Domain::m_participants;


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

Publisher* Domain::createPublisher(Participant* part,PublisherAttributes& att,
        PublisherListener* listen )
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

Subscriber* Domain::createSubscriber(Participant* part,SubscriberAttributes& att,
        SubscriberListener* listen )
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

} /* namespace fastrtps */
} /* namespace eprosima */
