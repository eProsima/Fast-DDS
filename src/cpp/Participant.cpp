/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * Participant.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/Participant.h"

#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/writer/RTPSWriter.h"


//#include "eprosimartps/dds/DomainParticipant.h"

#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"
#include "eprosimartps/discovery/SimplePDP.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"

namespace eprosima {
namespace rtps {

typedef std::vector<RTPSReader*>::iterator p_ReaderIterator;
typedef std::vector<RTPSWriter*>::iterator p_WriterIterator;


ParticipantImpl::ParticipantImpl(const ParticipantAttributes& PParam,const GuidPrefix_t& guidP,uint32_t ID):

							m_defaultUnicastLocatorList(PParam.defaultUnicastLocatorList),
							m_defaultMulticastLocatorList(PParam.defaultMulticastLocatorList),
							m_participantName(PParam.name),
							m_guid(guidP,c_EntityId_Participant),
							mp_ResourceSemaphore(new boost::interprocess::interprocess_semaphore(0)),
							IdCounter(0),
							mp_PDP(NULL),
							m_participantID(ID)

{
	Locator_t loc;
	loc.port = PParam.defaultSendPort;
	m_send_thr.initSend(loc);

	m_event_thr.init_thread();

	if(m_defaultUnicastLocatorList.empty())
	{
		pWarning("Participant created with NO default Unicast Locator List, adding Locator 0.0.0.0:10042"<<endl);
		LocatorList_t myIP;
		IPFinder::getIPAddress(&myIP);
		for(LocatorListIterator lit = myIP.begin();lit!=myIP.end();++lit)
		{
			lit->port=10042;
			m_defaultUnicastLocatorList.push_back(*lit);
		}
	}

	pInfo("Participant \"" <<  m_participantName << "\" with guidPrefix: " <<m_guid.guidPrefix<< endl);


	m_discovery = PParam.discovery;

	if(m_discovery.use_SIMPLE_ParticipantDiscoveryProtocol)
	{
		mp_PDP = (ParticipantDiscoveryProtocol*) new SimplePDP(this);
		mp_PDP->initPDP(PParam.discovery, this->getParticipantId());
	}
}


ParticipantImpl::~ParticipantImpl()
{
	pDebugInfo("Participant destructor"<<endl;);
	//Destruct threads:
	for(std::vector<ResourceListen*>::iterator it=m_threadListenList.begin();
			it!=m_threadListenList.end();++it)
		delete(*it);

	for(std::vector<RTPSReader*>::iterator it=m_userReaderList.begin();
			it!=m_userReaderList.end();++it)
		delete(*it);

	for(std::vector<RTPSWriter*>::iterator it=m_userWriterList.begin();
			it!=m_userWriterList.end();++it)
		delete(*it);

	delete(this->mp_ResourceSemaphore);

	if(mp_PDP!=NULL)
		delete(mp_PDP);
}

bool ParticipantImpl::createStatelessWriter(StatelessWriter** SW_out, PublisherAttributes& param,
		uint32_t payload_size,bool isBuiltin,DDSTopicDataType* ptype,PublisherListener* plisten,const EntityId_t& entityId)
{
	pDebugInfo("Creating Stateless Writer"<<endl);
	EntityId_t entId;
	if(entityId== c_EntityId_Unknown)
	{
		if(param.topic.getTopicKind() == NO_KEY)
			entId.value[3] = 0x03;
		else if(param.topic.getTopicKind() == WITH_KEY)
			entId.value[3] = 0x02;
		IdCounter++;
		octet* c = (octet*)&IdCounter;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
	}
	else
	{
		entId = entityId;
	}
	StatelessWriter* SLWriter = new StatelessWriter(param,m_guid.guidPrefix,entId,ptype);
	SLWriter->setListener(plisten);
	if(this->initWriter((RTPSWriter*)SLWriter,isBuiltin))
	{
		*SW_out = SLWriter;
		return true;
	}
	else
		return false;
}

bool ParticipantImpl::createStatefulWriter(StatefulWriter** SFW_out, PublisherAttributes& param,
		uint32_t payload_size,bool isBuiltin,DDSTopicDataType* ptype,PublisherListener* plisten,const EntityId_t& entityId)
{
	pDebugInfo("Creating StatefulWriter"<<endl);
	EntityId_t entId;
	if(entityId== c_EntityId_Unknown)
	{
		if(param.topic.getTopicKind() == NO_KEY)
			entId.value[3] = 0x03;
		else if(param.topic.getTopicKind() == WITH_KEY)
			entId.value[3] = 0x02;
		IdCounter++;
		octet* c = (octet*)&IdCounter;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
	}
	else
	{
		entId = entityId;
	}
	StatefulWriter* SFWriter = new StatefulWriter(param, m_guid.guidPrefix,entId,ptype);
	SFWriter->setListener(plisten);
	if(this->initWriter((RTPSWriter*)SFWriter,isBuiltin))
	{
		*SFW_out = SFWriter;
		return true;
	}
	else return false;
}

bool ParticipantImpl::initWriter(RTPSWriter*W,bool isBuiltin)
{
	pDebugInfo("Writer created, initializing"<<endl);
	//Check if locator lists are empty:
	if(W->unicastLocatorList.empty() && W->getStateType() == STATEFUL && !isBuiltin)
	{
		pWarning("Reliable Writer defined with NO unicast locator, assigning default"<<endl);
		W->unicastLocatorList = m_defaultUnicastLocatorList;
	}
	if(W->unicastLocatorList.empty() && W->getStateType() == STATEFUL)
		W->multicastLocatorList = m_defaultMulticastLocatorList;
	//Assign participant pointer
	W->mp_send_thr = &this->m_send_thr;
	W->mp_event_thr = &this->m_event_thr;


	//Look for receiving threads that are already listening to this writer receiving addresses.
	if(assignEnpointToListenResources((Endpoint*)W,'W',isBuiltin))
	{
		//Wait until the thread is correctly created
		if(!isBuiltin)
		{
			m_userWriterList.push_back(W);
			if(mp_PDP!=NULL)
				mp_PDP->localWriterMatching(W,true);
		}
		m_allWriterList.push_back(W);
		pDebugInfo("Finished Writer creation"<<endl);
		return true;
	}
	else
	{
		pDebugInfo("Finished Writer creation (FAILED)"<<endl);
		return false;
	}

}

bool ParticipantImpl::createStatelessReader(StatelessReader** SR_out,
		SubscriberAttributes& param,uint32_t payload_size,bool isBuiltin,DDSTopicDataType* ptype,SubscriberListener* slisten, const EntityId_t& entityId)
{
	pInfo("Creating StatelessReader"<<endl);
	EntityId_t entId;
	if(entityId == c_EntityId_Unknown)
	{
		if(param.topic.getTopicKind() == NO_KEY)
			entId.value[3] = 0x04;
		else if(param.topic.getTopicKind() == WITH_KEY)
			entId.value[3] = 0x07;
		IdCounter++;
		octet* c = (octet*)&IdCounter;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
	}
	else
		entId = entityId;
	StatelessReader* SReader = new StatelessReader(param, m_guid.guidPrefix,entId,ptype);
	SReader->setListener(slisten);
	if(initReader((RTPSReader*)SReader,isBuiltin))
	{
		*SR_out = SReader;
		return true;
	}
	else
		return false;
}

bool ParticipantImpl::createStatefulReader(StatefulReader** SR_out,
		SubscriberAttributes& param,uint32_t payload_size,bool isBuiltin,DDSTopicDataType* ptype,SubscriberListener* slisten,const EntityId_t& entityId)
{
	pDebugInfo("Creating StatefulReader"<<endl);
	EntityId_t entId;
	if(entityId == c_EntityId_Unknown)
	{
		if(param.topic.getTopicKind() == NO_KEY)
			entId.value[3] = 0x04;
		else if(param.topic.getTopicKind() == WITH_KEY)
			entId.value[3] = 0x07;
		IdCounter++;
		octet* c = (octet*)&IdCounter;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
	}
	else
		entId = entityId;
	StatefulReader* SReader = new StatefulReader(param, m_guid.guidPrefix,entId,ptype);
	SReader->setListener(slisten);
	if(initReader((RTPSReader*)SReader,isBuiltin))
	{
		*SR_out = SReader;
		return true;
	}
	else
	{
		*SR_out = NULL;
		return false;
	}
}



bool ParticipantImpl::initReader(RTPSReader* p_R,bool isBuiltin)
{
	//If NO UNICAST
	if(p_R->unicastLocatorList.empty() && !isBuiltin)
	{
		pWarning("Subscriber created with no unicastLocatorList, adding default List"<<endl);
		p_R->unicastLocatorList = m_defaultUnicastLocatorList;
	}
	//IF NO MULTICAST
	if(p_R->multicastLocatorList.empty())
		p_R->multicastLocatorList = m_defaultMulticastLocatorList;
	//Assignthread pointers
	p_R->mp_send_thr = &this->m_send_thr;
	p_R->mp_event_thr = &this->m_event_thr;

	//Look for receiving threads that are already listening to this writer receiving addresses.

	if(this->assignEnpointToListenResources((Endpoint*)p_R,'R',isBuiltin))
	{
		if(!isBuiltin)
		{
			m_userReaderList.push_back(p_R);
			if(mp_PDP!=NULL)
				mp_PDP->localReaderMatching(p_R,true);
		}
		m_allReaderList.push_back(p_R);

		return true;
	}
	else
		return false;
}

bool ParticipantImpl::createReader(RTPSReader** ReaderOut,
		SubscriberAttributes& param, uint32_t payload_size, bool isBuiltin,
		StateKind_t kind, DDSTopicDataType* ptype, SubscriberListener* slisten,
		const EntityId_t& entityId)
{
	pDebugInfo("Creating StatefulReader"<<endl);
	EntityId_t entId;
	if(entityId == c_EntityId_Unknown)
	{
		if(param.topic.getTopicKind() == NO_KEY)
			entId.value[3] = 0x04;
		else if(param.topic.getTopicKind() == WITH_KEY)
			entId.value[3] = 0x07;
		IdCounter++;
		octet* c = (octet*)&IdCounter;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
	}
	else
		entId = entityId;
	RTPSReader* SReader = NULL;
	if(kind == STATELESS)
		SReader = (RTPSReader*)new StatelessReader(param, m_guid.guidPrefix,entId,ptype);
	else if(kind == STATEFUL)
		SReader = (RTPSReader*)new StatefulReader(param, m_guid.guidPrefix,entId,ptype);
	if(SReader==NULL)
		return false;
	//If NO UNICAST
	if(SReader->unicastLocatorList.empty() && !isBuiltin)
	{
		pWarning("Subscriber created with no unicastLocatorList, adding default List"<<endl);
		SReader->unicastLocatorList = m_defaultUnicastLocatorList;
	}
	//IF NO MULTICAST
	if(SReader->multicastLocatorList.empty())
		SReader->multicastLocatorList = m_defaultMulticastLocatorList;
	//Assignthread pointers
	SReader->mp_send_thr = &this->m_send_thr;
	SReader->mp_event_thr = &this->m_event_thr;
	for(LocatorListIterator lit = SReader->unicastLocatorList.begin();lit!=SReader->unicastLocatorList.end();++lit)
	{

	}
	return true;
}

bool ParticipantImpl::assignLocator2ResourceListen(Endpoint* pend,LocatorListIterator lit,bool isMulticast,bool isFixed)
{
	//First Check if the same locator is already being listened:
	bool listening = false;
	for(std::vector<ResourceListen*>::iterator thit=m_threadListenList.begin();thit!=m_threadListenList.end();++thit)
	{
		if((*thit)->isListeningTo(*lit))
		{
			if((*thit)->addAssociatedEndpoint(pend))
			{
				return true;
			}
		}
	}
	if(!listening)
	{
		int aux = 3;
		ResourceListen* RL = new ResourceListen(this,isMulticast,isFixed,aux);
		if(RL!=NULL)
		{
			if(RL->init_thread(lit))
			{
				mp_ResourceSemaphore->wait();
				m_threadListenList.push_back(RL);
				RL->addAssociatedEndpoint(pend);
			}
		}
	}
	return false;
}





bool ParticipantImpl::assignEnpointToListenResources(Endpoint* p_endpoint, char type,bool isBuiltin) {
	if(type !='R' && type!='W')
	{
		return false;
	}

	std::vector<ResourceListen*>::iterator thit;
	std::vector<Locator_t>::iterator locit_th;
	std::vector<Locator_t>::iterator locit_e;
	bool assigned = false;

	for(locit_e = p_endpoint->unicastLocatorList.begin();locit_e!=p_endpoint->unicastLocatorList.end();++locit_e)
	{
		assigned = false;
		for(thit=m_threadListenList.begin();thit!=m_threadListenList.end();++thit)
		{
			if((*thit)->isListeningTo(*locit_e))
			{
				if((*thit)->addAssociatedEndpoint(p_endpoint))
				{
					assigned = true;
					break;
				}
			}
		}
		if(!assigned) //Create new listen thread
		{
			ResourceListen* thListen = NULL;
			if(addNewListenResource(*locit_e,&thListen,false,isBuiltin))
			{//Add new listen thread to participant
				mp_ResourceSemaphore->wait();
				thListen->addAssociatedEndpoint(p_endpoint);
				assigned = true;
			}
			else
				return false;
		}
	}
	for(locit_e = p_endpoint->multicastLocatorList.begin();locit_e!=p_endpoint->multicastLocatorList.end();++locit_e)
	{
		assigned = false;
		for(thit=m_threadListenList.begin();thit!=m_threadListenList.end();++thit)
		{
			if((*thit)->isListeningTo(*locit_e))
			{
				if((*thit)->addAssociatedEndpoint(p_endpoint))
				{
					assigned = true;
					break;
				}
			}
		}
		if(!assigned) //Create new listen thread
		{
			ResourceListen* thListen = NULL;
			if(addNewListenResource(*locit_e,&thListen,true,isBuiltin))
			{//Add new listen thread to participant
				mp_ResourceSemaphore->wait();
				thListen->addAssociatedEndpoint(p_endpoint);
				assigned = true;
			}
			else
				return false;
		}
	}
	return true;
}

bool ParticipantImpl::addNewListenResource(Locator_t& loc,ResourceListen** thlisten_in,bool isMulticast,bool isBuiltin)
{
	*thlisten_in = new ResourceListen(this,isMulticast,isBuiltin);
	if((*thlisten_in)->init_thread(loc))
	{
		m_threadListenList.push_back(*thlisten_in);
		return true;
	}
	else
		return false;
}

bool ParticipantImpl::deleteUserEndpoint(Endpoint* p_endpoint,char type)
{
	bool found = false;
	if(type == 'W')
	{
		for(p_WriterIterator wit=m_userWriterList.begin();
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
	if(type == 'R')
	{
		for(p_ReaderIterator rit=m_userReaderList.begin()
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
	//Remove it from threadListenList
	std::vector<ResourceListen*>::iterator thit;
	for(thit=m_threadListenList.begin();
			thit!=m_threadListenList.end();thit++)
	{
		(*thit)->removeEndpointFromAssociated(p_endpoint);
		if(!(*thit)->hasAssociatedEndpoints())
			delete(*thit);
	}
	delete(p_endpoint);
	return true;
}

void ParticipantImpl::announceParticipantState()
{
	this->mp_PDP->announceParticipantState(false);
}


} /* namespace rtps */
} /* namespace eprosima */


