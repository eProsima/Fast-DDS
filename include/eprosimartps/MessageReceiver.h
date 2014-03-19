/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MessageReceiver.h
 *	MessageReceiver to read and process CDR messages.
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "rtps_all.h"
#include "ParameterList_t.h"
#include "common/rtps_messages.h"

#ifndef MESSAGERECEIVER_H_
#define MESSAGERECEIVER_H_



namespace eprosima {
namespace rtps {

class ThreadListen;

/**
 * Class MessageReceiver, process the received messages.
  * @ingroup COMMONMODULE
 */
class MessageReceiver {
public:
	MessageReceiver();
	virtual ~MessageReceiver();
	//!Reset the MessageReceiver to process a new message.
	void reset();
	/**
	 * Process a new CDR message.
	 * @param[in] participantguidprefix Participant Guid Prefix
	 * @param[in] loc Locator indicating the sending address.
	 * @param[in] msg Pointer to the message
	 */
	void processCDRMsg(GuidPrefix_t participantguidprefix,
					Locator_t loc, CDRMessage_t*msg);

	//!Pointer to the Listen Thread that contains this MessageReceiver.
	ThreadListen* threadListen_ptr;

private:
	//!CDRMEssage_t being processed.
	CDRMessage_t msg;
	//!Protocolverison of the message
	ProtocolVersion_t sourceVersion;
	//!VendorID that created the message
	VendorId_t sourceVendorId;
	//!GuidPrefix of the entity that created the message
	GuidPrefix_t sourceGuidPrefix;
	//!GuidPrefix of the entity that receives the message. GuidPrefix of the Participant.
	GuidPrefix_t destGuidPrefix;
	//!Reply addresses (unicast).
	std::vector<Locator_t> unicastReplyLocatorList;
	//!Reply addresses (multicast).
	std::vector<Locator_t> multicastReplyLocatorList;
	//!Has the message timestamp?
	bool haveTimestamp;
	//!Timestamp associated with the message
	Time_t timestamp;
	//!Version of the protocol used by the receiving end.
	ProtocolVersion_t destVersion;


	/** @name Read methods.
	 * These methods read a specific structure from a CDRMessage.
	 * @param[in] msg Pointer to the message that is being read.
	 * @param[out] SubM Pointer to the structure that is being read.
	 * @param[in] W Pointer to the writer (if needed to retrieve inlineQos).
	 * @param[out] last Pointer to a bool field to indicate if the submessage if the last.
	 * @return True if correct.
	 */

	///@{
	bool readHeader(CDRMessage_t* msg, Header_t*H);

	bool readSubmessageHeader(CDRMessage_t*msg, SubmessageHeader_t* smh);

	bool readSubmessageData(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last,SubmsgData_t* SubmsgData);



	//!NOT IMPLEMENTED
	bool readSubmessageHeartbeat(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	//!NOT IMPLEMENTED
	bool readSubmessageGap(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	//!NOT IMPLEMENTED
	bool readSubmessageAcknak(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	//!NOT IMPLEMENTED
	bool readSubmessagePad(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	//!NOT IMPLEMENTED
	bool readSubmessageInfoDestination(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	//!NOT IMPLEMENTED
	bool readSubmessageInfoSource(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	//!NOT IMPLEMENTED
	bool readSubmessageInfoTimestamp(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	//!NOT IMPLEMENTED
	bool readSubmessageInfoReply(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);


	///@}

	/** @name Process methods.
	 * These methods process a specific message structure.
	 * These methods change the state of the MessageReceiver.
	 * @param[in] SubM Pointer to the structure that is being processed.
	 * @return True if correct.
	 */

	///@{

	void processHeader(Header_t*H);

	void processSubmessageData(SubmsgData_t* SubmsgData);

	///@}

	/** @name Reading and process methods.
	 * These methods read the specific message type and perform the necesary actions in the MessageReceiver.
	 * @return True if correct.
	 */
	///@{

	bool proc_Submsg_Data(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);


	///@}


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* MESSAGERECEIVER_H_ */
