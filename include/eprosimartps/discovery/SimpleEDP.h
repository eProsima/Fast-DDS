/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleEDP.h
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SIMPLEEDP_H_
#define SIMPLEEDP_H_

#include "EndpointDiscoveryProtocol.h"

namespace eprosima {
namespace rtps {

class SimpleEDP: public eprosima::rtps::EndpointDiscoveryProtocol {
public:
	SimpleEDP();
	virtual ~SimpleEDP();

	bool initEDP(DiscoveryAttributes& attributes);

	StatefulWriter* mp_SEDPbuiltinPublicationsWriter;
		StatefulWriter* mp_SEDPbuiltinSubscriptionsWriter;
		StatefulWriter* mp_SEDPbuiltinTopicsWriter;
		StatefulReader* mp_SEDPbuiltinPublicationsReader;
		StatefulReader* mp_SEDPbuiltinSubscriptionsReader;
		StatefulReader* mp_SEDPbuiltinTopicsReader;

		bool localEndpointMatching(Endpoint* endpoint);
			 bool localWriterMatching(RTPSWriter* writer);
			 bool localReaderMatching(RTPSReader* reader);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEEDP_H_ */
