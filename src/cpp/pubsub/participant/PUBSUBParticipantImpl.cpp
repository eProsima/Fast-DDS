/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PUBSUBParticipantImpl.cpp
 *
 */

#include "fastrtps/pubsub/participant/PUBSUBParticipantImpl.h"
#include "fastrtps/pubsub/participant/PUBSUBParticipant.h"

#include "fastrtps/pubsub/TopicDataType.h"

#include "fastrtps/rtps/participant/RTPSParticipant.h"

#include "fastrtps/pubsub/attributes/PublisherAttributes.h"
#include "fastrtps/pubsub/publisher/PublisherImpl.h"
#include "fastrtps/pubsub/publisher/Publisher.h"


#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/utils/RTPSLog.h"

namespace eprosima {
namespace pubsub {

static const char* const CLASS_NAME = "PUBSUBParticipantImpl";

PUBSUBParticipantImpl::PUBSUBParticipantImpl(RTPSParticipantAttributes& patt,PUBSUBParticipant* pspart,RTPSParticipant* part):
		m_att(patt),
		mp_rtpsParticipant(part),
		mp_pubsubParticipant(pspart)
{
	mp_pubsubParticipant->mp_impl = this;
}

PUBSUBParticipantImpl::~PUBSUBParticipantImpl()
{
	// TODO Auto-generated destructor stub
}

const GUID_t& PUBSUBParticipantImpl::getGuid() const
{
	return this->mp_rtpsParticipant->getGuid();
}

Publisher* PUBSUBParticipantImpl::createPublisher(PublisherAttributes& att,
		PublisherListener* listen)
{
	const char* const METHOD_NAME = "createPublisher";
	logInfo(PUBSUB_PARTICIPANT,"CREATING PUBLISHER",C_B_YELLOW)
	//Look for the correct type registration

	TopicDataType* p_type = nullptr;

	if(!getRegisteredType(att.topic.getTopicDataType().c_str(),&p_type))
	{
		logError(PUBSUB_PARTICIPANT,"Type : "<< att.topic.getTopicDataType() << " Not Registered");
		return nullptr;
	}
	if(att.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		logError(PUBSUB_PARTICIPANT,"Keyed Topic needs getKey function");
		return nullptr;
	}
	if(m_att.builtin.use_STATIC_EndpointDiscoveryProtocol)
	{
		if(att.getUserDefinedID() <= 0)
		{
			logError(PUBSUB_PARTICIPANT,"Static EDP requires user defined Id");
			return nullptr;
		}
	}
	if(!att.qos.checkQos() || !att.topic.checkQos())
		return nullptr;

	PublisherImpl* pubimpl = new PublisherImpl(this,p_type,att);
	Publisher* pub = new Publisher(pubimpl);

	WriterAttributes watt;
	watt.endpoint.durabilityKind = att.qos.m_durability.kind == VOLATILE_DURABILITY_QOS ? VOLATILE : TRANSIENT_LOCAL;
	watt.endpoint.endpointKind = WRITER;
	watt.endpoint.multicastLocatorList = att.multicastLocatorList;
	watt.endpoint.reliabilityKind = att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
	watt.endpoint.topicKind = att.topic.topicKind;
	watt.endpoint.unicastLocatorList = att.unicastLocatorList;
	watt.endpoint.setEntityID(att.getEntityID());
	watt.endpoint.setUserDefinedID(att.getUserDefinedID());
	watt.times = att.times;

	RTPSWriter* writer = RTPSDomain::createRTPSWriter(this->mp_rtpsParticipant,
												watt,
												(WriterHistory*)&pubimpl->m_history,
												(WriterListener*)&pubimpl->m_writerListener);
	if(writer == nullptr)
	{
		logError(PUBSUB_PARTICIPANT,"Problem creating associated Writer");
		return nullptr;
	}
	pubimpl->mp_writer = writer;
	//SAVE THE PUBLICHER PAIR
	t_p_PublisherPair pubpair;
	pubpair.first = pub;
	pubpair.second = pubimpl;
	m_publishers.push_back(pubpair);

	//REGISTER THE WRITER
	//this->mp_rtpsParticipant->registerWriter(writer,att.topic,att.qos);

	return pub;
};

bool PUBSUBParticipantImpl::getRegisteredType(const char* typeName, TopicDataType** type)
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

} /* namespace pubsub */
} /* namespace eprosima */
