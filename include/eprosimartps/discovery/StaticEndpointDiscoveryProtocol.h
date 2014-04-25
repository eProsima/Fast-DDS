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

namespace eprosima {
namespace rtps {

class RTPSWriter;
class RTPSReader;


typedef struct EndpointStaticInfo_t{
LocatorList_t m_unicastLocatorList;
LocatorList_t m_multicastLocatorList;
	bool m_expectsInlineQos;
	StateKind_t m_state;
	ReliabilityKind_t m_reliability;
	HistoryKind_t m_kind;
	std::string m_topicName;
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
	bool remoteParticipantMatching(std::string participant_name,GuidPrefix_t& outpartGuidPrefix,Participant* p_Par);
	bool localWriterMatching(RTPSWriter* writer);
	bool localReaderMatching(RTPSReader* reader);
	std::vector<ParticipantStaticInfo_t> m_StaticParticipantInfo;
	Participant* mp_Participant;
	bool printLoadedXMLInfo();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATICENDPOINTDISCOVERYPROTOCOL_H_ */
