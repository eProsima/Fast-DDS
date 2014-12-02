/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipantAttributes.h 	
 */

#ifndef RTPSParticipantPARAMETERS_H_
#define RTPSParticipantPARAMETERS_H_

#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/common/Locator.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

/**
 * Attributes of the Simple Endpoint Discovery Protocol.
 */
class SimpleEDPAttributes{
public:
	bool use_PublicationWriterANDSubscriptionReader;
	bool use_PublicationReaderANDSubscriptionWriter;
	SimpleEDPAttributes():
		use_PublicationWriterANDSubscriptionReader(true),
		use_PublicationReaderANDSubscriptionWriter(true)
	{

	}
};

/**
 * Port Parameters related with the RTPS protocol.
 */
class PortParameters
{
public:
	PortParameters()
	{
		portBase = 7400;
		participantIDGain = 2;
		domainIDGain = 250;
		offsetd0 = 0;
		offsetd1 = 10;
		offsetd2 = 1;
		offsetd3 = 11;
	};
	virtual ~PortParameters(){}
	/**
	 * Get a multicast port based on the domain ID.
	 *
	 * @param domainId Domain ID.
	 * @return Multicast port
	 */
	inline uint32_t getMulticastPort(uint32_t domainId)
	{
		return portBase+ domainIDGain * domainId+ offsetd0;
	}
	/**
	 * Get a unicast port baes on the domain ID and the participant ID.
	 *
 	 * @param domainId Domain ID.
	 * @param RTPSParticipantID Participant ID.
	 * @return Unicast port
	 */
	inline uint32_t getUnicastPort(uint32_t domainId,uint32_t RTPSParticipantID)
	{
		return portBase+ domainIDGain * domainId	+ offsetd1	+ participantIDGain * RTPSParticipantID;
	}

	uint16_t portBase;
	uint16_t domainIDGain;
	uint16_t participantIDGain;
	uint16_t offsetd0;
	uint16_t offsetd1;
	uint16_t offsetd2;
	uint16_t offsetd3;
};

/**
 * Class BuiltinAttributes used to define the behavior of the RTPSParticipant builtin protocols.
 * @ingroup ATTRIBUTESMODULE
 */
	class BuiltinAttributes{
public:
	/**
	 * If set to false, NO discovery whatsoever would be used.
	 * Publisher and Subscriber defined with the same topic name would NOT be linked. All matching must be done
	 * manually through the addReaderLocator, addReaderProxy, addWriterProxy methods.
	 */
	bool use_SIMPLE_RTPSParticipantDiscoveryProtocol;

	//!Indicates to use the WriterLiveliness protocol.
	bool use_WriterLivelinessProtocol;
	/**
	 * If set to true, SimpleEDP would be used.
	 */
	bool use_SIMPLE_EndpointDiscoveryProtocol;
	/**
	 * If set to true, StaticEDP based on an XML file would be implemented.
	 * The XML filename must be provided.
	 */
	bool use_STATIC_EndpointDiscoveryProtocol;
	
	/**
	 * DomainId to be used by the RTPSParticipant (80 by default).
	 */
	uint32_t domainId;
	//!Lease Duration of the RTPSParticipant, indicating how much time remote RTPSParticipants should consider this RTPSParticipant alive.
	Duration_t leaseDuration;
	/**
	 * The period for the RTPSParticipant to send its Discovery Message to all other discovered RTPSParticipants
	 * as well as to all Multicast ports.
	 */
	Duration_t leaseDuration_announcementperiod;
	//!Attributes of the SimpleEDP protocol
	SimpleEDPAttributes m_simpleEDP;
	//!Metatraffic Unicast Locator List
	LocatorList_t metatrafficUnicastLocatorList;
	//!Metatraffic Multicast Locator List.
	LocatorList_t metatrafficMulticastLocatorList;

	BuiltinAttributes()
	{
		use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
		use_SIMPLE_EndpointDiscoveryProtocol = true;
		use_STATIC_EndpointDiscoveryProtocol = false;
		m_staticEndpointXMLFilename = "";
		domainId = 80;
		leaseDuration.seconds = 500;
		leaseDuration_announcementperiod.seconds = 250;
		use_WriterLivelinessProtocol = true;
		
	};
	virtual ~BuiltinAttributes(){};
	/**
	* Get the static endpoint XML filename
	* @return Static endpoint XML filename
	*/
	const char* getStaticEndpointXMLFilename(){ return m_staticEndpointXMLFilename.c_str(); };
	/**
	* Set the static endpoint XML filename
	* @param str Static endpoint XML filename
	*/
	void setStaticEndpointXMLFilename(const char* str){ m_staticEndpointXMLFilename = std::string(str); };
private:
		//! StaticEDP XML filename, only necessary if use_STATIC_EndpointDiscoveryProtocol=true
		std::string m_staticEndpointXMLFilename;
};



/**
 * Class RTPSParticipantParameters used to define different aspects of a RTPSParticipant.
 * @ingroup ATTRIBUTESMODULE
 */
class RTPSParticipantAttributes {
public:
	RTPSParticipantAttributes()
{
		defaultSendPort = 10040;
		setName("RTPSParticipant");
		sendSocketBufferSize = 8712;
		listenSocketBufferSize = 17424;
		use_IP4_to_send = true;
		use_IP6_to_send = false;
		participantID = -1;
}
	virtual ~RTPSParticipantAttributes(){};
	/**
	 * Default list of Unicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the case
	 * that it was defined with NO UnicastLocators. At least ONE locator should be included in this list.
	 */
	LocatorList_t defaultUnicastLocatorList;
	/**
	 * Default list of Multicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the case
	 * that it was defined with NO UnicastLocators. This is usually left empty.
	 */
	LocatorList_t defaultMulticastLocatorList;
	/**
	 * Default send port that all Endpoints in the RTPSParticipant would use to send messages.
	 * In this release all Endpoints use the same resource (socket) to send messages.
	 * THis will change in future releases.
	 */
	uint32_t defaultSendPort;
	//!Send socket buffer for the send resource.
	uint32_t sendSocketBufferSize;
	//!Listen socket buffer for all listen resources.
	uint32_t listenSocketBufferSize;
	//! Builtin parameters.
	BuiltinAttributes builtin;
	//!Port Parameters
	PortParameters port;
	//!User Data of the participant
	std::vector<octet> userData;
	//!Participant ID
	int32_t participantID;
	//!Use IP4 to send messages.
	bool use_IP4_to_send;
	//!Use IP6 to send messages.
	bool use_IP6_to_send;
	//!Set the name of the participant.
	inline void setName(const char* nam){name = nam;}
	//!Get the name of the participant.
	inline const char* getName(){return name.c_str();}

private:
	std::string name;


};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSParticipantPARAMETERS_H_ */
