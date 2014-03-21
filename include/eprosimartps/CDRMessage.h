/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessage.h
 *	CDR Message initialization, adding and reading elements.
 *  Created on: Feb 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/rtps_all.h"

#include "eprosimartps/dds/ParameterTypes.h"

using eprosima::dds::ParameterId_t;

#ifndef CDRMESSAGE_H_
#define CDRMESSAGE_H_

namespace eprosima {
namespace rtps {

/**
 * Class CDRMessage, contains static methods to initialize CDRMessage_t and add or read different data types.
 @ingroup COMMONMODULE
 */
class CDRMessage {
public:
	CDRMessage();
	virtual ~CDRMessage();

	/** @name Read from a CDRMessage_t.
	 * Methods to read different data types from a CDR message. Pointers to the message and to the data types are provided.
	 * The read position is updated in the message. It fails if you attempt to read outside the
	 * boundaries of the message.
	 * @param[in] msg Pointer to message.
	 * @param[out] data_ptr Pointer to data.
	 * @param[in] size Number of bytes (if necessary).
	 * @return True if correct.
	 */

	/// @{
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

	///@}
//	/**
//	 * @brief Initialize given CDR message with a given byte size. It frees the memory already allocated and reserves new one.
//	 * @param[in,out] msg Pointer to the message to initialize.
//	 * @param[in] size Maximum size of the message.
//	 * @return True if correct.
//	 */
//	static bool initCDRMsg(CDRMessage_t* msg,uint size);
	/**
	 * Initialize given CDR message with default size. It frees the memory already allocated and reserves new one.
	 * @param[in,out] msg Pointer to the message to initialize.
	 * @return True if correct.
	 */
	static bool initCDRMsg(CDRMessage_t* msg);
	/**
	 * Append given CDRMessage to existing CDR Message. Joins two messages into the first one if it has space.
	 * @param[out] first Pointer to first message.
	 * @param[in] second Pointer to second message.
	 * @return True if correct.
	 */
	static bool appendMsg(CDRMessage_t* first,CDRMessage_t* second);


	/** @name Add to a CDRMessage_t.
	 * Methods to add different data types to a CDR message. Pointers to the message and to the data types are provided.
	 * The write position is updated in the message. It fails if you attempt to write outside the
	 * boundaries of the message.
	 * @param[in,out] Pointer to message.
	 * @param[in] data Data to add (might be a pointer).
	 * @param[in] byteSize Number of bytes (if necessary).
	 * @return True if correct.
	 */
	/// @{

	static bool addData(CDRMessage_t*,octet*,uint number_bytes);
	static bool addDataReversed(CDRMessage_t*,octet*,uint byte_number);
	static bool addOctet(CDRMessage_t*msg,octet o);
	static bool addUInt16(CDRMessage_t*msg,uint16_t us);
	static bool addInt32(CDRMessage_t*msg,int32_t lo);
	static bool addUInt32(CDRMessage_t*msg,uint32_t lo);
	static bool addInt64(CDRMessage_t*msg,int64_t lo);
	static bool addEntityId(CDRMessage_t*msg,EntityId_t* id);
	static bool addSequenceNumber(CDRMessage_t*msg,SequenceNumber_t* sn);
	static bool addSequenceNumberSet(CDRMessage_t*msg,SequenceNumberSet_t* sns);
	static bool addLocator(CDRMessage_t*msg,Locator_t*loc);
	static bool addParameterStatus(CDRMessage_t*msg,octet status);
	static bool addParameterKey(CDRMessage_t*msg,InstanceHandle_t* iHandle);
	static bool addParameterSentinel(CDRMessage_t*msg);
	static bool addParameterId(CDRMessage_t*msg,ParameterId_t pid);
	///@}
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CDRMESSAGE_H_ */
