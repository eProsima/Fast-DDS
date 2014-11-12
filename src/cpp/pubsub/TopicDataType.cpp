/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSTopicDataType.cpp
 *
 */

#include "eprosimartps/pubsub/TopicDataType.h"

namespace eprosima {
namespace pubsub {

TopicDataType::TopicDataType() {
	this->m_typeSize = 0;
	this->m_isGetKeyDefined = false;
}

TopicDataType::~TopicDataType() {

}


bool TopicDataType::getKey(void* data, InstanceHandle_t* ihandle) {
	return false;
}

} /* namespace rtps */
} /* namespace eprosima */
