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
 * @brief Generation of serialized CDR RTPS Messages.
 */
class CDRMessageCreator {
public:
	CDRMessageCreator();
	virtual ~CDRMessageCreator();


	/**
	 * @brief Create a Header to the serialized message.
	 * @param msg Pointer to the Message.
	 * @param H Pointer to the header structure.
	 * @return true or false
	 */
	bool createHeader(CDRMessage_t*msg ,Header_t* H);
	/**
	 * @brief Create SubmessageHeader
	 * @param msg Pointer to the CDRMessage
	 * @param SubMH Pointer to the SubMessageHeader
	 * @param length Length of the corresponding message
	 * @return true or false
	 */
	bool createSubmessageHeader(CDRMessage_t* msg,SubmessageHeader_t* SubMH,unsigned short submsgsize);
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
	bool addData(CDRMessage_t*,octet*,uint);
	/**
	 * add data to stream in a reversed manner
	 * @param Pointer to message.
	 * @param Pointer to data
	 * @param Number of bytes
	 * @return
	 */
	bool addDataReversed(CDRMessage_t*,octet*,uint);
	/**
	 * Add octet to buffer.
	 * @param Pointer to message.
	 * @param octet to add.
	 * @return
	 */
	bool addOctet(CDRMessage_t*msg,octet oc);
	/**
	 *
	 * @param msg
	 * @param us
	 * @return
	 */
	bool addUInt16(CDRMessage_t*msg,uint16_t us);
	/**
	 *
	 * @param msg
	 * @param lo
	 * @return
	 */
	bool addInt32(CDRMessage_t*msg,int32_t lo);
	/**
	 *
	 * @param msg
	 * @param lo
	 * @return
	 */
	bool addUInt32(CDRMessage_t*msg,uint32_t lo);
	/**
	 *
	 * @param msg
	 * @param lo
	 * @return
	 */
	bool addInt64(CDRMessage_t*msg,int64_t lo);
	/**
	 *
	 * @param msg
	 * @param id
	 * @return
	 */
	bool addEntityId(CDRMessage_t*msg,EntityId_t* id);
	/**
	 *
	 * @param msg
	 * @param param
	 * @return
	 */
	bool addParameter(CDRMessage_t*msg,Parameter_t* param);
	/**
	 *
	 * @param msg
	 * @param sn
	 * @return
	 */
	bool addSequenceNumber(CDRMessage_t*msg,SequenceNumber_t* sn);

	/**
	 * Add a SequenceNumberSet to the serialized message. More information...
	 * @param msg
	 * @param sns
	 * @return
	 */
	bool addSequenceNumberSet(CDRMessage_t*msg,SequenceNumberSet_t* sns);

	//**************************************************************************
	// SUBMESSAGES FUNCTION DEFINITION  ******************************
	/**
	 * Create a Data RTPS Message.
	 * @param msg CDR serialized message pointer where the message is going to be stores.
	 * @param guidprefix guid prefix
	 * @param DataSubM Data submessage structure
	 * @return
	 */
	bool createMessageData(CDRMessage_t* msg,GuidPrefix_t guidprefix,SubmsgData_t* DataSubM);
	/**
	 * @brief This function creates a CDR submessage representation for the Data Submessage.
	 * @param submsg pointer to the submessage
	 * @param DataSubM  pointer to the submessage structure
	 * @return true if everything is correct.
	 */
	bool createSubmessageData(CDRMessage_t* submsg,SubmsgData_t* DataSubM);

	/**
	 *
	 * @param msg
	 * @param guidprefix
	 * @param HBSubM
	 * @return
	 */
	bool createMessageHeartbeat(CDRMessage_t* msg,GuidPrefix_t guidprefix,SubmsgHeartbeat_t* HBSubM);
	/**
	 *
	 * @param submsg
	 * @param HBSubM
	 * @return
	 */
	bool createSubmessageHeartbeat(CDRMessage_t* submsg,SubmsgHeartbeat_t* HBSubM);
	/**
	 *
	 * @param msg
	 * @param guidprefix
	 * @param SubM
	 * @return
	 */
	bool createMessageAcknack(CDRMessage_t* msg,GuidPrefix_t guidprefix,SubmsgAcknack_t* SubM);
	/**
	 *
	 * @param submsg
	 * @param SubM
	 * @return
	 */
	bool createSubmessageAcknack(CDRMessage_t* submsg,SubmsgAcknack_t* SubM);
	/**
	 *
	 * @param msg
	 * @param guidprefix
	 * @param SubM
	 * @return
	 */
	bool createMessageGap(CDRMessage_t* msg,GuidPrefix_t guidprefix, SubmsgGap_t* SubM);
	/**
	 *
	 * @param msg
	 * @param SubM
	 * @return
	 */
	bool createSubmessageGap(CDRMessage_t* msg, SubmsgGap_t* SubM);




};

}; /* namespace rtps */
}; /* namespace eprosima */

#endif /* CDRMESSAGECREATOR_H_ */
