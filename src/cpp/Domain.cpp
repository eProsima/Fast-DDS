/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Domain.cpp
 *
 */

#include "fastrtps/Domain.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/participant/Participant.h"
#include "fastrtps/participant/ParticipantImpl.h"

#include "fastrtps/utils/RTPSLog.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

static const char* const CLASS_NAME = "Domain";


Domain::Domain()
{
	// TODO Auto-generated constructor stub

}

Domain::~Domain()
{

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
	for(auto it : m_participants)
	{
		if(it.second->getGuid() == part->getGuid())
		{
			return part->mp_impl->createPublisher(att,listen);
		}
	}
	return nullptr;
}

Subscriber* Domain::createSubscriber(Participant* part,SubscriberAttributes& att,
		SubscriberListener* listen )
{
	for(auto it : m_participants)
	{
		if(it.second->getGuid() == part->getGuid())
		{
			return part->mp_impl->createSubscriber(att,listen);
		}
	}
	return nullptr;
}

bool Domain::registerType(Participant* part, TopicDataType* type)
{
	for(auto it : m_participants)
	{
		if(it.second->getGuid() == part->getGuid())
		{
			return part->mp_impl->registerType(type);
		}
	}
	return false;
}


} /* namespace pubsub */
} /* namespace eprosima */
