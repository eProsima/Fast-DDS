/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherAttributes.h	
 */

#ifndef PUBLISHERPARAMETERS_H_
#define PUBLISHERPARAMETERS_H_

#include <iostream>

#include "eprosimartps/rtps/common/Locator.h"
#include "eprosimartps/rtps/common/Time_t.h"
#include "eprosimartps/rtps/attributes/WriterAttributes.h"
#include "eprosimartps/pubsub/attributes/TopicAttributes.h"
#include "eprosimartps/pubsub/qos/PublisherQos.h"

using namespace eprosima::rtps;

namespace eprosima {
namespace pubsub {

/**
 * Class PublisherAttributes, used by the user to define the attributes of a Publisher.
 * @ingroup ATTRIBUTESMODULE
 */
class PublisherAttributes {

public:
	PublisherAttributes(){	};
	virtual ~PublisherAttributes(){};
	//!Topic Attributes for the Publisher
	TopicAttributes topic;
	//!QOS for the Publisher
	PublisherQos qos;
	//!Writer Attributes
	WriterAttributes writer;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PUBLISHERPARAMETERS_H_ */
