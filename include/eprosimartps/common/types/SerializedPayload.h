/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SerializedPayload.h 	
 */

#ifndef SERIALIZEDPAYLOAD_H_
#define SERIALIZEDPAYLOAD_H_

#include "eprosimartps/common/types/common_types.h"
#include <cstring>
#include <stdint.h>



namespace eprosima{
namespace rtps{
//Pre define data encapsulation schemes
#define CDR_BE 0x0000
#define CDR_LE 0x0001
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003


//!@brief Structure SerializedPayload_t.
struct SerializedPayload_t{
	//!Encapsulation of the data as suggested in the RTPS 2.1 specification chapter 10.
	uint16_t encapsulation;
	//!Actual length of the data
	uint16_t length;
	//!Pointer to the data.
	octet* data;
	uint16_t max_size;
	SerializedPayload_t(){
		length = 0;
		data = NULL;
		encapsulation = CDR_BE;
		max_size = 0;
	}
	SerializedPayload_t(short len){
		encapsulation = CDR_BE;
		length = 0;
		data = (octet*)malloc(len);
		max_size = len;
	}
	~SerializedPayload_t(){
		this->empty();
	}
	/*!
	 * Copy another structure (including allocating new space for the data.)
	 * @param[in] serData Pointer to the structure to copy
	 * @return True if correct
	 */
	bool copy(SerializedPayload_t* serData){
		if(serData->length>max_size)
			return false;
		length = serData->length;
		encapsulation = serData->encapsulation;
		if(data == NULL)
			data = (octet*)malloc(length);
		memcpy(data,serData->data,length);
		return true;
	}
	void empty()
	{
		length= 0;
		encapsulation = CDR_BE;
		max_size = 0;
		if(data!=NULL)
			free(data);
		data = NULL;
	}
};

}
};

#endif /* SERIALIZEDPAYLOAD_H_ */
