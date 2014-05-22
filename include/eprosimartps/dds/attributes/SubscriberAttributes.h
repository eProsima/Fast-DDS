/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberAttributes.h
 *	Subscriber Attributes
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SUBSCRIBERPARAMS_H_
#define SUBSCRIBERPARAMS_H_

#include "eprosimartps/common/types/common_types.h"
#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/common/types/Locator.h"

#include "eprosimartps/qos/ReaderQos.h"

namespace eprosima {
namespace dds {

class SubscriberTimes{
	//!Delay the response to a HB.
		Duration_t heartbeatResponseDelay;
		//!Ignore too son received HB.
		Duration_t heartbeatSupressionDuration;
		SubscriberTimes()
		{
			heartbeatResponseDelay.nanoseconds = 500*1000*1000;
		}
		~SubscriberTimes(){};
};

/**
 * Class SubscriberAttributes, used by the user to define the attributes of a Subscriber.
 * @ingroup ATTRIBUTESMODULE
 */
class SubscriberAttributes {
public:
	SubscriberAttributes()
{
		expectsInlineQos = false;
		historyMaxSize = 50;
		userDefinedId = -1;
};
	virtual ~SubscriberAttributes(){};
	//! Expects Inline Qos (true or false)
	bool expectsInlineQos;
	//!Maximum size of the History associated with this Subscriber.
	uint16_t historyMaxSize;
	//!Unicast Locator List that the Subscriber should be listening.
	LocatorList_t unicastLocatorList;
	//!Multicas LocatorList where the Subscriber should be listening.
	LocatorList_t multicastLocatorList;
	//!Times attributes of the Subscriber.
	SubscriberTimes times;
	//!Topic Attributes of the topic associated with this subscriber.
	TopicAttributes topic;
	//!User defined Id, only necessary if the participant uses StaticEndpointDiscoveryProtocol.
	int16_t userDefinedId;
	//! Reader qos
	ReaderQos qos;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SUBSCRIBERPARAMS_H_ */
