/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimple.cpp
 *
 */

#include "fastrtps/rtps/builtin/discovery/participant/PDPSimple.h"
#include "fastrtps/rtps/builtin/discovery/participant/PDPSimpleListener.h"

#include "fastrtps/rtps/builtin/BuiltinProtocols.h"
#include "fastrtps/rtps/builtin/liveliness/WLP.h"

#include "fastrtps/rtps/builtin/data/ParticipantProxyData.h"
#include "fastrtps/rtps/builtin/data/ReaderProxyData.h"
#include "fastrtps/rtps/builtin/data/WriterProxyData.h"

#include "fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h"
#include "fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h"

#include "fastrtps/rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h"


#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"

#include "fastrtps/rtps/writer/StatelessWriter.h"
#include "fastrtps/rtps/reader/StatelessReader.h"

#include "fastrtps/rtps/history/WriterHistory.h"
#include "fastrtps/rtps/history/ReaderHistory.h"


//#include "fastrtps/utils/eClock.h"
#include "fastrtps/utils/TimeConversion.h"

#include "fastrtps/utils/RTPSLog.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "PDPSimple";

PDPSimple::PDPSimple(BuiltinProtocols* built):
	mp_builtin(built),
	mp_RTPSParticipant(nullptr),
	mp_SPDPWriter(nullptr),
	mp_SPDPReader(nullptr),
	mp_EDP(nullptr),
	m_hasChangedLocalPDP(true),
	mp_resendParticipantTimer(nullptr),
	mp_listener(nullptr),
	mp_topicDataType(nullptr),
	mp_SPDPWriterHistory(nullptr),
	mp_SPDPReaderHistory(nullptr)
{

}

PDPSimple::~PDPSimple() {
	// TODO Auto-generated destructor stub
}

bool PDPSimple::initPDP(RTPSParticipantImpl* part)
{
	const char* const METHOD_NAME = "initPDP";
	logInfo(RTPS_PDP,"Beginning",C_B_CYAN);
	mp_RTPSParticipant = part;
	m_discovery = mp_RTPSParticipant->getAttributes().builtin;

	if(!createSPDPEndpoints())
		return false;
	mp_builtin->updateMetatrafficLocators(this->mp_SPDPReader->getAttributes()->unicastLocatorList);
	boost::lock_guard<boost::recursive_mutex> guardR(*this->mp_SPDPReader->getMutex());
	boost::lock_guard<boost::recursive_mutex> guardW(*this->mp_SPDPWriter->getMutex());
	m_participantProxies.push_back(new ParticipantProxyData());
	m_participantProxies.front()->initializeData(mp_RTPSParticipant,this);

	//INIT EDP
	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EDP*)(new EDPStatic(this,mp_RTPSParticipant));
		mp_EDP->initEDP(m_discovery);
	}
	else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EDP*)(new EDPSimple(this,mp_RTPSParticipant));
		mp_EDP->initEDP(m_discovery);
	}
	else
	{
		logWarning(RTPS_PDP,"No EndpointDiscoveryProtocol defined");
		return false;
	}

	mp_resendParticipantTimer = new ResendParticipantProxyDataPeriod(this,TimeConv::Time_t2MilliSecondsDouble(m_discovery.leaseDuration_announcementperiod));

	return true;
}

void PDPSimple::stopParticipantAnnouncement()
{
	mp_resendParticipantTimer->stop_timer();
}

void PDPSimple::resetParticipantAnnouncement()
{
	mp_resendParticipantTimer->restart_timer();
}

void PDPSimple::announceParticipantState(bool new_change)
{
	const char* const METHOD_NAME = "announceParticipantState";
	logInfo(RTPS_PDP,"Announcing RTPSParticipant State (new change: "<< new_change <<")",C_CYAN);
	CacheChange_t* change = nullptr;
	if(new_change || m_hasChangedLocalPDP)
	{
		this->getLocalParticipantProxyData()->m_manualLivelinessCount++;
		if(mp_SPDPWriterHistory->getHistorySize() > 0)
			mp_SPDPWriterHistory->remove_min_change();
		change = mp_SPDPWriter->new_change(ALIVE);
		change->instanceHandle = getLocalParticipantProxyData()->m_key;
		if(getLocalParticipantProxyData()->toParameterList())
		{
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = getLocalParticipantProxyData()->m_QosList.allQos.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,getLocalParticipantProxyData()->m_QosList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_SPDPWriterHistory->add_change(change);
		}
		m_hasChangedLocalPDP = false;
	}
	else
	{
		mp_SPDPWriter->unsent_changes_reset();
	}

}

bool PDPSimple::lookupReaderProxyData(const GUID_t& reader, ReaderProxyData** rdata)
{
	for (auto pit = m_participantProxies.begin();
		pit != m_participantProxies.end();++pit)
	{
		for (auto rit = (*pit)->m_readers.begin();
			rit != (*pit)->m_readers.end();++rit)
		{
			if((*rit)->m_guid == reader)
			{
				*rdata = *rit;
				return true;
			}
		}
	}
	return false;
}

bool PDPSimple::lookupWriterProxyData(const GUID_t& writer, WriterProxyData** wdata)
{
	for (auto pit = m_participantProxies.begin();
		pit != m_participantProxies.end(); ++pit)
	{
		for (auto wit = (*pit)->m_writers.begin();
			wit != (*pit)->m_writers.end(); ++wit)
		{
			if((*wit)->m_guid == writer)
			{
				*wdata = *wit;
				return true;
			}
		}
	}
	return false;
}

bool PDPSimple::removeReaderProxyData(ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "removeReaderProxyData";
	logInfo(RTPS_PDP,rdata->m_guid,C_CYAN);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
				rit!=(*pit)->m_readers.end();++rit)
		{
			if((*rit)->m_guid == rdata->m_guid)
			{
				(*pit)->m_readers.erase(rit);
				delete(rdata);
				return true;
			}
		}
	}
	return false;
}

bool PDPSimple::removeWriterProxyData(WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "removeWriterProxyData";
	logInfo(RTPS_PDP,wdata->m_guid,C_CYAN);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
				wit!=(*pit)->m_writers.end();++wit)
		{
			if((*wit)->m_guid == wdata->m_guid)
			{
				(*pit)->m_writers.erase(wit);
				delete(wdata);
				return true;
			}
		}
	}
	return false;
}


bool PDPSimple::lookupParticipantProxyData(const GUID_t& pguid,ParticipantProxyData** pdata)
{
	const char* const METHOD_NAME = "lookupParticipantProxyData";
	logInfo(RTPS_PDP,pguid,C_CYAN);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		if((*pit)->m_guid == pguid)
		{
			*pdata = *pit;
				return true;
		}
	}
	return false;
}

bool PDPSimple::createSPDPEndpoints()
{
	const char* const METHOD_NAME = "createSPDPEndpoints";
	logInfo(RTPS_PDP,"Beginning",C_CYAN);
	//SPDP BUILTIN RTPSParticipant WRITER
	HistoryAttributes hatt;
	hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
	hatt.initialReservedCaches = 5;
	hatt.maximumReservedCaches = 10;
	mp_SPDPWriterHistory = new WriterHistory(hatt);
	WriterAttributes watt;
	watt.endpoint.endpointKind = WRITER;
	watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
	watt.endpoint.reliabilityKind = BEST_EFFORT;
	watt.endpoint.topicKind = WITH_KEY;
	RTPSWriter* wout;
	if(mp_RTPSParticipant->createWriter(&wout,watt,mp_SPDPWriterHistory,nullptr,c_EntityId_SPDPWriter,true))
	{
		mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
		RemoteReaderAttributes ratt;
		for(LocatorListIterator lit = mp_builtin->m_metatrafficMulticastLocatorList.begin();
				lit!=mp_builtin->m_metatrafficMulticastLocatorList.end();++lit)
			mp_SPDPWriter->add_locator(ratt,*lit);
		if(this->mp_builtin->m_useMandatory)
			mp_SPDPWriter->add_locator(ratt,mp_builtin->m_mandatoryMulticastLocator);
	}
	else
	{
		logError(RTPS_PDP,"SimplePDP Writer creation failed",C_CYAN);
		delete(mp_SPDPWriterHistory);
		mp_SPDPWriterHistory = nullptr;
		return false;
	}
	hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
	hatt.initialReservedCaches = 250;
	hatt.maximumReservedCaches = 5000;
	mp_SPDPReaderHistory = new ReaderHistory(hatt);
	ReaderAttributes ratt;
	ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
	ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
	ratt.endpoint.topicKind = WITH_KEY;
	ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
	ratt.endpoint.reliabilityKind = BEST_EFFORT;
	mp_listener = new PDPSimpleListener(this);
	RTPSReader* rout;
	if(mp_RTPSParticipant->createReader(&rout,ratt,mp_SPDPReaderHistory,mp_listener,c_EntityId_SPDPReader,true))
	{
		mp_SPDPReader = dynamic_cast<StatelessReader*>(rout);
	}
	else
	{
		logError(RTPS_PDP,"SimplePDP Reader creation failed",C_CYAN);
		delete(mp_SPDPReaderHistory);
		mp_SPDPReaderHistory = nullptr;
		delete(mp_listener);
		mp_listener = nullptr;
		return false;
	}

	logInfo(RTPS_PDP,"SPDP Endpoints creation finished",C_CYAN);
	return true;
}

bool PDPSimple::addReaderProxyData(ReaderProxyData* rdata,bool copydata,
		ReaderProxyData** returnReaderProxyData,ParticipantProxyData** pdata)
{
	const char* const METHOD_NAME = "addReaderProxyData";
	logInfo(RTPS_PDP,rdata->m_guid,C_CYAN);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		if((*pit)->m_guid.guidPrefix == rdata->m_guid.guidPrefix)
		{
			//CHECK THAT IT IS NOT ALREADY THERE:
			for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
					rit!=(*pit)->m_readers.end();++rit)
			{
				if((*rit)->m_guid.entityId == rdata->m_guid.entityId)
				{
					if(copydata)
					{
						*returnReaderProxyData = *rit;
						*pdata = *pit;
					}
					return false;
				}
			}
			if(copydata)
			{
				ReaderProxyData* newRPD = new ReaderProxyData();
				newRPD->copy(rdata);
				(*pit)->m_readers.push_back(newRPD);
				*returnReaderProxyData = newRPD;
				*pdata = *pit;
			}
			else
			{
				(*pit)->m_readers.push_back(rdata);
			}
			return true;
		}
	}
	return false;
}

bool PDPSimple::addWriterProxyData(WriterProxyData* wdata,bool copydata,
		WriterProxyData** returnWriterProxyData,ParticipantProxyData** pdata)
{
	const char* const METHOD_NAME = "addWriterProxyData";
	logInfo(RTPS_PDP,wdata->m_guid,C_CYAN);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		if((*pit)->m_guid.guidPrefix == wdata->m_guid.guidPrefix)
		{
			//CHECK THAT IT IS NOT ALREADY THERE:
			for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
					wit!=(*pit)->m_writers.end();++wit)
			{
				if((*wit)->m_guid.entityId == wdata->m_guid.entityId)
				{
					if(copydata)
					{
						*returnWriterProxyData = *wit;
						*pdata = *pit;
					}
					return false;
				}
			}
			if(copydata)
			{
				WriterProxyData* newWPD = new WriterProxyData();
				newWPD->copy(wdata);
				(*pit)->m_writers.push_back(newWPD);
				*returnWriterProxyData = newWPD;
				*pdata = *pit;
			}
			else
			{
				(*pit)->m_writers.push_back(wdata);
			}
			return true;
			return true;
		}
	}
	return false;
}

void PDPSimple::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "assignRemoteEndpoints";
	logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid.guidPrefix,C_CYAN);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
	if(auxendp!=0)
	{
		RemoteWriterAttributes watt;
		watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		watt.guid.entityId = c_EntityId_SPDPWriter;
		watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		watt.endpoint.reliabilityKind = BEST_EFFORT;
		watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		pdata->m_builtinWriters.push_back(watt);
		mp_SPDPReader->matched_writer_add(watt);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
	if(auxendp!=0)
	{
		RemoteReaderAttributes ratt;
		ratt.expectsInlineQos = false;
		ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		ratt.guid.entityId = c_EntityId_SPDPReader;
		ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		ratt.endpoint.reliabilityKind = BEST_EFFORT;
		ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		pdata->m_builtinReaders.push_back(ratt);
		mp_SPDPWriter->matched_reader_add(ratt);
	}

	//Inform EDP of new RTPSParticipant data:
	if(mp_EDP!=nullptr)
		mp_EDP->assignRemoteEndpoints(pdata);
	if(mp_builtin->mp_WLP !=nullptr)
		mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
}


void PDPSimple::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "removeRemoteEndpoints";
	logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid,C_CYAN);
	for(auto it = pdata->m_builtinReaders.begin();
			it!=pdata->m_builtinReaders.end();++it)
	{
		if((*it).guid.entityId == c_EntityId_SPDPReader && this->mp_SPDPWriter !=nullptr)
			mp_SPDPWriter->matched_reader_remove(*it);
	}
	for(auto it = pdata->m_builtinWriters.begin();
			it!=pdata->m_builtinWriters.end();++it)
	{
		if((*it).guid.entityId == c_EntityId_SPDPWriter && this->mp_SPDPReader !=nullptr)
			mp_SPDPReader->matched_writer_remove(*it);
	}
}

bool PDPSimple::removeRemoteParticipant(GUID_t& partGUID)
{
	const char* const METHOD_NAME = "removeRemoteParticipant";
	logInfo(RTPS_PDP,partGUID,C_CYAN );
	boost::lock_guard<boost::recursive_mutex> guardW(*this->mp_SPDPWriter->getMutex());
	boost::lock_guard<boost::recursive_mutex> guardR(*this->mp_SPDPReader->getMutex());
	ParticipantProxyData* pdata=nullptr;
	//Remove it from our vector or RTPSParticipantProxies:
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		if((*pit)->m_guid == partGUID)
		{
			pdata = *pit;
			m_participantProxies.erase(pit);
			break;
		}
	}
	if(pdata !=nullptr)
	{
		for(std::vector<ReaderProxyData*>::iterator rit = pdata->m_readers.begin();
				rit!= pdata->m_readers.end();++rit)
		{
			mp_EDP->unpairReaderProxy(*rit);
		}
		for(std::vector<WriterProxyData*>::iterator wit = pdata->m_writers.begin();
				wit!=pdata->m_writers.end();++wit)
		{
			mp_EDP->unpairWriterProxy(*wit);
		}
		this->mp_builtin->mp_WLP->removeRemoteEndpoints(pdata);
		this->mp_EDP->removeRemoteEndpoints(pdata);
		this->removeRemoteEndpoints(pdata);
		for(std::vector<CacheChange_t*>::iterator it=this->mp_SPDPReaderHistory->changesBegin();
				it!=this->mp_SPDPReaderHistory->changesEnd();++it)
		{
			if((*it)->instanceHandle == pdata->m_key)
			{
				this->mp_SPDPReaderHistory->remove_change(*it);
				break;
			}
		}
		delete(pdata);
		return true;
	}

	return false;

}


void PDPSimple::assertRemoteParticipantLiveliness(GuidPrefix_t& guidP)
{
	const char* const METHOD_NAME = "assertRemoteParticipantLiveliness";
	for(std::vector<ParticipantProxyData*>::iterator it = this->m_participantProxies.begin();
			it!=this->m_participantProxies.end();++it)
	{
		if((*it)->m_guid.guidPrefix == guidP)
		{
			logInfo(RTPS_LIVELINESS,"RTPSParticipant "<< (*it)->m_guid << " is Alive",C_MAGENTA);
			(*it)->isAlive = true;
			break;
		}
	}
}

void PDPSimple::assertLocalWritersLiveliness(LivelinessQosPolicyKind kind)
{
	const char* const METHOD_NAME = "assertLocalWritersLiveliness";
	logInfo(RTPS_LIVELINESS,"of type " << (kind==AUTOMATIC_LIVELINESS_QOS?"AUTOMATIC":"")
			<<(kind==MANUAL_BY_PARTICIPANT_LIVELINESS_QOS?"MANUAL_BY_PARTICIPANT":""),C_MAGENTA);
	for(std::vector<WriterProxyData*>::iterator wit = this->m_participantProxies.front()->m_writers.begin();
			wit!=this->m_participantProxies.front()->m_writers.end();++wit)
	{
		if((*wit)->m_qos.m_liveliness.kind == kind)
		{
			logInfo(RTPS_LIVELINESS,"Local Writer "<< (*wit)->m_guid.entityId << " marked as ALIVE",C_MAGENTA);
			(*wit)->m_isAlive = true;
		}
	}
}

void PDPSimple::assertRemoteWritersLiveliness(GuidPrefix_t& guidP,LivelinessQosPolicyKind kind)
{
	for(std::vector<ParticipantProxyData*>::iterator pit=this->m_participantProxies.begin();
			pit!=this->m_participantProxies.end();++pit)
	{
		if((*pit)->m_guid.guidPrefix == guidP)
		{
			for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
					wit != (*pit)->m_writers.end();++wit)
			{
				if((*wit)->m_qos.m_liveliness.kind == kind)
					(*wit)->m_isAlive;
			}
			break;
		}
	}
}

bool PDPSimple::newRemoteEndpointStaticallyDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind)
{
	ParticipantProxyData* pdata;
	if(lookupParticipantProxyData(pguid, &pdata))
	{
		if(kind == WRITER)
			dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteWriter(pdata,userDefinedId);
		else
			dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteReader(pdata,userDefinedId);
	}
	return false;
}

}
} /* namespace rtps */
} /* namespace eprosima */


