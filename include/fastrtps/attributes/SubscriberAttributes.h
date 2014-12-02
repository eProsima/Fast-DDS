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
	//!Times for a RELIABLE Reader
	ReaderTimes times;
	//!Unicast locator list
	LocatorList_t unicastLocatorList;
	//!Multicast locator list
	LocatorList_t multicastLocatorList;
	//!Expects Inline QOS
	bool expectsInlineQos;
	
	/**
	 * Get the user defined ID
	 * @return User defined ID
	 */
	inline int16_t getUserDefinedID() const {return m_userDefinedID;}
	
	/**
	 * Get the entity defined ID
	 * @return Entity ID
	 */
	inline int16_t getEntityID() const {return m_entityID;}
	
	/**
	 * Set the user defined ID
	 * @param User defined ID to be set
	 */
	inline void setUserDefinedID(uint8_t id){m_userDefinedID = id;	};
	
	/**
	 * Set the entity ID
	 * @param Entity ID to be set
	 */
	inline void setEntityID(uint8_t id){m_entityID = id;	};
private:
	int16_t m_userDefinedID;
	int16_t m_entityID;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* SUBSCRIBERPARAMS_H_ */
