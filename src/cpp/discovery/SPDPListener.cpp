/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SPDPListener2.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/SPDPListener.h"
#include "eprosimartps/discovery/SimpleParticipantDiscoveryProtocol.h"

namespace eprosima {
namespace rtps {

void SPDPListener::newMessageCallback()
	{
		mp_SPDP->new_change_added();
	}

} /* namespace rtps */
} /* namespace eprosima */
