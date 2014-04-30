/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StaticEndpointDiscoveryProtocol.h
 *
 *  Created on: Apr 23, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef STATICENDPOINTDISCOVERYPROTOCOL_H_
#define STATICENDPOINTDISCOVERYPROTOCOL_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"
namespace eprosima {
namespace rtps {

class RTPSWriter;
class RTPSReader;
class Endpoint;


typedef struct EndpointStaticInfo_t{
LocatorList_t m_unicastLocatorList;
LocatorList_t m_multicastLocatorList;
	bool m_expectsInlineQos;
	StateKind_t m_state;
	ReliabilityKind_t m_reliability;
	HistoryKind_t m_kind;
	std::string m_topicName;
	TopicKind_t m_topicKind;
	int16_t m_id;
}EndpointStaticInfo_t;


typedef struct ParticipantStaticInfo_t{
	std::string m_name;
	std::vector<EndpointStaticInfo_t> m_endpoints;
}ParticipantStaticInfo_t;

class Participant;

class StaticEndpointDiscoveryProtocol {
public:
	StaticEndpointDiscoveryProtocol(Participant* p_par);
	virtual ~StaticEndpointDiscoveryProtocol();
	bool loadStaticEndpointFile(const std::string& filename);
	bool loadStaticEndpointFile();
	std::vector<ParticipantStaticInfo_t> m_StaticParticipantInfo;
	Participant* mp_Participant;
	bool printLoadedXMLInfo();

	bool localEndpointMatching(Endpoint* endpoint, char type);
	bool localEndpointMatching(Endpoint* endpoint,DiscoveredParticipantData* dpd,char type);
	bool localWriterMatching(RTPSWriter* pwriter,DiscoveredParticipantData* dpd);
	bool localReaderMatching(RTPSReader* preader,DiscoveredParticipantData* dpd);

	std::string m_staticEndpointFilename;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATICENDPOINTDISCOVERYPROTOCOL_H_ */
