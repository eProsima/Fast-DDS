/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberParams.h
 *
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SUBSCRIBERPARAMS_H_
#define SUBSCRIBERPARAMS_H_

namespace eprosima {
namespace rtps {

class SubscriberAttributes {
public:
	SubscriberAttributes()
{
		expectsInlineQos = false;
		historyMaxSize = 50;
		userDefinedId = -1;
};
	virtual ~SubscriberAttributes();
	bool expectsInlineQos;
	uint16_t historyMaxSize;
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	SubscriberReliability reliability;
	TopicAttributes topic;
	int16_t userDefinedId;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SUBSCRIBERPARAMS_H_ */
