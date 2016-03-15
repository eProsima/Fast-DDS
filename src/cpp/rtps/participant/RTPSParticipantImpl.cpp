/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipant.cpp
 *
 */

#include "RTPSParticipantImpl.h"

#include <fastrtps/rtps/resources/ResourceSend.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include "../resources/AsyncWriterThread.h"
#include <fastrtps/rtps/resources/ListenResource.h>



#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/eClock.h>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <fastrtps/utils/RTPSLog.h>



namespace eprosima {
namespace fastrtps{
namespace rtps {


static const char* const CLASS_NAME = "RTPSParticipantImpl";

static EntityId_t TrustedWriter(const EntityId_t& reader)
{
	if(reader == c_EntityId_SPDPReader) return c_EntityId_SPDPWriter;
	if(reader == c_EntityId_SEDPPubReader) return c_EntityId_SEDPPubWriter;
	if(reader == c_EntityId_SEDPSubReader) return c_EntityId_SEDPSubWriter;
	if(reader == c_EntityId_ReaderLiveliness) return c_EntityId_WriterLiveliness;

	return c_EntityId_Unknown;
}


RTPSParticipantImpl::RTPSParticipantImpl(const RTPSParticipantAttributes& PParam,
		const GuidPrefix_t& guidP,
		RTPSParticipant* par,
		RTPSParticipantListener* plisten):	m_guid(guidP,c_EntityId_RTPSParticipant),
				mp_send_thr(nullptr),
				mp_event_thr(nullptr),
                async_writers_thread_(nullptr),
				mp_builtinProtocols(nullptr),
				mp_ResourceSemaphore(new boost::interprocess::interprocess_semaphore(0)),
				IdCounter(0),
				mp_participantListener(plisten),
				mp_userParticipant(par),
				mp_mutex(new boost::recursive_mutex()),
				m_threadID(0)

{
	const char* const METHOD_NAME = "RTPSParticipantImpl";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	mp_userParticipant->mp_impl = this;
	m_att = PParam;
	Locator_t loc;
	loc.port = PParam.defaultSendPort;
	mp_send_thr = new ResourceSend();
	mp_send_thr->initSend(this,loc,m_att.sendSocketBufferSize,m_att.use_IP4_to_send,m_att.use_IP6_to_send);
	mp_event_thr = new ResourceEvent();
	mp_event_thr->init_thread(this);
    async_writers_thread_ = new AsyncWriterThread();
	bool hasLocatorsDefined = true;
	//If no default locator is defined you define one.
	if(m_att.defaultUnicastLocatorList.empty() && m_att.defaultMulticastLocatorList.empty())
	{
		hasLocatorsDefined = false;
		Locator_t loc2;
		loc2.port=m_att.port.portBase+
				m_att.port.domainIDGain*PParam.builtin.domainId+
				m_att.port.offsetd3+
				m_att.port.participantIDGain*m_att.participantID;
		loc2.kind = LOCATOR_KIND_UDPv4;
		m_att.defaultUnicastLocatorList.push_back(loc2);
	}
	LocatorList_t defcopy = m_att.defaultUnicastLocatorList;
	m_att.defaultUnicastLocatorList.clear();
	for(LocatorListIterator lit = defcopy.begin();lit!=defcopy.end();++lit)
	{
		ListenResource* LR = new ListenResource(this,++m_threadID,true);
		if(LR->init_thread(this,*lit,m_att.listenSocketBufferSize,false,false))
		{
			m_att.defaultUnicastLocatorList.push_back(LR->getListenLocators());
			this->m_listenResourceList.push_back(LR);
		}
		else
		{
			delete(LR);
		}
	}
	if(!hasLocatorsDefined)
		logInfo(RTPS_PARTICIPANT,m_att.getName()<<" Created with NO default Unicast Locator List, adding Locators: "<<m_att.defaultUnicastLocatorList);
	defcopy = m_att.defaultMulticastLocatorList;
	m_att.defaultMulticastLocatorList.clear();
	for(LocatorListIterator lit = defcopy.begin();lit!=defcopy.end();++lit)
	{
		ListenResource* LR = new ListenResource(this,++m_threadID,true);
		if(LR->init_thread(this,*lit,m_att.listenSocketBufferSize,true,false))
		{
			m_att.defaultMulticastLocatorList.push_back(LR->getListenLocators());
			this->m_listenResourceList.push_back(LR);
		}
		else
		{
			delete(LR);
		}
	}


	logInfo(RTPS_PARTICIPANT,"RTPSParticipant \"" <<  m_att.getName() << "\" with guidPrefix: " <<m_guid.guidPrefix);
	//START BUILTIN PROTOCOLS
	mp_builtinProtocols = new BuiltinProtocols();
	if(!mp_builtinProtocols->initBuiltinProtocols(this,m_att.builtin))
	{
		logWarning(RTPS_PARTICIPANT, "The builtin protocols were not corecctly initialized");
	}
	//eClock::my_sleep(300);
}


RTPSParticipantImpl::~RTPSParticipantImpl()
{
	const char* const METHOD_NAME = "~RTPSParticipantImpl";
	logInfo(RTPS_PARTICIPANT,"removing "<<this->getGuid());


	while(m_userReaderList.size()>0)
		RTPSDomain::removeRTPSReader(*m_userReaderList.begin());

	while(m_userWriterList.size()>0)
		RTPSDomain::removeRTPSWriter(*m_userWriterList.begin());

	//Destruct threads:
	for(std::vector<ListenResource*>::iterator it=m_listenResourceList.begin();
			it!=m_listenResourceList.end();++it)
		delete(*it);

	delete(this->mp_builtinProtocols);

	delete(this->mp_ResourceSemaphore);
	delete(this->mp_userParticipant);

    delete(this->async_writers_thread_);
	delete(this->mp_send_thr);
	delete(this->mp_event_thr);
	delete(this->mp_mutex);
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
	logInfo(RTPS_PARTICIPANT," of type " << type,C_B_YELLOW);
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
	if(!param.endpoint.unicastLocatorList.isValid())
	{
		logError(RTPS_PARTICIPANT,"Unicast Locator List for Writer contains invalid Locator");
		return false;
	}
	if(!param.endpoint.multicastLocatorList.isValid())
	{
		logError(RTPS_PARTICIPANT,"Multicast Locator List for Writer contains invalid Locator");
		return false;
	}

	RTPSWriter* SWriter = nullptr;
	GUID_t guid(m_guid.guidPrefix,entId);
	if(param.endpoint.reliabilityKind == BEST_EFFORT)
		SWriter = (RTPSWriter*)new StatelessWriter(this,guid,param,hist,listen);
	else if(param.endpoint.reliabilityKind == RELIABLE)
		SWriter = (RTPSWriter*)new StatefulWriter(this,guid,param,hist,listen);

	if(SWriter==nullptr)
		return false;

	if(param.endpoint.reliabilityKind == RELIABLE)
	{
		if(!assignEndpointListenResources((Endpoint*)SWriter,isBuiltin))
		{
			delete(SWriter);
			return false;
		}
	}

    // Check asynchornous thread is running.
    if(SWriter->isAsync())
    {
        async_writers_thread_->addWriter(SWriter);
    }

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	m_allWriterList.push_back(SWriter);
	if(!isBuiltin)
		m_userWriterList.push_back(SWriter);
	*WriterOut = SWriter;
	return true;
}


bool RTPSParticipantImpl::createReader(RTPSReader** ReaderOut,
		ReaderAttributes& param,ReaderHistory* hist,ReaderListener* listen, const EntityId_t& entityId,bool isBuiltin, bool enable)
{
	const char* const METHOD_NAME = "createReader";
	std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" :"BEST_EFFORT";
	logInfo(RTPS_PARTICIPANT," of type " << type,C_B_YELLOW);
	EntityId_t entId;
	if(entityId== c_EntityId_Unknown)
	{
		if(param.endpoint.topicKind == NO_KEY)
			entId.value[3] = 0x04;
		else if(param.endpoint.topicKind == WITH_KEY)
			entId.value[3] = 0x07;
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
			logError(RTPS_PARTICIPANT,"A reader with the same entityId already exists in this RTPSParticipant");
			return false;
		}
	}
	else
	{
		entId = entityId;
	}
	if(!param.endpoint.unicastLocatorList.isValid())
	{
		logError(RTPS_PARTICIPANT,"Unicast Locator List for Reader contains invalid Locator");
		return false;
	}
	if(!param.endpoint.multicastLocatorList.isValid())
	{
		logError(RTPS_PARTICIPANT,"Multicast Locator List for Reader contains invalid Locator");
		return false;
	}
	RTPSReader* SReader = nullptr;
	GUID_t guid(m_guid.guidPrefix,entId);
	if(param.endpoint.reliabilityKind == BEST_EFFORT)
		SReader = (RTPSReader*)new StatelessReader(this,guid,param,hist,listen);
	else if(param.endpoint.reliabilityKind == RELIABLE)
		SReader = (RTPSReader*)new StatefulReader(this,guid,param,hist,listen);

	if(SReader==nullptr)
		return false;

	//SReader->setListener(inlisten);
	//SReader->setQos(param.qos,true);
	if(isBuiltin)
	{
		SReader->setTrustedWriter(TrustedWriter(SReader->getGuid().entityId));
	}

    if(enable)
    {
        if(!assignEndpointListenResources((Endpoint*)SReader,isBuiltin))
        {
            delete(SReader);
            return false;
        }
    }

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	m_allReaderList.push_back(SReader);
	if(!isBuiltin)
		m_userReaderList.push_back(SReader);
	*ReaderOut = SReader;
	return true;
}

bool RTPSParticipantImpl::enableReader(RTPSReader *reader, bool isBuiltin)
{
    if(!assignEndpointListenResources((Endpoint*)reader,isBuiltin))
    {
        return false;
    }

    return true;
}




bool RTPSParticipantImpl::registerWriter(RTPSWriter* Writer,TopicAttributes& topicAtt,WriterQos& wqos)
{
	return this->mp_builtinProtocols->addLocalWriter(Writer,topicAtt,wqos);
}

bool RTPSParticipantImpl::registerReader(RTPSReader* reader,TopicAttributes& topicAtt,ReaderQos& rqos)
{
	return this->mp_builtinProtocols->addLocalReader(reader,topicAtt,rqos);
}

bool RTPSParticipantImpl::updateLocalWriter(RTPSWriter* Writer,WriterQos& wqos)
{
	return this->mp_builtinProtocols->updateLocalWriter(Writer,wqos);
}

bool RTPSParticipantImpl::updateLocalReader(RTPSReader* reader,ReaderQos& rqos)
{
	return this->mp_builtinProtocols->updateLocalReader(reader,rqos);
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
		for(std::vector<RTPSReader*>::const_iterator it = m_userReaderList.begin();
				it!=m_userReaderList.end();++it)
		{
			if(ent == (*it)->getGuid().entityId)
				return true;
		}
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
	//boost::lock_guard<boost::recursive_mutex> guard(*endp->getMutex()); //  Fixed bug #914
	bool unicastempty = endp->getAttributes()->unicastLocatorList.empty();
	bool multicastempty = endp->getAttributes()->multicastLocatorList.empty();
    LocatorList_t uniList, mulList;

    if(!unicastempty)
        uniList = endp->getAttributes()->unicastLocatorList;
    if(!multicastempty)
        mulList = endp->getAttributes()->multicastLocatorList;

	if(unicastempty && !isBuiltin && multicastempty)
	{
		std::string auxstr = endp->getAttributes()->endpointKind == WRITER ? "WRITER" : "READER";
		logInfo(RTPS_PARTICIPANT,"Adding default Locator list to this " << auxstr);
		valid &= assignEndpoint2LocatorList(endp,m_att.defaultUnicastLocatorList,false,false);
        boost::lock_guard<boost::recursive_mutex> guard(*endp->getMutex());
		endp->getAttributes()->unicastLocatorList = m_att.defaultUnicastLocatorList;
	}
	else
	{
        valid &= assignEndpoint2LocatorList(endp, uniList, false, !isBuiltin);
        boost::lock_guard<boost::recursive_mutex> guard(*endp->getMutex());
        endp->getAttributes()->unicastLocatorList = uniList;
	}
	//MULTICAST
	if(multicastempty && !isBuiltin && unicastempty)
	{
		valid &= assignEndpoint2LocatorList(endp,m_att.defaultMulticastLocatorList,true,false);
        boost::lock_guard<boost::recursive_mutex> guard(*endp->getMutex());
		endp->getAttributes()->multicastLocatorList = m_att.defaultMulticastLocatorList;
	}
	else
	{
        valid &= assignEndpoint2LocatorList(endp, mulList, true, !isBuiltin);
        boost::lock_guard<boost::recursive_mutex> guard(*endp->getMutex());
        endp->getAttributes()->multicastLocatorList = mulList;
	}
	return valid;
}

bool RTPSParticipantImpl::assignLocatorForBuiltin_unsafe(LocatorList_t& list, bool isMulti, bool isFixed)
{
	bool valid = true;
	LocatorList_t finalList;
	bool added = false;
	for(auto lit = list.begin();lit != list.end();++lit)
	{
		added = false;
		for(std::vector<ListenResource*>::iterator it = m_listenResourceList.begin();it!=m_listenResourceList.end();++it)
		{
			if((*it)->isListeningTo(*lit))
			{
				LocatorList_t locList = (*it)->getListenLocators();
				finalList.push_back(locList);
				added = true;
			}
		}
		if(added)
			continue;
		ListenResource* LR = new ListenResource(this,++m_threadID,false);
		if(LR->init_thread(this,*lit,m_att.listenSocketBufferSize,isMulti,isFixed))
		{
			LocatorList_t locList = LR->getListenLocators();
			finalList.push_back(locList);
			m_listenResourceList.push_back(LR);
			added = true;
		}
		else
		{
			delete(LR);
			valid &= false;
		}
	}
	if(valid && added)
		list = finalList;
	return valid;
}

bool RTPSParticipantImpl::assignEndpoint2LocatorList(Endpoint* endp,LocatorList_t& list,bool isMulti,bool isFixed)
{
	bool valid = true;
	LocatorList_t finalList;
	bool added = false;
	for(auto lit = list.begin();lit != list.end();++lit)
	{
		added = false;
		boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
		for(std::vector<ListenResource*>::iterator it = m_listenResourceList.begin();it!=m_listenResourceList.end();++it)
		{
			if((*it)->isListeningTo(*lit))
			{
				(*it)->addAssociatedEndpoint(endp);
				LocatorList_t locList = (*it)->getListenLocators();
				finalList.push_back(locList);
				added = true;
			}
		}
		if(added)
			continue;
		ListenResource* LR = new ListenResource(this,++m_threadID,false);
		if(LR->init_thread(this,*lit,m_att.listenSocketBufferSize,isMulti,isFixed))
		{
			LR->addAssociatedEndpoint(endp);
			LocatorList_t locList = LR->getListenLocators();
			finalList.push_back(locList);
			m_listenResourceList.push_back(LR);
			added = true;
		}
		else
		{
			delete(LR);
			valid &= false;
		}
	}
	if(valid && added)
		list = finalList;
	return valid;
}


bool RTPSParticipantImpl::deleteUserEndpoint(Endpoint* p_endpoint)
{
	bool found = false;
	{
		if(p_endpoint->getAttributes()->endpointKind == WRITER)
		{
			boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
			for(auto wit=m_userWriterList.begin();
					wit!=m_userWriterList.end();++wit)
			{
				if((*wit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
				{
                    // If writer is asynchronous, remove from async thread.
                    async_writers_thread_->removeWriter(*wit);

					m_userWriterList.erase(wit);
					found = true;
					break;
				}
			}
		}
		else
		{
			boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
			for(auto rit=m_userReaderList.begin()
					;rit!=m_userReaderList.end();++rit)
			{
				if((*rit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
				{
					m_userReaderList.erase(rit);
					found = true;
					break;
				}
			}
		}
		if(!found)
			return false;
		//REMOVE FOR BUILTINPROTOCOLS
		if(p_endpoint->getAttributes()->endpointKind == WRITER)
			mp_builtinProtocols->removeLocalWriter((RTPSWriter*)p_endpoint);
		else
			mp_builtinProtocols->removeLocalReader((RTPSReader*)p_endpoint);
		//BUILTINPROTOCOLS
		//Remove it from threadListenList
		std::vector<ListenResource*>::iterator thit;
		for(thit=m_listenResourceList.begin();
				thit!=m_listenResourceList.end();thit++)
		{
			(*thit)->removeAssociatedEndpoint(p_endpoint);
		}

		boost::lock_guard<boost::recursive_mutex> guardParticipant(*mp_mutex);
		bool continue_removing = true;
		while(continue_removing)
		{
			continue_removing = false;
			for(thit=m_listenResourceList.begin();
					thit!=m_listenResourceList.end();thit++)
			{
				if(!(*thit)->hasAssociatedEndpoints() && ! (*thit)->m_isDefaultListenResource)
				{
					delete(*thit);
					m_listenResourceList.erase(thit);
					continue_removing = true;
					break;
				}
			}
		}
	}
	//	boost::lock_guard<boost::recursive_mutex> guardEndpoint(*p_endpoint->getMutex());
	delete(p_endpoint);
	return true;
}


ResourceEvent& RTPSParticipantImpl::getEventResource()
{
	return *this->mp_event_thr;
}

void RTPSParticipantImpl::sendSync(CDRMessage_t* msg, const Locator_t& loc)
{
	return mp_send_thr->sendSync(msg, loc);
}

void RTPSParticipantImpl::announceRTPSParticipantState()
{
	return mp_builtinProtocols->announceRTPSParticipantState();
}

void RTPSParticipantImpl::stopRTPSParticipantAnnouncement()
{
	return mp_builtinProtocols->stopRTPSParticipantAnnouncement();
}

void RTPSParticipantImpl::resetRTPSParticipantAnnouncement()
{
	return mp_builtinProtocols->resetRTPSParticipantAnnouncement();
}

void RTPSParticipantImpl::loose_next_change()
{
	this->mp_send_thr->loose_next_change();
}


bool RTPSParticipantImpl::newRemoteEndpointDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind)
{
	const char* const METHOD_NAME = "newRemoteEndpointDiscovered";
	if(m_att.builtin.use_STATIC_EndpointDiscoveryProtocol == false)
	{
		logWarning(RTPS_PARTICIPANT,"Remote Endpoints can only be activated with static discovery protocol");
		return false;
	}
	return mp_builtinProtocols->mp_PDP->newRemoteEndpointStaticallyDiscovered(pguid,userDefinedId,kind);
}

void RTPSParticipantImpl::ResourceSemaphorePost()
{
	if(mp_ResourceSemaphore != nullptr)
	{
		mp_ResourceSemaphore->post();
	}
}

void RTPSParticipantImpl::ResourceSemaphoreWait()
{
	if (mp_ResourceSemaphore != nullptr)
	{
		mp_ResourceSemaphore->wait();
	}

}

boost::recursive_mutex* RTPSParticipantImpl::getSendMutex()
{
	return mp_send_thr->getMutex();
}

void RTPSParticipantImpl::assertRemoteRTPSParticipantLiveliness(const GuidPrefix_t& guidP)
{
	this->mp_builtinProtocols->mp_PDP->assertRemoteParticipantLiveliness(guidP);
}



}
} /* namespace rtps */
} /* namespace eprosima */


