/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantAttributes.h
 *	Participant Attributes
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
	/**
	 * If set to false, NO discovery whatsoever would be used.
	 * Publisher and Subscriber defined with the same topic name would NOT be linked. All matching must be done
	 * manually through the addReaderLocator, addReaderProxy, addWriterProxy methods.
	 */
	bool use_SIMPLE_ParticipantDiscoveryProtocol;
	/**
	 * If set to true, SimpleEDP would be used.
	 * This is NOT included in release 0.3.
	 */
	bool use_SIMPLE_EndpointDiscoveryProtocol;
	/**
	 * If set to true, StaticEDP based on an XML file would be implemented.
	 * The XML filename must be provided.
	 */
	bool use_STATIC_EndpointDiscoveryProtocol;
	/**
	 * The period for the Participant to send its Discovery Message to all other discovered Participants
	 * as well as to all Multicast ports.
	 */
	Duration_t resendDiscoveryParticipantDataPeriod;
	//! StaticEDP XML filename, only necessary if use_STATIC_EndpointDiscoveryProtocol=true
	std::string m_staticEndpointXMLFilename;
	/**
		 * DomainId to be used by the Participant (80 by default).
		 */
		uint32_t domainId;
		Duration_t leaseDuration;
	DiscoveryAttributes()
	{
		use_SIMPLE_ParticipantDiscoveryProtocol = false;
		use_SIMPLE_EndpointDiscoveryProtocol = false;
		use_STATIC_EndpointDiscoveryProtocol = false;
		resendDiscoveryParticipantDataPeriod.seconds = 30;
		m_staticEndpointXMLFilename = "/home/grcanosa/workspace/eRTPS/utils/pcTests/StaticParticipantInfo.xml";
		domainId = 80;
		leaseDuration.seconds = 100;
	};
	virtual ~DiscoveryAttributes(){};
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

}
	virtual ~ParticipantAttributes(){};
	/**
	 * Default list of Unicast Locators to be used for any Endpoint defined inside this participant in the case
	 * that it was defined with NO UnicastLocators. At least ONE locator should be included in this list.
	 */
	LocatorList_t defaultUnicastLocatorList;
	/**
		 * Default list of Multicast Locators to be used for any Endpoint defined inside this participant in the case
		 * that it was defined with NO UnicastLocators. This is usually left empty.
		 */
	LocatorList_t defaultMulticastLocatorList;
	/**
	 * Default send port that all Endpoints in the Participant would use to send messages.
	 * In this release all Endpoints use the same resource (socket) to send messages.
	 * THis will change in future releases.
	 */
	uint32_t defaultSendPort;

	/**
	 * Participant name.
	 */
	std::string name;
	//! Discovery parameters.
	DiscoveryAttributes discovery;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTPARAMETERS_H_ */
