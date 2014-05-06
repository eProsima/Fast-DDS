/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherAttributes.h
 *
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PUBLISHERPARAMETERS_H_
#define PUBLISHERPARAMETERS_H_

namespace eprosima {
namespace dds {


/**
 * Class PublisherAttributes, used by the user to define the attributes of a Publisher.
 * @ingroup ATTRIBUTESMODULE
 */
class PublisherAttributes {
public:
	PublisherAttributes()
{
		pushMode = true;
		historyMaxSize = 10;
		userDefinedId = -1;
};
	virtual ~PublisherAttributes(){};
	bool pushMode;
	uint16_t historyMaxSize;
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	PublisherReliability reliability;
	TopicAttributes topic;
	int16_t userDefinedId;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PUBLISHERPARAMETERS_H_ */
