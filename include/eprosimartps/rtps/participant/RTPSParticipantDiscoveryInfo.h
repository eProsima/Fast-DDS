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

#ifndef RTPSPARTICIPANTDISCOVERYINFO_H_
#define RTPSPARTICIPANTDISCOVERYINFO_H_

namespace eprosima{
namespace rtps{


enum DISCOVERY_STATUS
{
	DISCOVERED_RTPSPARTICIPANT,
	CHANGED_QOS_RTPSPARTICIPANT,
	REMOVED_RTPSPARTICIPANT
};

typedef std::vector<std::pair<std::string,std::string>> PropertyList;
typedef std::vector<octet> UserData;

class RTPSParticipantDiscoveryInfo {
public:
	RTPSParticipantDiscoveryInfo():m_status(DISCOVERED_RTPSPARTICIPANT){};
	virtual ~RTPSParticipantDiscoveryInfo(){};
	DISCOVERY_STATUS m_status;
	GUID_t m_guid;
	PropertyList m_propertyList;
	UserData m_userData;
	std::string m_RTPSParticipantName;
};
}
}



#endif /* RTPSPARTICIPANTDISCOVERYINFO_H_ */
