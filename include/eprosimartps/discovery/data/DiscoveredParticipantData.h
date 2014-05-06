/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredParticipantData.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DISCOVEREDPARTICIPANTDATA_H_
#define DISCOVEREDPARTICIPANTDATA_H_

#include "eprosimartps/discovery/data/ParticipantProxy.h"
#include "eprosimartps/qos/ParameterList.h"

namespace eprosima {
namespace rtps {

#define DISCOVERY_PARTICIPANT_DATA_MAX_SIZE 500




typedef struct ParticipantBuiltinTopicData{
	InstanceHandle_t m_key;
//	ParameterUserData_t m_user_data;
}ParticipantBuiltinTopicData;

/**
 * Class DiscoveredParticipantData used by the SPDP.
 */
class DiscoveredParticipantData {
public:
	DiscoveredParticipantData(){};
	virtual ~DiscoveredParticipantData(){};
	ParticipantProxy m_proxy;
	ParticipantBuiltinTopicData m_topicData;
	Duration_t leaseDuration;
	std::vector<std::pair<uint16_t,EntityId_t>> m_staticedpEntityId;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDPARTICIPANTDATA_H_ */
