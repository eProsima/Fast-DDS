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

namespace eprosima {
namespace dds {

bool DomainParticipantImpl::instanceFlag = false;
DomainParticipant* DomainParticipantImpl::single = NULL;
DomainParticipantImpl* DomainParticipantImpl::getInstance()
{
	if(! instanceFlag)
	{
		single = new DomainParticipant();
		instanceFlag = true;
		return single;
	}
	else
	{
		return single;
	}
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
	for(std::vector<Participant*>::iterator it=m_participants.begin();
			it!=m_participants.end();++it)
	{
		delete(*it);
	}
	pDebugInfo("Participants deleted correctly "<< endl);
	for(std::vector<Publisher*>::iterator it=m_publisherList.begin();
			it!=m_publisherList.end();++it)
	{
		delete(*it);
	}
	pDebugInfo("Publishers deleted correctly "<< endl);
	for(std::vector<Subscriber*>::iterator it=m_subscriberList.begin();
			it!=m_subscriberList.end();++it)
	{
		delete(*it);
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
	for(std::vector<ParticipantImpl*>::iterator it = m_participants.begin();
			it!=m_participants.end();++it)
	{
		if((*it)->getGuid() == p->getGuid())
		{
			*pimpl = *it;
			return true;
		}
	}
	return false;
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
	Publisher* Pub = NULL;
	if(p->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	{
		if(WParam.userDefinedId <= 0)
		{
			pError("Static EDP requires user defined Id"<<endl);
			return NULL;
		}
	}
	if(WParam.qos.m_reliability == BEST_EFFORT_RELIABILITY_QOS)
	{
		StatelessWriter* SW;
		if(!p->createStatelessWriter(&SW,WParam,p_type->m_typeSize,false))
			return NULL;
		SW->m_qos.setQos(WParam.qos,true);
		Pub = new Publisher((RTPSWriter*)SW);
		pDebugInfo("Publisher in topic: "<<Pub->getTopicName()<<" created."<<endl);
		SW->m_Pub = Pub;

		Pub->mp_type = p_type;
		SW->mp_type = p_type;

	}
	else if(WParam.qos.m_reliability == RELIABLE_RELIABILITY_QOS)
	{
		StatefulWriter* SF;
		if(!p->createStatefulWriter(&SF,WParam,p_type->m_typeSize,false))
			return NULL;
		SF->m_qos.setQos(WParam.qos,true);
		Pub = new Publisher((RTPSWriter*)SF);
		SF->m_Pub = Pub;
		Pub->mp_type = p_type;
		SF->mp_type = p_type;

	}
	if(Pub != NULL)
	{
		pInfo(B_YELLOW<<"PUBLISHER CREATED"<<DEF<<endl);
		m_publisherList.push_back(Pub);
	}
	else
	{
		pError("Publisher not created"<<endl);
	}
	return Pub;
}

Subscriber* DomainParticipantImpl::createSubscriber(Participant* pin,	SubscriberAttributes& RParam)
{
	ParticipantImpl* p = NULL;
	if(!getParticipantImpl(pin,&p))
	{
		pError("Participant not registered"<<endl);
		return NULL;
	}
	//Look for the correct type registration
	pInfo("Creating Subscriber"<<endl;);
	DDSTopicDataType* p_type = NULL;
	if(!DomainParticipant::getRegisteredType(RParam.topic.topicDataType,&p_type))
	{
		pError("Type Not Registered"<<endl;);
		return NULL;
	}

	if(RParam.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		pError("Keyed Topic needs getKey function"<<endl);
		return NULL;
	}
	if(p->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	{
		if(RParam.userDefinedId <= 0)
		{
			pError("Static EDP requires user defined Id"<<endl);
			return NULL;
		}
	}
	Subscriber* Sub = NULL;
	if(RParam.qos.m_reliability == BEST_EFFORT_RELIABILITY_QOS)
	{
		StatelessReader* SR;
		if(!p->createStatelessReader(&SR,RParam,p_type->m_typeSize,false))
		{
			pError("Error creating subscriber"<<endl);
			return NULL;
		}
		SR->m_qos.setQos(RParam.qos,true);
		Sub = new Subscriber((RTPSReader*) SR);
		SR->mp_Sub = Sub;
		SR->mp_type = p_type;
		Sub->topicName = RParam.topic.topicName;
		Sub->topicDataType = RParam.topic.topicDataType;
	}
	else if(RParam.qos.m_reliability == RELIABLE_RELIABILITY_QOS)
	{
		pDebugInfo("Stateful"<<endl);
		StatefulReader* SFR;
		if(!p->createStatefulReader(&SFR,RParam,p_type->m_typeSize,false))
		{
			pError("Error creating subscriber"<<endl);
			return NULL;
		}
		SFR->m_qos.setQos(RParam.qos,true);
		Sub = new Subscriber((RTPSReader*) SFR);
		SFR->mp_Sub = Sub;
		SFR->mp_type = p_type;
		Sub->topicName = RParam.topic.topicName;
		Sub->topicDataType = RParam.topic.topicDataType;

	}
	if(Sub!=NULL)
	{
		pInfo(B_YELLOW<<"SUBSCRIBER CORRECTLY CREATED"<<DEF<<endl);

		m_subscriberList.push_back(Sub);
	}
	else
	{pError("Subscriber not created"<<endl);}
	return Sub;
}

Participant* DomainParticipantImpl::createParticipant(const ParticipantAttributes& PParam)
{

	uint32_t id = getNewId();
	Participant* p = new Participant(PParam,id);

	m_participants.push_back(p);
	return p;
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
		bool found = false;
		for(std::vector<Participant*>::iterator it=m_participants.begin();
				it!=m_participants.end();++it)
		{
			if((*it)->m_guid == p->m_guid)
			{
				found = true;
				m_participants.erase(it);
				break;
			}
		}
		if(found)
		{
			delete(p);
			return true;
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
	if(p->removeUserEndpoint((Endpoint*)(pub->mp_Writer),'W'))
	{
		for(std::vector<Publisher*>::iterator it=m_publisherList.begin();
				it!=m_publisherList.end();++it)
		{
			if((*it)->mp_Writer->m_guid == pub->mp_Writer->m_guid)
			{
				m_publisherList.erase(it);
			}
		}
		delete(pub->mp_Writer);
		delete(pub);
		return true;
	}
	else
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
	if(p->removeUserEndpoint((Endpoint*)(sub->mp_Reader),'R'))
	{
		for(std::vector<Subscriber*>::iterator it=m_subscriberList.begin();
				it!=m_subscriberList.end();++it)
		{
			if((*it)->mp_Reader->m_guid == sub->mp_Reader->m_guid)
			{
				m_subscriberList.erase(it);
			}
		}
		delete(sub->mp_Reader);
		delete(sub);
		return true;
	}
	else
		return false;
}




} /* namespace dds */
} /* namespace eprosima */


