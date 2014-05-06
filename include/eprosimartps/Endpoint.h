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
#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/DDSTopicDataType.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class ResourceListen;
class Participant;
class ResourceSend;
class ResourceEvent;


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
	TopicKind_t topicKind;
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
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
