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

typedef struct EndpointStaticInfo_t{
	std::vector<Locator_t> m_unicastLocatorList;
	std::vector<Locator_t> m_multicastLocatorList;
	bool m_expectsInlineQos;
	StateKind_t m_state;
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
	StaticEndpointDiscoveryProtocol();
	virtual ~StaticEndpointDiscoveryProtocol();
	bool loadStaticEndpointFile();
	bool matchEndpoints(std::string participant_name,GuidPrefix_t& outpartGuidPrefix,Participant* p_Par);
	std::vector<ParticipantStaticInfo_t> m_participants;
	bool printLoadedXMLInfo();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATICENDPOINTDISCOVERYPROTOCOL_H_ */
