/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSTopicDataType.h
 *
 *  Created on: Apr 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DDSTOPICDATATYPE_H_
#define DDSTOPICDATATYPE_H_

#include "eprosimartps/rtps_all.h"

using namespace eprosima::rtps;

namespace eprosima {
namespace dds {

class DDSTopicDataType {
public:
	DDSTopicDataType();
	virtual ~DDSTopicDataType();
	virtual bool serialize(void*data,SerializedPayload_t* payload)=0;
	virtual bool deserialize(SerializedPayload_t* payload,void * data)=0;
	virtual bool getKey(void*data,InstanceHandle_t* ihandle);
	std::string m_topicDataTypeName;
	uint32_t m_typeSize;
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* RTPSTOPICDATATYPE_H_ */
