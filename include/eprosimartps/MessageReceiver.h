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

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/ParameterList.h"
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
	void processCDRMsg(GuidPrefix_t& participantguidprefix,
					Locator_t* loc, CDRMessage_t*msg);

	//!Pointer to the Listen Thread that contains this MessageReceiver.
	ThreadListen* mp_threadListen;

	CDRMessage_t m_rec_msg;
	ParameterList_t m_ParamList;

private:
//	//!CDRMEssage_t being processed.
//	CDRMessage_t msg;
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
	//!Default locator used in reset
	Locator_t defUniLoc;


	/**@name Processing methods.
	 * These methods are designed to read a part of the message
	 * and perform the corresponding actions:
	 * -Modify the message receiver state if necessary.
	 * -Add information to the history.
	 * -Return an error if the message is malformed.
	 * @param[in] msg Pointer to the message
	 * @param[out] params Different parameters depending on the message
	 * @return True if correct, false otherwise
	 */

	///@{

	bool checkRTPSHeader(CDRMessage_t*msg);
	bool readSubmessageHeader(CDRMessage_t*msg, SubmessageHeader_t* smh);

	bool proc_Submsg_Data(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);

	bool proc_Submsg_Acknack(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_Heartbeat(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_Gap(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_InfoTS(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);

	///@}





};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* MESSAGERECEIVER_H_ */
