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

#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/builtin/BuiltinProtocols.h"
#include "eprosimartps/builtin/liveliness/WLP.h"

#include "eprosimartps/dds/DomainParticipant.h"


#include "eprosimartps/Participant.h"
#include "eprosimartps/ParticipantProxyData.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/builtin/discovery/endpoint/EDPSimple.h"
#include "eprosimartps/builtin/discovery/endpoint/EDPStatic.h"
#include "eprosimartps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
#include "eprosimartps/utils/TimeConversion.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

PDPSimple::PDPSimple(BuiltinProtocols* built):
																		mp_builtin(built),
																		mp_participant(NULL),
																		mp_SPDPWriter(NULL),
																		mp_SPDPReader(NULL),
																		mp_EDP(NULL),
																		m_hasChangedLocalPDP(true),
																		mp_resendParticipantTimer(NULL),
																		m_listener(this)
{

}

PDPSimple::~PDPSimple() {
	// TODO Auto-generated destructor stub
}

bool PDPSimple::initPDP(ParticipantImpl* part,uint32_t participantID)
{
	pInfo(RTPS_B_CYAN<<"Beginning ParticipantDiscoveryProtocol Initialization"<<RTPS_DEF<<endl);
	mp_participant = part;
	m_discovery = mp_participant->getBuiltinAttributes();

	if(!createSPDPEndpoints())
		return false;
	mp_builtin->updateMetatrafficLocators(this->mp_SPDPReader->unicastLocatorList);
	this->mp_SPDPReader->lock();
	this->mp_SPDPWriter->lock();

	m_participantProxies.push_back(new ParticipantProxyData());
	m_participantProxies.front()->initializeData(mp_participant,this);

	//INIT EDP
	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EDP*)(new EDPStatic(this,mp_participant));
		mp_EDP->initEDP(m_discovery);
	}
	else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EDP*)(new EDPSimple(this,mp_participant));
		mp_EDP->initEDP(m_discovery);
	}
	else
	{
		pWarning("No EndpointDiscoveryProtocol defined"<<endl);
		return false;
	}

	mp_resendParticipantTimer = new ResendParticipantProxyDataPeriod(this,mp_participant->getEventResource(),
			boost::posix_time::milliseconds(TimeConv::Time_t2MilliSecondsInt64(m_discovery.leaseDuration_announcementperiod)));

	this->mp_SPDPReader->unlock();
	this->mp_SPDPWriter->unlock();
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
	pInfo("Announcing Participant State"<<endl);
	CacheChange_t* change = NULL;
	if(new_change || m_hasChangedLocalPDP)
	{
		m_participantProxies.front()->m_manualLivelinessCount++;
		if(mp_SPDPWriter->getHistoryCacheSize() > 0)
			mp_SPDPWriter->removeMinSeqCacheChange();
		mp_SPDPWriter->new_change(ALIVE,NULL,&change);
		change->instanceHandle = m_participantProxies.front()->m_key;
		if(m_participantProxies.front()->toParameterList())
		{
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = m_participantProxies.front()->m_QosList.allQos.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,m_participantProxies.front()->m_QosList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_SPDPWriter->add_change(change);
		}
	}
	else
	{
		if(!mp_SPDPWriter->get_last_added_cache(&change))
		{
			pWarning("Error getting last added change"<<endl);
			return;
		}
	}
	mp_SPDPWriter->unsent_change_add(change);
}

bool PDPSimple::lookupReaderProxyData(const GUID_t& reader, ReaderProxyData** rdata)
{
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
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
	for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
			pit!=m_participantProxies.end();++pit)
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
	pInfo(RTPS_CYAN<<"SimplePDP removing ReaderProxyData: "<<rdata->m_guid<<RTPS_DEF<<endl);
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
	pInfo(RTPS_CYAN<<"SimplePDP removing WriterProxyData: "<<wdata->m_guid<<RTPS_DEF<<endl);
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

bool PDPSimple::createSPDPEndpoints()
{
	pInfo(RTPS_CYAN<<"Creating SPDP Endpoints"<<RTPS_DEF<<endl);
	//SPDP BUILTIN PARTICIPANT WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	//Wparam.historyMaxSize = 1;
	//Locators where it is going to listen
	Wparam.topic.topicName = "DCPSParticipant";
	Wparam.topic.topicDataType = "DiscoveredParticipantData";
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
	if(mp_participant->createWriter(&wout,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,true,STATELESS,NULL,NULL,c_EntityId_SPDPWriter))
	{
		mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
		for(LocatorListIterator lit = mp_builtin->m_metatrafficMulticastLocatorList.begin();
				lit!=mp_builtin->m_metatrafficMulticastLocatorList.end();++lit)
			mp_SPDPWriter->reader_locator_add(*lit,false);
	}
	else
	{
		pError("SimplePDP Writer creation failed"<<endl);
		return false;
	}
	//SPDP BUILTIN PARTICIPANT READER
	SubscriberAttributes Rparam;
	//	Rparam.historyMaxSize = 100;
	//  Locators where it is going to listen
	Rparam.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
	Rparam.topic.topicKind = WITH_KEY;
	Rparam.topic.topicName = "DCPSParticipant";
	Rparam.topic.topicDataType = "DiscoveredParticipantData";
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
	if(mp_participant->createReader(&rout,Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,
			true,STATELESS,(DDSTopicDataType*)&m_topicDataType,(SubscriberListener*)&this->m_listener,c_EntityId_SPDPReader))
	{
		mp_SPDPReader = dynamic_cast<StatelessReader*>(rout);
		//mp_SPDPReader->setListener();
	}
	else
	{
		pError("SimplePDP Reader creation failed"<<endl);
		return false;
	}

	pInfo(RTPS_CYAN<< "SPDP Endpoints creation finished"<<RTPS_DEF<<endl)
	return true;
}

bool PDPSimple::addReaderProxyData(ReaderProxyData* rdata,bool copydata,
		ReaderProxyData** returnReaderProxyData,ParticipantProxyData** pdata)
{
	pInfo(RTPS_CYAN<<"SimplePDP adding ReaderProxyData: "<<rdata->m_guid<<RTPS_DEF<<endl);
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
	pInfo(RTPS_CYAN<<"SimplePDP adding WriterProxyData: "<<wdata->m_guid<<RTPS_DEF<<endl);
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
	pInfo(RTPS_CYAN<<"SimplePDP assign remote endpoints for participant: "<<pdata->m_guid.guidPrefix<<RTPS_DEF<<endl);
	WriterProxyData* wp = new WriterProxyData();
	wp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
	wp->m_guid.entityId = c_EntityId_SPDPWriter;
	wp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
	wp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
	wp->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	wp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	pdata->m_builtinWriters.push_back(wp);
	mp_SPDPReader->matched_writer_add(wp);

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

	//Inform EDP of new participant data:
	if(mp_EDP!=NULL)
		mp_EDP->assignRemoteEndpoints(pdata);
	if(mp_builtin->mp_WLP !=NULL)
		mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);

	//	//If staticEDP, perform matching:
	//	//FIXME: PUT HIS INSISE REMOTE ENDPOINTS FOR STATIC DISCOVERY
	//	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	//	{
	//		for(std::vector<RTPSReader*>::iterator it = mp_participant->userReadersListBegin();
	//				it!=mp_participant->userReadersListEnd();++it)
	//		{
	//			if((*it)->getUserDefinedId() > 0)
	//				mp_EDP->localReaderMatching(*it,false);
	//		}
	//		for(std::vector<RTPSWriter*>::iterator it = mp_participant->userWritersListBegin();
	//				it!=mp_participant->userWritersListEnd();++it)
	//		{
	//			if((*it)->getUserDefinedId() > 0)
	//				mp_EDP->localWriterMatching(*it,false);
	//		}
	//	}

}

bool PDPSimple::removeRemoteParticipant(GUID_t& partGUID)
{
	pWarning("Removing remote participant: "<<partGUID<<endl;);
	boost::lock_guard<Endpoint> guardW(*this->mp_SPDPWriter);
	boost::lock_guard<Endpoint> guardR(*this->mp_SPDPReader);
	ParticipantProxyData* pdata=NULL;
	//Remove it from our vector or participantProxies:
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
	if(pdata !=NULL)
	{
		for(std::vector<ReaderProxyData*>::iterator rit = pdata->m_readers.begin();
				rit!= pdata->m_readers.end();++rit)
		{
			mp_EDP->unpairReaderProxy(*rit);
			delete(*rit);

		}
		for(std::vector<WriterProxyData*>::iterator wit = pdata->m_writers.begin();
				wit!=pdata->m_writers.end();++wit)
		{
			mp_EDP->unpairWriterProxy(*wit);
			delete(*wit);

		}
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


} /* namespace rtps */
} /* namespace eprosima */


