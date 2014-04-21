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
 *  Created on: Apr 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/dds/DDSTopicDataType.h"

namespace eprosima {
namespace dds {

DDSTopicDataType::DDSTopicDataType() {
	// TODO Auto-generated constructor stub
	this->m_typeSize = 0;
	this->m_isGetKeyDefined = false;
}

DDSTopicDataType::~DDSTopicDataType() {
	// TODO Auto-generated destructor stub
}


bool DDSTopicDataType::getKey(void* data, InstanceHandle_t* ihandle) {
	return false;
}

} /* namespace rtps */
} /* namespace eprosima */
