/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipantDiscoveryInfo.h
 *
 */

#ifndef RTPSParticipantDISCOVERYINFO_H_
#define RTPSParticipantDISCOVERYINFO_H_

#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/qos/QosPolicies.h"

namespace eprosima {
namespace pubsub {

enum DISCOVERY_STATUS
{
	DISCOVERED_RTPSParticipant,
	CHANGED_QOS_RTPSParticipant,
	REMOVED_RTPSParticipant
};

typedef std::vector<std::pair<std::string,std::string>> PropertyList;
typedef std::vector<octet> UserData;

class RTPSParticipantDiscoveryInfo {
public:
	RTPSParticipantDiscoveryInfo(){};
	virtual ~RTPSParticipantDiscoveryInfo(){};
	DISCOVERY_STATUS m_status;
	GUID_t m_guid;
	PropertyList m_propertyList;
	UserData m_userData;
	std::string m_RTPSParticipantName;
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* RTPSParticipantDISCOVERYINFO_H_ */
