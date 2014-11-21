/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimple.cpp
 *
 */

#include "eprosimartps/rtps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/rtps/builtin/BuiltinProtocols.h"
#include "eprosimartps/rtps/builtin/liveliness/WLP.h"

//#include "eprosimartps/pubsub/RTPSDomain.h"


#include "eprosimartps/RTPSParticipant.h"
#include "eprosimartps/RTPSParticipantProxyData.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/builtin/discovery/endpoint/EDPSimple.h"
#include "eprosimartps/builtin/discovery/endpoint/EDPStatic.h"
#include "eprosimartps/builtin/discovery/RTPSParticipant/timedevent/ResendRTPSParticipantProxyDataPeriod.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
#include "eprosimartps/utils/TimeConversion.h"

using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "PDPSimple";

PDPSimple::PDPSimple(BuiltinProtocols* built):
	mp_builtin(built),
	mp_RTPSParticipant(nullptr),
	mp_SPDPWriter(nullptr),
	mp_SPDPReader(nullptr),
	mp_EDP(nullptr),
	m_hasChangedLocalPDP(true),
	mp_resendRTPSParticipantTimer(nullptr),
	#pragma warning(disable: 4355)
	m_listener(this)
{

}

PDPSimple::~PDPSimple() {
	// TODO Auto-generated destructor stub
}

bool PDPSimple::initPDP(RTPSParticipantImpl* part)
{
	const char* const METHOD_NAME = "initPDP";
	logInfo(RTPS_PDP,"Beginning",EPRO_B_CYAN);
	mp_RTPSParticipant = part;
	m_discovery = mp_RTPSParticipant->getAttributes().builtin;

	if(!createSPDPEndpoints())
		return false;
	mp_builtin->updateMetatrafficLocators(this->mp_SPDPReader->m_att.unicastLocatorList);
	boost::lock_guard<boost::recursive_mutex> guardR(*this->mp_SPDPReader->getMutex());
	boost::lock_guard<boost::recursive_mutex> guardW(*this->mp_SPDPWriter->getMutex());
	m_RTPSParticipantProxies.push_back(new RTPSParticipantProxyData());
	m_RTPSParticipantProxies.front()->initializeData(mp_RTPSParticipant,this);

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

	mp_resendRTPSParticipantTimer = new ResendRTPSParticipantProxyDataPeriod(this,mp_RTPSParticipant->getEventResource(),
			boost::posix_time::milliseconds(TimeConv::Time_t2MilliSecondsInt64(m_discovery.leaseDuration_announcementperiod)));

	return true;
}

void PDPSimple::stopRTPSParticipantAnnouncement()
{
	mp_resendRTPSParticipantTimer->stop_timer();
}

void PDPSimple::resetRTPSParticipantAnnouncement()
{
	mp_resendRTPSParticipantTimer->restart_timer();
}

void PDPSimple::announceRTPSParticipantState(bool new_change)
{
	const char* const METHOD_NAME = "announceRTPSParticipantState";
	logInfo(RTPS_PDP,"Announcing RTPSParticipant State (new change: "<< new_change <<")",EPRO_CYAN);
	CacheChange_t* change = nullptr;
	if(new_change || m_hasChangedLocalPDP)
	{
		m_RTPSParticipantProxies.front()->m_manualLivelinessCount++;
		if(mp_SPDPWriter->getHistoryCacheSize() > 0)
			mp_SPDPWriter->removeMinSeqCacheChange();
		mp_SPDPWriter->new_change(ALIVE,nullptr,&change);
		change->instanceHandle = m_RTPSParticipantProxies.front()->m_key;
		if(m_RTPSParticipantProxies.front()->toParameterList())
		{
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = m_RTPSParticipantProxies.front()->m_QosList.allQos.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,m_RTPSParticipantProxies.front()->m_QosList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_SPDPWriter->add_change(change);
		}
		m_hasChangedLocalPDP = false;
		mp_SPDPWriter->unsent_change_add(change);
	}
	else
	{
		mp_SPDPWriter->unsent_changes_reset();
	}

}

bool PDPSimple::lookupReaderProxyData(const GUID_t& reader, ReaderProxyData** rdata)
{
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
	{
		for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
				rit!=(*pit)->m_readers.end();++rit)
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
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
	{
		for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
				wit!=(*pit)->m_writers.end();++wit)
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
	logInfo(RTPS_PDP,rdata->m_guid,EPRO_CYAN);
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
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
	logInfo(RTPS_PDP,wdata->m_guid,EPRO_CYAN);
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
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


bool PDPSimple::lookupRTPSParticipantProxyData(const GUID_t& pguid,RTPSParticipantProxyData** pdata)
{
	const char* const METHOD_NAME = "lookupRTPSParticipantProxyData";
	logInfo(RTPS_PDP,pguid,EPRO_CYAN);
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
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
	logInfo(RTPS_PDP,"Beginning",EPRO_CYAN);
	//SPDP BUILTIN RTPSParticipant WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	//Wparam.historyMaxSize = 1;
	//Locators where it is going to listen
	Wparam.topic.topicName = "DCPSRTPSParticipant";
	Wparam.topic.topicDataType = "DiscoveredRTPSParticipantData";
	Wparam.topic.topicKind = WITH_KEY;
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;
	Wparam.topic.resourceLimitsQos.max_instances = 1;
	Wparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
	Wparam.topic.resourceLimitsQos.max_samples = 2;
	Wparam.topic.resourceLimitsQos.allocated_samples = 2;
	Wparam.payloadMaxSize = 10000;
	Wparam.userDefinedId = -99;
	Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	RTPSWriter* wout;
	if(mp_RTPSParticipant->createWriter(&wout,Wparam,DISCOVERY_RTPSParticipant_DATA_MAX_SIZE,true,STATELESS,nullptr,nullptr,c_EntityId_SPDPWriter))
	{
		mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
		for(LocatorListIterator lit = mp_builtin->m_metatrafficMulticastLocatorList.begin();
				lit!=mp_builtin->m_metatrafficMulticastLocatorList.end();++lit)
			mp_SPDPWriter->reader_locator_add(*lit,false);
		if(this->mp_builtin->m_useMandatory)
			mp_SPDPWriter->reader_locator_add(mp_builtin->m_mandatoryMulticastLocator,false);
	}
	else
	{
		logError(RTPS_PDP,"SimplePDP Writer creation failed",EPRO_CYAN);
		return false;
	}
	//SPDP BUILTIN RTPSParticipant READER
	SubscriberAttributes Rparam;
	//	Rparam.historyMaxSize = 100;
	//  Locators where it is going to listen
	Rparam.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
	Rparam.topic.topicKind = WITH_KEY;
	Rparam.topic.topicName = "DCPSRTPSParticipant";
	Rparam.topic.topicDataType = "DiscoveredRTPSParticipantData";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 1;
	Rparam.topic.resourceLimitsQos.max_instances = 1000;
	Rparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
	Rparam.topic.resourceLimitsQos.max_samples = 1000;
	Rparam.topic.resourceLimitsQos.allocated_samples = 500;
	Rparam.payloadMaxSize = 10000;
	Rparam.userDefinedId = -99;
	Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	RTPSReader* rout;
	if(mp_RTPSParticipant->createReader(&rout,Rparam,DISCOVERY_RTPSParticipant_DATA_MAX_SIZE,
			true,STATELESS,(TopicDataType*)&m_topicDataType,(SubscriberListener*)&this->m_listener,c_EntityId_SPDPReader))
	{
		mp_SPDPReader = dynamic_cast<StatelessReader*>(rout);
		//mp_SPDPReader->setListener();
	}
	else
	{
		logError(RTPS_PDP,"SimplePDP Reader creation failed",EPRO_CYAN);
		return false;
	}

	logInfo(RTPS_PDP,"SPDP Endpoints creation finished",EPRO_CYAN);
	return true;
}

bool PDPSimple::addReaderProxyData(ReaderProxyData* rdata,bool copydata,
		ReaderProxyData** returnReaderProxyData,RTPSParticipantProxyData** pdata)
{
	const char* const METHOD_NAME = "addReaderProxyData";
	logInfo(RTPS_PDP,rdata->m_guid,EPRO_CYAN);
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
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
		WriterProxyData** returnWriterProxyData,RTPSParticipantProxyData** pdata)
{
	const char* const METHOD_NAME = "addWriterProxyData";
	logInfo(RTPS_PDP,wdata->m_guid,EPRO_CYAN);
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
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

void PDPSimple::assignRemoteEndpoints(RTPSParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "assignRemoteEndpoints";
	logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid.guidPrefix,EPRO_CYAN);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_RTPSParticipant_ANNOUNCER;
	if(auxendp!=0)
	{
		WriterProxyData* wp = new WriterProxyData();
		wp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		wp->m_guid.entityId = c_EntityId_SPDPWriter;
		wp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		wp->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
		wp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		pdata->m_builtinWriters.push_back(wp);
		mp_SPDPReader->matched_writer_add(wp);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_RTPSParticipant_DETECTOR;
	if(auxendp!=0)
	{
		ReaderProxyData* rp = new ReaderProxyData();
		rp->m_expectsInlineQos = false;
		rp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		rp->m_guid.entityId = c_EntityId_SPDPReader;
		rp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		rp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		rp->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
		pdata->m_builtinReaders.push_back(rp);
		mp_SPDPWriter->matched_reader_add(rp);
	}

	//Inform EDP of new RTPSParticipant data:
	if(mp_EDP!=nullptr)
		mp_EDP->assignRemoteEndpoints(pdata);
	if(mp_builtin->mp_WLP !=nullptr)
		mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
}


void PDPSimple::removeRemoteEndpoints(RTPSParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "removeRemoteEndpoints";
	logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid,EPRO_CYAN);
	for(std::vector<ReaderProxyData*>::iterator it = pdata->m_builtinReaders.begin();
			it!=pdata->m_builtinReaders.end();++it)
	{
		if((*it)->m_guid.entityId == c_EntityId_SPDPReader && this->mp_SPDPWriter !=nullptr)
			mp_SPDPWriter->matched_reader_remove(*it);
	}
	for(std::vector<WriterProxyData*>::iterator it = pdata->m_builtinWriters.begin();
			it!=pdata->m_builtinWriters.end();++it)
	{
		if((*it)->m_guid.entityId == c_EntityId_SPDPWriter && this->mp_SPDPReader !=nullptr)
			mp_SPDPReader->matched_writer_remove(*it);
	}
}

bool PDPSimple::removeRemoteRTPSParticipant(GUID_t& partGUID)
{
	const char* const METHOD_NAME = "removeRemoteRTPSParticipant";
	logInfo(RTPS_PDP,partGUID,EPRO_CYAN );
	boost::lock_guard<boost::recursive_mutex> guardW(*this->mp_SPDPWriter->getMutex());
	boost::lock_guard<boost::recursive_mutex> guardR(*this->mp_SPDPReader->getMutex());
	RTPSParticipantProxyData* pdata=nullptr;
	//Remove it from our vector or RTPSParticipantProxies:
	for(std::vector<RTPSParticipantProxyData*>::iterator pit = m_RTPSParticipantProxies.begin();
			pit!=m_RTPSParticipantProxies.end();++pit)
	{
		if((*pit)->m_guid == partGUID)
		{
			pdata = *pit;
			m_RTPSParticipantProxies.erase(pit);
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
		for(std::vector<CacheChange_t*>::iterator it=this->mp_SPDPReader->readerHistoryCacheBegin();
				it!=this->mp_SPDPReader->readerHistoryCacheEnd();++it)
		{
			if((*it)->instanceHandle == pdata->m_key)
			{
				this->mp_SPDPReader->change_removed_by_history(*it);
				break;
			}
		}
		delete(pdata);
		return true;
	}

	return false;

}


void PDPSimple::assertRemoteRTPSParticipantLiveliness(GuidPrefix_t& guidP)
{
	const char* const METHOD_NAME = "assertRemoteRTPSParticipantLiveliness";
	for(std::vector<RTPSParticipantProxyData*>::iterator it = this->m_RTPSParticipantProxies.begin();
			it!=this->m_RTPSParticipantProxies.end();++it)
	{
		if((*it)->m_guid.guidPrefix == guidP)
		{
			logInfo(RTPS_LIVELINESS,"RTPSParticipant "<< (*it)->m_guid << " is Alive",EPRO_MAGENTA);
			(*it)->isAlive = true;
			break;
		}
	}
}

void PDPSimple::assertLocalWritersLiveliness(LivelinessQosPolicyKind kind)
{
	const char* const METHOD_NAME = "assertLocalWritersLiveliness";
	logInfo(RTPS_LIVELINESS,"of type " << (kind==AUTOMATIC_LIVELINESS_QOS?"AUTOMATIC":"")
			<<(kind==MANUAL_BY_RTPSParticipant_LIVELINESS_QOS?"MANUAL_BY_RTPSParticipant":""),EPRO_MAGENTA);
	for(std::vector<WriterProxyData*>::iterator wit = this->m_RTPSParticipantProxies.front()->m_writers.begin();
			wit!=this->m_RTPSParticipantProxies.front()->m_writers.end();++wit)
	{
		if((*wit)->m_qos.m_liveliness.kind == kind)
		{
			logInfo(RTPS_LIVELINESS,"Local Writer "<< (*wit)->m_guid.entityId << " marked as ALIVE",EPRO_MAGENTA);
			(*wit)->m_isAlive = true;
		}
	}
}

void PDPSimple::assertRemoteWritersLiveliness(GuidPrefix_t& guidP,LivelinessQosPolicyKind kind)
{
	for(std::vector<RTPSParticipantProxyData*>::iterator pit=this->m_RTPSParticipantProxies.begin();
			pit!=this->m_RTPSParticipantProxies.end();++pit)
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
	RTPSParticipantProxyData* pdata;
	if(lookupRTPSParticipantProxyData(pguid, &pdata))
	{
		if(kind == WRITER)
			dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteWriter(pdata,userDefinedId);
		else
			dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteReader(pdata,userDefinedId);
	}
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */


