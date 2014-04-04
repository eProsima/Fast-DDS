/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Clock.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "Clock.h"

namespace eprosima {
namespace rtps {

Clock::Clock() {
	// TODO Auto-generated constructor stub
	time_epoch.tm_year = 1900;
	time_epoch.tm_mon = 0;
	time_epoch.tm_mday = 1;
	time_epoch_seconds = mktime(&time_epoch);
}

Clock::~Clock() {
	// TODO Auto-generated destructor stub
}

} /* namespace rtps */
} /* namespace eprosima */
