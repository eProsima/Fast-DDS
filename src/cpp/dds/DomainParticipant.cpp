/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * DomainParticipant.cpp
 *
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

#include <cstdlib>
#include <ctime>


namespace eprosima {
namespace dds {

static const char* const CLASS_NAME = "DomainParticipantImpl";

bool DomainParticipantImpl::instanceFlag = false;
DomainParticipantImpl* DomainParticipantImpl::single = nullptr;
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
	m_maxParticipantID = 0;//private constructor
	m_portBase = 7400;
	m_participantIdGain = 2;
	m_domainIdGain = 250;
	m_offsetd0 = 0;
	m_offsetd1 = 10;
	m_offsetd2 = 1;
	m_offsetd3 = 11;
	m_DomainId = 80;
	srand (static_cast <unsigned> (time(0)));

}

DomainParticipantImpl::~DomainParticipantImpl()
{
	const char* const METHOD_NAME = "~DomainParticipantImpl";
	logInfo(RTPS_PARTICIPANT,"Deleting everything");
	for(std::vector<ParticipantPair>::iterator it=m_participants.begin();
			it!=m_participants.end();++it)
	{
		delete(it->first);
		delete(it->second);
	}
	logInfo(RTPS_PARTICIPANT,"Participants deleted correctly ");
	for(std::vector<PublisherPair>::iterator it=m_publisherList.begin();
			it!=m_publisherList.end();++it)
	{
		delete(it->first);
		delete(it->second);
	}
	logInfo(RTPS_PARTICIPANT,"Publishers deleted correctly.");
	for(std::vector<SubscriberPair>::iterator it=m_subscriberList.begin();
			it!=m_subscriberList.end();++it)
	{
		delete(it->first);
		delete(it->second);
	}
	logInfo(RTPS_PARTICIPANT,"Subscribers deleted correctly.");
	DomainParticipantImpl::instanceFlag = false;
}


uint32_t DomainParticipantImpl::getNewId()
{
	return ++m_maxParticipantID;
}

void DomainParticipantImpl::stopAll()
{
	delete(this);
}

bool DomainParticipantImpl::getParticipantImpl(Participant*p,ParticipantImpl**pimpl)
{
	if(p == nullptr)
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

Participant* DomainParticipantImpl::createParticipant(const ParticipantAttributes& PParam,
														ParticipantListener* listen)
{
	const char* const METHOD_NAME = "createParticipant";
	logInfo(RTPS_PARTICIPANT,"");

	if(PParam.builtin.leaseDuration < c_TimeInfinite && PParam.builtin.leaseDuration <= PParam.builtin.leaseDuration_announcementperiod)
	{
		logError(RTPS_PARTICIPANT,"Participant Attributes: LeaseDuration should be >= leaseDuration announcement period");
		return nullptr;
	}
	uint32_t ID;
	if(PParam.participantId < 0)
	{
		ID = getNewId();
		while(this->m_participantIDs.insert(ID).second == false)
			ID = getNewId();
	}
	else
	{
		ID = PParam.participantId;
		if(this->m_participantIDs.insert(ID).second == false)
		{
			logError(RTPS_PARTICIPANT,"Participant with the same ID already exists" << endl;)
			return nullptr;
		}
	}
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

	Participant* p = new Participant(nullptr);

	ParticipantImpl* pimpl = new ParticipantImpl(PParam,guidP,ID,p,listen);
	this->setMaxParticipantId(pimpl->getParticipantId());

	m_participants.push_back(ParticipantPair(p,pimpl));
	return p;
}



Publisher* DomainParticipantImpl::createPublisher(Participant* pin, PublisherAttributes& WParam,PublisherListener* plisten)
{
	const char* const METHOD_NAME = "createPublisher";
	ParticipantImpl* p = nullptr;
	if(pin == nullptr || !getParticipantImpl(pin,&p))
	{
		logError(RTPS_PARTICIPANT,"Participant not registered");
		return nullptr;
	}
	logInfo(RTPS_PARTICIPANT,"CREATING PUBLISHER",EPRO_B_YELLOW)
	//Look for the correct type registration
	DDSTopicDataType* p_type = nullptr;
	if(!getRegisteredType(WParam.topic.topicDataType,&p_type))
	{
		logError(RTPS_PARTICIPANT,"Type : "<< WParam.topic.topicDataType << " Not Registered");
		return nullptr;
	}
	if(WParam.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		logError(RTPS_PARTICIPANT,"Keyed Topic needs getKey function");
		return nullptr;
	}
	PublisherImpl* pubImpl = nullptr;
	if(p->getBuiltinAttributes().use_STATIC_EndpointDiscoveryProtocol)
	{
		if(WParam.userDefinedId <= 0)
		{
			logError(RTPS_PARTICIPANT,"Static EDP requires user defined Id");
			return nullptr;
		}
	}
	WParam.payloadMaxSize = p_type->m_typeSize;
	if(!WParam.qos.checkQos() || !WParam.topic.checkQos())
		return nullptr;
	RTPSWriter* SW;
	if(WParam.qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS)
	{
		if(!p->createWriter(&SW,WParam,p_type->m_typeSize,false,STATELESS,p_type,plisten,c_EntityId_Unknown))
			return nullptr;
		pubImpl = new PublisherImpl(p,(RTPSWriter*)SW,p_type,WParam);
	}
	else if(WParam.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
	{
		if(!p->createWriter(&SW,WParam,p_type->m_typeSize,false,STATEFUL,p_type,plisten,c_EntityId_Unknown))
			return nullptr;
		pubImpl = new PublisherImpl(p,(RTPSWriter*)SW,p_type,WParam);
	}
	else
		logWarning(RTPS_PARTICIPANT,"Incorrect Reliability Kind");
	if(pubImpl != nullptr)
	{
		logInfo(RTPS_PARTICIPANT,"PUBLISHER CREATED",EPRO_B_YELLOW);
		Publisher* Pub = new Publisher(pubImpl);
		m_publisherList.push_back(PublisherPair(Pub,pubImpl));
		//Now we do discovery (in our event thread):
		//p->getEventResource()->io_service.post(boost::bind(&ParticipantImpl::registerWriter,p,SW));
		p->registerWriter(SW);
		//p->WriterDiscovery(SW);
		return Pub;
	}

	logError(RTPS_PARTICIPANT,"Publisher not created");

	return nullptr;
}

Subscriber* DomainParticipantImpl::createSubscriber(Participant* pin,	SubscriberAttributes& RParam,SubscriberListener* slisten)
{
	const char* const METHOD_NAME = "createSubscriber";
	ParticipantImpl* p = nullptr;
	if(pin == nullptr || !getParticipantImpl(pin,&p))
	{
		logError(RTPS_PARTICIPANT,"Participant not registered");
		return nullptr;
	}
	logInfo(RTPS_PARTICIPANT,"CREATING SUBSCRIBER",EPRO_B_YELLOW)
	//Look for the correct type registration
	DDSTopicDataType* p_type = nullptr;
	if(!getRegisteredType(RParam.topic.topicDataType,&p_type))
	{
		logError(RTPS_PARTICIPANT,"Type: " <<RParam.topic.topicDataType << " Not Registered");
		return nullptr;
	}
	if(RParam.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		logError(RTPS_PARTICIPANT,"Keyed Topic needs getKey function");
		return nullptr;
	}
	SubscriberImpl* subImpl = nullptr;
	if(p->getBuiltinAttributes().use_STATIC_EndpointDiscoveryProtocol)
	{
		if(RParam.userDefinedId <= 0)
		{
			logError(RTPS_PARTICIPANT,"Static EDP requires user defined Id");
			return nullptr;
		}
	}
	RParam.payloadMaxSize = p_type->m_typeSize;
	if(!RParam.qos.checkQos() || !RParam.topic.checkQos())
		return nullptr;
	RTPSReader* SR;
	if(RParam.qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS)
	{
		if(!p->createReader(&SR,RParam,p_type->m_typeSize,false,STATELESS,p_type,slisten))
			return nullptr;
		subImpl = new SubscriberImpl(p,(RTPSReader*)SR,p_type,RParam);
	}
	else if(RParam.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
	{
		if(!p->createReader(&SR,RParam,p_type->m_typeSize,false,STATEFUL,p_type,slisten))
			return nullptr;
		subImpl = new SubscriberImpl(p,(RTPSReader*)SR,p_type,RParam);
	}
	if(subImpl != nullptr)
	{
		logInfo(RTPS_PARTICIPANT,"SUBSCRIBER CREATED",EPRO_B_YELLOW);
		Subscriber* Sub = new Subscriber(subImpl);
		m_subscriberList.push_back(SubscriberPair(Sub,subImpl));
		//p->getEventResource()->io_service.post(boost::bind(&ParticipantImpl::registerReader,p,SR));
		p->registerReader(SR);
		//p->ReaderDiscovery(SR);
		return Sub;
	}

	logError(RTPS_PARTICIPANT,"Subscriber not created");

	return nullptr;
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
	const char* const METHOD_NAME = "registerType";
	for(std::vector<DDSTopicDataType*>::iterator it = m_registeredTypes.begin();it!=m_registeredTypes.end();++it)
	{
		if((*it)->m_topicDataTypeName == type->m_topicDataTypeName)
			return false;
	}
	if(type->m_typeSize <=0)
	{
		logError(RTPS_PARTICIPANT,"Registered Type must have maximum byte size > 0");
		return false;
	}
	if(type->m_typeSize > PAYLOAD_MAX_SIZE)
	{
		logError(RTPS_PARTICIPANT,"Current version only supports types of sizes < "<<PAYLOAD_MAX_SIZE);
		return false;
	}
	if(type->m_topicDataTypeName.size() <=0)
	{
		logError(RTPS_PARTICIPANT,"Registered Type must have a name");
		return false;
	}
	m_registeredTypes.push_back(type);
	logInfo(RTPS_PARTICIPANT,"Type "<<type->m_topicDataTypeName << " registered.");
	return true;
}

bool DomainParticipantImpl::removeParticipant(Participant* p)
{
	const char* const METHOD_NAME = "removeParticipant";
	if(p!=nullptr)
	{
		std::vector<PublisherPair> auxListPub;
		for(std::vector<PublisherPair>::iterator it=m_publisherList.begin();
				it!=m_publisherList.end();++it)
		{
			if(it->second->getGuid().guidPrefix == p->getGuid().guidPrefix)
			{
				delete(it->first);
				delete(it->second);
			}
			else
			{
				auxListPub.push_back(*it);
			}
		}
		m_publisherList = auxListPub;
		logInfo(RTPS_PARTICIPANT,"Publishers deleted correctly.");
		std::vector<SubscriberPair> auxListSub;
		for(std::vector<SubscriberPair>::iterator it=m_subscriberList.begin();
				it!=m_subscriberList.end();++it)
		{
			if(it->second->getGuid().guidPrefix == p->getGuid().guidPrefix)
			{
				delete(it->first);
				delete(it->second);
			}
			else
			{
				auxListSub.push_back(*it);
			}
		}
		m_subscriberList = auxListSub;
		logInfo(RTPS_PARTICIPANT,"Subscribers deleted correctly.");
		for(std::vector<ParticipantPair>::iterator it=m_participants.begin();
				it!=m_participants.end();++it)
		{
			if(it->second->getGuid() == p->getGuid())
			{
				int32_t pID = it->second->getParticipantID();
				m_participantIDs.erase(pID);
				
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
	const char* const METHOD_NAME = "removePublisher";
	ParticipantImpl* p = nullptr;
	if(!getParticipantImpl(pin,&p))
	{
		logError(RTPS_PARTICIPANT,"Participant not registered");
		return nullptr;
	}
	if(p==nullptr || pub==nullptr)
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
				logInfo(RTPS_PARTICIPANT, " OK");
				return true;
			}
		}
	}
	logInfo(RTPS_PARTICIPANT, " Not found.");
	return false;
}

bool DomainParticipantImpl::removeSubscriber(Participant* pin,Subscriber* sub)
{
	const char* const METHOD_NAME = "removeSubscriber";
	ParticipantImpl* p = nullptr;
	if(!getParticipantImpl(pin,&p))
	{
		logError(RTPS_PARTICIPANT,"Participant not registered");
		return nullptr;
	}
	if(p==nullptr || sub==nullptr)
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
				logInfo(RTPS_PARTICIPANT, " OK");
				return true;
			}
		}
	}
	logInfo(RTPS_PARTICIPANT, " Not found.");
	return false;
}




} /* namespace dds */
} /* namespace eprosima */


