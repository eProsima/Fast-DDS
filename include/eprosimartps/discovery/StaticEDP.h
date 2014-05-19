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
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef STATICEDP_H_
#define STATICEDP_H_

#include "eprosimartps/discovery/EndpointDiscoveryProtocol.h"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"



using boost::property_tree::ptree;
namespace eprosima {
namespace rtps {

class StaticEDP: public EndpointDiscoveryProtocol {
public:
	StaticEDP();
	virtual ~StaticEDP();

	bool initEDP(DiscoveryAttributes& attributes);

	bool localWriterMatching(RTPSWriter* writer);
	bool localReaderMatching(RTPSReader* reader);


	bool loadXMLFile(const std::string& filename);
	bool loadXMLWriterEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata);
	bool loadXMLReaderEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata);

	void assignRemoteEndpoints(DiscoveredParticipantData* pdata) {};

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

	DiscoveryAttributes m_discovery;


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATICEDP_H_ */
