/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * CDRMessageCreator.h
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#ifndef CDRMESSAGECREATOR_H_
#define CDRMESSAGECREATOR_H_

#include "rtps_all.h"

namespace eprosima {
namespace rtps{
/**
 * CDR Message Struct
 */
typedef struct CDRMessage_t{
	CDRMessage_t(){
		w_pos = 0;
		buffer = 0;
		max_size = RTPSMESSAGE_MAX_SIZE;
		msg_endian = BIGEND;
	}
	~CDRMessage_t(){
		free(buffer);
	}
	octet* buffer;
	uint w_pos; //current w_pos in bytes
	uint max_size; // max size of buffer in bytes
	Endianness_t msg_endian;
}CDRMessage_t;


class CDRMessageCreator {
public:
	CDRMessageCreator();
	~CDRMessageCreator();


	/**
	 * @brief Create a Header to the serialized message
	 * @param Pointer to the Message
	 * @param Pointer to the header
	 * @return true or false
	 */
	bool createHeader(CDRMessage_t*,Header_t*);
	/**
	 * @brief Create SubmessageHeader
	 * @param Pointer to the CDRMessage
	 * @param Pointer to the SubMessageHeader
	 * @param Length of the corresponding message
	 * @return true or false
	 */
	bool createSubmessageHeader(CDRMessage_t* msg,SubmessageHeader_t* SubMH,unsigned short submsgsize);
	/**
	 * @brief This function creates a CDR submessage representation for the Data Submessage.
	 * @param submsg pointer to the submessage
	 * @param DataSubM  pointer to the submessage structure
	 * @return true if everything is correct.
	 */
	bool createSubmessageData(CDRMessage_t* submsg,DataSubM_t* DataSubM);
	/**
	 * Create a Data RTPS Message.
	 * @param msg CDR serialized message pointer where the message is going to be stores.
	 * @param guidprefix guid prefix
	 * @param DataSubM Data submessage structure
	 * @return
	 */
	bool createMessageData(CDRMessage_t* msg,GuidPrefix_t guidprefix,DataSubM_t* DataSubM);

	/**
	 * @brief Initialize CDR message with a given byte size.
	 * @param size Given byte size.
	 * @return
	 */
	bool initCDRMsg(CDRMessage_t*,uint size);
	/**
	 * Initialize with default size.
	 * @return
	 */
	bool initCDRMsg(CDRMessage_t*);
	/**
	 * Append given CDRMessage to existing CDR Message.
	 * @param Pointer to first message.
	 * @param Pointer to second message.
	 * @return
	 */
	bool appendMsg(CDRMessage_t*,CDRMessage_t*);
	/**
	 * Add data to buffer.
	 * @param Pointer to message.
	 * @param Pointer to data
	 * @param Number of bytes
	 * @return
	 */
	bool addData(CDRMessage_t*,const void*,uint);
	/**
	 * add data to stream in a reversed manner
	 * @param Pointer to message.
	 * @param Pointer to data
	 * @param Number of bytes
	 * @return
	 */
	bool addDataReversed(CDRMessage_t*,const void*,uint);
	/**
	 * Add octet to buffer.
	 * @param Pointer to message.
	 * @param octet to add.
	 * @return
	 */
	bool addOctet(CDRMessage_t*,octet);
	bool addUshort(CDRMessage_t*,unsigned short);
	bool addEntityId(CDRMessage_t*,EntityId_t*);
	bool addParameter(CDRMessage_t*,Parameter_t*);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CDRMESSAGECREATOR_H_ */
