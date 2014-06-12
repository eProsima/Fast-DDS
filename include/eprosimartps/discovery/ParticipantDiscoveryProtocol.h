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
*/

#ifndef PARTICIPANTDISCOVERYPROTOCOL_H_
#define PARTICIPANTDISCOVERYPROTOCOL_H_


#include "eprosimartps/dds/attributes/TopicAttributes.h"
#include "eprosimartps/dds/attributes/ParticipantAttributes.h"
#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"


namespace eprosima {
namespace rtps {

class EndpointDiscoveryProtocol;
class ParticipantImpl;
class RTPSWriter;
class RTPSReader;

/**
 * Base class of the ParticipantDiscoveryProtocol. Currently only SimplePDP is implemented, please refer to this class for further documentation.
 * @ingroup DISCOVERYMODULE
 */
class ParticipantDiscoveryProtocol {
public:
	ParticipantDiscoveryProtocol(ParticipantImpl* p_part);
	virtual ~ParticipantDiscoveryProtocol();
	/**
	 * Initialize the PDP.
	 * @param attributes DiscoveryAttributes reference.
	 * @param participantID Id of the participant to obtain the default port numbers.
	 * @return True if correct.
	 */
	virtual bool initPDP(const DiscoveryAttributes& attributes,uint32_t participantID)=0;
	//! Pointer to the DPD data object of the local Participant.
	DiscoveredParticipantData* mp_localDPData;
	//! Vector containing pointers to all DPD objects for all discovered participants.
	std::vector<DiscoveredParticipantData*> m_discoveredParticipants;
	//! Discovery Attributes.
	DiscoveryAttributes m_discovery;
	//! Pointer to the local Participant Implementation
	ParticipantImpl* mp_participant;
	//! Pointer to the EndpointDiscoveryProtocol used.
	EndpointDiscoveryProtocol* mp_EDP;

	virtual void announceParticipantState(bool new_change)=0;
	virtual void stopParticipantAnnouncement()=0;
	virtual void resetParticipantAnnouncement()=0;
	virtual void localParticipantHasChanged()=0;
	virtual bool localWriterMatching(RTPSWriter* W,bool first_time)=0;
	virtual bool localReaderMatching(RTPSReader* R,bool first_time)=0;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTDISCOVERYPROTOCOL_H_ */
