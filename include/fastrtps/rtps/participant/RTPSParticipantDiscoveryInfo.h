// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file RTPSParticipantDiscoveryInfo.h
 *
 */

#ifndef RTPSPARTICIPANTDISCOVERYINFO_H_
#define RTPSPARTICIPANTDISCOVERYINFO_H_

#include <vector>
#include "../common/Types.h"
#include "../common/Guid.h"
namespace eprosima{
namespace fastrtps{
namespace rtps{

//!Enum DISCOVERY_STATUS, three different status for discovered participants.
//!@ingroup RTPS_MODULE
#if defined(_WIN32)
enum RTPS_DllAPI DISCOVERY_STATUS
#else
enum  DISCOVERY_STATUS
#endif
{
	DISCOVERED_RTPSPARTICIPANT,
	CHANGED_QOS_RTPSPARTICIPANT,
	REMOVED_RTPSPARTICIPANT
};

typedef std::vector<std::pair<std::string,std::string>> PropertyList;
typedef std::vector<octet> UserData;

/**
* Class RTPSParticipantDiscoveryInfo with discovery information of the RTPS Participant.
* @ingroup RTPS_MODULE
*/
class RTPSParticipantDiscoveryInfo{
public:
	RTPSParticipantDiscoveryInfo():m_status(DISCOVERED_RTPSPARTICIPANT){};
	virtual ~RTPSParticipantDiscoveryInfo(){};
	//!Status
	DISCOVERY_STATUS m_status;
	//!Associated GUID
	GUID_t m_guid;
	//!Property list
	PropertyList m_propertyList;
	//!User data
	UserData m_userData;
	//!Participant name
	std::string m_RTPSParticipantName;
};
}
}
}



#endif /* RTPSPARTICIPANTDISCOVERYINFO_H_ */
