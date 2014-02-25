/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * CDRMessage.h
 *
 *  Created on: Feb 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "rtps_all.h"

#ifndef CDRMESSAGE_H_
#define CDRMESSAGE_H_

namespace eprosima {
namespace rtps {

class CDRMessage {
public:
	CDRMessage();
	virtual ~CDRMessage();

	static bool readEntityId(CDRMessage_t* msg,EntityId_t*id);
	static bool readData(CDRMessage_t* msg,octet* o,uint16_t length);
	static bool readDataReversed(CDRMessage_t* msg,octet* o,uint16_t length);
	static bool readInt32(CDRMessage_t* msg,int32_t* lo);
	static bool readUInt32(CDRMessage_t* msg,uint32_t* ulo);
	static bool readSequenceNumber(CDRMessage_t* msg,SequenceNumber_t* sn);
	static bool readInt16(CDRMessage_t* msg,int16_t* i16);
	static bool readUInt16(CDRMessage_t* msg,uint16_t* i16);
	static bool readParameterList(CDRMessage_t* msg,ParameterList_t* list,int16_t* size);
	static bool readOctet(CDRMessage_t* msg,octet* o);
	/**
	 * @brief Initialize CDR message with a given byte size.
	 * @param size Given byte size.
	 * @return
	 */
	static bool initCDRMsg(CDRMessage_t*,uint size);
	/**
	 * Initialize with default size.
	 * @return
	 */
	static bool initCDRMsg(CDRMessage_t*);
	/**
	 * Append given CDRMessage to existing CDR Message.
	 * @param Pointer to first message.
	 * @param Pointer to second message.
	 * @return
	 */
	static bool appendMsg(CDRMessage_t*,CDRMessage_t*);
	/**
	 * Add data to buffer.
	 * @param Pointer to message.
	 * @param Pointer to data
	 * @param Number of bytes
	 * @return
	 */
	static bool addData(CDRMessage_t*,octet*,uint);
	/**
	 * add data to stream in a reversed manner
	 * @param Pointer to message.
	 * @param Pointer to data
	 * @param Number of bytes
	 * @return
	 */
	static bool addDataReversed(CDRMessage_t*,octet*,uint);
	/**
	 * Add octet to buffer.
	 * @param Pointer to message.
	 * @param octet to add.
	 * @return
	 */
	static bool addOctet(CDRMessage_t*msg,octet oc);
	/**
	 *
	 * @param msg
	 * @param us
	 * @return
	 */
	static bool addUInt16(CDRMessage_t*msg,uint16_t us);
	/**
	 *
	 * @param msg
	 * @param lo
	 * @return
	 */
	static bool addInt32(CDRMessage_t*msg,int32_t lo);
	/**
	 *
	 * @param msg
	 * @param lo
	 * @return
	 */
	static bool addUInt32(CDRMessage_t*msg,uint32_t lo);
	/**
	 *
	 * @param msg
	 * @param lo
	 * @return
	 */
	static bool addInt64(CDRMessage_t*msg,int64_t lo);
	/**
	 *
	 * @param msg
	 * @param id
	 * @return
	 */
	static bool addEntityId(CDRMessage_t*msg,EntityId_t* id);
	/**
	 *
	 * @param msg
	 * @param param
	 * @return
	 */
	static bool addParameter(CDRMessage_t*msg,Parameter_t* param);
	/**
	 *
	 * @param msg
	 * @param sn
	 * @return
	 */
	static bool addSequenceNumber(CDRMessage_t*msg,SequenceNumber_t* sn);

	/**
	 * Add a SequenceNumberSet to the serialized message. More information...
	 * @param msg
	 * @param sns
	 * @return
	 */
	static bool addSequenceNumberSet(CDRMessage_t*msg,SequenceNumberSet_t* sns);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CDRMESSAGE_H_ */
