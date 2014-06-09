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
 * Base class of the ParticipantDiscoveryProtocol. Currently only SimplePDP is implemented, please refer to this class for futrther documentation.
 */
class ParticipantDiscoveryProtocol {
public:
	ParticipantDiscoveryProtocol(ParticipantImpl* p_part);
	virtual ~ParticipantDiscoveryProtocol();

	virtual bool initPDP(const DiscoveryAttributes& attributes,uint32_t participantID)=0;

	DiscoveredParticipantData* mp_localDPData;
	std::vector<DiscoveredParticipantData*> m_discoveredParticipants;

	DiscoveryAttributes m_discovery;
	ParticipantImpl* mp_participant;
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
