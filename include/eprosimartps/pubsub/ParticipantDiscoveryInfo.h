/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantDiscoveryInfo.h
 *
 */

#ifndef PARTICIPANTDISCOVERYINFO_H_
#define PARTICIPANTDISCOVERYINFO_H_

#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/qos/QosPolicies.h"

namespace eprosima {
namespace pubsub {

enum DISCOVERY_STATUS
{
	DISCOVERED_PARTICIPANT,
	CHANGED_QOS_PARTICIPANT,
	REMOVED_PARTICIPANT
};

typedef std::vector<std::pair<std::string,std::string>> PropertyList;
typedef std::vector<octet> UserData;

class ParticipantDiscoveryInfo {
public:
	ParticipantDiscoveryInfo(){};
	virtual ~ParticipantDiscoveryInfo(){};
	DISCOVERY_STATUS m_status;
	GUID_t m_guid;
	PropertyList m_propertyList;
	UserData m_userData;
	std::string m_participantName;
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PARTICIPANTDISCOVERYINFO_H_ */
