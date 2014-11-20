/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EndpointAttributes.h
 */

#ifndef ENDPOINTATTRIBUTES_H_
#define ENDPOINTATTRIBUTES_H_

#include "eprosimartps/rtps/common/Types.h"

namespace eprosima {
namespace rtps {

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
	virtual ~EndpointAttributes();
	EndpointKind_t endpointKind;
	TopicKind_t topicKind;
	ReliabilityKind_t reliabilityKind;
	DurabilityKind_t durabilityKind;
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	inline int16_t getUserDefinedID() const {return m_userDefinedID;}
	inline int16_t getEntityID() const {return m_entityID;}
	inline void setUserDefinedID(uint16_t id){m_userDefinedID = id;	};
	inline void setEntityID(uint16_t id){m_entityID = id;	};
private:
	int16_t m_userDefinedID;
	int16_t m_entityID;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /*  */
