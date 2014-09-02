/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ZeroMQPublisher.cpp
 *
 */

#include "ZeroMQPublisher.h"

uint32_t dataspub[] = {12,28,60,124,252,508,1020,2044,4092,8188,12284};
std::vector<uint32_t> data_size_pub (dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );

ZeroMQPublisher::ZeroMQPublisher() {
	// TODO Auto-generated constructor stub

}

ZeroMQPublisher::~ZeroMQPublisher() {
	// TODO Auto-generated destructor stub
}

