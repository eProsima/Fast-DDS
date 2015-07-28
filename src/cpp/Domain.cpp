/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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

#include <fastrtps/utils/RTPSLog.h>

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

static const char* const CLASS_NAME = "Domain";

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
	Log::removeLog();
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
	const char* const METHOD_NAME = "createParticipant";

	Participant* pubsubpar = new Participant();
	ParticipantImpl* pspartimpl = new ParticipantImpl(att,pubsubpar,listen);

	RTPSParticipant* part = RTPSDomain::createParticipant(att.rtps,&pspartimpl->m_rtps_listener);

	if(part == nullptr)
	{
		logError(PARTICIPANT,"Problem creating RTPSParticipant");
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


} /* namespace pubsub */
} /* namespace eprosima */
