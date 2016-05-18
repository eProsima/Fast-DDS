/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantImpl.cpp
 *
 */

#include "ParticipantImpl.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/participant/ParticipantListener.h>

#include <fastrtps/TopicDataType.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>

#include <fastrtps/attributes/PublisherAttributes.h>
#include "../publisher/PublisherImpl.h"
#include <fastrtps/publisher/Publisher.h>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include "../subscriber/SubscriberImpl.h"
#include <fastrtps/subscriber/Subscriber.h>

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/utils/RTPSLog.h>

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

static const char* const CLASS_NAME = "ParticipantImpl";

ParticipantImpl::ParticipantImpl(ParticipantAttributes& patt,Participant* pspart,ParticipantListener* listen):
												m_att(patt),
												mp_rtpsParticipant(nullptr),
												mp_participant(pspart),
												mp_listener(listen),
#pragma warning (disable : 4355 )
												m_rtps_listener(this)
{
	mp_participant->mp_impl = this;
}

ParticipantImpl::~ParticipantImpl()
{
	delete(mp_participant);
	while(m_publishers.size()>0)
	{
		this->removePublisher(m_publishers.begin()->first);
	}
	while(m_subscribers.size()>0)
	{
		this->removeSubscriber(m_subscribers.begin()->first);
	}
	RTPSDomain::removeRTPSParticipant(this->mp_rtpsParticipant);
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

Publisher* ParticipantImpl::createPublisher(PublisherAttributes& att,
		PublisherListener* listen)
{
	const char* const METHOD_NAME = "createPublisher";
	logInfo(PARTICIPANT,"CREATING PUBLISHER IN TOPIC: "<<att.topic.getTopicName(),C_B_YELLOW)
	//Look for the correct type registration

	TopicDataType* p_type = nullptr;

    /// Preconditions
    // Check the type was registered.
	if(!getRegisteredType(att.topic.getTopicDataType().c_str(),&p_type))
	{
		logError(PARTICIPANT,"Type : "<< att.topic.getTopicDataType() << " Not Registered");
		return nullptr;
	}
    // Check the type supports keys.
	if(att.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		logError(PARTICIPANT,"Keyed Topic needs getKey function");
		return nullptr;
	}
    // Check the maximun size of the type and the asynchronous of the writer.
    if(p_type->m_typeSize > PAYLOAD_MAX_SIZE && att.qos.m_publishMode.kind != ASYNCHRONOUS_PUBLISH_MODE)
    {
		logError(PARTICIPANT,"Big data has to be sent using an asynchronous publisher");
		return nullptr;
    }
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
	if(!att.qos.checkQos() || !att.topic.checkQos())
		return nullptr;

	//TODO CONSTRUIR LA IMPLEMENTACION DENTRO DEL OBJETO DEL USUARIO.
	PublisherImpl* pubimpl = new PublisherImpl(this,p_type,att,listen);
	Publisher* pub = new Publisher(pubimpl);
	pubimpl->mp_userPublisher = pub;
	pubimpl->mp_rtpsParticipant = this->mp_rtpsParticipant;

	WriterAttributes watt;
	watt.endpoint.durabilityKind = att.qos.m_durability.kind == VOLATILE_DURABILITY_QOS ? VOLATILE : TRANSIENT_LOCAL;
	watt.endpoint.endpointKind = WRITER;
	watt.endpoint.multicastLocatorList = att.multicastLocatorList;
	watt.endpoint.reliabilityKind = att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
	watt.endpoint.topicKind = att.topic.topicKind;
	watt.endpoint.unicastLocatorList = att.unicastLocatorList;
	watt.mode = att.qos.m_publishMode.kind == eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE ? SYNCHRONOUS_WRITER : ASYNCHRONOUS_WRITER;
	if(att.getEntityID()>0)
		watt.endpoint.setEntityID((uint8_t)att.getEntityID());
	if(att.getUserDefinedID()>0)
		watt.endpoint.setUserDefinedID((uint8_t)att.getUserDefinedID());
	watt.times = att.times;

	RTPSWriter* writer = RTPSDomain::createRTPSWriter(this->mp_rtpsParticipant,
			watt,
			(WriterHistory*)&pubimpl->m_history,
			(WriterListener*)&pubimpl->m_writerListener);
	if(writer == nullptr)
	{
		logError(PARTICIPANT,"Problem creating associated Writer");
		delete(pubimpl);
		return nullptr;
	}
	pubimpl->mp_writer = writer;
	//SAVE THE PUBLISHER PAIR
	t_p_PublisherPair pubpair;
	pubpair.first = pub;
	pubpair.second = pubimpl;
	m_publishers.push_back(pubpair);

	//REGISTER THE WRITER
	this->mp_rtpsParticipant->registerWriter(writer,att.topic,att.qos);

	return pub;
}


std::pair<StatefulReader*,StatefulReader*> ParticipantImpl::getEDPReaders(){

	return mp_rtpsParticipant->getEDPReaders();
}

int ParticipantImpl::get_no_publishers(char *target_topic){
	int count = 0;
	std::string target_string(target_topic);
	//Calculate the number of publishers that match the target topic
	
	for(auto it=m_publishers.begin(); it!=m_publishers.end(); ++it){
		if(target_string.compare( (*it).second->getAttributes().topic.topicName) == 0){
			//Strings are equal
			count++;
		}	

	}
	return count;	
}

int ParticipantImpl::get_no_subscribers(char *target_topic){
	int count = 0;
	std::string target_string(target_topic);

	for(auto it=m_subscribers.begin(); it!=m_subscribers.end(); ++it){
		if(target_string.compare( (*it).second->getAttributes().topic.topicName) == 0){
			count++;
		}
	}
	return count;

}
Subscriber* ParticipantImpl::createSubscriber(SubscriberAttributes& att,
		SubscriberListener* listen)
{
	const char* const METHOD_NAME = "createSubscriber";
	logInfo(PARTICIPANT,"CREATING SUBSCRIBER IN TOPIC: "<<att.topic.getTopicName(),C_B_YELLOW)
	//Look for the correct type registration

	TopicDataType* p_type = nullptr;

	if(!getRegisteredType(att.topic.getTopicDataType().c_str(),&p_type))
	{
		logError(PARTICIPANT,"Type : "<< att.topic.getTopicDataType() << " Not Registered");
		return nullptr;
	}
	if(att.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		logError(PARTICIPANT,"Keyed Topic needs getKey function");
		return nullptr;
	}
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
	if(!att.qos.checkQos() || !att.topic.checkQos())
		return nullptr;

	SubscriberImpl* subimpl = new SubscriberImpl(this,p_type,att,listen);
	Subscriber* sub = new Subscriber(subimpl);
	subimpl->mp_userSubscriber = sub;
	subimpl->mp_rtpsParticipant = this->mp_rtpsParticipant;

	ReaderAttributes ratt;
	ratt.endpoint.durabilityKind = att.qos.m_durability.kind == VOLATILE_DURABILITY_QOS ? VOLATILE : TRANSIENT_LOCAL;
	ratt.endpoint.endpointKind = READER;
	ratt.endpoint.multicastLocatorList = att.multicastLocatorList;
	ratt.endpoint.reliabilityKind = att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
	ratt.endpoint.topicKind = att.topic.topicKind;
	ratt.endpoint.unicastLocatorList = att.unicastLocatorList;
	ratt.expectsInlineQos = att.expectsInlineQos;
	if(att.getEntityID()>0)
		ratt.endpoint.setEntityID((uint8_t)att.getEntityID());
	if(att.getUserDefinedID()>0)
		ratt.endpoint.setUserDefinedID((uint8_t)att.getUserDefinedID());
	ratt.times = att.times;

	RTPSReader* reader = RTPSDomain::createRTPSReader(this->mp_rtpsParticipant,
			ratt,
			(ReaderHistory*)&subimpl->m_history,
			(ReaderListener*)&subimpl->m_readerListener);
	if(reader == nullptr)
	{
		logError(PARTICIPANT,"Problem creating associated Reader");
		delete(subimpl);
		return nullptr;
	}
	subimpl->mp_reader = reader;
	//SAVE THE PUBLICHER PAIR
	t_p_SubscriberPair subpair;
	subpair.first = sub;
	subpair.second = subimpl;
	m_subscribers.push_back(subpair);

	//REGISTER THE READER
	this->mp_rtpsParticipant->registerReader(reader,att.topic,att.qos);

	return sub;
}


bool ParticipantImpl::getRegisteredType(const char* typeName, TopicDataType** type)
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
	const char* const METHOD_NAME = "registerType";
	if (type->m_typeSize <= 0)
	{
		logError(PARTICIPANT, "Registered Type must have maximum byte size > 0");
		return false;
	}
	if (std::string(type->getName()).size() <= 0)
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


void ParticipantImpl::MyRTPSParticipantListener::onRTPSParticipantDiscovery(RTPSParticipant* part,RTPSParticipantDiscoveryInfo rtpsinfo)
{
	if(this->mp_participantimpl->mp_listener!=nullptr)
	{
		ParticipantDiscoveryInfo info;
		info.rtps = rtpsinfo;
		this->mp_participantimpl->mp_rtpsParticipant = part;
		this->mp_participantimpl->mp_listener->onParticipantDiscovery(mp_participantimpl->mp_participant,info);
	}
}

bool ParticipantImpl::newRemoteEndpointDiscovered(const GUID_t& partguid, uint16_t endpointId,
		EndpointKind_t kind)
{
	if (kind == WRITER)
		return this->mp_rtpsParticipant->newRemoteWriterDiscovered(partguid, endpointId);
	else 
		return this->mp_rtpsParticipant->newRemoteReaderDiscovered(partguid, endpointId);
}

} /* namespace pubsub */
} /* namespace eprosima */
