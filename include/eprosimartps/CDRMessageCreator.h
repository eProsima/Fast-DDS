/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessageCreator2.h
 *	CDR Message creator functions.
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "rtps_all.h"



#ifndef CDRMESSAGECREATOR_H_
#define CDRMESSAGECREATOR_H_



namespace eprosima {
namespace rtps{

class ParameterList_t;

/**
 * @brief Class CDRMessageCreator, allows the generation of serialized CDR RTPS Messages.
 * @ingroup COMMONMODULE
 */
class CDRMessageCreator {
public:
	CDRMessageCreator();
	virtual ~CDRMessageCreator();


	/**
	 * Create a Header to the serialized message.
	 * @param msg Pointer to the Message.
	 * @param Prefix Participant prefix of the message.
	 * @param version Protocol version.
	 * @param vendorId Vendor Id.
	 * @return True if correct.
	 */
	static bool createHeader(CDRMessage_t*msg ,GuidPrefix_t Prefix,ProtocolVersion_t version,VendorId_t vendorId);

	static bool createHeader(CDRMessage_t*msg ,GuidPrefix_t Prefix);

	/**
	 * Create SubmessageHeader.
	 * @param msg Pointer to the CDRMessage.
	 * @param id SubMessage Id.
	 * @param flags Submessage flags.
	 * @param size Submessage size.
	 * @return True if correct.
	 */
	static bool createSubmessageHeader(CDRMessage_t* msg,octet id,octet flags,uint16_t size);


	/** @name CDR messages creation methods.
	 * These methods create a CDR message for different types
	 * Depending on the function a complete message (with RTPS Header is created) or only the submessage.
	 * @param[out] msg Pointer to where the message is going to be created and stored.
	 * @param[in] guidPrefix Guid Prefix of the participant.
	 * @param[in] param Different parameters depending on the message.
	 * @return True if correct.
	 */

	/// @{



	static bool createMessageData(CDRMessage_t* msg,GuidPrefix_t guidprefix,CacheChange_t* change,
			TopicKind_t topicKind,EntityId_t readerId,ParameterList_t* inlineQos);
	static bool createSubmessageData(CDRMessage_t* msg,CacheChange_t* change,
			TopicKind_t topicKind,EntityId_t readerId,ParameterList_t* inlineQos);

	static bool createMessageGap(CDRMessage_t* msg,GuidPrefix_t guidprefix,
			SequenceNumber_t seqNumFirst,SequenceNumberSet_t seqNumList,EntityId_t readerId,EntityId_t writerId);
	static bool createSubmessageGap(CDRMessage_t* msg,SequenceNumber_t seqNumFirst,SequenceNumberSet_t seqNumList,EntityId_t readerId,EntityId_t writerId);

	static bool createMessageHeartbeat(CDRMessage_t* msg,GuidPrefix_t guidprefix,EntityId_t readerId,EntityId_t writerId,
			SequenceNumber_t firstSN,SequenceNumber_t lastSN,int32_t count,bool isFinal,bool livelinessFlag);

	static bool createSubmessageHeartbeat(CDRMessage_t* msg,EntityId_t readerId,EntityId_t writerId,
			SequenceNumber_t firstSN,SequenceNumber_t lastSN,int32_t count,bool isFinal,bool livelinessFlag);

	static bool createMessageAcknack(CDRMessage_t* msg,GuidPrefix_t guidprefix,
			EntityId_t readerId,EntityId_t writerId,SequenceNumberSet_t SNSet,int32_t count,bool finalFlag);

	static bool createSubmessageAcknack(CDRMessage_t* msg,
			EntityId_t readerId,EntityId_t writerId,SequenceNumberSet_t SNSet,int32_t count,bool finalFlag);


	static bool createSubmessageInfoTS(CDRMessage_t* msg,Time_t time,bool invalidateFlag);
	static bool createSubmessageInfoTS_Now(CDRMessage_t* msg,bool invalidateFlag);

	///@}


};

}; /* namespace rtps */
}; /* namespace eprosima */

#endif /* CDRMESSAGECREATOR_H_ */
