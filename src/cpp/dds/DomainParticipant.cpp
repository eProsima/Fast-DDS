/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * DomainParticipant.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/dds/SubscriberListener.h"
#include "eprosimartps/dds/PublisherListener.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/Participant.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"

namespace eprosima {
namespace dds {

bool DomainParticipantImpl::instanceFlag = false;
DomainParticipantImpl* DomainParticipantImpl::single = NULL;
DomainParticipantImpl* DomainParticipantImpl::getInstance()
{
	if(! instanceFlag)
	{
		single = new DomainParticipantImpl();
		instanceFlag = true;
	}

	return single;
}

DomainParticipantImpl::DomainParticipantImpl()
{
	id = 0;//private constructor
	m_portBase = 7400;
	m_participantIdGain = 2;
	m_domainIdGain = 250;
	m_offsetd0 = 0;
	m_offsetd1 = 10;
	m_offsetd2 = 1;
	m_offsetd3 = 11;
	m_DomainId = 80;
}

DomainParticipantImpl::~DomainParticipantImpl()
{
	pDebugInfo("DomainParticipant destructor"<<endl;);
	for(std::vector<ParticipantPair>::iterator it=m_participants.begin();
			it!=m_participants.end();++it)
	{
		delete(it->first);
		delete(it->second);
	}
	pDebugInfo("Participants deleted correctly "<< endl);
	for(std::vector<PublisherPair>::iterator it=m_publisherList.begin();
			it!=m_publisherList.end();++it)
	{
		delete(it->first);
		delete(it->second);
	}
	pDebugInfo("Publishers deleted correctly "<< endl);
	for(std::vector<SubscriberPair>::iterator it=m_subscriberList.begin();
			it!=m_subscriberList.end();++it)
	{
		delete(it->first);
		delete(it->second);
	}
	pDebugInfo("Subscribers deleted correctly "<< endl);
	DomainParticipantImpl::instanceFlag = false;
	delete(RTPSLog::getInstance());
}


uint32_t DomainParticipantImpl::getNewId()
{
	return ++id;
}

void DomainParticipantImpl::stopAll()
{
	delete(this);
}

bool DomainParticipantImpl::getParticipantImpl(Participant*p,ParticipantImpl**pimpl)
{
	if(p == NULL)
		return false;
	for(std::vector<ParticipantPair>::iterator it = m_participants.begin();
			it!=m_participants.end();++it)
	{
		if(it->second->getGuid() == p->getGuid())
		{
			*pimpl = (it->second);
			return true;
		}
	}
	return false;
}

Participant* DomainParticipantImpl::createParticipant(const ParticipantAttributes& PParam)
{
	uint32_t ID = getNewId();
	int pid;
#if defined(_WIN32)
	pid = (int)_getpid();
#else
	pid = (int)getpid();
#endif
	GuidPrefix_t guidP;
	LocatorList_t loc;
	IPFinder::getIPAddress(&loc);
	if(loc.size()>0)
	{
		guidP.value[0] = loc.begin()->address[12];
		guidP.value[1] = loc.begin()->address[13];
		guidP.value[2] = loc.begin()->address[14];
		guidP.value[3] = loc.begin()->address[15];
	}
	else
	{
		guidP.value[0] = 127;
		guidP.value[1] = 0;
		guidP.value[2] = 0;
		guidP.value[3] = 1;
	}
	guidP.value[4] = ((octet*)&pid)[0];
	guidP.value[5] = ((octet*)&pid)[1];
	guidP.value[6] = ((octet*)&pid)[2];
	guidP.value[7] = ((octet*)&pid)[3];
	guidP.value[8] = ((octet*)&ID)[0];
	guidP.value[9] = ((octet*)&ID)[1];
	guidP.value[10] = ((octet*)&ID)[2];
	guidP.value[11] = ((octet*)&ID)[3];
	ParticipantImpl* pimpl = new ParticipantImpl(PParam,guidP);
	Participant* p = new Participant(pimpl);

	m_participants.push_back(ParticipantPair(p,pimpl));
	return p;
}



Publisher* DomainParticipantImpl::createPublisher(Participant* pin, PublisherAttributes& WParam)
{
	ParticipantImpl* p = NULL;
	if(!getParticipantImpl(pin,&p))
	{
		pError("Participant not registered"<<endl);
		return NULL;
	}
	pInfo("Creating Publisher"<<endl)
	//Look for the correct type registration
	DDSTopicDataType* p_type = NULL;
	if(!getRegisteredType(WParam.topic.topicDataType,&p_type))
	{
		pError("Type Not Registered"<<endl;);
		return NULL;
	}
	if(WParam.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		pError("Keyed Topic needs getKey function"<<endl);
		return NULL;
	}
	PublisherImpl* pubImpl = NULL;
	if(p->getDiscoveryAttributes().use_STATIC_EndpointDiscoveryProtocol)
	{
		if(WParam.userDefinedId <= 0)
		{
			pError("Static EDP requires user defined Id"<<endl);
			return NULL;
		}
	}
	if(WParam.qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS)
	{
		StatelessWriter* SW;
		if(!p->createStatelessWriter(&SW,WParam,p_type->m_typeSize,false))
			return NULL;
		SW->setQos(WParam.qos,true);
		pubImpl = new PublisherImpl((RTPSWriter*)SW,p_type);
	}
	else if(WParam.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
	{
		StatefulWriter* SW;
		if(!p->createStatefulWriter(&SW,WParam,p_type->m_typeSize,false))
			return NULL;
		SW->setQos(WParam.qos,true);
		pubImpl = new PublisherImpl((RTPSWriter*)SW,p_type);
	}
	if(pubImpl != NULL)
	{
		pInfo(B_YELLOW<<"PUBLISHER CREATED"<<DEF<<endl);
		Publisher* Pub = new Publisher(pubImpl);
		m_publisherList.push_back(PublisherPair(Pub,pubImpl));
		return Pub;
	}

	pError("Publisher not created"<<endl);

	return NULL;
}

Subscriber* DomainParticipantImpl::createSubscriber(Participant* pin,	SubscriberAttributes& RParam)
{
	ParticipantImpl* p = NULL;
	if(!getParticipantImpl(pin,&p))
	{
		pError("Participant not registered"<<endl);
		return NULL;
	}
	pInfo("Creating Publisher"<<endl)
	//Look for the correct type registration
	DDSTopicDataType* p_type = NULL;
	if(!getRegisteredType(RParam.topic.topicDataType,&p_type))
	{
		pError("Type Not Registered"<<endl;);
		return NULL;
	}
	if(RParam.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		pError("Keyed Topic needs getKey function"<<endl);
		return NULL;
	}
	SubscriberImpl* subImpl = NULL;
	if(p->getDiscoveryAttributes().use_STATIC_EndpointDiscoveryProtocol)
	{
		if(RParam.userDefinedId <= 0)
		{
			pError("Static EDP requires user defined Id"<<endl);
			return NULL;
		}
	}
	if(RParam.qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS)
	{
		StatelessReader* SR;
		if(!p->createStatelessReader(&SR,RParam,p_type->m_typeSize,false))
			return NULL;
		SR->setQos(RParam.qos,true);
		subImpl = new SubscriberImpl((RTPSReader*)SR,p_type);
	}
	else if(RParam.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
	{
		StatefulReader* SR;
		if(!p->createStatefulReader(&SR,RParam,p_type->m_typeSize,false))
			return NULL;
		SR->setQos(RParam.qos,true);
		subImpl = new SubscriberImpl((RTPSReader*)SR,p_type);
	}
	if(subImpl != NULL)
	{
		pInfo(B_YELLOW<<"PUBLISHER CREATED"<<DEF<<endl);
		Subscriber* Sub = new Subscriber(subImpl);
		m_subscriberList.push_back(SubscriberPair(Sub,subImpl));
		return Sub;
	}

	pError("Publisher not created"<<endl);

	return NULL;
}



bool DomainParticipantImpl::getRegisteredType(std::string type_name,DDSTopicDataType** type_ptr)
{

	for(std::vector<DDSTopicDataType*>::iterator it=m_registeredTypes.begin();
			it!=m_registeredTypes.end();++it)
	{
		if((*it)->m_topicDataTypeName == type_name)
		{
			*type_ptr = *it;
			return true;
		}
	}
	return false;
}

bool DomainParticipantImpl::registerType(DDSTopicDataType* type)
{
	for(std::vector<DDSTopicDataType*>::iterator it = m_registeredTypes.begin();it!=m_registeredTypes.end();++it)
	{
		if((*it)->m_topicDataTypeName == type->m_topicDataTypeName)
			return false;
	}
	if(type->m_typeSize <=0)
	{
		pError("Registered Type must have size > 0"<<endl);
		return false;
	}
	if(type->m_topicDataTypeName.size() <=0)
	{
		pError("Registered Type must have a name"<<endl);
		return false;
	}
	m_registeredTypes.push_back(type);
	pInfo("Type "<<type->m_topicDataTypeName << " registered."<<endl);
	return true;
}

bool DomainParticipantImpl::removeParticipant(Participant* p)
{
	if(p!=NULL)
	{
		for(std::vector<ParticipantPair>::iterator it=m_participants.begin();
				it!=m_participants.end();++it)
		{
			if(it->second->getGuid() == p->getGuid())
			{
				delete(it->first);
				delete(it->second);
				m_participants.erase(it);
				return true;
			}
		}
	}
	return false;
}

bool DomainParticipantImpl::removePublisher(Participant* pin,Publisher* pub)
{
	ParticipantImpl* p = NULL;
	if(!getParticipantImpl(pin,&p))
	{
		pError("Participant not registered"<<endl);
		return NULL;
	}
	if(p==NULL || pub==NULL)
		return false;

	for(std::vector<PublisherPair>::iterator it=m_publisherList.begin();
			it!=m_publisherList.end();++it)
	{
			if(it->second->getGuid() == pub->getGuid())
			{
				if(p->deleteUserEndpoint((Endpoint*)(it->second->getWriterPtr()),'W'))
				{
					delete(it->first);
					delete(it->second);
					m_publisherList.erase(it);
					return true;
				}
			}
	}
	return false;
}

bool DomainParticipantImpl::removeSubscriber(Participant* pin,Subscriber* sub)
{
	ParticipantImpl* p = NULL;
	if(!getParticipantImpl(pin,&p))
	{
		pError("Participant not registered"<<endl);
		return NULL;
	}
	if(p==NULL || sub==NULL)
		return false;

	for(std::vector<SubscriberPair>::iterator it=m_subscriberList.begin();
			it!=m_subscriberList.end();++it)
	{
		if(it->second->getGuid() == sub->getGuid())
		{
			if(p->deleteUserEndpoint((Endpoint*)(it->second->getReaderPtr()),'R'))
			{
				delete(it->first);
				delete(it->second);
				m_subscriberList.erase(it);
				return true;
			}
		}
	}
	return false;
}




} /* namespace dds */
} /* namespace eprosima */


