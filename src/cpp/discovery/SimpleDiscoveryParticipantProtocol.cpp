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
		mp_Participant(p)
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

	Locator_t locator;
	locator.kind = LOCATOR_KIND_UDPv4;
	locator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	locator.set_IP4_address(239,255,0,1);
	m_DPD.m_proxy.m_metatrafficMulticastLocatorList.push_back(locator);

	std::vector<Locator_t> locators;
	DomainParticipant::getIPAddress(&locators);
	for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
	{
		locator.port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
	}

	WriterParams_t Wparam;
	Wparam.pushMode = true;
	Wparam.historySize = 1;
	Wparam.multicastLocatorList.push_back(m_defaultMulticastLocator);
	Wparam.unicastLocatorList.push_back(m_defaultUnicastLocator);
	Wparam.topicName = "DCPSParticipant";
	Wparam.topicDataType = "DiscoveredParticipantData";
	Wparam.topicKind = WITH_KEY;
	mp_Participant->createStatelessWriter(&m_SPDPbPWriter,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	//m_SPDPbPWriter = new StatelessWriter(Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	ReaderParams_t Rparam;
	Rparam.multicastLocatorList.push_back(m_defaultMulticastLocator);
	Rparam.topicKind = WITH_KEY;
	Rparam.topicName = "DCPSParticipant";
	Rparam.topicDataType = "DiscoveredParticipantData";
//	m_SPDPbPReader = new StatelessReader(Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	mp_Participant->createStatelessReader(&m_SPDPbPReader,Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	m_DPD.m_proxy.m_guidPrefix = mp_Participant->m_guid.guidPrefix;



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



	ParameterList::updateCDRMsg(&m_DPDAsParamList.allQos,EPROSIMA_ENDIAN);
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
