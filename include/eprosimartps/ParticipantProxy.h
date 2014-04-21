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


namespace eprosima {
namespace rtps {

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



class ParticipantProxy {
public:
	ParticipantProxy():
		m_expectsInlineQos(false),
		m_availableBuiltinEndpoints(0),
		m_manualLivelinessCount(0)
	{
		VENDORID_EPROSIMA(m_VendorId);
		PROTOCOLVERSION(m_protocolVersion);
		m_participantName = "undefined";
		//TODOG FIX AVAILABLE BUILTIN ENDPOINTS
//		m_availableBuiltinEndpoints &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
//		m_availableBuiltinEndpoints &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
//		m_availableBuiltinEndpoints &= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
//		m_availableBuiltinEndpoints &= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
//		m_availableBuiltinEndpoints &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
//		m_availableBuiltinEndpoints &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;

	};
	virtual ~ParticipantProxy()
	{

	};
	ProtocolVersion_t m_protocolVersion;
	GuidPrefix_t m_guidPrefix;
	VendorId_t m_VendorId;
	bool m_expectsInlineQos;
	BuiltinEndpointSet_t m_availableBuiltinEndpoints;
	std::vector<Locator_t> m_metatrafficUnicastLocatorList;
	std::vector<Locator_t> m_metatrafficMulticastLocatorList;
	std::vector<Locator_t> m_defaultUnicastLocatorList;
	std::vector<Locator_t> m_defaultMulticastLocatorList;
	Count_t m_manualLivelinessCount;
	std::string m_participantName;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTPROXY_H_ */
