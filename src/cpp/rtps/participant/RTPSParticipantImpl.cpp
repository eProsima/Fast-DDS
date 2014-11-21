/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipant.cpp
 *
 */

#include "eprosimartps/rtps/resources/ResourceSend.h"
#include "eprosimartps/rtps/resources/ResourceEvent.h"
#include "eprosimartps/rtps/resources/ListenResource.h"

#include "eprosimartps/rtps/participant/RTPSParticipantImpl.h"

#include "eprosimartps/rtps/writer/StatelessWriter.h"
#include "eprosimartps/rtps/writer/StatefulWriter.h"

//#include "eprosimartps/reader/StatelessReader.h"
//#include "eprosimartps/reader/StatefulReader.h"

#include "eprosimartps/rtps/participant/RTPSParticipant.h"

#include "eprosimartps/rtps/RTPSDomain.h"

#include "eprosimartps/rtps/builtin/BuiltinProtocols.h"
#include "eprosimartps/rtps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/utils/eClock.h"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "eprosimartps/utils/RTPSLog.h"

using namespace pubsub;

namespace eprosima {
namespace rtps {


static const char* const CLASS_NAME = "RTPSParticipantImpl";


RTPSParticipantImpl::RTPSParticipantImpl(const RTPSParticipantAttributes& PParam,
		const GuidPrefix_t& guidP,
		RTPSParticipant* par,
		RTPSParticipantListener* plisten):
											m_guid(guidP,c_EntityId_RTPSParticipant),
											mp_send_thr(nullptr),
											mp_event_thr(nullptr),
											mp_builtinProtocols(nullptr),
											mp_ResourceSemaphore(new boost::interprocess::interprocess_semaphore(0)),
											IdCounter(0),
											mp_participantListener(plisten),
											mp_userParticipant(par)

{
	const char* const METHOD_NAME = "RTPSParticipantImpl";
	mp_userParticipant->mp_impl = this;
	m_att = PParam;
	Locator_t loc;
	loc.port = PParam.defaultSendPort;
	mp_send_thr = new ResourceSend();
	mp_send_thr->initSend(this,loc,m_att.sendSocketBufferSize,m_att.use_IP4_to_send,m_att.use_IP6_to_send);
	mp_event_thr = new ResourceEvent();
	mp_event_thr->init_thread(this);

	if(m_att.defaultMulticastLocatorList.empty() && m_att.defaultMulticastLocatorList.empty())
	{
		LocatorList_t myIP;
		IPFinder::getIPAddress(&myIP);
		std::stringstream ss;

		for(LocatorListIterator lit = myIP.begin();lit!=myIP.end();++lit)
		{
			lit->port=m_att.port.portBase+
					m_att.port.domainIDGain*PParam.builtin.domainId+
					m_att.port.offsetd3+
					m_att.port.participantIDGain*m_att.participantID;
			m_att.defaultUnicastLocatorList.push_back(*lit);
			ss << *lit << ";";
		}

		std::string auxstr = ss.str();
		logWarning(RTPS_PARTICIPANT,"RTPSParticipant created with NO default Unicast Locator List, adding Locators: "<<auxstr);
	}
	LocatorList_t defcopy = m_att.defaultUnicastLocatorList;
	m_att.defaultUnicastLocatorList.clear();
	for(LocatorListIterator lit = defcopy.begin();lit!=defcopy.end();++lit)
	{
		ListenResource* LR = new ListenResource();
		Locator_t loc = LR->init_thread(this,*lit,m_att.listenSocketBufferSize,false,false);
		m_att.defaultUnicastLocatorList.push_back(loc);
		this->m_listenResourceList.push_back(LR);
	}
	defcopy = m_att.defaultMulticastLocatorList;
	m_att.defaultMulticastLocatorList.clear();
	for(LocatorListIterator lit = defcopy.begin();lit!=defcopy.end();++lit)
	{
		ListenResource* LR = new ListenResource();
		Locator_t loc = LR->init_thread(this,*lit,m_att.listenSocketBufferSize,true,false);
		m_att.defaultMulticastLocatorList.push_back(loc);
		this->m_listenResourceList.push_back(LR);
	}


	logInfo(RTPS_PARTICIPANT,"RTPSParticipant \"" <<  m_att.getName() << "\" with guidPrefix: " <<m_guid.guidPrefix);
	//START BUILTIN PROTOCOLS
	mp_builtinProtocols = new BuiltinProtocols();
	if(!mp_builtinProtocols->initBuiltinProtocols(this,m_att.builtin))
	{
		logWarning(RTPS_PARTICIPANT, "The builtin protocols were not corecctly initialized");
	}

}


RTPSParticipantImpl::~RTPSParticipantImpl()
{
	const char* const METHOD_NAME = "~RTPSParticipantImpl";
	logInfo(RTPS_PARTICIPANT,"removing "<<this->getGuid());
	//Destruct threads:
	for(std::vector<ListenResource*>::iterator it=m_listenResourceList.begin();
			it!=m_listenResourceList.end();++it)
		delete(*it);

	//	for(std::vector<RTPSReader*>::iterator it=m_userReaderList.begin();
	//			it!=m_userReaderList.end();++it)
	//		delete(*it);

	for(std::vector<RTPSWriter*>::iterator it=m_userWriterList.begin();
			it!=m_userWriterList.end();++it)
		delete(*it);

	delete(this->mp_ResourceSemaphore);
	delete(this->mp_userParticipant);

}

/*
 *
 * MAIN RTPSParticipant IMPL API
 *
 */


bool RTPSParticipantImpl::createWriter(RTPSWriter** WriterOut,
		WriterAttributes& param,WriterHistory* hist,WriterListener* listen, const EntityId_t& entityId,bool isBuiltin)
{
	const char* const METHOD_NAME = "createWriter";
	std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" :"BEST_EFFORT";
	logInfo(RTPS_PARTICIPANT," of type " << type);
	EntityId_t entId;
	if(entityId== c_EntityId_Unknown)
	{
		if(param.endpoint.topicKind == NO_KEY)
			entId.value[3] = 0x03;
		else if(param.endpoint.topicKind == WITH_KEY)
			entId.value[3] = 0x02;
		uint32_t idnum;
		if(param.endpoint.getEntityID()>0)
			idnum = param.endpoint.getEntityID();
		else
		{
			IdCounter++;
			idnum = IdCounter;
		}

		octet* c = (octet*)&idnum;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
		if(this->existsEntityId(entId,WRITER))
		{
			logError(RTPS_PARTICIPANT,"A writer with the same entityId already exists in this RTPSParticipant");
			return false;
		}
	}
	else
	{
		entId = entityId;
	}
	RTPSWriter* SWriter = nullptr;
	GUID_t guid(m_guid.guidPrefix,entId);
	if(param.endpoint.reliabilityKind == BEST_EFFORT)
		SWriter = (RTPSWriter*)new StatelessWriter(this,guid,param,hist,listen);
	else if(param.endpoint.reliabilityKind == RELIABLE)
		SWriter = (RTPSWriter*)new StatefulWriter(this,guid,param,hist,listen);

	if(SWriter==nullptr)
		return false;

	//SWriter->setListener(inlisten);
	//SWriter->setQos(param.qos,true);
	if(param.endpoint.reliabilityKind == RELIABLE)
	{
		if(!assignEndpointListenResources((Endpoint*)SWriter,isBuiltin))
		{
			delete(SWriter);
			return false;
		}
	}
	m_allWriterList.push_back(SWriter);
	*WriterOut = SWriter;
	return true;
}

bool RTPSParticipantImpl::registerWriter(RTPSWriter* Writer,TopicAttributes& topicAtt,WriterQos& wqos)
{
	return this->mp_builtinProtocols->addLocalWriter(Writer,topicAtt,wqos);
}


/*
 *
 * AUXILIARY METHODS
 *
 *  */


bool RTPSParticipantImpl::existsEntityId(const EntityId_t& ent,EndpointKind_t kind) const
{
	if(kind == WRITER)
	{
		for(std::vector<RTPSWriter*>::const_iterator it = m_userWriterList.begin();
				it!=m_userWriterList.end();++it)
		{
			if(ent == (*it)->getGuid().entityId)
				return true;
		}
	}
	else
	{
		//		for(std::vector<RTPSReader*>::const_iterator it = m_userReaderList.begin();
		//			it!=m_userReaderList.end();++it)
		//		{
		//			if(ent == (*it)->getGuid().entityId)
		//				return true;
		//		}
	}
	return false;
}


/*
 *
 * LISTEN RESOURCE METHODS
 *
 */


bool RTPSParticipantImpl::assignEndpointListenResources(Endpoint* endp,bool isBuiltin)
{
	const char* const METHOD_NAME = "assignEndpointListenResources";
	bool valid = true;
	boost::lock_guard<boost::recursive_mutex> guard(*endp->getMutex());
	bool unicastempty = endp->getAttributes()->unicastLocatorList.empty();
	bool multicastempty = endp->getAttributes()->multicastLocatorList.empty();
	if(unicastempty && !isBuiltin && multicastempty)
	{
		std::string auxstr = endp->getAttributes()->endpointKind == WRITER ? "WRITER" : "READER";
		logWarning(RTPS_PARTICIPANT,auxstr << " created with no unicastLocatorList, adding default List");
		for(LocatorListIterator lit = m_att.defaultUnicastLocatorList.begin();lit!=m_att.defaultUnicastLocatorList.end();++lit)
		{
			assignEndpoint2Locator(endp,lit,false,false);
		}
		endp->getAttributes()->unicastLocatorList = m_att.defaultUnicastLocatorList;
	}
	else
	{
		for(LocatorListIterator lit = endp->getAttributes()->unicastLocatorList.begin();
				lit!=endp->getAttributes()->unicastLocatorList.end();++lit)
		{
			valid &= assignEndpoint2Locator(endp,lit,false,!isBuiltin);
		}
	}
	//MULTICAST
	if(multicastempty && !isBuiltin && unicastempty)
	{
		for(LocatorListIterator lit =m_att.defaultMulticastLocatorList.begin();
				lit!=m_att.defaultMulticastLocatorList.end();++lit)
		{
			valid &= assignEndpoint2Locator(endp,lit,true,false);
		}
		endp->getAttributes()->multicastLocatorList = m_att.defaultMulticastLocatorList;
	}
	else
	{
		for(LocatorListIterator lit = endp->getAttributes()->multicastLocatorList.begin();
				lit!=endp->getAttributes()->multicastLocatorList.end();++lit)
		{
			valid &= assignEndpoint2Locator(endp,lit,true,!isBuiltin);
		}
	}
	return valid;
}


bool RTPSParticipantImpl::assignEndpoint2Locator(Endpoint* endp,LocatorListIterator lit,bool isMulti,bool isFixed)
{
	for(std::vector<ListenResource*>::iterator it = m_listenResourceList.begin();it!=m_listenResourceList.end();++it)
	{
		if((*it)->isListeningTo(*lit))
		{
			(*it)->addAssociatedEndpoint(endp);
			return true;
		}
	}
	ListenResource* LR = new ListenResource();
	Locator_t loc = LR->init_thread(this,*lit,m_att.listenSocketBufferSize,isMulti,isFixed);
	if(loc.kind>0)
	{
		LR->addAssociatedEndpoint(endp);
		*lit = loc;
		m_listenResourceList.push_back(LR);
		return true;
	}
	else
	{
		delete(LR);
		return false;
	}
}


bool RTPSParticipantImpl::deleteUserEndpoint(Endpoint* p_endpoint)
{
	bool found = false;
	{
		boost::lock_guard<boost::recursive_mutex> guard(*p_endpoint->getMutex());
		if(p_endpoint->getAttributes()->endpointKind == WRITER)
		{
			for(auto wit=m_userWriterList.begin();
					wit!=m_userWriterList.end();++wit)
			{
				if((*wit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
				{
					m_userWriterList.erase(wit);
					found = true;
					break;
				}
			}
		}
		//	if(type == 'R')
		//	{
		//		for(p_ReaderIterator rit=m_userReaderList.begin()
		//				;rit!=m_userReaderList.end();++rit)
		//		{
		//			if((*rit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
		//			{
		//				m_userReaderList.erase(rit);
		//				found = true;
		//				break;
		//			}
		//		}
		//	}
		if(!found)
			return false;
		//REMOVE FOR BUILTINPROTOCOLS
		//	if(p_endpoint->getAttributes()->endpointKind == WRITER)
		//		m_builtinProtocols.removeLocalWriter((RTPSWriter*)p_endpoint);
		//	else
		//		m_builtinProtocols.removeLocalReader((RTPSReader*)p_endpoint);
		//	//BUILTINPROTOCOLS
		//Remove it from threadListenList
		std::vector<ListenResource*>::iterator thit;
		for(thit=m_listenResourceList.begin();
				thit!=m_listenResourceList.end();thit++)
		{
			(*thit)->removeAssociatedEndpoint(p_endpoint);
		}
		for(thit=m_listenResourceList.begin();
				thit!=m_listenResourceList.end();thit++)
		{
			if(!(*thit)->hasAssociatedEndpoints())
			{
				delete(*thit);
				m_listenResourceList.erase(thit);
				break;
			}
		}

	}
	delete(p_endpoint);
	return true;
}









//
//
//
//
//
//
//
//

//
//
//
//static EntityId_t TrustedWriter(const EntityId_t& reader)
//{
//	if(reader == c_EntityId_SPDPReader) return c_EntityId_SPDPWriter;
//	if(reader == c_EntityId_SEDPPubReader) return c_EntityId_SEDPPubWriter;
//	if(reader == c_EntityId_SEDPSubReader) return c_EntityId_SEDPSubWriter;
//	if(reader == c_EntityId_ReaderLiveliness) return c_EntityId_WriterLiveliness;
//
//	return c_EntityId_Unknown;
//}
//
//bool RTPSParticipantImpl::createReader(RTPSReader** ReaderOut,
//		SubscriberAttributes& param, uint32_t payload_size, bool isBuiltin,
//		StateKind_t kind, TopicDataType* ptype, SubscriberListener* inlisten,
//		const EntityId_t& entityId)
//{
//	const char* const METHOD_NAME = "createReader";
//	std::string type = (kind == STATELESS) ? "STATELESS" :"STATEFUL";
//	logInfo(RTPS_RTPSParticipant," on topic: "<<param.topic.getTopicName());
//	EntityId_t entId;
//	if(entityId == c_EntityId_Unknown)
//	{
//		if(param.topic.getTopicKind() == NO_KEY)
//			entId.value[3] = 0x04;
//		else if(param.topic.getTopicKind() == WITH_KEY)
//			entId.value[3] = 0x07;
//		uint32_t idnum;
//		if(param.getEntityId()>0)
//			idnum = param.getEntityId();
//		else
//		{
//			IdCounter++;
//			idnum = IdCounter;
//		}
//		octet* c = (octet*)&idnum;
//		entId.value[2] = c[0];
//		entId.value[1] = c[1];
//		entId.value[0] = c[2];
//		if(this->existsEntityId(entId,READER))
//		{
//			logError(RTPS_RTPSParticipant,"A reader with the same entityId already exists in this RTPSParticipant");
//			return false;
//		}
//	}
//	else
//		entId = entityId;
//	RTPSReader* SReader = NULL;
//	if(kind == STATELESS)
//		SReader = (RTPSReader*)new StatelessReader(param, m_guid.guidPrefix,entId,ptype);
//	else if(kind == STATEFUL)
//		SReader = (RTPSReader*)new StatefulReader(param, m_guid.guidPrefix,entId,ptype);
//	if(SReader==NULL)
//		return false;
//	SReader->setListener(inlisten);
//	SReader->setQos(param.qos,true);
//	SReader->mp_send_thr = &this->m_send_thr;
//	SReader->mp_event_thr = &this->m_event_thr;
//	if(isBuiltin)
//	{
//		SReader->setTrustedWriter(TrustedWriter(SReader->getGuid().entityId));
//	}
//	if(!assignEndpointListenResources((Endpoint*)SReader,isBuiltin))
//	{
//		delete(SReader);
//		return false;
//	}
//	m_allReaderList.push_back(SReader);
//
//	*ReaderOut = SReader;
//	return true;
//}
//
//void RTPSParticipantImpl::registerReader(RTPSReader* SReader)
//{
//	eClock::my_sleep(30);
//	m_userReaderList.push_back(SReader);
//	m_builtinProtocols.addLocalReader(SReader);
//}
//
//void RTPSParticipantImpl::registerWriter(RTPSWriter* SWriter)
//{
//	eClock::my_sleep(30);
//	m_userWriterList.push_back(SWriter);
//	m_builtinProtocols.addLocalWriter(SWriter);
//}
//
//

//
//

//
//
//
//
//bool RTPSParticipantImpl::deleteUserEndpoint(Endpoint* p_endpoint,char type)
//{
//	bool found = false;
//	{
//	boost::lock_guard<boost::recursive_mutex> guard(*p_endpoint->getMutex());
//	if(type == 'W')
//	{
//		for(p_WriterIterator wit=m_userWriterList.begin();
//				wit!=m_userWriterList.end();++wit)
//		{
//			if((*wit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
//			{
//				m_userWriterList.erase(wit);
//				found = true;
//				break;
//			}
//		}
//	}
//	if(type == 'R')
//	{
//		for(p_ReaderIterator rit=m_userReaderList.begin()
//				;rit!=m_userReaderList.end();++rit)
//		{
//			if((*rit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
//			{
//				m_userReaderList.erase(rit);
//				found = true;
//				break;
//			}
//		}
//	}
//	if(!found)
//		return false;
//	if(p_endpoint->getEndpointKind()==WRITER)
//		m_builtinProtocols.removeLocalWriter((RTPSWriter*)p_endpoint);
//	else
//		m_builtinProtocols.removeLocalReader((RTPSReader*)p_endpoint);
//	//Remove it from threadListenList
//	std::vector<ListenResource*>::iterator thit;
//	for(thit=m_listenResourceList.begin();
//			thit!=m_listenResourceList.end();thit++)
//	{
//		(*thit)->removeAssociatedEndpoint(p_endpoint);
//    }
//	for(thit=m_listenResourceList.begin();
//			thit!=m_listenResourceList.end();thit++)
//	{
//		if(!(*thit)->hasAssociatedEndpoints())
//		{
//						delete(*thit);
//			m_listenResourceList.erase(thit);
//			break;
//		}
//	}
//
//	}
//	delete(p_endpoint);
//	return true;
//}
//
//void RTPSParticipantImpl::announceRTPSParticipantState()
//{
//	this->m_builtinProtocols.announceRTPSParticipantState();
//}
//
//void RTPSParticipantImpl::stopRTPSParticipantAnnouncement()
//{
//	this->m_builtinProtocols.stopRTPSParticipantAnnouncement();
//}
//
//void RTPSParticipantImpl::resetRTPSParticipantAnnouncement()
//{
//	this->m_builtinProtocols.resetRTPSParticipantAnnouncement();
//}
//
//void RTPSParticipantImpl::ResourceSemaphorePost()
//{
//	if(mp_ResourceSemaphore!=NULL)
//	{
//		mp_ResourceSemaphore->post();
//	}
//}
//
//void RTPSParticipantImpl::ResourceSemaphoreWait()
//{
//	if(mp_ResourceSemaphore!=NULL)
//	{
//		mp_ResourceSemaphore->wait();
//	}
//
//}
//
//bool RTPSParticipantImpl::newRemoteEndpointDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind)
//{
//	const char* const METHOD_NAME = "newRemoteEndpointDiscovered";
//	if(m_builtin.use_STATIC_EndpointDiscoveryProtocol == false)
//	{
//		logWarning(RTPS_RTPSParticipant,"Remote Endpoints can only be activated with static discovery protocol");
//		return false;
//	}
//	return m_builtinProtocols.mp_PDP->newRemoteEndpointStaticallyDiscovered(pguid,userDefinedId,kind);
//}

} /* namespace rtps */
} /* namespace eprosima */


