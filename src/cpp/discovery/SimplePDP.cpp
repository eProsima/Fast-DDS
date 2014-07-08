/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDPD.cpp
 *
 */
#include <iostream>
#include "eprosimartps/discovery/SimplePDP.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/discovery/timedevent/ResendDiscoveryDataPeriod.h"



#include "eprosimartps/Participant.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatefulWriter.h"

#include "eprosimartps/liveliness/WriterLiveliness.h"

#include "eprosimartps/utils/eClock.h"


using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

SimplePDP::SimplePDP(ParticipantImpl* p_part):
	ParticipantDiscoveryProtocol(p_part),
	mp_SPDPWriter(NULL),mp_SPDPReader(NULL),
	m_listener(this),
	m_hasChangedLocalPDP(true),
	m_resendDataTimer(NULL)
{
	// TODO Auto-generated constructor stub
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = 7400;
	m_SPDP_WELL_KNOWN_UNICAST_PORT = 7400;
}

SimplePDP::~SimplePDP()
{
	if(mp_EDP!=NULL)
		delete(mp_EDP);
	if(mp_WL!=NULL)
		delete(mp_WL);
}

bool SimplePDP::initPDP(const DiscoveryAttributes& attributes,uint32_t participantID)
{
	pInfo(RTPS_B_CYAN<<"Beginning ParticipantDiscoveryProtocol Initialization"<<RTPS_DEF<<endl)
	m_discovery = attributes;
	DomainParticipantImpl* dp = DomainParticipantImpl::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getMulticastPort(m_discovery.domainId);

	m_SPDP_WELL_KNOWN_UNICAST_PORT =  dp->getUnicastPort(m_discovery.domainId,participantID);

	//FIXME: FIx creation of endpoints when port is already being used.
	updateLocalParticipantData();
	if(!createSPDPEndpoints())
		return false;
	mp_SPDPReader->lock();
	updateLocalParticipantData();

	//INIT EDP
	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EndpointDiscoveryProtocol*)new StaticEDP(this);
	}
	else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EndpointDiscoveryProtocol*)new SimpleEDP(this);
	}
	else
	{
		pWarning("No EndpointDiscoveryProtocol defined"<<endl);
		return false;
	}


	if(mp_EDP->initEDP(m_discovery))
	{
		mp_SPDPReader->unlock();
	}
	else
		return false;
	//START WRITER LIVELINESS PROTOCOL
	if(m_discovery.use_WriterLivelinessProtocol)
	{
		mp_WL = new WriterLiveliness(mp_participant);
		mp_WL->createEndpoints(mp_localDPData->m_metatrafficUnicastLocatorList,mp_localDPData->m_metatrafficMulticastLocatorList);
//		mp_WL->assignRemoteEndpoints(mp_localDPData);
		pInfo(RTPS_MAGENTA<<"Liveliness Protocol initialized"<<RTPS_DEF << endl;);
	}
	this->announceParticipantState(true);
	//		eClock::my_sleep(50);
	//		this->announceParticipantState(false);
	//		eClock::my_sleep(50);
	//		this->announceParticipantState(false);
	m_resendDataTimer = new ResendDiscoveryDataPeriod(this,mp_participant->getEventResource(),
						boost::posix_time::milliseconds(Time_t2MilliSec(m_discovery.resendDiscoveryParticipantDataPeriod)));
	m_resendDataTimer->restart_timer();

	eClock::my_sleep(100);

	return true;

}

bool SimplePDP::updateLocalParticipantData()
{
	if(mp_localDPData == NULL)
	{
		mp_localDPData = new DiscoveredParticipantData();
		m_discoveredParticipants.push_back(mp_localDPData);
	}
	mp_localDPData->leaseDuration = m_discovery.leaseDuration;
	VENDORID_EPROSIMA(mp_localDPData->m_VendorId);
	//FIXME: add correct builtIn Endpoints
	mp_localDPData->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
	mp_localDPData->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
	if(m_discovery.use_WriterLivelinessProtocol)
	{
		mp_localDPData->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
		mp_localDPData->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
	}
	if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		if(m_discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
		{
			mp_localDPData->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
			mp_localDPData->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
		}
		if(m_discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
		{
			mp_localDPData->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
			mp_localDPData->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
		}
	}

	mp_localDPData->m_defaultUnicastLocatorList = mp_participant->m_defaultUnicastLocatorList;
	mp_localDPData->m_defaultMulticastLocatorList = mp_participant->m_defaultMulticastLocatorList;
	mp_localDPData->m_expectsInlineQos = false;
	mp_localDPData->m_guidPrefix = mp_participant->getGuid().guidPrefix;
	for(uint8_t i =0;i<16;++i)
	{
		if(i<12)
			mp_localDPData->m_key.value[i] = mp_participant->getGuid().guidPrefix.value[i];
		if(i>=16)
			mp_localDPData->m_key.value[i] = mp_participant->getGuid().entityId.value[i];
	}
	//FIXME: Do something with livelinesscount
	//pdata->m_manualLivelinessCount;

	Locator_t multiLocator;
	multiLocator.kind = LOCATOR_KIND_UDPv4;
	multiLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	multiLocator.set_IP4_address(239,255,0,1);
	mp_localDPData->m_metatrafficMulticastLocatorList.push_back(multiLocator);

	if(mp_SPDPReader == NULL)
	{
		LocatorList_t locators;
		IPFinder::getIPAddress(&locators);
		for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
		{
			it->port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
			mp_localDPData->m_metatrafficUnicastLocatorList.push_back(*it);
		}
	}
	else
	{
		mp_localDPData->m_metatrafficUnicastLocatorList = mp_SPDPReader->unicastLocatorList;
	}


	mp_localDPData->m_participantName = mp_participant->getParticipantName();

	return true;
}

void SimplePDP::localParticipantHasChanged()
{
	this->m_hasChangedLocalPDP = true;
}

bool SimplePDP::createSPDPEndpoints()
{
	pInfo(RTPS_CYAN<<"Creating SPDP Endpoints"<<endl);
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
	Wparam.payloadMaxSize = 1000;
	Wparam.userDefinedId = -99;
	Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	RTPSWriter* wout;
	if(mp_participant->createWriter(&wout,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,true,STATELESS,NULL,NULL,c_EntityId_SPDPWriter))
	{
		mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
		for(LocatorListIterator lit = mp_localDPData->m_metatrafficMulticastLocatorList.begin();
				lit!=mp_localDPData->m_metatrafficMulticastLocatorList.end();++lit)
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
	Rparam.multicastLocatorList = mp_localDPData->m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = mp_localDPData->m_metatrafficUnicastLocatorList;
	Rparam.topic.topicKind = WITH_KEY;
	Rparam.topic.topicName = "DCPSParticipant";
	Rparam.topic.topicDataType = "DiscoveredParticipantData";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 1;
	Rparam.topic.resourceLimitsQos.max_instances = 1000;
	Rparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
	Rparam.topic.resourceLimitsQos.max_samples = 1000;
	Rparam.topic.resourceLimitsQos.allocated_samples = 500;
	Rparam.payloadMaxSize = 1000;
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

void SimplePDP::announceParticipantState(bool new_change)
{
	pInfo("Announcing Participant State"<<endl);
	CacheChange_t* change = NULL;
	if(new_change || m_hasChangedLocalPDP)
	{
		updateLocalParticipantData();
		if(mp_SPDPWriter->getHistoryCacheSize() > 0)
			mp_SPDPWriter->removeMinSeqCacheChange();
		mp_SPDPWriter->new_change(ALIVE,NULL,&change);
		change->instanceHandle = mp_localDPData->m_key;
		if(updateParameterList())
		{
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = m_localDPDasQosList.allQos.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,m_localDPDasQosList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
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

bool SimplePDP::updateParameterList()
{
	if(m_hasChangedLocalPDP)
	{
		m_localDPDasQosList.allQos.deleteParams();
		m_localDPDasQosList.allQos.resetList();
		m_localDPDasQosList.inlineQos.resetList();
		bool valid = QosList::addQos(&m_localDPDasQosList,PID_PROTOCOL_VERSION,mp_localDPData->m_protocolVersion);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_VENDORID,mp_localDPData->m_VendorId);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_EXPECTS_INLINE_QOS,mp_localDPData->m_expectsInlineQos);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PARTICIPANT_GUID,mp_participant->getGuid());
		for(std::vector<Locator_t>::iterator it=mp_localDPData->m_metatrafficMulticastLocatorList.begin();
				it!=mp_localDPData->m_metatrafficMulticastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_METATRAFFIC_MULTICAST_LOCATOR,*it);
		}
		for(std::vector<Locator_t>::iterator it=mp_localDPData->m_metatrafficUnicastLocatorList.begin();
				it!=mp_localDPData->m_metatrafficUnicastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_METATRAFFIC_UNICAST_LOCATOR,*it);
		}
		for(std::vector<Locator_t>::iterator it=mp_localDPData->m_defaultUnicastLocatorList.begin();
				it!=mp_localDPData->m_defaultUnicastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_DEFAULT_UNICAST_LOCATOR,*it);
		}
		for(std::vector<Locator_t>::iterator it=mp_localDPData->m_defaultMulticastLocatorList.begin();
				it!=mp_localDPData->m_defaultMulticastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_DEFAULT_MULTICAST_LOCATOR,*it);
		}
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PARTICIPANT_LEASE_DURATION,mp_localDPData->leaseDuration);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_BUILTIN_ENDPOINT_SET,(uint32_t)mp_localDPData->m_availableBuiltinEndpoints);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_ENTITY_NAME,mp_localDPData->m_participantName);

		if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
			valid&= this->addStaticEDPInfo();

		valid &=ParameterList::updateCDRMsg(&m_localDPDasQosList.allQos,EPROSIMA_ENDIAN);
		if(valid)
			m_hasChangedLocalPDP = false;
		return valid;
	}
	else
		return true;
}

bool SimplePDP::addStaticEDPInfo()
{
	std::stringstream ss;
	std::string str1,str2;
	bool valid = true;
	for(std::vector<RTPSReader*>::iterator it = this->mp_participant->userReadersListBegin();
			it!=this->mp_participant->userReadersListBegin();++it)
	{
		if((*it)->getUserDefinedId() <= 0)
			continue;
		ss.clear();ss.str(std::string());
		ss << "staticedp_reader_" << (*it)->getUserDefinedId();
		str1 = ss.str();
		ss.clear();	ss.str(std::string());
		ss << (int)(*it)->getGuid().entityId.value[0] <<".";ss << (int)(*it)->getGuid().entityId.value[1] <<".";
		ss << (int)(*it)->getGuid().entityId.value[2] <<".";ss << (int)(*it)->getGuid().entityId.value[3];
		str2 = ss.str();
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PROPERTY_LIST,str1,str2);
	}
	for(std::vector<RTPSWriter*>::iterator it = this->mp_participant->userWritersListBegin();
			it!=this->mp_participant->userWritersListEnd();++it)
	{
		if((*it)->getUserDefinedId() <= 0)
			continue;
		ss.clear();	ss.str(std::string());
		ss << "staticedp_writer_" << (*it)->getUserDefinedId();
		str1 = ss.str();ss.clear();
		ss.str(std::string());
		ss << (int)(*it)->getGuid().entityId.value[0] <<".";ss << (int)(*it)->getGuid().entityId.value[1] <<".";
		ss << (int)(*it)->getGuid().entityId.value[2] <<".";ss << (int)(*it)->getGuid().entityId.value[3];
		str2 = ss.str();
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PROPERTY_LIST,str1,str2);
	}
	return valid;
}

bool SimplePDP::localWriterMatching(RTPSWriter* W,bool first_time)
{
	pInfo("SimplePDP localWriterMatching"<<endl);
	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
		this->m_hasChangedLocalPDP = true;
	if(mp_EDP!=NULL)
		return this->mp_EDP->localWriterMatching(W,first_time);
	else
		return false;
}

bool SimplePDP::localReaderMatching(RTPSReader* R,bool first_time)
{
	pInfo("SimplePDP localReaderMatching"<<endl);
	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
			this->m_hasChangedLocalPDP = true;
	if(mp_EDP!=NULL)
		return this->mp_EDP->localReaderMatching(R,first_time);
	else
		return false;
}

void SimplePDP::stopParticipantAnnouncement()
{
	m_resendDataTimer->stop_timer();
}

void SimplePDP::resetParticipantAnnouncement()
{
	m_resendDataTimer->restart_timer();
}

typedef std::vector<DiscoveredParticipantData*>::iterator TDPDit;

bool SimplePDP::removeRemoteParticipant(const GuidPrefix_t& guidP)
{
	pWarning("Removing remote participant: "<<guidP<<endl;)
	boost::lock_guard<Endpoint> guardW(*this->mp_SPDPWriter);
	boost::lock_guard<Endpoint> guardR(*this->mp_SPDPReader);
	TDPDit dpdit;
	bool found = false;
	for(dpdit= this->m_discoveredParticipants.begin();
			dpdit!=this->m_discoveredParticipants.end();++dpdit)
	{
		if((*dpdit)->m_guidPrefix == guidP)
		{
			found = true;
			break;
		}
	}
	if(!found)
		return false;
	if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		dynamic_cast<SimpleEDP*>(this->mp_EDP)->removeRemoteEndpoints(guidP);
	}
	//Now We Remove the remote Readers and Writers from our local Endpoints:
	for(std::vector<DiscoveredWriterData*>::iterator wit = (*dpdit)->m_writers.begin();
			wit!=(*dpdit)->m_writers.end();++wit)
	{
		for(std::vector<RTPSReader*>::iterator rit = this->mp_participant->userReadersListBegin();
				rit!=this->mp_participant->userReadersListEnd();++rit)
		{
			if((*rit)->getStateType() == STATELESS)
			{
				(dynamic_cast<StatelessReader*>(*rit))->matched_writer_remove((*wit)->m_writerProxy.remoteWriterGuid);
			}
			else if((*rit)->getStateType() == STATEFUL)
			{
				(dynamic_cast<StatefulReader*>(*rit))->matched_writer_remove((*wit)->m_writerProxy.remoteWriterGuid);
			}
		}
	}
	for(std::vector<DiscoveredReaderData*>::iterator rit = (*dpdit)->m_readers.begin();
			rit!=(*dpdit)->m_readers.end();++rit)
	{
		for(std::vector<RTPSWriter*>::iterator wit = this->mp_participant->userWritersListBegin();
				wit!=this->mp_participant->userWritersListEnd();++wit)
		{
			if((*wit)->getStateType() == STATELESS)
			{
				for(std::vector<Locator_t>::iterator lit = (*rit)->m_readerProxy.unicastLocatorList.begin();
						lit!=(*rit)->m_readerProxy.unicastLocatorList.end();++lit)
				{
					(dynamic_cast<StatelessWriter*>(*wit))->reader_locator_remove(*lit);
				}
			}
			else if((*wit)->getStateType() == STATEFUL)
			{
				(dynamic_cast<StatefulWriter*>(*wit))->matched_reader_remove((*rit)->m_readerProxy.remoteReaderGuid);
			}
		}
	}
	m_discoveredParticipants.erase(dpdit);
	delete(*dpdit);
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */
