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
#include "eprosimartps/discovery/EndpointDiscoveryProtocol.h"

namespace eprosima {
namespace rtps {

class ParticipantDiscoveryProtocol {
public:
	ParticipantDiscoveryProtocol(Participant* p_part);
	virtual ~ParticipantDiscoveryProtocol();

	virtual bool initPDP(const DiscoveryAttributes& attributes,uint32_t participantID)=0;

	DiscoveredParticipantData* mp_localPDP;
	std::vector<DiscoveredParticipantData> m_discoveredParticipants;
	uint16_t m_domainId;
	DiscoveryAttributes m_discovery;
	Participant* mp_participant;
	EndpointDiscoveryProtocol* mp_EDP;


	virtual void announceParticipantState(bool new_change)=0;

	virtual void localParticipantHasChanged()=0;

	virtual bool localWriterMatching(RTPSWriter* W)=0;
	virtual bool localReaderMatching(RTPSReader* R)=0;

	virtual void assignRemoteEndpoints(DiscoveredParticipantData* pdata)=0;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTDISCOVERYPROTOCOL_H_ */
