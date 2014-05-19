/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredParticipantData.h
 *	DiscoveredParticipantData
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DISCOVEREDPARTICIPANTDATA_H_
#define DISCOVEREDPARTICIPANTDATA_H_

#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/discovery/data/DiscoveredReaderData.h"
#include "eprosimartps/discovery/data/DiscoveredWriterData.h"
#include "eprosimartps/discovery/data/DiscoveredTopicData.h"

namespace eprosima {
namespace rtps {

#define DISCOVERY_PARTICIPANT_DATA_MAX_SIZE 500
#define DISCOVERY_TOPIC_DATA_MAX_SIZE 400
#define DISCOVERY_PUBLICATION_DATA_MAX_SIZE 600
#define DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE 600

#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER 0x00000001 << 0;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR 0x00000001 << 1;
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER 0x00000001 << 2;
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR 0x00000001 << 3;
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER 0x00000001 << 4;
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR 0x00000001 << 5;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER 0x00000001 << 6;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR 0x00000001 << 7;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER 0x00000001 << 8;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR 0x00000001 << 9;
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER 0x00000001 << 10;
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER 0x00000001 << 11;



/**
 * Class DiscoveredParticipantData used by the SPDP.
 */
class DiscoveredParticipantData {
public:
	DiscoveredParticipantData():
		m_expectsInlineQos(false),
		m_availableBuiltinEndpoints(0),
		m_manualLivelinessCount(0),
		isAlive(false)
{

};
	virtual ~DiscoveredParticipantData(){};
	ProtocolVersion_t m_protocolVersion;
	GuidPrefix_t m_guidPrefix;
	VendorId_t m_VendorId;
	bool m_expectsInlineQos;
	BuiltinEndpointSet_t m_availableBuiltinEndpoints;
	LocatorList_t m_metatrafficUnicastLocatorList;
	LocatorList_t m_metatrafficMulticastLocatorList;
	LocatorList_t m_defaultUnicastLocatorList;
	LocatorList_t m_defaultMulticastLocatorList;
	Count_t m_manualLivelinessCount;
	std::string m_participantName;
	InstanceHandle_t m_key;
	Duration_t leaseDuration;

	std::vector<DiscoveredWriterData> m_writers;
	std::vector<DiscoveredReaderData> m_readers;
	std::vector<DiscoveredTopicData> m_topics;

	bool isAlive;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDPARTICIPANTDATA_H_ */
