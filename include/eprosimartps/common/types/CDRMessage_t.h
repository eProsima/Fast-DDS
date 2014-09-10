/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessage_t.h	
 */

#ifndef CDRMESSAGE_T_H_
#define CDRMESSAGE_T_H_

#include "eprosimartps/common/types/common_types.h"

namespace eprosima{
namespace rtps{


//!Max size of RTPS message in bytes.
#define RTPSMESSAGE_DEFAULT_SIZE 10500  //max size of rtps message in bytes
#define RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE 500 //common payload a rtps message has
#define RTPSMESSAGE_COMMON_DATA_PAYLOAD_SIZE 10000 //common data size
#define RTPSMESSAGE_HEADER_SIZE 20  //header size in bytes
#define RTPSMESSAGE_SUBMESSAGEHEADER_SIZE 4
#define RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE 4
#define RTPSMESSAGE_INFOTS_SIZE 12

#define RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG 16 //may change in future versions
#define RTPSMESSAGE_DATA_MIN_LENGTH 24

/**
 * @brief Structure CDRMessage_t, contains a serialized message.
 * @ingroup COMMONMODULE
 */
struct CDRMessage_t{
	CDRMessage_t(){
		pos = 0;
		length = 0;
		buffer = (octet*)malloc(RTPSMESSAGE_DEFAULT_SIZE);
		max_size = RTPSMESSAGE_DEFAULT_SIZE;

		msg_endian = EPROSIMA_ENDIAN;
	}
	~CDRMessage_t()
	{
		if(buffer != NULL)
			free(buffer);
	}
	CDRMessage_t(uint32_t size)
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
	uint32_t pos;
	//!Max size of the message.
	uint32_t max_size;
	//!Current length of the message.
	uint32_t length;
	//!Endianness of the message.
	Endianness_t msg_endian;
};

}
}

#endif /* CDRMESSAGE_T_H_ */
