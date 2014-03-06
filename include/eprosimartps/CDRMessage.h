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

/**
 * Class CDRMessage, contains static methods to initialize CDRMessage_t and add or read different data types.
 @ingroup RTPSMODULE
 */
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
	static bool readLocator(CDRMessage_t* msg,Locator_t* loc);
	static bool readOctet(CDRMessage_t* msg,octet* o);
	/**
	 * @brief Initialize given CDR message with a given byte size. It frees the memory already allocated and reserves new one.
	 * @param size Given byte size.
	 * @return True if succeeded.
	 */
	static bool initCDRMsg(CDRMessage_t*,uint size);
	/**
	 * Initialize given CDR message with default size. It frees the memory already allocated and reserves new one.
	 * @return True if succeeded.
	 */
	static bool initCDRMsg(CDRMessage_t*);
	/**
	 * Append given CDRMessage to existing CDR Message. Joins two messages into the first one if it has space.
	 * @param Pointer to first message.
	 * @param Pointer to second message.
	 * @return Tur if suceeded.
	 */
	static bool appendMsg(CDRMessage_t*,CDRMessage_t*);
	/**
	 * Add data to CDR message buffer.  The length and write position of the message are updated.
	 * @param Pointer to message.
	 * @param Pointer to data.
	 * @param Number of bytes.
	 * @return True if succeeded.
	 */
	static bool addData(CDRMessage_t*,octet*,uint);
	/**
	 * Add data to stream in a reversed manner.  The length and write position of the message are updated.
	 * @param Pointer to message.
	 * @param Pointer to data.
	 * @param Number of bytes.
	 * @return True if succeeded.
	 */
	static bool addDataReversed(CDRMessage_t*,octet*,uint);
	/**
	 * Add octet to buffer. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param in Octet to add.
	 * @return True if succeeded.
	 */
	static bool addOctet(CDRMessage_t*msg,octet in);
	/**
	 * Add UInt16 to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param us Uint16 to add.
	 * @return True if succedeed. 
	 */
	static bool addUInt16(CDRMessage_t*msg,uint16_t us);
	/**
	 *
	 * Add UInt32 to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param lo Uint32 to add.
	 * @return True if succedeed. 
	 */
	static bool addInt32(CDRMessage_t*msg,int32_t lo);
	/**
	 * Add Int32 to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param lo Uint16 to add.
	 * @return True if succedeed. 
	 */
	static bool addUInt32(CDRMessage_t*msg,uint32_t lo);
	/**
	 * Add Int64 to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param lo Uint16 to add.
	 * @return True if succedeed. 
	 */
	static bool addInt64(CDRMessage_t*msg,int64_t lo);
	/**
	 * Add EntityId to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param id Pointer to EntityId to add.
	 * @return True if succedeed. 
	 */
	static bool addEntityId(CDRMessage_t*msg,EntityId_t* id);
	/**
	 * Add SequenceNumber to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param sn Pointer to SequenceNumber to add.
	 * @return True if succedeed. 
	 */
	static bool addSequenceNumber(CDRMessage_t*msg,SequenceNumber_t* sn);

	/**
	 * Add SequenceNumberSet to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param sns Pointer to SequenceNumberSet to add.
	 * @return True if succedeed.
	 */
	static bool addSequenceNumberSet(CDRMessage_t*msg,SequenceNumberSet_t* sns);
	/**
	 * Add Locator_t to CDRMessage. The length and write position of the message are updated.
	 * @param msg Pointer to message.
	 * @param loc Pointer to Locator_t to add.
	 * @return True if succedeed.
	 */
	static bool addLocator(CDRMessage_t*msg,Locator_t*loc);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CDRMESSAGE_H_ */
