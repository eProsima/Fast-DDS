/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimple.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimpleListener.h>

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h>


#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>


#include <fastrtps/utils/TimeConversion.h>

#include <fastrtps/utils/RTPSLog.h>

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
					mp_SPDPWriterHistory(nullptr),
					mp_SPDPReaderHistory(nullptr),
					mp_mutex(new boost::recursive_mutex())
{

}

PDPSimple::~PDPSimple()
{
	mp_mutex->lock();
	if(mp_EDP!=nullptr)
		delete(mp_EDP);
	delete(mp_SPDPWriter);
	delete(mp_SPDPReader);
	delete(mp_SPDPWriterHistory);
	delete(mp_SPDPReaderHistory);
	if(mp_resendParticipantTimer!=nullptr)
		delete(mp_resendParticipantTimer);
	delete(mp_listener);
	for(auto it = this->m_participantProxies.begin();
			it!=this->m_participantProxies.end();++it)
	{
		delete(*it);
	}
	mp_mutex->unlock();
	delete(mp_mutex);
}

bool PDPSimple::initPDP(RTPSParticipantImpl* part)
{
	const char* const METHOD_NAME = "initPDP";
	logInfo(RTPS_PDP,"Beginning",C_B_CYAN);
	mp_RTPSParticipant = part;
	m_discovery = mp_RTPSParticipant->getAttributes().builtin;
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
	//CREATE ENDPOINTS
	if(!createSPDPEndpoints())
		return false;
	//UPDATE METATRAFFIC.
	mp_builtin->updateMetatrafficLocators(this->mp_SPDPReader->getAttributes()->unicastLocatorList);
	//boost::lock_guard<boost::recursive_mutex> guardR(*this->mp_SPDPReader->getMutex());
	//boost::lock_guard<boost::recursive_mutex> guardW(*this->mp_SPDPWriter->getMutex());
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

    if(!mp_RTPSParticipant->enableReader(mp_SPDPReader, true))
        return false;

	mp_resendParticipantTimer = new ResendParticipantProxyDataPeriod(this,TimeConv::Time_t2MilliSecondsDouble(m_discovery.leaseDuration_announcementperiod));

	return true;
}

void PDPSimple::stopParticipantAnnouncement()
{
	mp_resendParticipantTimer->cancel_timer();
}

void PDPSimple::resetParticipantAnnouncement()
{
	mp_resendParticipantTimer->restart_timer();
}

void PDPSimple::announceParticipantState(bool new_change, bool dispose)
{
	const char* const METHOD_NAME = "announceParticipantState";
	logInfo(RTPS_PDP,"Announcing RTPSParticipant State (new change: "<< new_change <<")",C_CYAN);
	CacheChange_t* change = nullptr;

	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);

    if(!dispose)
    {
        if(new_change || m_hasChangedLocalPDP)
        {
            this->getLocalParticipantProxyData()->m_manualLivelinessCount++;
            if(mp_SPDPWriterHistory->getHistorySize() > 0)
                mp_SPDPWriterHistory->remove_min_change();
            change = mp_SPDPWriter->new_change(ALIVE,getLocalParticipantProxyData()->m_key);
            if(getLocalParticipantProxyData()->toParameterList())
            {
#if EPROSIMA_BIG_ENDIAN
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
#else
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
#endif
                change->serializedPayload.length = (uint16_t)getLocalParticipantProxyData()->m_QosList.allQos.m_cdrmsg.length;
                //TODO Optimizacion, intentar quitar la copia.
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
    else
    {
        if(mp_SPDPWriterHistory->getHistorySize() > 0)
            mp_SPDPWriterHistory->remove_min_change();
        change = mp_SPDPWriter->new_change(NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key);
        if(getLocalParticipantProxyData()->toParameterList())
        {
#if EPROSIMA_BIG_ENDIAN
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
#endif
            change->serializedPayload.length = (uint16_t)getLocalParticipantProxyData()->m_QosList.allQos.m_cdrmsg.length;
            //TODO Optimizacion, intentar quitar la copia.
            memcpy(change->serializedPayload.data,getLocalParticipantProxyData()->m_QosList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
            mp_SPDPWriterHistory->add_change(change);
        }
    }

}

bool PDPSimple::lookupReaderProxyData(const GUID_t& reader, ReaderProxyData** rdata, ParticipantProxyData** pdata)
{
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
	for (auto pit = m_participantProxies.begin();
			pit != m_participantProxies.end();++pit)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
		for (auto rit = (*pit)->m_readers.begin();
				rit != (*pit)->m_readers.end();++rit)
		{
			if((*rit)->m_guid == reader)
			{
				*rdata = *rit;
                *pdata = *pit;
				return true;
			}
		}
	}
	return false;
}

bool PDPSimple::lookupWriterProxyData(const GUID_t& writer, WriterProxyData** wdata, ParticipantProxyData** pdata)
{
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
	for (auto pit = m_participantProxies.begin();
			pit != m_participantProxies.end(); ++pit)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
		for (auto wit = (*pit)->m_writers.begin();
				wit != (*pit)->m_writers.end(); ++wit)
		{
			if((*wit)->m_guid == writer)
			{
				*wdata = *wit;
                *pdata = *pit;
				return true;
			}
		}
	}
	return false;
}

bool PDPSimple::removeReaderProxyData(ParticipantProxyData* pdata, ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "removeReaderProxyData";
	logInfo(RTPS_PDP,rdata->m_guid,C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
    boost::lock_guard<boost::recursive_mutex> guard(*pdata->mp_mutex);
    for(std::vector<ReaderProxyData*>::iterator rit = pdata->m_readers.begin();
        rit != pdata->m_readers.end(); ++rit)
	{
		if((*rit)->m_guid == rdata->m_guid)
		{
            pdata->m_readers.erase(rit);
			delete(rdata);
			return true;
		}
	}
	return false;
}

bool PDPSimple::removeWriterProxyData(ParticipantProxyData* pdata, WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "removeWriterProxyData";
	logInfo(RTPS_PDP,wdata->m_guid,C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
    boost::lock_guard<boost::recursive_mutex> guard(*pdata->mp_mutex);
    for(std::vector<WriterProxyData*>::iterator wit = pdata->m_writers.begin();
        wit != pdata->m_writers.end(); ++wit)
	{
		if((*wit)->m_guid == wdata->m_guid)
		{
            pdata->m_writers.erase(wit);
			delete(wdata);
			return true;
		}
	}
	return false;
}


bool PDPSimple::lookupParticipantProxyData(const GUID_t& pguid,ParticipantProxyData** pdata)
{
	const char* const METHOD_NAME = "lookupParticipantProxyData";
	logInfo(RTPS_PDP,pguid,C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
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
	hatt.initialReservedCaches = 20;
	hatt.maximumReservedCaches = 100;
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
	if(mp_RTPSParticipant->createReader(&rout,ratt,mp_SPDPReaderHistory,mp_listener,c_EntityId_SPDPReader,true, false))
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
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
		if((*pit)->m_guid.guidPrefix == rdata->m_guid.guidPrefix)
		{
			//CHECK THAT IT IS NOT ALREADY THERE:
			for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
					rit!=(*pit)->m_readers.end();++rit)
			{
				if((*rit)->m_guid.entityId == rdata->m_guid.entityId)
				{
					if(copydata)
						*returnReaderProxyData = *rit;
                    if(pdata != nullptr)
						*pdata = *pit;
					return false;
				}
			}
			if(copydata)
			{
				ReaderProxyData* newRPD = new ReaderProxyData();
				newRPD->copy(rdata);
				(*pit)->m_readers.push_back(newRPD);
				*returnReaderProxyData = newRPD;
                if(pdata != nullptr)
				    *pdata = *pit;
			}
			else
			{
				(*pit)->m_readers.push_back(rdata);
                if(pdata != nullptr)
                    *pdata = *pit;
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
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
		if((*pit)->m_guid.guidPrefix == wdata->m_guid.guidPrefix)
		{
			//CHECK THAT IT IS NOT ALREADY THERE:
			for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
					wit!=(*pit)->m_writers.end();++wit)
			{
				if((*wit)->m_guid.entityId == wdata->m_guid.entityId)
				{
					if(copydata)
						*returnWriterProxyData = *wit;
                    if(pdata != nullptr)
						*pdata = *pit;
					return false;
				}
			}
			if(copydata)
			{
				WriterProxyData* newWPD = new WriterProxyData();
				newWPD->copy(wdata);
				(*pit)->m_writers.push_back(newWPD);
				*returnWriterProxyData = newWPD;
                if(pdata != nullptr)
				    *pdata = *pit;
			}
			else
			{
				(*pit)->m_writers.push_back(wdata);
                if(pdata != nullptr)
                    *pdata = *pit;
			}
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
    // TODO Review because the mutex is already take in PDPSimpleListener.
	boost::lock_guard<boost::recursive_mutex> guard(*pdata->mp_mutex);
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
	boost::lock_guard<boost::recursive_mutex> guard(*pdata->mp_mutex);
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
	boost::unique_lock<boost::recursive_mutex> guardW(*this->mp_SPDPWriter->getMutex());
	boost::unique_lock<boost::recursive_mutex> guardR(*this->mp_SPDPReader->getMutex());
	ParticipantProxyData* pdata = nullptr;
	//Remove it from our vector or RTPSParticipantProxies:
	boost::unique_lock<boost::recursive_mutex> guardPDP(*this->mp_mutex);
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
		if((*pit)->m_guid == partGUID)
		{
			pdata = *pit;
			m_participantProxies.erase(pit);
			break;
		}
	}

	if(pdata !=nullptr)
	{
		pdata->mp_mutex->lock();
		if(mp_EDP!=nullptr)
		{
			for(std::vector<ReaderProxyData*>::iterator rit = pdata->m_readers.begin();
					rit!= pdata->m_readers.end();++rit)
			{
				mp_EDP->unpairReaderProxy(pdata, *rit);
			}
			for(std::vector<WriterProxyData*>::iterator wit = pdata->m_writers.begin();
					wit!=pdata->m_writers.end();++wit)
			{
				mp_EDP->unpairWriterProxy(pdata, *wit);
			}
		}
		if(mp_builtin->mp_WLP != nullptr)
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
		pdata->mp_mutex->unlock();

        guardPDP.unlock();
        guardW.unlock();
        guardR.unlock();

		delete(pdata);
		return true;
	}

	return false;
}


void PDPSimple::assertRemoteParticipantLiveliness(const GuidPrefix_t& guidP)
{
	const char* const METHOD_NAME = "assertRemoteParticipantLiveliness";
	boost::lock_guard<boost::recursive_mutex> guardPDP(*this->mp_mutex);
	for(std::vector<ParticipantProxyData*>::iterator it = this->m_participantProxies.begin();
			it!=this->m_participantProxies.end();++it)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*it)->mp_mutex);
		if((*it)->m_guid.guidPrefix == guidP)
		{
			logInfo(RTPS_LIVELINESS,"RTPSParticipant "<< (*it)->m_guid << " is Alive",C_MAGENTA);
            // TODO Ricardo: Study if isAlive attribute is necessary.
			(*it)->isAlive = true;
            if((*it)->mp_leaseDurationTimer != nullptr)
            {
                (*it)->mp_leaseDurationTimer->cancel_timer();
                (*it)->mp_leaseDurationTimer->restart_timer();
            }
			break;
		}
	}
}

void PDPSimple::assertLocalWritersLiveliness(LivelinessQosPolicyKind kind)
{
	const char* const METHOD_NAME = "assertLocalWritersLiveliness";
	logInfo(RTPS_LIVELINESS,"of type " << (kind==AUTOMATIC_LIVELINESS_QOS?"AUTOMATIC":"")
			<<(kind==MANUAL_BY_PARTICIPANT_LIVELINESS_QOS?"MANUAL_BY_PARTICIPANT":""),C_MAGENTA);
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_mutex);
	boost::lock_guard<boost::recursive_mutex> guard2(*this->m_participantProxies.front()->mp_mutex);
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
	boost::lock_guard<boost::recursive_mutex> pguard(*this->mp_mutex);
	const char* const METHOD_NAME = "assertRemoteWritersLiveliness";
	logInfo(RTPS_LIVELINESS,"of type " << (kind==AUTOMATIC_LIVELINESS_QOS?"AUTOMATIC":"")
				<<(kind==MANUAL_BY_PARTICIPANT_LIVELINESS_QOS?"MANUAL_BY_PARTICIPANT":""),C_MAGENTA);
	for(std::vector<ParticipantProxyData*>::iterator pit=this->m_participantProxies.begin();
			pit!=this->m_participantProxies.end();++pit)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
		if((*pit)->m_guid.guidPrefix == guidP)
		{
			for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
					wit != (*pit)->m_writers.end();++wit)
			{
				if((*wit)->m_qos.m_liveliness.kind == kind)
				{
					(*wit)->m_isAlive = true;
					boost::lock_guard<boost::recursive_mutex> guardP(*mp_RTPSParticipant->getParticipantMutex());
					for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
							rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
					{
						if((*rit)->getAttributes()->reliabilityKind == RELIABLE)
						{
							StatefulReader* sfr = (StatefulReader*)(*rit);
							WriterProxy* WP;
							if(sfr->matched_writer_lookup((*wit)->m_guid,&WP))
							{
								WP->assertLiveliness();
								continue;
							}
						}
					}
					}
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


