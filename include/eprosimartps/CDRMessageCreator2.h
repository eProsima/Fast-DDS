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



/**
 * @brief Class CDRMessageCreator, allows the generation of serialized CDR RTPS Messages.
 * @ingroup COMMONMODULE
 */
class CDRMessageCreator2 {
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

	/**
	 * Create SubmessageHeader.
	 * @param msg Pointer to the CDRMessage.
	 * @param id SubMessage Id.
	 * @param flags Submessage flags.
	 * @param size Submessage size.
	 * @return True if correct.
	 */
	static bool createSubmessageHeader(CDRMessage_t* msg,octet id,octet flags,uint16_t size);



	static bool createMessageData(CDRMessage_t* msg,GuidPrefix_t guidprefix,CacheChange_t* change,TopicKind_t topicKind,bool expectsInline);
	static bool createSubmessageData(CDRMessage_t* msg,CacheChange_t* change,TopicKind_t topicKind,bool expectsInline);





	/** @name CDR messages creation methods.
	 * These methods create a CDR message from a Submessage structure.
	 * Depending on the function a complete message (with RTPS Header is created) or only the submessage.
	 * @param[out] msg Pointer to where the message is going to be created and stored.
	 * @param[in] guidPrefix Guid Prefix of the participant.
	 * @param[in] SubM Submessage structure.
	 * @param[in] W Pointer to the writer (if needed to retrieve inlineQos).
	 */

	/// @{
	//!Create a Data Message
	bool createMessageData(CDRMessage_t* msg,GuidPrefix_t guidprefix,SubmsgData_t* DataSubM,RTPSWriter* W);



	//! Create a Data Submessage.
	bool createSubmessageData(CDRMessage_t* submsg,SubmsgData_t* DataSubM,RTPSWriter* W);
	//! Create a Heartbeat message. NOT YET TESTED.
	bool createMessageHeartbeat(CDRMessage_t* msg,GuidPrefix_t guidprefix,SubmsgHeartbeat_t* HBSubM);
	//! Create a Heartbeat submessage. NOT YET TESTED.
	bool createSubmessageHeartbeat(CDRMessage_t* submsg,SubmsgHeartbeat_t* HBSubM);
	//! Create a Acknack message. NOT YET TESTED.
	bool createMessageAcknack(CDRMessage_t* msg,GuidPrefix_t guidprefix,SubmsgAcknack_t* SubM);
	//! Create a Acknack submessage. NOT YET TESTED.
	bool createSubmessageAcknack(CDRMessage_t* submsg,SubmsgAcknack_t* SubM);
	//! Create a Gap message. NOT YET TESTED.
	bool createMessageGap(CDRMessage_t* msg,GuidPrefix_t guidprefix, SubmsgGap_t* SubM);
	//! Create a Gap submessage. NOT YET TESTED.
	bool createSubmessageGap(CDRMessage_t* msg, SubmsgGap_t* SubM);
	/// @}



};

}; /* namespace rtps */
}; /* namespace eprosima */

#endif /* CDRMESSAGECREATOR_H_ */
