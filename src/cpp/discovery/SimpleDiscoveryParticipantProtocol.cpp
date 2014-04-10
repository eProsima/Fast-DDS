/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDiscoveryParticipantProtocol.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "SimpleDiscoveryParticipantProtocol.h"
#include "eprosimartps/Participant.h"

namespace eprosima {
namespace rtps {

SimpleDiscoveryParticipantProtocol::SimpleDiscoveryParticipantProtocol(Participant* p):
		mp_Participant(p),
		m_DPDMsgHeader(RTPSMESSAGE_HEADER_SIZE)
{
	// TODO Auto-generated constructor stub
	m_resendData = NULL;
	m_SPDPbPWriter = NULL;
	m_SPDPbPReader = NULL;
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = 0;
	m_SPDP_WELL_KNOWN_UNICAST_PORT = 0;
	m_domainId = 0;
	m_hasChanged_DPDMsg = true;
}

SimpleDiscoveryParticipantProtocol::~SimpleDiscoveryParticipantProtocol()
{
	delete(m_resendData);
	delete(m_SPDPbPWriter);
	delete(m_SPDPbPReader);
}

bool SimpleDiscoveryParticipantProtocol::initSPDP(uint16_t domainId,
		uint16_t participantId, uint16_t resendDataPeriod_sec)
{
	m_domainId = domainId;
	DomainParticipant* dp = DomainParticipant::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getPortBase()
									+ dp->getDomainIdGain() * domainId
									+ dp->getOffsetd0();
	m_SPDP_WELL_KNOWN_UNICAST_PORT =  dp->getPortBase()
										+ dp->getDomainIdGain() * m_domainId
										+ dp->getOffsetd1()
										+ dp->getParticipantIdGain() *participantId;


	m_DPD.m_proxy.m_guidPrefix = mp_Participant->m_guid.guidPrefix;

	//FIXME: register type correctly
	//dp->registerType("DiscoveredParticipantData",&this->ser,&this->deser,&this->getKey);

	CDRMessage::initCDRMsg(&m_DPDMsgHeader);
	RTPSMessageCreator::addHeader(&m_DPDMsg,m_DPD.m_proxy.m_guidPrefix);


	Locator_t multiLocator;
	multiLocator.kind = LOCATOR_KIND_UDPv4;
	multiLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	multiLocator.set_IP4_address(239,255,0,1);
	m_DPD.m_proxy.m_metatrafficMulticastLocatorList.push_back(multiLocator);

	std::vector<Locator_t> locators;
	DomainParticipant::getIPAddress(&locators);
	for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
	{
		it->port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
		m_DPD.m_proxy.m_metatrafficUnicastLocatorList.push_back(*it);
		m_DPD.m_proxy.m_defaultUnicastLocatorList.push_back(*it);
	}

	m_DPD.leaseDuration.seconds = 100;



	//SPDP BUILTIN PARTICIPANT WRITER
	WriterParams_t Wparam;
	Wparam.pushMode = true;
	Wparam.historySize = 1;
	//Locators where it is going to listen
	Wparam.multicastLocatorList = m_DPD.m_proxy.m_metatrafficMulticastLocatorList;
	Wparam.unicastLocatorList = m_DPD.m_proxy.m_metatrafficUnicastLocatorList;
	Wparam.topicName = "DCPSParticipant";
	Wparam.topicDataType = "DiscoveredParticipantData";
	Wparam.topicKind = WITH_KEY;
	mp_Participant->createStatelessWriter(&m_SPDPbPWriter,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	//m_SPDPbPWriter = new StatelessWriter(Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	m_SPDPbPWriter->m_guid.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;
	ReaderLocator multiReaderLoc;
	multiReaderLoc.expectsInlineQos = false;
	multiReaderLoc.locator = multiLocator;
	m_SPDPbPWriter->reader_locator_add(multiReaderLoc);





	//SPDP BUILTIN PARTICIPANT READER
	ReaderParams_t Rparam;
	Rparam.historySize = 100;
	//Locators where it is going to listen
	Rparam.multicastLocatorList = m_DPD.m_proxy.m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = m_DPD.m_proxy.m_metatrafficUnicastLocatorList;
	Rparam.topicKind = WITH_KEY;
	Rparam.topicName = "DCPSParticipant";
	Rparam.topicDataType = "DiscoveredParticipantData";
//	m_SPDPbPReader = new StatelessReader(Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	mp_Participant->createStatelessReader(&m_SPDPbPReader,Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	m_SPDPbPReader->m_guid.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;



	m_resendData = new ResendDataPeriod((&m_SPDPbPWriter,
					boost::posix_time::milliseconds(resendDataPeriod_sec*1000)));

	const boost::system::error_code ec_sucess(boost::system::errc::success);
	m_resendData->event(ec_sucess);

	return true;
}

bool SimpleDiscoveryParticipantProtocol::updateParamList()
{
	m_DPDAsParamList.allQos.deleteParams();
	m_DPDAsParamList.allQos.resetList();
	m_DPDAsParamList.inlineQos.resetList();
	QosList::addQos(&m_DPDAsParamList,PID_PROTOCOL_VERSION,m_DPD.m_proxy.m_protocolVersion);
	QosList::addQos(&m_DPDAsParamList,PID_VENDORID,m_DPD.m_proxy.m_VendorId);
	QosList::addQos(&m_DPDAsParamList,PID_EXPECTS_INLINE_QOS,m_DPD.m_proxy.m_expectsInlineQos);
	QosList::addQos(&m_DPDAsParamList,PID_PARTICIPANT_GUID,mp_Participant->m_guid);
	for(std::vector<Locator_t>::iterator it=m_DPD.m_proxy.m_metatrafficMulticastLocatorList.begin();
			it!=m_DPD.m_proxy.m_metatrafficMulticastLocatorList.end();++it)
	{
		QosList::addQos(&m_DPDAsParamList,PID_METATRAFFIC_MULTICAST_LOCATOR,*it);
	}
	for(std::vector<Locator_t>::iterator it=m_DPD.m_proxy.m_metatrafficUnicastLocatorList.begin();
			it!=m_DPD.m_proxy.m_metatrafficUnicastLocatorList.end();++it)
	{
		QosList::addQos(&m_DPDAsParamList,PID_METATRAFFIC_UNICAST_LOCATOR,*it);
	}
	for(std::vector<Locator_t>::iterator it=m_DPD.m_proxy.m_defaultUnicastLocatorList.begin();
			it!=m_DPD.m_proxy.m_defaultUnicastLocatorList.end();++it)
	{
		QosList::addQos(&m_DPDAsParamList,PID_DEFAULT_UNICAST_LOCATOR,*it);
	}
	QosList::addQos(&m_DPDAsParamList,PID_PARTICIPANT_LEASE_DURATION,m_DPD.leaseDuration);
	QosList::addQos(&m_DPDAsParamList,PID_PARTICIPANT_BUILTIN_ENDPOINTS,m_DPD.m_proxy.m_availableBuiltinEndpoints);


	ParameterList::updateCDRMsg(&m_DPDAsParamList.allQos,EPROSIMA_ENDIAN);
}

bool SimpleDiscoveryParticipantProtocol::sendDPDMsg()
{
	CacheChange_t* change=NULL;
	if(m_DPDAsParamList.allQos.m_hasChanged || m_hasChanged_DPD)
	{
		m_SPDPbPWriter->m_writer_cache.remove_all_changes();
		m_SPDPbPWriter->new_change(ALIVE,NULL,&change);
		updateParamList();
		change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
		change->serializedPayload.length = m_DPDAsParamList.allQos.m_cdrmsg.length;
		memcpy(change->serializedPayload.data,m_DPDAsParamList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
		m_SPDPbPWriter->m_writer_cache.add_change(change);
	}
	else
	{
		m_SPDPbPWriter->m_writer_cache.get_last_added_cache(&change);
	}
	m_SPDPbPWriter->unsent_change_add(change);
	return true;
}

bool SimpleDiscoveryParticipantProtocol::updateDPDMsg()
{
	m_SPDPbPWriter->m_writer_cache.remove_all_changes();
	CacheChange_t* change=NULL;
	m_SPDPbPWriter->new_change(ALIVE,NULL,&change);
	//FIXME: update change serialized payload
	if(EPROSIMA_ENDIAN == BIGEND)
		change->serializedPayload.encapsulation = PL_CDR_BE;
	else
		change->serializedPayload.encapsulation = PL_CDR_LE;

	updateParamList();

	change->serializedPayload.length = m_DPDAsParamList.allQos.m_cdrmsg.length;
	memcpy(change->serializedPayload.data,m_DPDAsParamList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);

	m_SPDPbPWriter->m_writer_cache.add_change(change);

	CDRMessage::initCDRMsg(&m_DPDMsg);
	RTPSMessageCreator::addHeader(&m_DPDMsg,m_DPD.m_proxy.m_guidPrefix);
	RTPSMessageCreator::addSubmessageInfoTS_Now(&m_DPDMsg,false);

	m_SPDPbPWriter->m_writer_cache.get_last_added_cache(&change);
	RTPSMessageCreator::addSubmessageData(&m_DPDMsg,change,WITH_KEY,c_EntityId_Unknown,NULL);
	m_hasChanged_DPDMsg = false;
	return true;
}



} /* namespace rtps */
} /* namespace eprosima */
