/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Endpoint.h
 *	Endpoint definition.
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */



#ifndef ENDPOINT_H_
#define ENDPOINT_H_
#include "eprosimartps/common/types/common_types.h"
#include "eprosimartps/common/types/Locator.h"
#include "eprosimartps/common/types/Guid.h"

//#include "eprosimartps/dds/DDSTopicDataType.h"

#include "eprosimartps/dds/attributes/TopicAttributes.h"
//#include "eprosimartps/common/attributes/ReliabilityAttributes.h"
//#include "eprosimartps/common/attributes/PublisherAttributes.h"
//#include "eprosimartps/common/attributes/SubscriberAttributes.h"
//#include "eprosimartps/common/attributes/ParticipantAttributes.h"

using namespace eprosima::dds;

namespace eprosima {

namespace dds{class DDSTopicDataType;}

namespace rtps {

class ResourceListen;
class Participant;
class ResourceSend;
class ResourceEvent;

typedef enum StateKind_t{
	STATELESS,//!< STATELESS
	STATEFUL  //!< STATEFUL
}StateKind_t;


/**
 * Class Endpoint, all entities of the RTPS network are a specification of this class.
 * Although the Participant is also defined as an endpoint in the RTPS specification in this implementation
 * the Participant class DOESN'T inherit from this class. The elements needed where added directly to the
 * Participant class. This way each instance of our class (Endpoint) has a pointer to the participant they belong to.
 * @ingroup COMMONMODULE
 */
class Endpoint {
public:
	Endpoint();
	virtual ~Endpoint();
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	GUID_t m_guid;
	//!Vector of pointer to the listening resources associated with this endpoint.
	std::vector<ResourceListen*> m_listenThList;
	//!Pointer to the participant this endpoint belongs to.
	ResourceSend* mp_send_thr;
	ResourceEvent* mp_event_thr;

	DDSTopicDataType* mp_type;
	int16_t m_userDefinedId;
	TopicAttributes m_topic;

	//!State type of the endpoitn
		StateKind_t m_stateType;
	/**
	 * Get the topic Data Type Name
	 * @return The name of the data type.
	 */
	const std::string& getTopicDataType() const {
		return m_topic.topicDataType;
	}

	StateKind_t getStateType() const {
			return m_stateType;
		}

	/**
	 * Get the topic name.
	 * @return Topic name.
	 */
	const std::string& getTopicName() const {
		return m_topic.topicName;
	}

	TopicKind_t getTopicKind() const {
		return m_topic.topicKind;
	}

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
