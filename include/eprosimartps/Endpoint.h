/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Endpoint.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "rtps_all.h"

#ifndef ENDPOINT_H_
#define ENDPOINT_H_

namespace eprosima {
namespace rtps {

class ThreadListen;
class Participant;

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
	ReliabilityKind_t reliabilityKind;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	GUID_t guid;
	//!Vector of pointer to the listening threads associated with this endpoint.
	std::vector<ThreadListen*> endpointThreadListenList;
	//!Pointer to the participant this endpoint belongs to.
	Participant* participant;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
