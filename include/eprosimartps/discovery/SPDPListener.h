/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SPDPListener.h
 *	SPDPListener
 *  Created on: Apr 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SPDPLISTENER2_H_
#define SPDPLISTENER2_H_

#include "eprosimartps/dds/SubscriberListener.h"

namespace eprosima {

using namespace dds;

namespace rtps {

class SimpleParticipantDiscoveryProtocol;


/**
 * Class SPDPListener, specification of SubscriberListener used by the SPDP to perform the History check when a new message is received.
 * This class is implemented in order to use the same structure than with any other RTPSReader.
 */
class SPDPListener: public SubscriberListener {
public:
	SPDPListener(SimpleParticipantDiscoveryProtocol* in_SPDP):mp_SPDP(in_SPDP){};
	virtual ~SPDPListener(){};
	//!Pointer to the associated mp_SPDP;
	SimpleParticipantDiscoveryProtocol* mp_SPDP;
	//!Method to be called when a new data message is received.
	void onNewDataMessage();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SPDPLISTENER2_H_ */


