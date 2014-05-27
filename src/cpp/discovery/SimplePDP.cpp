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
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/SimplePDP.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/timedevent/ResendDiscoveryDataPeriod.h"

#include "eprosimartps/Participant.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"


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
}

bool SimplePDP::initPDP(const DiscoveryAttributes& attributes,uint32_t participantID)
{
	pInfo(B_CYAN<<"Beginning ParticipantDiscoveryProtocol Initialization"<<DEF<<endl)
	m_discovery = attributes;
	DomainParticipantImpl* dp = DomainParticipantImpl::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getPortBase()
														+ dp->getDomainIdGain() * m_discovery.domainId
														+ dp->getOffsetd0();
	m_SPDP_WELL_KNOWN_UNICAST_PORT =  dp->getPortBase()
															+ dp->getDomainIdGain() * m_discovery.domainId
															+ dp->getOffsetd1()
															+ dp->getParticipantIdGain() * participantID;

	addLocalParticipant(mp_participant);

	createSPDPEndpoints();

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
		this->announceParticipantState(true);
		m_resendDataTimer = new ResendDiscoveryDataPeriod(this,mp_participant->getEventResource(),
				boost::posix_time::milliseconds(m_discovery.resendDiscoveryParticipantDataPeriod.to64time()*1000));
		m_resendDataTimer->restart_timer();
		return true;
	}

	return false;
}

bool SimplePDP::addLocalParticipant(ParticipantImpl* p)
{
	DiscoveredParticipantData* pdata = new DiscoveredParticipantData();
	pdata->leaseDuration = m_discovery.leaseDuration;
	VENDORID_EPROSIMA(pdata->m_VendorId);
	//FIXME: add correct builtIn Endpoints
	pdata->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
	pdata->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
	if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		if(m_discovery.m_simpleEDP.use_Publication_Writer)
			pdata->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
		if(m_discovery.m_simpleEDP.use_Publication_Reader)
			pdata->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
		if(m_discovery.m_simpleEDP.use_Subscription_Reader)
			pdata->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
		if(m_discovery.m_simpleEDP.use_Subscription_Writer)
			pdata->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
	}

	pdata->m_defaultUnicastLocatorList = p->m_defaultUnicastLocatorList;
	pdata->m_defaultMulticastLocatorList = p->m_defaultMulticastLocatorList;
	pdata->m_expectsInlineQos = false;
	pdata->m_guidPrefix = p->getGuid().guidPrefix;
	for(uint8_t i =0;i<16;++i)
	{
		if(i<12)
			pdata->m_key.value[i] = p->getGuid().guidPrefix.value[i];
		if(i>=16)
			pdata->m_key.value[i] = p->getGuid().entityId.value[i];
	}
	//FIXME: Do something with livelinesscount
	//pdata->m_manualLivelinessCount;

	Locator_t multiLocator;
	multiLocator.kind = LOCATOR_KIND_UDPv4;
	multiLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	multiLocator.set_IP4_address(239,255,0,1);
	pdata->m_metatrafficMulticastLocatorList.push_back(multiLocator);

	LocatorList_t locators;
	IPFinder::getIPAddress(&locators);
	for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
	{
		it->port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
		pdata->m_metatrafficUnicastLocatorList.push_back(*it);
	}

	pdata->m_participantName = p->getParticipantName();


	m_discoveredParticipants.push_back(pdata);
	mp_localDPData = pdata;

	return true;
}

void SimplePDP::localParticipantHasChanged()
{
	this->m_hasChangedLocalPDP = true;
}

bool SimplePDP::createSPDPEndpoints()
{
	pInfo(CYAN<<"Creating SPDP Endpoints"<<endl);
	//SPDP BUILTIN PARTICIPANT WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	Wparam.historyMaxSize = 1;
	//Locators where it is going to listen
	Wparam.topic.topicName = "DCPSParticipant";
	Wparam.topic.topicDataType = "DiscoveredParticipantData";
	Wparam.topic.topicKind = WITH_KEY;
	Wparam.userDefinedId = -1;

	if(mp_participant->createStatelessWriter(&mp_SPDPWriter,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,true,NULL,NULL,c_EntityId_SPDPWriter))
	{
		for(LocatorListIterator lit = mp_localDPData->m_metatrafficMulticastLocatorList.begin();
				lit!=mp_localDPData->m_metatrafficMulticastLocatorList.end();++lit)
			mp_SPDPWriter->reader_locator_add(*lit,false);

//		for(LocatorListIterator lit = mp_localDPData->m_metatrafficUnicastLocatorList.begin();
//				lit!=mp_localDPData->m_metatrafficUnicastLocatorList.end();++lit)
//			mp_SPDPWriter->reader_locator_add(*lit,false);

	}
	else
	{
		pError("SimplePDP Writer creation failed"<<endl);
		return false;
	}
	//SPDP BUILTIN PARTICIPANT READER
	SubscriberAttributes Rparam;
	Rparam.historyMaxSize = 100;
	//Locators where it is going to listen
	Rparam.multicastLocatorList = mp_localDPData->m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = mp_localDPData->m_metatrafficUnicastLocatorList;
	Rparam.topic.topicKind = WITH_KEY;
	Rparam.topic.topicName = "DCPSParticipant";
	Rparam.topic.topicDataType = "DiscoveredParticipantData";
	Rparam.userDefinedId = -1;
	if(mp_participant->createStatelessReader(&mp_SPDPReader,Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,true,NULL,NULL,c_EntityId_SPDPReader))
	{
		mp_SPDPReader->setListener(&this->m_listener);
	}
	else
	{
		pError("SimplePDP Reader creation failed"<<endl);
			return false;
	}
	pInfo(CYAN<< "SPDP Endpoints creation finished"<<DEF<<endl)
	return true;
}

void SimplePDP::announceParticipantState(bool new_change)
{
	pInfo("Announcing Participant State"<<endl);
	CacheChange_t* change = NULL;
	if(new_change || m_hasChangedLocalPDP)
	{
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




} /* namespace rtps */
} /* namespace eprosima */
