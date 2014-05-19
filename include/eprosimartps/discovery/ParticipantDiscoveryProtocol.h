/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantDiscoveryProtocol.h
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARTICIPANTDISCOVERYPROTOCOL_H_
#define PARTICIPANTDISCOVERYPROTOCOL_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"

namespace eprosima {
namespace rtps {

class ParticipantDiscoveryProtocol {
public:
	ParticipantDiscoveryProtocol(Participant* p_part);
	virtual ~ParticipantDiscoveryProtocol();

	virtual bool initDPD(DiscoveryAttributes& attributes);

	DiscoveredParticipantData* mp_localDPD;
	std::vector<DiscoveredParticipantData> m_discoveredParticipants;
	uint16_t m_domainId;
	DiscoveryAttributes m_discovery;
	Participant* mp_participant;
	EndpointDiscoveryProtocol* mp_EDP;

	bool addLocalParticipant(Participant* p)=0;

	void announceParticipantState()=0;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTDISCOVERYPROTOCOL_H_ */
