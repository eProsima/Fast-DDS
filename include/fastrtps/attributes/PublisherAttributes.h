/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherAttributes.h	
 */

#ifndef PUBLISHERATTRIBUTES_H_
#define PUBLISHERATTRIBUTES_H_


#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/qos/WriterQos.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps{


/**
 * Class PublisherAttributes, used by the user to define the attributes of a Publisher.
 * @ingroup ATTRIBUTESMODULE
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
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	inline int16_t getUserDefinedID() const {return m_userDefinedID;}
	inline int16_t getEntityID() const {return m_entityID;}
	inline void setUserDefinedID(uint8_t id){m_userDefinedID = id;	};
	inline void setEntityID(uint8_t id){m_entityID = id;	};
private:
	int16_t m_userDefinedID;
	int16_t m_entityID;
};

}
} /* namespace eprosima */

#endif /* PUBLISHERATTRIBUTES_H_ */
