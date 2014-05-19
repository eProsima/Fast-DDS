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
using boost::property_tree::ptree;
namespace eprosima {
namespace rtps {

class StaticEDP: public EndpointDiscoveryProtocol {
public:
	StaticEDP();
	virtual ~StaticEDP();

	bool initEDP(DiscoveryAttributes& attributes);
	bool localEndpointMatching(Endpoint* endpoint);
	bool localWriterMatching(RTPSWriter* writer);
	bool localReaderMatching(RTPSReader* reader);


	bool loadXMLFile(const std::string& filename);
	bool loadXMLWriterEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata);
	bool loadXMLReaderEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata);

	bool addRemoteEndpoint(uint16_t userId,EntityId_t entityId,DiscoveredParticipantData* pdata);
	std::vector<uint16_t> m_endpointIds;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATICEDP_H_ */
