/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StaticEndpointDiscoveryProtocol.h
 *
 *  Created on: Apr 23, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef STATICENDPOINTDISCOVERYPROTOCOL_H_
#define STATICENDPOINTDISCOVERYPROTOCOL_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"
namespace eprosima {
namespace rtps {

class RTPSWriter;
class RTPSReader;
class Endpoint;

/**
 * Structure EndpointStaticInfo_t, used to contain the endpoint information defined in the XML file.
 * @ingroup DISCOVERYMODULE
 */
typedef struct EndpointStaticInfo_t{
	LocatorList_t m_unicastLocatorList;
	LocatorList_t m_multicastLocatorList;
	bool m_expectsInlineQos;
	StateKind_t m_state;
	ReliabilityKind_t m_reliability;
	HistoryKind_t m_kind;
	std::string m_topicName;
	TopicKind_t m_topicKind;
	int16_t m_id;
	std::string m_topicDataType;
	EndpointStaticInfo_t()
	{
		m_state = STATELESS;
		m_expectsInlineQos= false;
		m_reliability = BEST_EFFORT;
		m_kind = READER;
		m_topicName = "UNDEFINED";
		m_topicKind = NO_KEY;
		m_topicDataType = "UNDEFINED";
		m_id = -1;
	}
}EndpointStaticInfo_t;

/**
 * Structure ParticipantStaticInfo_t, used to contain the participant information defined in the XML file.
 * @ingroup DISCOVERYMODULE
 */
typedef struct ParticipantStaticInfo_t{
	std::string m_name;
	std::vector<EndpointStaticInfo_t> m_endpoints;
}ParticipantStaticInfo_t;

class Participant;

/**
 * Class StaticEndpointDiscoveryProtocol, that implements the Static Endpoint Discovery.
 * @ingroup DISCOVERYMODULE
 */
class StaticEndpointDiscoveryProtocol {
public:
	StaticEndpointDiscoveryProtocol(Participant* p_par);
	virtual ~StaticEndpointDiscoveryProtocol();
	/**
	 * Load a StaticEndpoint XML file, providing a name.
	 * @param[in] filename Name of the file to be loaded.
	 * @return True if correct.
	 */
	bool loadStaticEndpointFile(const std::string& filename);
	/**
	 * Load the Static Endpoint XML file that is stored in the class.
	 * @return True if correct.
	 */
	bool loadStaticEndpointFile();
	//! Vector containing a ParticipantStaticInfo_t for each participant defined in the XML file.
	std::vector<ParticipantStaticInfo_t> m_StaticParticipantInfo;
	//! Pointer to the associated local Participant.
	Participant* mp_Participant;
	//! Print the loaded function.
	bool printLoadedXMLInfo();
	/**
	 * Match a local endpoint with all discovered participants, including the local participant.
	 * @param[in,out] endpoint Pointer to the endpoint.
	 * @param[in] type Type of the endpoint (W o R)
	 * @return True if correct.
	 */
	bool localEndpointMatching(Endpoint* endpoint, char type);
	/**
	 * Match a local endpoint to a specific Participant described by a DiscoveredParticipantData object.
	 * @param[in,out] endpoint Pointer to the endpoint.
	 * @param[in] dpd Pointer to the DPD object.
	 * @param[in] type Type of the endpoint
	 * @return True if correct.
	 */
	bool localEndpointMatching(Endpoint* endpoint,DiscoveredParticipantData* dpd,char type);
	/**
	 * Match a Writer against a specific participant.
	 * @param pwriter Pointer to the Writer.
	 * @param dpd Pointer to the Participant Data.
	 * @return True if correct.
	 */
	bool localWriterMatching(RTPSWriter* pwriter,DiscoveredParticipantData* dpd);
	/**
	 * Match a Reader against a specific participant.
	 * @param preader Pointer to the Reader.
	 * @param dpd Pointer to the Participant Data.
	 * @return True if correct.
	 */
	bool localReaderMatching(RTPSReader* preader,DiscoveredParticipantData* dpd);
    //!StaticEndpoint filename, defined in ParticipantAttributes when the participant is created.
	std::string m_staticEndpointFilename;
	//!Endpint Ids as being read from the XML. This is used to avoid duplicity of endpoint ID in the same XML file.
	std::vector<int16_t> m_endpointIds;

	/**
	 * Check the PublisherAttributes to check if they match those defined in the
	 * XML file.
	 * If the current participant is defined in the XML file, then the Publisher should be defined
	 * as in the XML file. A warning is issued if this is not the case.
	 * If the participant is not included, then the function returns true since no information is available.
	 * @param[in] wparam PublisherAttributes to check agains the xml file.
	 * @return True if correct.
	 */
	bool checkLocalWriterCreation(PublisherAttributes& wparam);
	/**
	 * Check the SubscriberAttributes to check if they match those defined in the
	 * XML file.
	 * If the current participant is defined in the XML file, then the Publisher should be defined
	 * as in the XML file. A warning is issued if this is not the case.
	 * If the participant is not included, then the function returns true since no information is available.
	 * @param[in] rparam SubscriberAttributes to check agains the xml file.
	 * @return True if correct.
	 */
	bool checkLocalReaderCreation(SubscriberAttributes& rparam);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATICENDPOINTDISCOVERYPROTOCOL_H_ */
