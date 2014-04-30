/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleEndpointDiscoveryProtocol.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SIMPLEENDPOINTDISCOVERYPROTOCOL_H_
#define SIMPLEENDPOINTDISCOVERYPROTOCOL_H_

namespace eprosima {
namespace rtps {

class SimpleEndpointDiscoveryProtocol {
public:
	SimpleEndpointDiscoveryProtocol();
	virtual ~SimpleEndpointDiscoveryProtocol();




private:
	Participant* mp_Participant;

	StatefulWriter* mp_SEDPbuiltinPublicationsWriter;
	StatefulWriter* mp_SEDPbuiltinSubscriptionsWriter;
	StatefulWriter* mp_SEDPbuiltinTopicsWriter;
	StatefulReader* mp_SEDPbuiltinPublicationsReader;
	StatefulReader* mp_SEDPbuiltinSubscriptionsReader;
	StatefulReader* mp_SEDPbuiltinTopicsReader;

	bool createSEDPbuiltinPublicationsWriter();
	bool createSEDPbuiltinSubscriptionsWriter();
	bool createSEDPbuiltinTopicsWriter();
	bool createSEDPbuiltinPublicationsReader();
	bool createSEDPbuiltinSubscriptionsReader();
	bool createSEDPbuiltinTopicsReader();




};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEENDPOINTDISCOVERYPROTOCOL_H_ */
