/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file rtps_common.h
 *	Common definitions.
 *  Created on: Feb 18, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      		grcanosa@gmail.com
 */




#ifndef RTPS_COMMON_H_
#define RTPS_COMMON_H_
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <vector>
#include <iostream>
 #include <bitset>
 #include <string>
#include <sstream>

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima{


namespace rtps{



/**
 * @brief Structure CDRMessage_t, contains a serialized message.
 * @ingroup COMMONMODULE
 */
typedef struct CDRMessage_t{
	CDRMessage_t(){
		pos = 0;
		length = 0;
		buffer = (octet*)malloc(RTPSMESSAGE_MAX_SIZE);
		max_size = RTPSMESSAGE_MAX_SIZE;

		msg_endian = EPROSIMA_ENDIAN;
	}
	~CDRMessage_t()
	{
		if(buffer != NULL)
			free(buffer);
	}
	CDRMessage_t(uint16_t size)
	{
		pos = 0;
		length = 0;
		buffer = (octet*)malloc(size);
		max_size = size;
		msg_endian = EPROSIMA_ENDIAN;
	}
	//!Pointer to the buffer where the data is stored.
	octet* buffer;
	//!Read or write position.
	uint16_t pos;
	//!Max size of the message.
	uint16_t max_size;
	//!Current length of the message.
	uint16_t length;
	//!Endianness of the message.
	Endianness_t msg_endian;
}CDRMessage_t;





//Pre define data encapsulation schemes
#define CDR_BE 0x0000
#define CDR_LE 0x0001
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003


//!@brief Structure SerializedPayload_t.
typedef struct SerializedPayload_t{
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
		length = serData->length;
		max_size = length;
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
}SerializedPayload_t;


typedef struct InstanceHandle_t{
	octet value[16];
	InstanceHandle_t()
	{
		for(uint8_t i=0;i<16;i++)
			value[i] = 0;
	}
	InstanceHandle_t& operator=(const InstanceHandle_t& ihandle){

		for(uint8_t i =0;i<16;i++)
		{
			value[i] = ihandle.value[i];
		}
		return *this;
	}
	bool operator==(const InstanceHandle_t& ihandle)
	{
		for(uint8_t i =0;i<16;++i)
		{
			if(this->value[i] != ihandle.value[i])
				return false;
		}
		return true;
	}
}InstanceHandle_t;

typedef uint32_t BuiltinEndpointSet_t;





typedef uint32_t Count_t;

//!Structure Time_t, used to describe times.
typedef struct Time_t{
	int32_t seconds;
	uint32_t nanoseconds;
	int64_t to64time(){
		return (int64_t)seconds+((int64_t)(nanoseconds/pow(2.0,32)));
	}
	Time_t()
	{
		seconds = 0;
		nanoseconds = 0;
	}
}Time_t;

#define TIME_ZERO(t){t.seconds=0;t.nanoseconds=0;}
#define TIME_INVALID(t){t.seconds=-1;t.nanoseconds=0xffffffff;}
#define TIME_INFINITE(t){t.seconds=0x7fffffff;t.nanoseconds=0xffffffff;}



/**
 * Enum StateKind_t, type of Writer or reader.
 */
typedef enum StateKind_t{
	STATELESS,//!< STATELESS
	STATEFUL  //!< STATEFUL
}StateKind_t;

//!Enum HistoryKind_t, indicates whether the HistoryCache belongs to a reader or a writer
typedef enum HistoryKind_t{
	READER = 0,
	WRITER = 1
}HistoryKind_t;


typedef Time_t Duration_t;

//!@brief Structure ProtocolVersion_t, contains the protocol version.
typedef struct ProtocolVersion_t{
	octet m_major;
	octet m_minor;
	ProtocolVersion_t():
		m_major(2),
		m_minor(1)
	{

	};
	ProtocolVersion_t(octet maj,octet min):
		m_major(maj),
		m_minor(min)
	{

	}
} ProtocolVersion_t;

#define PROTOCOLVERSION_1_0(pv) {pv.m_major=1;pv.m_minor=0;}
#define PROTOCOLVERSION_1_1(pv) {pv.m_major=1;pv.m_minor=1;}
#define PROTOCOLVERSION_2_0(pv) {pv.m_major=2;pv.m_minor=0;}
#define PROTOCOLVERSION_2_1(pv) {pv.m_major=2;pv.m_minor=1;}
#define PROTOCOLVERSION PROTOCOLVERSION_2_1




#define VENDORID_UNKNOWN(vi) {vi[0]=0;vi[1]=0;}
#define VENDORID_EPROSIMA(vi) {vi[0]=42;vi[1]=42;}
//!@brief Structure VendorId_t.
typedef octet VendorId_t[2];


#define PROTOCOL_RTPS "RTPS"




}
}




#endif /* RTPS_COMMON_H_ */
