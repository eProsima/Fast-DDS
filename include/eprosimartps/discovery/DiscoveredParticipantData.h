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

#include "eprosimartps/ParticipantProxy.h"
#include "eprosimartps/dds/ParameterList.h"

namespace eprosima {
namespace rtps {

#define DISCOVERY_PARTICIPANT_DATA_MAX_SIZE 400


typedef InstanceHandle_t BuiltinTopicKey_t;

typedef struct ParticipantBuiltinTopicData{
	BuiltinTopicKey_t m_key;
	ParameterUserData_t m_user_data;
}ParticipantBuiltinTopicData;


class DiscoveredParticipantData {
public:
	DiscoveredParticipantData(){};
	virtual ~DiscoveredParticipantData(){};
	ParticipantProxy m_proxy;
	ParticipantBuiltinTopicData m_topicData;
	Duration_t leaseDuration;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDPARTICIPANTDATA_H_ */
