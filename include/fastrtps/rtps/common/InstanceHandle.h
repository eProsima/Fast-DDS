/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file InstanceHandle.h 	
 */

#ifndef INSTANCEHANDLE_H_
#define INSTANCEHANDLE_H_
#include "fastrtps/config/fastrtps_dll.h"
#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/common/Guid.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * Struct InstanceHandle_t, used to contain the key for WITH_KEY topics.
 */
struct RTPS_DllAPI InstanceHandle_t{
	//!Value
	octet value[16];
	InstanceHandle_t()
	{
		for(uint8_t i=0;i<16;i++)
			value[i] = 0;
	}
	
	/**
	* Assingment operator
	* @param ihandle Instance handle to copy the data from
	*/
	InstanceHandle_t& operator=(const InstanceHandle_t& ihandle){

		for(uint8_t i =0;i<16;i++)
		{
			value[i] = ihandle.value[i];
		}
		return *this;
	}
	
	/**
	* Assingment operator
	* @param guid GUID to copy the data from
	*/
	InstanceHandle_t& operator=(const GUID_t& guid)
	{
		for(uint8_t i =0;i<16;i++)
		{
			if(i<12)
				value[i] = guid.guidPrefix.value[i];
			else
				value[i] = guid.entityId.value[i-12];
		}
		return *this;
	}

	/**
	* Know if the instance handle is defined
	* @return True if the values are not zero.
	*/
	bool isDefined()
	{
		for(uint8_t i=0;i<16;++i)
		{
			if(value[i]!=0)
				return true;
		}
		return false;
	}
};

const InstanceHandle_t c_InstanceHandle_Unknown;

/**
* Comparison operator
* @param ihandle1 First InstanceHandle_t to compare
* @param ihandle2 Second InstanceHandle_t to compare
* @return True if equal
*/
inline bool operator==(const InstanceHandle_t & ihandle1, const InstanceHandle_t& ihandle2)
{
	for(uint8_t i =0;i<16;++i)
	{
		if(ihandle1.value[i] != ihandle2.value[i])
			return false;
	}
	return true;
}

/**
* Convert InstanceHandle_t to GUID
* @param guid GUID to store the results
* @param ihandle InstanceHandle_t to copy
*/
inline void iHandle2GUID(GUID_t& guid,const InstanceHandle_t& ihandle)
{
	for(uint8_t i = 0;i<16;++i)
	{
		if(i<12)
			guid.guidPrefix.value[i] = ihandle.value[i];
		else
			guid.entityId.value[i-12] = ihandle.value[i];
	}
	return;
}

/**
* Convert GUID to InstanceHandle_t
* @param ihandle InstanceHandle_t to store the results
* @param guid GUID to copy
*/
inline GUID_t iHandle2GUID(const InstanceHandle_t& ihandle)
{
	GUID_t guid;
	for(uint8_t i = 0;i<16;++i)
	{
		if(i<12)
			guid.guidPrefix.value[i] = ihandle.value[i];
		else
			guid.entityId.value[i-12] = ihandle.value[i];
	}
	return guid;
}

/**
* 
* @param output 
* @param iHandle
*/
inline std::ostream& operator<<(std::ostream& output,const InstanceHandle_t& iHandle)
{
	output << std::hex;
	for(uint8_t i =0;i<15;++i)
		output << (int)iHandle.value[i] << ".";
	output << (int)iHandle.value[15] << std::dec;
	return output;
}
}
}
}

#endif /* INSTANCEHANDLE_H_ */
