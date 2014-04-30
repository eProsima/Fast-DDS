/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantProxy.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARTICIPANTPROXY_H_
#define PARTICIPANTPROXY_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/ParameterList.h"

namespace eprosima {
namespace rtps {

class ParticipantProxy {
public:
public:
	ParticipantProxy():
		m_expectsInlineQos(false),
		m_availableBuiltinEndpoints(0),
		m_manualLivelinessCount(0)
	{};
	virtual ~ParticipantProxy(){};
	ProtocolVersion_t m_protocolVersion;
	GuidPrefix_t m_guidPrefix;
	VendorId_t m_VendorId;
	bool m_expectsInlineQos;
	uint32_t m_availableBuiltinEndpoints;
	std::vector<Locator_t> m_metatrafficUnicastLocatorList;
	std::vector<Locator_t> m_metatrafficMulticastLocatorList;
	std::vector<Locator_t> m_defaultUnicastLocatorList;
	std::vector<Locator_t> m_defaultMulticastLocatorList;
	Count_t m_manualLivelinessCount;

};

typedef InstanceHandle_t BuiltinTopicKey_t;


class ParticipantBuiltinTopicData{
public:
	ParticipantBuiltinTopicData();
	virtual ~ParticipantBuiltinTopicData();
	BuiltinTopicKey_t m_builtinTopicKey;
	ParameterUserData_t m_userData;
};


class SPDPdiscoveredParticipantData{
public:
	SPDPdiscoveredParticipantData()
{
		m_leaseDuration.seconds = 60;
		VENDORID_EPROSIMA(m_ParticipantProxy.m_VendorId);
		m_ParticipantProxy.m_expectsInlineQos = false;
		PROTOCOLVERSION(m_ParticipantProxy.m_protocolVersion);
		m_ParticipantProxy.m_availableBuiltinEndpoints
}
	virtual ~SPDPdiscoveredParticipantData();
	Duration_t m_leaseDuration;
	ParticipantProxy m_ParticipantProxy;
	ParticipantBuiltinTopicData m_ParticipantBuiltinTopicData;
	bool serialize(SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t& payload);
};





} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTPROXY_H_ */
