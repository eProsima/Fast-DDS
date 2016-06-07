/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherAttributes.h	
 */

#ifndef PUBLISHERATTRIBUTES_H_
#define PUBLISHERATTRIBUTES_H_


#include "../rtps/common/Locator.h"
#include "../rtps/common/Time_t.h"
#include "../rtps/attributes/WriterAttributes.h"
#include <fastrtps/rtps/flowcontrol/ThroughputController.h>
#include "TopicAttributes.h"
#include "../qos/WriterQos.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps{


/**
 * Class PublisherAttributes, used by the user to define the attributes of a Publisher.
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
 */
class PublisherAttributes {

public:
	PublisherAttributes(){
		m_userDefinedID = -1;
		m_entityID = -1;
	};
	virtual ~PublisherAttributes(){};
	//!Topic Attributes for the Publisher
	TopicAttributes topic;
	//!QOS for the Publisher
	WriterQos qos;
	//!Writer Attributes
	WriterTimes times;
	//!Unicast locator list
	LocatorList_t unicastLocatorList;
	//!Multicast locator list
	LocatorList_t multicastLocatorList;
   //!Size filter descriptors for this publisher's writer
   std::vector<ThroughputControllerDescriptor> throughputControllers;

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
	 * @param id User defined ID to be set
	 */
	inline void setUserDefinedID(uint8_t id){m_userDefinedID = id;	};

	/**
	 * Set the entity ID
	 * @param id Entity ID to be set
	 */
	inline void setEntityID(uint8_t id){m_entityID = id;	};
private:
	//!User Defined ID, used for StaticEndpointDiscovery, default value -1.
	int16_t m_userDefinedID;
	//!Entity ID, if the user want to specify the EntityID of the enpoint, default value -1.
	int16_t m_entityID;
};

}
} /* namespace eprosima */

#endif /* PUBLISHERATTRIBUTES_H_ */
