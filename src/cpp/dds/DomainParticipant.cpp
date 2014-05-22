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

namespace eprosima {
namespace dds {

bool DomainParticipant::instanceFlag = false;
DomainParticipant* DomainParticipant::single = NULL;
DomainParticipant* DomainParticipant::getInstance()
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

DomainParticipant::DomainParticipant()
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

DomainParticipant::~DomainParticipant()
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
	DomainParticipant::instanceFlag = false;
	delete(RTPSLog::getInstance());
}


uint32_t DomainParticipant::getNewId()
{
	return ++id;
}

void DomainParticipant::stopAll()
{

	dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
	delete(dp);
}

Publisher* DomainParticipant::createPublisher(Participant* p, PublisherAttributes& WParam)
{
	pInfo("Creating Publisher"<<endl)
	//Look for the correct type registration
	DDSTopicDataType* p_type = NULL;
	if(!DomainParticipant::getRegisteredType(WParam.topic.topicDataType,&p_type))
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
		dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
		dp->m_publisherList.push_back(Pub);
	}
	else
	{
		pError("Publisher not created"<<endl);
	}
	return Pub;
}

Subscriber* DomainParticipant::createSubscriber(Participant* p,	SubscriberAttributes& RParam)
{
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
		dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
		dp->m_subscriberList.push_back(Sub);
	}
	else
	{pError("Subscriber not created"<<endl);}
	return Sub;
}

Participant* DomainParticipant::createParticipant(const ParticipantAttributes& PParam)
{
	dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
	uint32_t id = dp->getNewId();
	Participant* p = new Participant(PParam,id);

	dp->m_participants.push_back(p);
	return p;
}

bool DomainParticipant::getRegisteredType(std::string type_name,DDSTopicDataType** type_ptr)
{
	dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
	for(std::vector<DDSTopicDataType*>::iterator it=dp->m_registeredTypes.begin();
			it!=dp->m_registeredTypes.end();++it)
	{
		if((*it)->m_topicDataTypeName == type_name)
		{
			*type_ptr = *it;
			return true;
		}
	}
	return false;
}

bool DomainParticipant::registerType(DDSTopicDataType* type)
{
	dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
	for(std::vector<DDSTopicDataType*>::iterator it = dp->m_registeredTypes.begin();it!=dp->m_registeredTypes.end();++it)
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
	dp->m_registeredTypes.push_back(type);
	pInfo("Type "<<type->m_topicDataTypeName << " registered."<<endl);
	return true;
}

bool DomainParticipant::removeParticipant(Participant* p)
{
	if(p!=NULL)
	{
		bool found = false;
		dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
		for(std::vector<Participant*>::iterator it=dp->m_participants.begin();
				it!=dp->m_participants.end();++it)
		{
			if((*it)->m_guid == p->m_guid)
			{
				found = true;
				dp->m_participants.erase(it);
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

bool DomainParticipant::removePublisher(Participant* p,Publisher* pub)
{
	if(p==NULL || pub==NULL)
		return false;
	if(p->removeUserEndpoint((Endpoint*)(pub->mp_Writer),'W'))
	{
		dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
		for(std::vector<Publisher*>::iterator it=dp->m_publisherList.begin();
				it!=dp->m_publisherList.end();++it)
		{
			if((*it)->mp_Writer->m_guid == pub->mp_Writer->m_guid)
			{
				dp->m_publisherList.erase(it);
			}
		}
		delete(pub->mp_Writer);
		delete(pub);
		return true;
	}
	else
		return false;
}

bool DomainParticipant::removeSubscriber(Participant* p,Subscriber* sub)
{
	if(p==NULL || sub==NULL)
		return false;
	if(p->removeUserEndpoint((Endpoint*)(sub->mp_Reader),'R'))
	{
		dds::DomainParticipant *dp= dds::DomainParticipant::getInstance();
		for(std::vector<Subscriber*>::iterator it=dp->m_subscriberList.begin();
				it!=dp->m_subscriberList.end();++it)
		{
			if((*it)->mp_Reader->m_guid == sub->mp_Reader->m_guid)
			{
				dp->m_subscriberList.erase(it);
			}
		}
		delete(sub->mp_Reader);
		delete(sub);
		return true;
	}
	else
		return false;
}

void DomainParticipant::getIPAddress(LocatorList_t* locators)
{
	DomainParticipant* dp = DomainParticipant::getInstance();
	std::vector<std::string> ip_names;
	dp->m_IPFinder.getIP4s(&ip_names);

	locators->clear();
	for(std::vector<std::string>::iterator it=ip_names.begin();
			it!=ip_names.end();++it)
	{
		std::stringstream ss(*it);
		int a,b,c,d;
		char ch;
		ss >> a >>ch >>b >> ch >> c >>ch >>d;
		if(a== 127 && b== 0 && c== 0 && d == 1)
			continue;
		if(a==169 && b==254)
			continue;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 0;
		loc.address[12] = (octet)a;
		loc.address[13] = (octet)b;
		loc.address[14] = (octet)c;
		loc.address[15] = (octet)d;
		locators->push_back(loc);
	}
}


} /* namespace dds */
} /* namespace eprosima */


