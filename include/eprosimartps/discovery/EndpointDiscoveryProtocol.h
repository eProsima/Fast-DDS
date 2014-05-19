/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EndpointDiscoveryProtocol.h
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef ENDPOINTDISCOVERYPROTOCOL_H_
#define ENDPOINTDISCOVERYPROTOCOL_H_

#include "eprosimartps/rtps_all.h"

namespace eprosima {
namespace rtps {

class EndpointDiscoveryProtocol  {
public:
	EndpointDiscoveryProtocol();
	virtual ~EndpointDiscoveryProtocol();


	virtual bool initEDP(DiscoveryAttributes& attributes)=0;
	virtual bool localEndpointMatching(Endpoint* endpoint)=0;
	virtual bool localWriterMatching(RTPSWriter* writer)=0;
	virtual bool localReaderMatching(RTPSReader* reader)=0;



	Participant* mp_participant;
	ParticipantDiscoveryProtocol* mp_DPDP;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINTDISCOVERYPROTOCOL_H_ */
