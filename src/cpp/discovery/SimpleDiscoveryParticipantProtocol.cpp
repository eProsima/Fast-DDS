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

	m_defaultMulticastLocator.kind = LOCATOR_KIND_UDPv4;
	m_defaultMulticastLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	m_defaultMulticastLocator.set_IP4_address(239,255,0,1);

	WriterParams_t Wparam;
	Wparam.pushMode = true;
	Wparam.historySize = 1;
	Wparam.multicastLocatorList.push_back(m_defaultMulticastLocator);
	Wparam.topicName = "DCPSParticipant";
	Wparam.topicDataType = "DiscoveredParticipantData";
	Wparam.topicKind = WITH_KEY;
	m_SPDPbPWriter = new StatelessWriter(Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	ReaderParams_t Rparam;
	Rparam.multicastLocatorList.push_back(m_defaultMulticastLocator);
	Rparam.topicKind = WITH_KEY;
	Rparam.topicName = "DCPSParticipant";
	Rparam.topicDataType = "DiscoveredParticipantData";
	m_SPDPbPReader = new StatelessReader(Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);

	m_DPD.m_proxy.m_guidPrefix = mp_Participant->m_guid.guidPrefix;



	m_resendData = new ResendDataPeriod((&m_SPDPbPWriter,
					boost::posix_time::milliseconds(resendDataPeriod_sec*1000)));
	boost::system::error_code ec_sucess(boost::system::errc::success);
	m_resendData->event(ec_sucess);
	return true;
}

bool SimpleDiscoveryParticipantProtocol::DPDSerialize(
		SerializedPayload_t* p_payload)
{
	CDRMessage_t msg;
	if(EPROSIMA_ENDIAN == BIGEND)
	{
		p_payload->encapsulation = PL_CDR_BE;
		msg.msg_endian = BIGEND;
	}
	else if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		p_payload->encapsulation = PL_CDR_LE;
		msg.msg_endian = LITTLEEND;
	}
	else
		return false;
	CDRMessage::addUInt16(&msg,PID_PARTICIPANT_GUID);
	CDRMessage::addUInt16(&msg,16);
	CDRMessage::addData(&msg,mp_Participant->m_guid.guidPrefix.value,12);
	CDRMessage::addData(&msg,mp_Participant->m_guid.entityId.value,4);
	CDRMessage::addUInt16(&msg,PID_BUILTIN_ENDPOINT_SET);
	CDRMessage::addUInt16(&msg,4);
	CDRMessage::addUInt32(&msg,m_DPD.m_proxy.m_availableBuiltinEndpoints);
	CDRMessage::addUInt16(&msg,PID_PROTOCOL_VERSION);
	CDRMessage::addUInt16(&msg,4);
	CDRMessage::addOctet(&msg,m_DPD.m_proxy.m_protocolVersion.m_major);
	CDRMessage::addOctet(&msg,m_DPD.m_proxy.m_protocolVersion.m_minor);
	CDRMessage::addUInt16(&msg,0);
	CDRMessage::addUInt16(&msg,PID_DEFAULT_UNICAST_LOCATOR);
	CDRMessage::addUInt16(&msg,24);
	CDRMessage::addLocator(&msg,&m_DPD.m_proxy.m_defaultUnicastLocatorList[0]);




}

bool SimpleDiscoveryParticipantProtocol::DPDDeSerialize(
		DiscoveredParticipantData* p_dpd)
{

}

} /* namespace rtps */
} /* namespace eprosima */
