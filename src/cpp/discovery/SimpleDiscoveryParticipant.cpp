/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDiscoveryParticipant.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/SimpleDiscoveryParticipant.h"

namespace eprosima {
namespace rtps {

SimpleDiscoveryParticipant::SimpleDiscoveryParticipant(Participant*p,uint16_t domainId,uint16_t participantId,uint16_t resendDataPeriodSec):
	m_resendData(&m_SPDPbuiltinParticipantWriter,boost::posix_time::milliseconds(resendDataPeriodSec*1000))
{
	DomainParticipant* dp = DomainParticipant::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getPortBase()
									+ dp->getDomainIdGain() * domainId
									+ dp->getOffsetd0();
	m_SPDP_WELL_KNOWN_UNICAST_PORT = dp->getPortBase()
									+ dp->getDomainIdGain() * domainId
									+ dp->getOffsetd1()
									+ dp->getParticipantIdGain() * participantId;
	m_defaultMulticastLocator.kind = LOCATOR_KIND_UDPv4;
	m_defaultMulticastLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	m_defaultMulticastLocator.set_IP4_address(239,255,0,1);
	m_SPDPbuiltinParticipantWriter.multicastLocatorList.push_back(m_defaultMulticastLocator);
	m_SPDPbuiltinParticipantReader.multicastLocatorList.push_back(m_defaultMulticastLocator);


}

SimpleDiscoveryParticipant::~SimpleDiscoveryParticipant()
{
	// TODO Auto-generated destructor stub
}

} /* namespace rtps */
} /* namespace eprosima */
