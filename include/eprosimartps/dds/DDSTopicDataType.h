/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DDSTopicDataType.h	
 */

#ifndef DDSTOPICDATATYPE_H_
#define DDSTOPICDATATYPE_H_

#include "eprosimartps/common/types/common_types.h"
#include "eprosimartps/common/types/SerializedPayload.h"
#include "eprosimartps/common/types/InstanceHandle.h"

#include <string>

using namespace eprosima::rtps;

namespace eprosima {
namespace dds {


/**
 * Class DDSTopicDataType used to provide the DomainParticipant with the methods to serialize, deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one, where Serialize and deserialize methods MUST be implemented.
 * @ingroup DDSMODULE
 * @snippet dds_example.cpp ex_DDSTopicDataType
 */
class RTPS_DllAPI DDSTopicDataType {
public:
	DDSTopicDataType();
	virtual ~DDSTopicDataType();
	/**
	 * Serialize method, it should be implemented by the user, since it is abstract.
	 * It is VERY IMPORTANT that the user sets the serializedPaylaod length correctly.
	 * @param[in] data Pointer to the data
	 * @param[out] payload Pointer to the payload
	 * @return True if correct.
	 */
	virtual bool serialize(void*data,SerializedPayload_t* payload)=0;
	/**
	 * Deserialize method, it should be implemented by the user, since it is abstract.
	 * @param[in] payload Pointer to the payload
	 * @param[out] data Pointer to the data
	 * @return True if correct.
	 */
	virtual bool deserialize(SerializedPayload_t* payload,void * data)=0;
	/**
	 * Get the key associated with the data.
	 * @param[in] data Pointer to the data.
	 * @param[out] ihandle Pointer to the Handle.
	 * @return True if correct.
	 */
	virtual bool getKey(void*data,InstanceHandle_t* ihandle);
	//! Data Type Name.
	std::string m_topicDataTypeName;
	//! Maximum Type size in bytes. (If the type includes a string the user MUST ensure that the maximum
	//! size of the string respect the maximum defined size.).
	uint32_t m_typeSize;
	//! Indicates wheter the method to obtain the key has been implemented.
	bool m_isGetKeyDefined;
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* RTPSTOPICDATATYPE_H_ */
