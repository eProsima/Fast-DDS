/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberAttributes.h 	
 */

#ifndef SUBSCRIBERATTRIBUTES_H_
#define SUBSCRIBERATTRIBUTES_H_


#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/rtps/attributes/ReaderAttributes.h"
#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/qos/ReaderQos.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

/**
 * Class SubscriberAttributes, used by the user to define the attributes of a Subscriber.
 * @ingroup ATTRIBUTESMODULE
 */
class SubscriberAttributes {

public:
	SubscriberAttributes()
	{
		m_userDefinedID = -1;
		m_entityID = -1;
		expectsInlineQos = false;
	};
	virtual ~SubscriberAttributes(){};
	//!Topic Attributes
	TopicAttributes topic;
	//!QOS
	ReaderQos qos;
	//!Times
	ReaderTimes times;
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	bool expectsInlineQos;
	inline int16_t getUserDefinedID() const {return m_userDefinedID;}
	inline int16_t getEntityID() const {return m_entityID;}
	inline void setUserDefinedID(uint8_t id){m_userDefinedID = id;	};
	inline void setEntityID(uint8_t id){m_entityID = id;	};
private:
	int16_t m_userDefinedID;
	int16_t m_entityID;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* SUBSCRIBERPARAMS_H_ */
