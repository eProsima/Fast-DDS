/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSDomain.cpp
 *
 */

#include "eprosimartps/pubsub/RTPSDomain.h"
#include "eprosimartps/pubsub/TopicDataType.h"
#include "eprosimartps/pubsub/SubscriberListener.h"
#include "eprosimartps/pubsub/PublisherListener.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/pubsub/Publisher.h"
#include "eprosimartps/pubsub/Subscriber.h"
#include "eprosimartps/RTPSParticipant.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
#include "eprosimartps/utils/IPFinder.h"

#include <cstdlib>
#include <ctime>


namespace eprosima {
namespace pubsub {

static const char* const CLASS_NAME = "DomainRTPSParticipantImpl";
bool RTPSDomainImpl::instanceFlag = false;
RTPSDomainImpl* RTPSDomainImpl::single = NULL;

RTPSDomainImpl* RTPSDomainImpl::getInstance()
{
	if(! instanceFlag)
	{
		single = new RTPSDomainImpl();
		instanceFlag = true;
	}

	return single;
}

RTPSDomainImpl::RTPSDomainImpl()
{
	m_maxRTPSParticipantID = 0;//private constructor
	m_portBase = 7400;
	m_RTPSParticipantIdGain = 2;
	m_domainIdGain = 250;
	m_offsetd0 = 0;
	m_offsetd1 = 10;
	m_offsetd2 = 1;
	m_offsetd3 = 11;
	m_DomainId = 80;
	srand (static_cast <unsigned> (time(0)));

}

RTPSDomainImpl::~RTPSDomainImpl()
{


	const char* const METHOD_NAME = "~RTPSDomainImpl";
	logInfo(RTPS_RTPSParticipant,"DELETING ALL ENDPOINTS IN THIS DOMAIN",EPRO_WHITE);

	for(std::vector<RTPSParticipantPair>::iterator it=m_RTPSParticipants.begin();
			it!=m_RTPSParticipants.end();++it)
	{
		delete(it->first);
		delete(it->second);
	}
//<<<<<<< HEAD:src/cpp/pubsub/RTPSDomain.cpp
//	pDebugInfo("RTPSParticipants deleted correctly "<< endl);
//	for(std::vector<PublisherPair>::iterator it=m_publisherList.begin();
//			it!=m_publisherList.end();++it)
//	{
//		delete(it->first);
//		delete(it->second);
//	}
//	pDebugInfo("Publishers deleted correctly "<< endl);
//	for(std::vector<SubscriberPair>::iterator it=m_subscriberList.begin();
//			it!=m_subscriberList.end();++it)
//	{
//		delete(it->first);
//		delete(it->second);
//	}
//	pDebugInfo("Subscribers deleted correctly "<< endl);
//	RTPSDomainImpl::instanceFlag = false;
//	delete(RTPSLog::getInstance());
//=======
	logInfo(RTPS_RTPSParticipant,"RTPSParticipants deleted correctly ");
//	for(std::vector<PublisherPair>::iterator it=m_publisherList.begin();
//			it!=m_publisherList.end();++it)
//	{
//		delete(it->first);
//		delete(it->second);
//	}
//	logInfo(RTPS_RTPSParticipant,"Publishers deleted correctly.");
//	for(std::vector<SubscriberPair>::iterator it=m_subscriberList.begin();
//			it!=m_subscriberList.end();++it)
//	{
//		delete(it->first);
//		delete(it->second);
//	}
//	logInfo(RTPS_RTPSParticipant,"Subscribers deleted correctly.");
	RTPSDomainImpl::instanceFlag = false;
	eClock::my_sleep(100);
	Log::removeLog();

}


uint32_t RTPSDomainImpl::getNewId()
{
	return ++m_maxRTPSParticipantID;
}

void RTPSDomainImpl::stopAll()
{
	delete(this);
}

bool RTPSDomainImpl::getRTPSParticipantImpl(RTPSParticipant*p,RTPSParticipantImpl**pimpl)
{
	if(p == nullptr)
		return false;
	for(std::vector<RTPSParticipantPair>::iterator it = m_RTPSParticipants.begin();
			it!=m_RTPSParticipants.end();++it)
	{
		if(it->second->getGuid() == p->getGuid())
		{
			*pimpl = (it->second);
			return true;
		}
	}
	return false;
}

RTPSParticipant* RTPSDomainImpl::createRTPSParticipant(const RTPSParticipantAttributes& PParam,
														RTPSParticipantListener* listen)
{
	const char* const METHOD_NAME = "createRTPSParticipant";
	logInfo(RTPS_RTPSParticipant,"");

	if(PParam.builtin.leaseDuration < c_TimeInfinite && PParam.builtin.leaseDuration <= PParam.builtin.leaseDuration_announcementperiod)
	{
		logError(RTPS_RTPSParticipant,"RTPSParticipant Attributes: LeaseDuration should be >= leaseDuration announcement period");
		return nullptr;
	}
	uint32_t ID;
	if(PParam.RTPSParticipantId < 0)
	{
		ID = getNewId();
		while(this->m_RTPSParticipantIDs.insert(ID).second == false)
			ID = getNewId();
	}
	else
	{
		ID = PParam.RTPSParticipantId;
		if(this->m_RTPSParticipantIDs.insert(ID).second == false)
		{
			logError(RTPS_RTPSParticipant,"RTPSParticipant with the same ID already exists" << endl;)
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
		guidP.value[0] = c_eProsimaVendorId[0];
		guidP.value[1] = c_eProsimaVendorId[1];
		guidP.value[2] = loc.begin()->address[14];
		guidP.value[3] = loc.begin()->address[15];
	}
	else
	{
		guidP.value[0] = c_eProsimaVendorId[0];
		guidP.value[1] = c_eProsimaVendorId[1];
		guidP.value[2] = 127;
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

	RTPSParticipant* p = new RTPSParticipant(nullptr);

	RTPSParticipantImpl* pimpl = new RTPSParticipantImpl(PParam,guidP,ID,p,listen);
	this->setMaxRTPSParticipantId(pimpl->getRTPSParticipantId());

	m_RTPSParticipants.push_back(RTPSParticipantPair(p,pimpl));
	return p;
}



Publisher* RTPSDomainImpl::createPublisher(RTPSParticipant* pin, PublisherAttributes& WParam,PublisherListener* plisten)
{
	const char* const METHOD_NAME = "createPublisher";
	RTPSParticipantImpl* p = nullptr;
	if(pin == nullptr || !getRTPSParticipantImpl(pin,&p))
	{
		logError(RTPS_RTPSParticipant,"RTPSParticipant not registered");
		return nullptr;
	}
	logInfo(RTPS_RTPSParticipant,"CREATING PUBLISHER",EPRO_B_YELLOW)
	//Look for the correct type registration

	TopicDataType* p_type = nullptr;

	if(!getRegisteredType(WParam.topic.topicDataType,&p_type))
	{
		logError(RTPS_RTPSParticipant,"Type : "<< WParam.topic.topicDataType << " Not Registered");
		return nullptr;
	}
	if(WParam.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		logError(RTPS_RTPSParticipant,"Keyed Topic needs getKey function");
		return nullptr;
	}
	PublisherImpl* pubImpl = nullptr;
	if(p->getBuiltinAttributes().use_STATIC_EndpointDiscoveryProtocol)
	{
		if(WParam.userDefinedId <= 0)
		{
			logError(RTPS_RTPSParticipant,"Static EDP requires user defined Id");
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
		logWarning(RTPS_RTPSParticipant,"Incorrect Reliability Kind");
	if(pubImpl != nullptr)
	{
		logInfo(RTPS_RTPSParticipant,"PUBLISHER CREATED",EPRO_B_YELLOW);
		Publisher* Pub = new Publisher(pubImpl);
		m_publisherList.push_back(PublisherPair(Pub,pubImpl));
		//Now we do discovery (in our event thread):
		//p->getEventResource()->io_service.post(boost::bind(&RTPSParticipantImpl::registerWriter,p,SW));
		p->registerWriter(SW);
		//p->WriterDiscovery(SW);
		return Pub;
	}

	logError(RTPS_RTPSParticipant,"Publisher not created");

	return nullptr;
}

Subscriber* RTPSDomainImpl::createSubscriber(RTPSParticipant* pin,	SubscriberAttributes& RParam,SubscriberListener* slisten)
{
	const char* const METHOD_NAME = "createSubscriber";
	RTPSParticipantImpl* p = nullptr;
	if(pin == nullptr || !getRTPSParticipantImpl(pin,&p))
	{
		logError(RTPS_RTPSParticipant,"RTPSParticipant not registered");
		return nullptr;
	}
	logInfo(RTPS_RTPSParticipant,"CREATING SUBSCRIBER",EPRO_B_YELLOW)
	//Look for the correct type registration

	TopicDataType* p_type = nullptr;

	if(!getRegisteredType(RParam.topic.topicDataType,&p_type))
	{
		logError(RTPS_RTPSParticipant,"Type: " <<RParam.topic.topicDataType << " Not Registered");
		return nullptr;
	}
	if(RParam.topic.topicKind == WITH_KEY && !p_type->m_isGetKeyDefined)
	{
		logError(RTPS_RTPSParticipant,"Keyed Topic needs getKey function");
		return nullptr;
	}
	SubscriberImpl* subImpl = nullptr;
	if(p->getBuiltinAttributes().use_STATIC_EndpointDiscoveryProtocol)
	{
		if(RParam.userDefinedId <= 0)
		{
			logError(RTPS_RTPSParticipant,"Static EDP requires user defined Id");
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
		logInfo(RTPS_RTPSParticipant,"SUBSCRIBER CREATED",EPRO_B_YELLOW);
		Subscriber* Sub = new Subscriber(subImpl);
		m_subscriberList.push_back(SubscriberPair(Sub,subImpl));
		//p->getEventResource()->io_service.post(boost::bind(&RTPSParticipantImpl::registerReader,p,SR));
		p->registerReader(SR);
		//p->ReaderDiscovery(SR);
		return Sub;
	}

	logError(RTPS_RTPSParticipant,"Subscriber not created");

	return nullptr;
}



bool RTPSDomainImpl::getRegisteredType(std::string type_name,TopicDataType** type_ptr)
{

	for(std::vector<TopicDataType*>::iterator it=m_registeredTypes.begin();
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

bool RTPSDomainImpl::registerType(TopicDataType* type)
{

	const char* const METHOD_NAME = "registerType";
	for(std::vector<TopicDataType*>::iterator it = m_registeredTypes.begin();it!=m_registeredTypes.end();++it)
	{
		if((*it)->m_topicDataTypeName == type->m_topicDataTypeName)
			return false;
	}
	if(type->m_typeSize <=0)
	{
		logError(RTPS_RTPSParticipant,"Registered Type must have maximum byte size > 0");
		return false;
	}
	if(type->m_typeSize > PAYLOAD_MAX_SIZE)
	{
		logError(RTPS_RTPSParticipant,"Current version only supports types of sizes < "<<PAYLOAD_MAX_SIZE);
		return false;
	}
	if(type->m_topicDataTypeName.size() <=0)
	{
		logError(RTPS_RTPSParticipant,"Registered Type must have a name");
		return false;
	}
	m_registeredTypes.push_back(type);
	logInfo(RTPS_RTPSParticipant,"Type "<<type->m_topicDataTypeName << " registered.");
	return true;
}

bool RTPSDomainImpl::removeRTPSParticipant(RTPSParticipant* p)
{
	const char* const METHOD_NAME = "removeRTPSParticipant";
	RTPSParticipantImpl* pimpl = nullptr;
	if(p!=nullptr && getRTPSParticipantImpl(p,&pimpl))
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
		logInfo(RTPS_RTPSParticipant,"Publishers in "<<p->getGuid().guidPrefix << " deleted correctly.");
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
		logInfo(RTPS_RTPSParticipant,"Subscribers in "<<p->getGuid().guidPrefix << " deleted correctly.");
		for(std::vector<RTPSParticipantPair>::iterator it=m_RTPSParticipants.begin();
				it!=m_RTPSParticipants.end();++it)
		{
			if(it->second->getGuid() == p->getGuid())
			{
				int32_t pID = it->second->getRTPSParticipantID();
				m_RTPSParticipantIDs.erase(pID);
				
				delete(it->first);
				delete(it->second);
				m_RTPSParticipants.erase(it);
				return true;
			}
		}
	}
	logError(RTPS_RTPSParticipant,"RTPSParticipant not valid or not recognized");
	return false;
}

bool RTPSDomainImpl::removePublisher(RTPSParticipant* pin,Publisher* pub)
{
	const char* const METHOD_NAME = "removePublisher";
	RTPSParticipantImpl* p = nullptr;
	if(!getRTPSParticipantImpl(pin,&p))
	{
		logError(RTPS_RTPSParticipant,"RTPSParticipant not registered");
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
				logInfo(RTPS_RTPSParticipant, " OK");
				return true;
			}
		}
	}
	logInfo(RTPS_RTPSParticipant, " Not found.");
	return false;
}

bool RTPSDomainImpl::removeSubscriber(RTPSParticipant* pin,Subscriber* sub)
{
	const char* const METHOD_NAME = "removeSubscriber";
	RTPSParticipantImpl* p = nullptr;
	if(!getRTPSParticipantImpl(pin,&p))
	{
		logError(RTPS_RTPSParticipant,"RTPSParticipant not registered");
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
				logInfo(RTPS_RTPSParticipant, " OK");
				return true;
			}
		}
	}
	logInfo(RTPS_RTPSParticipant, " Not found.");
	return false;
}




} /* namespace pubsub */
} /* namespace eprosima */


