/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDiscoveryParticipant.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SIMPLEDISCOVERYPARTICIPANT_H_
#define SIMPLEDISCOVERYPARTICIPANT_H_

#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/timedevent/ResendDataPeriod.h"
#include "eprosimartps/discovery/ParticipantDiscoveryData.h"

namespace eprosima {
namespace rtps {

#define ERTPS_SPDP_RESEND_DATA_PERIOD_SECONDS 30;


class SimpleDiscoveryParticipant {
public:
	SimpleDiscoveryParticipant(Participant*p,uint16_t domainId,uint16_t participantId,uint16_t resendDataPeriodSec);
	virtual ~SimpleDiscoveryParticipant();


private:
	StatelessWriter m_SPDPbuiltinParticipantWriter;
	StatelessReader m_SPDPbuiltinParticipantReader;
	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;
	Locator_t m_defaultMulticastLocator;
	ResendDataPeriod m_resendData;
	SPDPdiscoveredParticipantData m_SPDPdiscoveredParticipantData;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEDISCOVERYPARTICIPANT_H_ */
