/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SPDPListener2.h
 *
 *  Created on: Apr 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SPDPLISTENER2_H_
#define SPDPLISTENER2_H_

#include "eprosimartps/dds/SubscriberListener.h"

namespace eprosima {
namespace rtps {

class SimpleParticipantDiscoveryProtocol;

class SPDPListener: public SubscriberListener {
public:
	SPDPListener(SimpleParticipantDiscoveryProtocol* in_SPDP):mp_SPDP(in_SPDP){};
	virtual ~SPDPListener(){};
	SimpleParticipantDiscoveryProtocol* mp_SPDP;
	void onNewDataMessage();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SPDPLISTENER2_H_ */


