/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantAttributes.h
 *
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARTICIPANTPARAMETERS_H_
#define PARTICIPANTPARAMETERS_H_

namespace eprosima {
namespace rtps {

/**
 * Class DiscoveryAttributes used to define the discovery behavior of the Participant.
 * @ingroup ATTRIBUTESMODULE
 */
class DiscoveryAttributes{
public:
	bool use_SIMPLE_ParticipantDiscoveryProtocol;
	bool use_SIMPLE_EndpointDiscoveryProtocol;
	bool use_STATIC_EndpointDiscoveryProtocol;
	uint16_t resendSPDPDataPeriod_sec;
	std::string m_staticEndpointXMLFilename;
	DiscoveryAttributes()
	{
		use_SIMPLE_ParticipantDiscoveryProtocol = false;
		use_SIMPLE_EndpointDiscoveryProtocol = false;
		use_STATIC_EndpointDiscoveryProtocol = false;
		resendSPDPDataPeriod_sec = 30;
		m_staticEndpointXMLFilename = "/home/grcanosa/workspace/eRTPS/utils/pcTests/StaticParticipantInfo.xml";
	};
	virtual ~DiscoveryAttributes();
};

/**
 * Class ParticipantParameters used to define different aspects of a participant.
 * @ingroup ATTRIBUTESMODULE
 */
class ParticipantAttributes {
public:
	ParticipantAttributes()
{
		defaultSendPort = 10040;
		name = "defaultParticipant";
		domainId = 80;
}
	virtual ~ParticipantAttributes();
	LocatorList_t defaultUnicastLocatorList;
	LocatorList_t defaultMulticastLocatorList;
	uint32_t defaultSendPort;
	uint32_t domainId;
	std::string name;
	DiscoveryAttributes discovery;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTPARAMETERS_H_ */
