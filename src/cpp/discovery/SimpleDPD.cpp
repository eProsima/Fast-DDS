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

#include "eprosimartps/discovery/SimpleDPD.h"
#include "eprosimartps/dds/DomainParticipant.h"
using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

SimpleDPD::SimpleDPD(Participant* p_part):
	ParticipantDiscoveryProtocol(p_part),
	mp_SPDPWriter(NULL),mp_SPDPReader(NULL),
	m_listener(this),
	m_hasChangedLocalDPD(true)
{
	// TODO Auto-generated constructor stub
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = 7400;
	m_SPDP_WELL_KNOWN_UNICAST_PORT = 7400;
}

SimpleDPD::~SimpleDPD()
{
	// TODO Auto-generated destructor stub
	delete(mp_EDP);
}

bool SimpleDPD::initDPD(DiscoveryAttributes& attributes,uint32_t participantId)
{
	m_discovery = attributes;
	DomainParticipant* dp = DomainParticipant::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getPortBase()
														+ dp->getDomainIdGain() * m_discovery.domainId
														+ dp->getOffsetd0();
	m_SPDP_WELL_KNOWN_UNICAST_PORT =  dp->getPortBase()
															+ dp->getDomainIdGain() * m_domainId
															+ dp->getOffsetd1()
															+ dp->getParticipantIdGain() * participantId;

	addLocalParticipant(mp_participant);

	//INIT EDP
	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EndpointDiscoveryProtocol*)new StaticEDP();
	}
	else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EndpointDiscoveryProtocol*)new SimpleEDP();
	}
	else
	{
		pWarning("No EndpointDiscoveryProtocol defined"<<endl);
	}
	createSPDPEndpoints();

	mp_EDP->initEDP(m_discovery);




	return true;
}

bool SimpleDPD::addLocalParticipant(Participant* p)
{
	DiscoveredParticipantData pdata;
	pdata.leaseDuration = m_discovery.leaseDuration;
	VENDORID_EPROSIMA(pdata.m_VendorId);
	//FIXME: add correct builtIn Endpoints
	pdata.m_defaultUnicastLocatorList = p->m_defaultUnicastLocatorList;
	pdata.m_defaultMulticastLocatorList = p->m_defaultMulticastLocatorList;
	pdata.m_expectsInlineQos = false;
	pdata.m_guidPrefix = p->m_guid.guidPrefix;
	for(uint8_t i =0;i<16;++i)
	{
		if(i<12)
			pdata.m_key.value[i] = p->m_guid.guidPrefix.value[i];
		if(i>=16)
			pdata.m_key.value[i] = p->m_guid.entityId.value[i];
	}
	pdata.m_manualLivelinessCount;

	Locator_t multiLocator;
	multiLocator.kind = LOCATOR_KIND_UDPv4;
	multiLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	multiLocator.set_IP4_address(239,255,0,1);
	pdata.m_metatrafficMulticastLocatorList.push_back(multiLocator);

	LocatorList_t locators;
	DomainParticipant::getIPAddress(&locators);
	for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
	{
		it->port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
		pdata.m_metatrafficUnicastLocatorList.push_back(*it);
		pdata.m_defaultUnicastLocatorList.push_back(*it);
	}

	pdata.m_participantName = p->m_participantName;


	m_discoveredParticipants.push_back(pdata);
	mp_localDPD = *m_discoveredParticipants.begin();

	return true;
}

bool SimpleDPD::createSPDPEndpoints()
{
	//SPDP BUILTIN PARTICIPANT WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	Wparam.historyMaxSize = 1;
	//Locators where it is going to listen
	Wparam.topic.topicName = "DCPSParticipant";
	Wparam.topic.topicDataType = "DiscoveredParticipantData";
	Wparam.topic.topicKind = WITH_KEY;
	Wparam.userDefinedId = -1;
	mp_participant->createStatelessWriter(&mp_SPDPWriter,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	mp_SPDPWriter->m_guid.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;
	for(LocatorListIterator lit = mp_localDPD->m_metatrafficMulticastLocatorList.begin();
			lit!=mp_localDPD->m_metatrafficMulticastLocatorList.end();++lit)
		mp_SPDPWriter->reader_locator_add(*lit,false);
	for(LocatorListIterator lit = mp_localDPD->m_metatrafficUnicastLocatorList.begin();
			lit!=mp_localDPD->m_metatrafficUnicastLocatorList.end();++lit)
		mp_SPDPWriter->reader_locator_add(*lit,false);

	//SPDP BUILTIN PARTICIPANT READER
	SubscriberAttributes Rparam;
	Rparam.historyMaxSize = 100;
	//Locators where it is going to listen
	Rparam.multicastLocatorList = mp_localDPD->m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = mp_localDPD->m_metatrafficUnicastLocatorList;
	Rparam.topic.topicKind = WITH_KEY;
	Rparam.topic.topicName = "DCPSParticipant";
	Rparam.topic.topicDataType = "DiscoveredParticipantData";
	Rparam.userDefinedId = -1;
	mp_participant->createStatelessReader(&mp_SPDPReader,Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	mp_SPDPReader->m_guid.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;
	mp_SPDPReader->mp_listener = &this->m_listener;
	return true;
}

void SimpleDPD::announceParticipantState(bool new_change)
{
	pInfo("Announcing Participant State"<<endl);
	CacheChange_t* change = NULL;
	if(new_change)
	{
		if(mp_SPDPWriter->getHistoryCacheSize() > 0)
			mp_SPDPWriter->removeMinSeqCacheChange();
		mp_SPDPWriter->new_change(ALIVE,NULL,&change);
		change->instanceHandle = mp_localDPD->m_key;
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

bool SimpleDPD::updateParameterList()
{
	if(m_hasChangedLocalDPD)
	{
		m_localDPDasQosList.allQos.deleteParams();
		m_localDPDasQosList.allQos.resetList();
		m_localDPDasQosList.inlineQos.resetList();
		bool valid = QosList::addQos(&m_localDPDasQosList,PID_PROTOCOL_VERSION,mp_localDPD->m_protocolVersion);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_VENDORID,mp_localDPD->m_VendorId);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_EXPECTS_INLINE_QOS,mp_localDPD->m_expectsInlineQos);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PARTICIPANT_GUID,mp_participant->m_guid);
		for(std::vector<Locator_t>::iterator it=mp_localDPD->m_metatrafficMulticastLocatorList.begin();
				it!=mp_localDPD->m_metatrafficMulticastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_METATRAFFIC_MULTICAST_LOCATOR,*it);
		}
		for(std::vector<Locator_t>::iterator it=mp_localDPD->m_metatrafficUnicastLocatorList.begin();
				it!=mp_localDPD->m_metatrafficUnicastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_METATRAFFIC_UNICAST_LOCATOR,*it);
		}
		for(std::vector<Locator_t>::iterator it=mp_localDPD->m_defaultUnicastLocatorList.begin();
				it!=mp_localDPD->m_defaultUnicastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_DEFAULT_UNICAST_LOCATOR,*it);
		}
		for(std::vector<Locator_t>::iterator it=mp_localDPD->m_defaultMulticastLocatorList.begin();
				it!=mp_localDPD->m_defaultMulticastLocatorList.end();++it)
		{
			valid &=QosList::addQos(&m_localDPDasQosList,PID_DEFAULT_MULTICAST_LOCATOR,*it);
		}
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PARTICIPANT_LEASE_DURATION,mp_localDPD->leaseDuration);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_BUILTIN_ENDPOINT_SET,mp_localDPD->m_availableBuiltinEndpoints);
		valid &=QosList::addQos(&m_localDPDasQosList,PID_ENTITY_NAME,mp_localDPD->m_participantName);

		if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
			valid&= this->addStaticEDPInfo();

		valid &=ParameterList::updateCDRMsg(&m_localDPDasQosList.allQos,EPROSIMA_ENDIAN);
		if(valid)
			m_hasChangedLocalDPD = false;
		return valid;
	}
	else
		return true;
}

bool SimpleDPD::addStaticEDPInfo()
{
	std::stringstream ss;
	std::string str1,str2;
	bool valid = true;
	for(std::vector<RTPSWriter*>::iterator it = this->mp_participant->m_writerList.begin();
			it!=this->mp_participant->m_writerList.end();++it)
	{
		if((*it)->m_userDefinedId <= 0)
			continue;
		ss.clear();ss.str(std::string());
		ss << "staticedp_writer_" << (*it)->m_userDefinedId;
		str1 = ss.str();
		ss.clear();	ss.str(std::string());
		ss << (int)(*it)->m_guid.entityId.value[0] <<".";ss << (int)(*it)->m_guid.entityId.value[1] <<".";
		ss << (int)(*it)->m_guid.entityId.value[2] <<".";ss << (int)(*it)->m_guid.entityId.value[3];
		str2 = ss.str();
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PROPERTY_LIST,str1,str2);
	}
	for(std::vector<RTPSReader*>::iterator it = this->mp_participant->m_readerList.begin();
			it!=this->mp_participant->m_readerList.end();++it)
	{
		if((*it)->m_userDefinedId <= 0)
			continue;
		ss.clear();	ss.str(std::string());
		ss << "staticedp_reader_" << (*it)->m_userDefinedId;
		str1 = ss.str();ss.clear();
		ss.str(std::string());
		ss << (int)(*it)->m_guid.entityId.value[0] <<".";ss << (int)(*it)->m_guid.entityId.value[1] <<".";
		ss << (int)(*it)->m_guid.entityId.value[2] <<".";ss << (int)(*it)->m_guid.entityId.value[3];
		str2 = ss.str();
		valid &=QosList::addQos(&m_localDPDasQosList,PID_PROPERTY_LIST,str1,str2);
	}
	return valid;
}




} /* namespace rtps */
} /* namespace eprosima */
