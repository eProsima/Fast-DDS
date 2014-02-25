/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Endpoint.h
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

class Endpoint {
public:
	Endpoint();
	virtual ~Endpoint();
	TopicKind_t topicKind;
	ReliabilityKind_t reliabilityLevel;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	GUID_t guid;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
