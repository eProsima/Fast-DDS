/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessage_t.h
 *
 *  Created on: May 22, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef CDRMESSAGE_T_H_
#define CDRMESSAGE_T_H_

#include "eprosimartps/common/types/common_types.h"

namespace eprosima{
namespace rtps{


//!Max size of RTPS message in bytes.
#define RTPSMESSAGE_MAX_SIZE 5000  //max size of rtps message in bytes
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

}
}

#endif /* CDRMESSAGE_T_H_ */
