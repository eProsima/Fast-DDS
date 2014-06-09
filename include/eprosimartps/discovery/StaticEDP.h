/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StaticEDP.h
 *
 */

#ifndef STATICEDP_H_
#define STATICEDP_H_

#include "eprosimartps/discovery/EndpointDiscoveryProtocol.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "eprosimartps/dds/attributes/ParticipantAttributes.h"



using boost::property_tree::ptree;


namespace eprosima {
namespace rtps {

class RTPSWriter;
class RTPSReader;
class DiscoveredParticipantData;

/**
 * Static EndpointDiscoveryProtocol that loads the characteristics of the Endpoitns from a XML file.
 */
class StaticEDP: public EndpointDiscoveryProtocol {
public:
	StaticEDP(ParticipantDiscoveryProtocol*p);
	virtual ~StaticEDP();
	/**
	 * Initializa the EDP.
	 * @param attributes DiscoveryAttributes reference.
	 * @return True if correctly initialized.
	 */
	bool initEDP(DiscoveryAttributes& attributes);

	/**
		 * Match a local Writer against all possible remote and local Readers with the same topic.
		 * @param writer Pointer to the Writer.
		 * @param first_time Whether or not is the first time (to create the corresponding DWD object).
		 * @return True if correct.
		 */
	bool localWriterMatching(RTPSWriter* writer,bool first_time);
	/**
		 * Match a local Reader against all possible remote and local Readers with the same topic.
		 * @param writer Pointer to the Reader.
		 * @param first_time Whether or not is the first time (to create the corresponding DRD object).
		 * @return True if correct.
		 */
	bool localReaderMatching(RTPSReader* reader,bool first_time);

	/**
	 * Load an XML file and parse its contents.
	 * @param filename Name of the file to load.
	 * @return True if correctly loaded and parsed.
	 */
	bool loadXMLFile(const std::string& filename);
	/**
	 * Load a Writer Endpoint from the XML file and save the data to the DPD data object.
	 * @param xml_endpoint Boost xml data.
	 * @param pdata Pointer to the DPD object.
	 * @return True if correct.
	 */
	bool loadXMLWriterEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata);
	/**
	 * Load a Reader Endpoint from the XML file and save the data to the DPD data object.
	 * @param xml_endpoint Boost xml data.
	 * @param pdata Pointer to the DPD object.
	 * @return True if correct.
	 */
	bool loadXMLReaderEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata);
	//!This method is not used in StaticEDP, is needed because is an abastract method of its base class.
	void assignRemoteEndpoints(DiscoveredParticipantData* pdata) {};

	//!Vector that stores the loaded Endpoints Ids in case one is repeated.
	std::vector<uint16_t> m_endpointIds;


	/**
	 * Check the writer to check if it match the one defined in the XML file.
	 * If the current participant is defined in the XML file, then the Publisher should be defined
	 * as in the XML file. A warning is issued if this is not the case.
	 * If the participant is not included, then the function returns true since no information is available.
	 * @param[in] wparam PublisherAttributes to check agains the xml file.
	 * @return True if correct.
	 */
	bool checkLocalWriterCreation(RTPSWriter* W);
	/**
	 * Check the writer to check if it match the one defined in the XML file.
	 * If the current participant is defined in the XML file, then the Publisher should be defined
	 * as in the XML file. A warning is issued if this is not the case.
	 * If the participant is not included, then the function returns true since no information is available.
	 * @param[in] rparam SubscriberAttributes to check agains the xml file.
	 * @return True if correct.
	 */
	bool checkLocalReaderCreation(RTPSReader* R);
//! DiscoveryAttributes of the EDP.
	DiscoveryAttributes m_discovery;


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATICEDP_H_ */
