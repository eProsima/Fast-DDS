/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EndpointAttributes.h
 */

#ifndef ENDPOINTATTRIBUTES_H_
#define ENDPOINTATTRIBUTES_H_

#include "fastrtps/rtps/common/Types.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

/**
 * Attributes associated with an Endpoint.
 */
class EndpointAttributes
{
public:
	EndpointAttributes()
	{
		topicKind = NO_KEY;
		reliabilityKind = BEST_EFFORT;
		durabilityKind = VOLATILE;
		m_userDefinedID = -1;
		m_entityID = -1;
		endpointKind = WRITER;
	};
	virtual ~EndpointAttributes(){};
	//!Endpoint kind
	EndpointKind_t endpointKind;
	//!Tipoc kind
	TopicKind_t topicKind;
	//!Reliability kind
	ReliabilityKind_t reliabilityKind;
	//!Durability kind
	DurabilityKind_t durabilityKind;
	//!Unicast locator list
	LocatorList_t unicastLocatorList;
	//!Multicast locator list
	LocatorList_t multicastLocatorList;
		
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
	int16_t m_userDefinedID;
	int16_t m_entityID;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /*  */
