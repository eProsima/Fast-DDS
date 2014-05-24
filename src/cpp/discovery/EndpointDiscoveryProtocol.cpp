/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EndpointDiscoveryProtocol.cpp
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/EndpointDiscoveryProtocol.h"
#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"

namespace eprosima {
namespace rtps {

EndpointDiscoveryProtocol::EndpointDiscoveryProtocol(ParticipantDiscoveryProtocol* p):
	mp_PDP(p)//,
	//mp_participant(p->mp_participant)
{


}

EndpointDiscoveryProtocol::~EndpointDiscoveryProtocol() {
	// TODO Auto-generated destructor stub
}

} /* namespace rtps */
} /* namespace eprosima */
