/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredReaderData.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/data/DiscoveredReaderData.h"

namespace eprosima {
namespace rtps {

DiscoveredReaderData::DiscoveredReaderData() {
	// TODO Auto-generated constructor stub
topicKind = NO_KEY;
userDefinedId = 0;
isAlive = false;
}

DiscoveredReaderData::~DiscoveredReaderData() {
	// TODO Auto-generated destructor stub
}

} /* namespace rtps */
} /* namespace eprosima */
