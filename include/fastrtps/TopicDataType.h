/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TopicDataType.h
 */

#ifndef TOPICDATATYPE_H_
#define TOPICDATATYPE_H_

#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/common/SerializedPayload.h"
#include "fastrtps/rtps/common/InstanceHandle.h"
#include "fastrtps/utils/md5.h"
#include <string>

using namespace eprosima::fastrtps::rtps;



namespace eprosima {
namespace fastrtps {


/**
 * Class TopicDataType used to provide the DomainRTPSParticipant with the methods to serialize, deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one, where Serialize and deserialize methods MUST be implemented.
 * @ingroup FASTRTPS_MODULE
 * @snippet fastrtps_example.cpp ex_TopicDataType
 */
class  TopicDataType {
public:
	RTPS_DllAPI TopicDataType(){
		this->m_typeSize = 0;
        this->m_isGetKeyDefined = false;
	}
	RTPS_DllAPI virtual ~TopicDataType(){};
	/**
	 * Serialize method, it should be implemented by the user, since it is abstract.
	 * It is VERY IMPORTANT that the user sets the serializedPaylaod length correctly.
	 * @param[in] data Pointer to the data
	 * @param[out] payload Pointer to the payload
	 * @return True if correct.
	 */
	RTPS_DllAPI virtual bool serialize(void*data, SerializedPayload_t* payload) = 0;
	
	/**
	 * Deserialize method, it should be implemented by the user, since it is abstract.
	 * @param[in] payload Pointer to the payload
	 * @param[out] data Pointer to the data
	 * @return True if correct.
	 */
	RTPS_DllAPI virtual bool deserialize(SerializedPayload_t* payload, void * data) = 0;
	/**
	 * Create a Data Type.
	 * @return Void pointer to the created object.
	 */
	RTPS_DllAPI virtual void * createData() = 0;
	/**
	 * Remove a previously created object.
	 * @param data Pointer to the created Data.
	 */
	RTPS_DllAPI virtual void deleteData(void * data) = 0;

	/**
	 * Get the key associated with the data.
	 * @param[in] data Pointer to the data.
	 * @param[out] ihandle Pointer to the Handle.
	 * @return True if correct.
	 */
	RTPS_DllAPI virtual bool getKey(void*data, InstanceHandle_t* ihandle){ return false; }

	/**
	* Set topic data type name
	* @param nam Topic data type name
	*/
	RTPS_DllAPI inline void setName(const char* nam) { m_topicDataTypeName = std::string(nam); }
	
	/**
	* Get topic data type name
	* @return Topic data type name
	*/
	RTPS_DllAPI inline const char* getName(){ return m_topicDataTypeName.c_str(); }
	
	//! Maximum Type size in bytes. (If the type includes a string the user MUST ensure that the maximum
	//! size of the string respect the maximum defined size.).
	uint32_t m_typeSize;
	
	//! Indicates whether the method to obtain the key has been implemented.
	bool m_isGetKeyDefined;
private:
	//! Data Type Name.
	std::string m_topicDataTypeName;



};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* RTPSTOPICDATATYPE_H_ */
