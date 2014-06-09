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

#include "eprosimartps/dds/DDSTopicDataType.h"

namespace eprosima {
namespace dds {

DDSTopicDataType::DDSTopicDataType() {
	this->m_typeSize = 0;
	this->m_isGetKeyDefined = false;
}

DDSTopicDataType::~DDSTopicDataType() {

}


bool DDSTopicDataType::getKey(void* data, InstanceHandle_t* ihandle) {
	return false;
}

} /* namespace rtps */
} /* namespace eprosima */
