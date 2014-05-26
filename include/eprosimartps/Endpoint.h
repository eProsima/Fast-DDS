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

#include "eprosimartps/dds/attributes/TopicAttributes.h"

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

typedef enum EndpointKind_t{
	READER,
	WRITER
}EndpointKind_t;


/**
 * Class Endpoint, all entities of the RTPS network are a specification of this class.
 * Although the Participant is also defined as an endpoint in the RTPS specification in this implementation
 * the Participant class DOESN'T inherit from this class. The elements needed where added directly to the
 * Participant class. This way each instance of our class (Endpoint) has a pointer to the participant they belong to.
 * @ingroup COMMONMODULE
 */
class Endpoint {
public:
	Endpoint(GuidPrefix_t guid,EntityId_t entId,TopicAttributes topic,DDSTopicDataType* ptype,StateKind_t state = STATELESS,EndpointKind_t end = WRITER,int16_t userDefinedId=-1);
	virtual ~Endpoint();



	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;

	//!Vector of pointer to the listening resources associated with this endpoint.
	std::vector<ResourceListen*> m_listenThList;
	//!Pointer to the participant this endpoint belongs to.
	ResourceSend* mp_send_thr;
	ResourceEvent* mp_event_thr;
	DDSTopicDataType* mp_type;


	const EndpointKind_t getEndpointKind() const {
		return m_endpointKind;
	}

	const GUID_t& getGuid() const {
		return m_guid;
	}

	const StateKind_t getStateType() const {
		return m_stateType;
	}

	const TopicAttributes& getTopic() const {
		return m_topic;
	}

	const int16_t getUserDefinedId() const {
		return m_userDefinedId;
	}




protected:
	//! Guid of the Endpoint
	const GUID_t m_guid;
	//! TOpic Attributes
	const TopicAttributes m_topic;
	//!State type (STATELESS or STATEFUL)
	const StateKind_t m_stateType;
	//! Endpoitn Type (READER or WRITER)
	const EndpointKind_t m_endpointKind;
	//!User Defined Id
	const int16_t m_userDefinedId;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
