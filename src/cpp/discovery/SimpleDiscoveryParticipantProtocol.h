/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDiscoveryParticipantProtocol.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SIMPLEDISCOVERYPARTICIPANTPROTOCOL_H_
#define SIMPLEDISCOVERYPARTICIPANTPROTOCOL_H_

#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/timedevent/ResendDataPeriod.h"

#include "eprosimartps/discovery/DiscoveredParticipantData.h"

namespace eprosima {
namespace rtps {



class SimpleDiscoveryParticipantProtocol {
public:
	SimpleDiscoveryParticipantProtocol(Participant* p);
	virtual ~SimpleDiscoveryParticipantProtocol();

	bool initSPDP(uint16_t domainId,uint16_t participantId,uint16_t resendDataPeriod_sec);

	bool DPDSerialize(SerializedPayload_t* p_payload);
	bool DPDDeSerialize(DiscoveredParticipantData* p_dpd);
private:
	Participant* mp_Participant;
	StatelessWriter* m_SPDPbPWriter;
	StatelessReader* m_SPDPbPReader;
	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;
	Locator_t m_defaultMulticastLocator;
	uint16_t m_domainId;
	ResendDataPeriod* m_resendData;

	DiscoveredParticipantData m_DPD;


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEDISCOVERYPARTICIPANTPROTOCOL_H_ */
