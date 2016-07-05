// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file EDPSimple.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include "EDPSimpleListeners.h"


#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>


#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/history/WriterHistory.h>


#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>


#include <fastrtps/utils/RTPSLog.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "EDPSimple";

EDPSimple::EDPSimple(PDPSimple* p,RTPSParticipantImpl* part):
												EDP(p,part),
												mp_pubListen(nullptr),
												mp_subListen(nullptr)

{
	// TODO Auto-generated constructor stub

}

EDPSimple::~EDPSimple()
{
	if(this->mp_PubReader.first !=nullptr)
	{
		delete(mp_PubReader.first);
		delete(mp_PubReader.second);
	}
	if(this->mp_SubReader.first !=nullptr)
	{
		delete(mp_SubReader.first);
		delete(mp_SubReader.second);
	}
	if(this->mp_PubWriter.first !=nullptr)
	{
		delete(mp_PubWriter.first);
		delete(mp_PubWriter.second);
	}
	if(this->mp_SubWriter.first !=nullptr)
	{
		delete(mp_SubWriter.first);
		delete(mp_SubWriter.second);
	}
	if(mp_pubListen!=nullptr)
		delete(mp_pubListen);
	if(mp_subListen !=nullptr)
		delete(mp_subListen);
}


bool EDPSimple::initEDP(BuiltinAttributes& attributes)
{
	const char* const METHOD_NAME = "initEDP";
	logInfo(RTPS_EDP,"Beginning Simple Endpoint Discovery Protocol",C_B_CYAN);
	m_discovery = attributes;

	if(!createSEDPEndpoints())
	{
		logError(RTPS_EDP,"Problem creation SimpleEDP endpoints");
		return false;
	}
	return true;
}


bool EDPSimple::createSEDPEndpoints()
{
	const char* const METHOD_NAME = "createSEDPEndpoints";
	logInfo(RTPS_EDP,"Beginning",C_CYAN);
	WriterAttributes watt;
	ReaderAttributes ratt;
	HistoryAttributes hatt;
	bool created = true;
	RTPSReader* raux = nullptr;
	RTPSWriter* waux = nullptr;
	if(m_discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
	{
		hatt.initialReservedCaches = 100;
		hatt.maximumReservedCaches = 5000;
		hatt.payloadInitialSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
		mp_PubWriter.second = new WriterHistory(hatt);
		//Wparam.pushMode = true;
		watt.endpoint.reliabilityKind = RELIABLE;
		watt.endpoint.topicKind = WITH_KEY;
		watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		created &=this->mp_RTPSParticipant->createWriter(&waux,watt,mp_PubWriter.second,nullptr,c_EntityId_SEDPPubWriter,true);
		if(created)
		{
			mp_PubWriter.first = dynamic_cast<StatefulWriter*>(waux);
			logInfo(RTPS_EDP,"SEDP Publication Writer created",C_CYAN);
		}
		else
		{
			delete(mp_PubWriter.second);
			mp_PubWriter.second = nullptr;
		}
		hatt.initialReservedCaches = 100;
		hatt.maximumReservedCaches = 1000000;
		hatt.payloadInitialSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
		mp_SubReader.second = new ReaderHistory(hatt);
		//Rparam.historyMaxSize = 100;
		ratt.expectsInlineQos = false;
		ratt.endpoint.reliabilityKind = RELIABLE;
		ratt.endpoint.topicKind = WITH_KEY;
		ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		this->mp_subListen = new EDPSimpleSUBListener(this);
		created &=this->mp_RTPSParticipant->createReader(&raux,ratt,mp_SubReader.second,mp_subListen,c_EntityId_SEDPSubReader,true);
		if(created)
		{
			mp_SubReader.first = dynamic_cast<StatefulReader*>(raux);
			logInfo(RTPS_EDP,"SEDP Subscription Reader created",C_CYAN);
		}
		else
		{
			delete(mp_SubReader.second);
			mp_SubReader.second = nullptr;
			delete(mp_subListen);
			mp_subListen = nullptr;
		}
	}
	if(m_discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
	{
		hatt.initialReservedCaches = 100;
		hatt.maximumReservedCaches = 1000000;
		hatt.payloadInitialSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
		mp_PubReader.second = new ReaderHistory(hatt);
		//Rparam.historyMaxSize = 100;
		ratt.expectsInlineQos = false;
		ratt.endpoint.reliabilityKind = RELIABLE;
		ratt.endpoint.topicKind = WITH_KEY;
		ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		this->mp_pubListen = new EDPSimplePUBListener(this);
		created &=this->mp_RTPSParticipant->createReader(&raux,ratt,mp_PubReader.second,mp_pubListen,c_EntityId_SEDPPubReader,true);
		if(created)
		{
			mp_PubReader.first = dynamic_cast<StatefulReader*>(raux);
			logInfo(RTPS_EDP,"SEDP Publication Reader created",C_CYAN);

		}
		else
		{
			delete(mp_PubReader.second);
			mp_PubReader.second = nullptr;
			delete(mp_pubListen);
			mp_pubListen = nullptr;
		}
		hatt.initialReservedCaches = 100;
		hatt.maximumReservedCaches = 5000;
		hatt.payloadInitialSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
		mp_SubWriter.second = new WriterHistory(hatt);
		//Wparam.pushMode = true;
		watt.endpoint.reliabilityKind = RELIABLE;
		watt.endpoint.topicKind = WITH_KEY;
		watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		created &=this->mp_RTPSParticipant->createWriter(&waux,watt,mp_SubWriter.second,nullptr,c_EntityId_SEDPSubWriter,true);
		if(created)
		{
			mp_SubWriter.first = dynamic_cast<StatefulWriter*>(waux);
			logInfo(RTPS_EDP,"SEDP Subscription Writer created",C_CYAN);

		}
		else
		{
			delete(mp_SubWriter.second);
			mp_SubWriter.second = nullptr;
		}
	}
	logInfo(RTPS_EDP,"Creation finished",C_CYAN);
	return created;
}


bool EDPSimple::processLocalReaderProxyData(ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "processLocalReaderProxyData";
	logInfo(RTPS_EDP,rdata->m_guid.entityId,C_CYAN);
	if(mp_SubWriter.first !=nullptr)
	{
		CacheChange_t* change = mp_SubWriter.first->new_change(ALIVE,rdata->m_key);
		if(change !=nullptr)
		{
			rdata->toParameterList();
#if EPROSIMA_BIG_ENDIAN
			ParameterList::updateCDRMsg(&rdata->m_parameterList, BIGEND);
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
#else
            ParameterList::updateCDRMsg(&rdata->m_parameterList, LITTLEEND);
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
#endif
			change->serializedPayload.length = (uint16_t)rdata->m_parameterList.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,rdata->m_parameterList.m_cdrmsg.buffer,change->serializedPayload.length);
            boost::unique_lock<boost::recursive_mutex> lock(*mp_SubWriter.second->getMutex());
			for(auto ch = mp_SubWriter.second->changesBegin();ch!=mp_SubWriter.second->changesEnd();++ch)
			{
				if((*ch)->instanceHandle == change->instanceHandle)
				{
					mp_SubWriter.second->remove_change(*ch);
					break;
				}
			}
            lock.unlock();
			mp_SubWriter.second->add_change(change);
			return true;
		}
		return false;
	}
	return true;
}
bool EDPSimple::processLocalWriterProxyData(WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "processLocalWriterProxyData";
	logInfo(RTPS_EDP, wdata->guid().entityId, C_CYAN);
	if(mp_PubWriter.first !=nullptr)
	{
		CacheChange_t* change = mp_PubWriter.first->new_change(ALIVE, wdata->key());
		if(change != nullptr)
		{
			wdata->toParameterList();
#if EPROSIMA_BIG_ENDIAN
			ParameterList::updateCDRMsg(&wdata->m_parameterList,BIGEND);
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
#else
            ParameterList::updateCDRMsg(&wdata->m_parameterList, LITTLEEND);
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
#endif
			change->serializedPayload.length = (uint16_t)wdata->m_parameterList.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,wdata->m_parameterList.m_cdrmsg.buffer,change->serializedPayload.length);
            boost::unique_lock<boost::recursive_mutex> lock(*mp_PubWriter.second->getMutex());
			for(auto ch = mp_PubWriter.second->changesBegin();ch!=mp_PubWriter.second->changesEnd();++ch)
			{
				if((*ch)->instanceHandle == change->instanceHandle)
				{
					mp_PubWriter.second->remove_change(*ch);
					break;
				}
			}
            lock.unlock();
			mp_PubWriter.second->add_change(change);
			return true;
		}
		return false;
	}
	return true;
}

bool EDPSimple::removeLocalWriter(RTPSWriter* W)
{
	const char* const METHOD_NAME = "removeLocalWriter";
	logInfo(RTPS_EDP,W->getGuid().entityId,C_CYAN);
	if(mp_PubWriter.first!=nullptr)
	{
		InstanceHandle_t iH;
		iH = W->getGuid();
		CacheChange_t* change = mp_PubWriter.first->new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
		if(change != nullptr)
		{
			boost::lock_guard<boost::recursive_mutex> guard(*mp_PubWriter.second->getMutex());
			for(auto ch = mp_PubWriter.second->changesBegin();ch!=mp_PubWriter.second->changesEnd();++ch)
			{
				if((*ch)->instanceHandle == change->instanceHandle)
				{
					mp_PubWriter.second->remove_change(*ch);
					break;
				}
			}
			mp_PubWriter.second->add_change(change);
		}
	}
	return removeWriterProxy(W->getGuid());
}

bool EDPSimple::removeLocalReader(RTPSReader* R)
{
	const char* const METHOD_NAME = "removeLocalReader";
	logInfo(RTPS_EDP,R->getGuid().entityId,C_CYAN);
	if(mp_SubWriter.first!=nullptr)
	{
		InstanceHandle_t iH;
		iH = (R->getGuid());
		CacheChange_t* change = mp_SubWriter.first->new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
		if(change != nullptr)
		{
			boost::lock_guard<boost::recursive_mutex> guard(*mp_SubWriter.second->getMutex());
			for(auto ch = mp_SubWriter.second->changesBegin();ch!=mp_SubWriter.second->changesEnd();++ch)
			{
				if((*ch)->instanceHandle == change->instanceHandle)
				{
					mp_SubWriter.second->remove_change(*ch);
					break;
				}
			}
			mp_SubWriter.second->add_change(change);
		}
	}
	return removeReaderProxy(R->getGuid());
}



void EDPSimple::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "assignRemoteEndpoints";
	logInfo(RTPS_EDP,"New DPD received, adding remote endpoints to our SimpleEDP endpoints",C_CYAN);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
	//FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
	//auxendp = 1;
	boost::lock_guard<boost::recursive_mutex> guard(*pdata->mp_mutex);
	if(auxendp!=0 && mp_PubReader.first!=nullptr) //Exist Pub Writer and i have pub reader
	{
		logInfo(RTPS_EDP,"Adding SEDP Pub Writer to my Pub Reader",C_CYAN);
		RemoteWriterAttributes watt;
		watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		watt.guid.entityId = c_EntityId_SEDPPubWriter;
		watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		watt.endpoint.reliabilityKind = RELIABLE;
		watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		pdata->m_builtinWriters.push_back(watt);
		mp_PubReader.first->matched_writer_add(watt);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
	//FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
	//auxendp = 1;
	if(auxendp!=0 && mp_PubWriter.first!=nullptr) //Exist Pub Detector
	{
		logInfo(RTPS_EDP,"Adding SEDP Pub Reader to my Pub Writer",C_CYAN);
		RemoteReaderAttributes ratt;
		ratt.expectsInlineQos = false;
		ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		ratt.guid.entityId = c_EntityId_SEDPPubReader;
		ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		ratt.endpoint.reliabilityKind = RELIABLE;
		pdata->m_builtinReaders.push_back(ratt);
		mp_PubWriter.first->matched_reader_add(ratt);
	}
	auxendp = endp;
	auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
	//FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
	//auxendp = 1;
	if(auxendp!=0 && mp_SubReader.first!=nullptr) //Exist Pub Announcer
	{
		logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader",C_CYAN);
		RemoteWriterAttributes watt;
		watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		watt.guid.entityId = c_EntityId_SEDPSubWriter;
		watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		watt.endpoint.reliabilityKind = RELIABLE;
		watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		pdata->m_builtinWriters.push_back(watt);
		mp_SubReader.first->matched_writer_add(watt);
	}
	auxendp = endp;
	auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
	//FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
	//auxendp = 1;
	if(auxendp!=0 && mp_SubWriter.first!=nullptr) //Exist Pub Announcer
	{
		logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer",C_CYAN);
		RemoteReaderAttributes ratt;
		ratt.expectsInlineQos = false;
		ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		ratt.guid.entityId = c_EntityId_SEDPSubReader;
		ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		ratt.endpoint.reliabilityKind = RELIABLE;
		pdata->m_builtinReaders.push_back(ratt);
		mp_SubWriter.first->matched_reader_add(ratt);
	}
}


void EDPSimple::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "removeRemoteEndpoints";
	logInfo(RTPS_EDP,"For RTPSParticipant: "<<pdata->m_guid,C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guard(*pdata->mp_mutex);
	for (auto it = pdata->m_builtinReaders.begin(); it != pdata->m_builtinReaders.end();++it)
	{
		if(it->guid.entityId == c_EntityId_SEDPPubReader && this->mp_PubWriter.first !=nullptr)
		{
			mp_PubWriter.first->matched_reader_remove(*it);
			continue;
		}
		if(it->guid.entityId == c_EntityId_SEDPSubReader && this->mp_SubWriter.first !=nullptr)
		{
			mp_SubWriter.first->matched_reader_remove(*it);
			continue;
		}
	}
	
	for (auto it = pdata->m_builtinWriters.begin(); it != pdata->m_builtinWriters.end(); ++it)
	{
		if(it->guid.entityId == c_EntityId_SEDPPubWriter && this->mp_PubReader.first !=nullptr)
		{
			mp_PubReader.first->matched_writer_remove(*it);
			continue;
		}
		if(it->guid.entityId == c_EntityId_SEDPSubWriter && this->mp_SubReader.first !=nullptr)
		{
			mp_SubReader.first->matched_writer_remove(*it);
			continue;
		}
	}
}





}
} /* namespace rtps */
} /* namespace eprosima */
