/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PUBSUBDomain.cpp
 *
 */

#include "fastrtps/pubsub/PUBSUBDomain.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/pubsub/participant/PUBSUBParticipant.h"
#include "fastrtps/pubsub/participant/PUBSUBParticipantImpl.h"

#include "fastrtps/utils/RTPSLog.h"

using namespace eprosima::rtps;

namespace eprosima {
namespace pubsub {

static const char* const CLASS_NAME = "PUBSUBDomain";


PUBSUBDomain::PUBSUBDomain()
{
	// TODO Auto-generated constructor stub

}

PUBSUBDomain::~PUBSUBDomain()
{

}

PUBSUBParticipant* PUBSUBDomain::createParticipant(RTPSParticipantAttributes& att,RTPSParticipantListener* listen)
{
	const char* const METHOD_NAME = "createParticipant";

	RTPSParticipant* part = RTPSDomain::createParticipant(att,listen);

	if(part == nullptr)
	{
		logError(PUBSUB_PARTICIPANT,"Problem creating RTPSParticipant");
		return nullptr;
	}
	PUBSUBParticipant* pubsubpar = new PUBSUBParticipant();
	PUBSUBParticipantImpl* pspartimpl = new PUBSUBParticipantImpl(att,pubsubpar,part);

	t_p_PUBSUBParticipant pubsubpair;
	pubsubpair.first = pubsubpar;
	pubsubpair.second = pspartimpl;
	m_pubsubParticipants.push_back(pubsubpair);
	return pubsubpar;
}

Publisher* PUBSUBDomain::createPublisher(PUBSUBParticipant* part,PublisherAttributes& att,
		PublisherListener* listen )
{
	for(auto it : m_pubsubParticipants)
	{
		if(it.second->getGuid() == part->getGuid())
		{
			return part->mp_impl->createPublisher(att,listen);
		}
	}
	return nullptr;
}


} /* namespace pubsub */
} /* namespace eprosima */
