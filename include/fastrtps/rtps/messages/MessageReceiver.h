/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MessageReceiver.h
 */



#ifndef MESSAGERECEIVER_H_
#define MESSAGERECEIVER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../common/all_common.h"
#include "../../qos/ParameterList.h"

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

class ListenResource;
struct SubmessageHeader_t;

/**
 * Class MessageReceiver, process the received messages.
 * @ingroup MANAGEMENT_MODULE
 */
class MessageReceiver {
public:
	/**
	* @param rec_buffer_size
	*/
	MessageReceiver(uint32_t rec_buffer_size);
	virtual ~MessageReceiver();
	//!Reset the MessageReceiver to process a new message.
	void reset();
	/**
	 * Process a new CDR message.
	 * @param[in] RTPSParticipantguidprefix RTPSParticipant Guid Prefix
	 * @param[in] loc Locator indicating the sending address.
	 * @param[in] msg Pointer to the message
	 */
	void processCDRMsg(const GuidPrefix_t& RTPSParticipantguidprefix,Locator_t* loc, CDRMessage_t*msg);

	//!Pointer to the Listen Resource that contains this MessageReceiver.
	ListenResource* mp_threadListen;
	//!Received message
	CDRMessage_t m_rec_msg;
	//!PArameter list
	ParameterList_t m_ParamList;



private:
	CacheChange_t* mp_change;
	//!Protocol version of the message
	ProtocolVersion_t sourceVersion;
	//!VendorID that created the message
	VendorId_t sourceVendorId;
	//!GuidPrefix of the entity that created the message
	GuidPrefix_t sourceGuidPrefix;
	//!GuidPrefix of the entity that receives the message. GuidPrefix of the RTPSParticipant.
	GuidPrefix_t destGuidPrefix;
	//!Reply addresses (unicast).
	LocatorList_t unicastReplyLocatorList;
	//!Reply addresses (multicast).
	LocatorList_t multicastReplyLocatorList;
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
	/**
	 * Check the RTPSHeader of a received message.
	 * @param msg Pointer to the message.
	 * @return True if correct.
	 */
	bool checkRTPSHeader(CDRMessage_t*msg);
	/**
	 * Read the submessage header of a message.
	 * @param msg Pointer to the CDRMessage_t to read.
	 * @param smh Pointer to the submessageheader structure.
	 * @return True if correctly read.
	 */
	bool readSubmessageHeader(CDRMessage_t*msg, SubmessageHeader_t* smh);
	/**
	 *
	 * @param msg
	 * @param smh
	 * @param last
	 * @return
	 */
	bool proc_Submsg_Data(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_DataFrag(CDRMessage_t*msg, SubmessageHeader_t* smh, bool*last);
	bool proc_Submsg_Acknack(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_Heartbeat(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_Gap(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_InfoTS(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_InfoDST(CDRMessage_t*msg,SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_InfoSRC(CDRMessage_t*msg,SubmessageHeader_t* smh,bool*last);
	bool proc_Submsg_NackFrag(CDRMessage_t*msg, SubmessageHeader_t* smh, bool*last);
	bool proc_Submsg_HeartbeatFrag(CDRMessage_t*msg, SubmessageHeader_t* smh, bool*last);

	///@}





};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* MESSAGERECEIVER_H_ */
