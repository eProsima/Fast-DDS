/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSMessageCreator.h
 *	RTPS Message creator functions.
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/utils/ObjectPool.h"
#include "eprosimartps/common/attributes/TopicAttributes.h"

#ifndef CDRMESSAGECREATOR_H_
#define CDRMESSAGECREATOR_H_

using namespace eprosima::dds;

namespace eprosima {
namespace rtps{

/**
 * @brief Class CDRMessageCreator, allows the generation of serialized CDR RTPS Messages.
 * @ingroup MANAGEMENTMODULE
 */
class RTPSMessageCreator {
public:
	RTPSMessageCreator();
	virtual ~RTPSMessageCreator();

	CDRMessage_t rtpsmc_submsgElem;
	CDRMessage_t rtpsmc_submsgHeader;
	/**
	 * Create a Header to the serialized message.
	 * @param msg Pointer to the Message.
	 * @param Prefix Participant prefix of the message.
	 * @param version Protocol version.
	 * @param vendorId Vendor Id.
	 * @return True if correct.
	 */
	static bool addHeader(CDRMessage_t*msg ,GuidPrefix_t& Prefix,ProtocolVersion_t version,VendorId_t vendorId);

	static bool addHeader(CDRMessage_t*msg ,GuidPrefix_t& Prefix);

	/**
	 * Create SubmessageHeader.
	 * @param msg Pointer to the CDRMessage.
	 * @param id SubMessage Id.
	 * @param flags Submessage flags.
	 * @param size Submessage size.
	 * @return True if correct.
	 */
	static bool addSubmessageHeader(CDRMessage_t* msg,octet id,octet flags,uint16_t size);


	/** @name CDR messages creation methods.
	 * These methods create a CDR message for different types
	 * Depending on the function a complete message (with RTPS Header is created) or only the submessage.
	 * @param[out] msg Pointer to where the message is going to be created and stored.
	 * @param[in] guidPrefix Guid Prefix of the participant.
	 * @param[in] param Different parameters depending on the message.
	 * @return True if correct.
	 */

	/// @{



	static bool addMessageData(CDRMessage_t* msg,GuidPrefix_t& guidprefix,CacheChange_t* change,
			TopicKind_t topicKind,const EntityId_t& readerId,ParameterList_t* inlineQos);
	static bool addSubmessageData(CDRMessage_t* msg,CacheChange_t* change,
			TopicKind_t topicKind,const EntityId_t& readerId,ParameterList_t* inlineQos);

	static bool addMessageGap(CDRMessage_t* msg,GuidPrefix_t& guidprefix,
			SequenceNumber_t& seqNumFirst,SequenceNumberSet_t& seqNumList,const EntityId_t& readerId,const EntityId_t& writerId);
	static bool addSubmessageGap(CDRMessage_t* msg,SequenceNumber_t& seqNumFirst,SequenceNumberSet_t& seqNumList,const EntityId_t& readerId,const EntityId_t& writerId);

	static bool addMessageHeartbeat(CDRMessage_t* msg,GuidPrefix_t& guidprefix,const EntityId_t& readerId,const EntityId_t& writerId,
			SequenceNumber_t& firstSN,SequenceNumber_t& lastSN,int32_t count,bool isFinal,bool livelinessFlag);

	static bool addSubmessageHeartbeat(CDRMessage_t* msg,const EntityId_t& readerId,const EntityId_t& writerId,
			SequenceNumber_t& firstSN,SequenceNumber_t& lastSN,int32_t count,bool isFinal,bool livelinessFlag);

	static bool addMessageAcknack(CDRMessage_t* msg,GuidPrefix_t& guidprefix,
			const EntityId_t& readerId,const EntityId_t& writerId,SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag);

	static bool addSubmessageAcknack(CDRMessage_t* msg,
			const EntityId_t& readerId,const EntityId_t& writerId,SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag);


	static bool addSubmessageInfoTS(CDRMessage_t* msg,Time_t& time,bool invalidateFlag);
	static bool addSubmessageInfoTS_Now(CDRMessage_t* msg,bool invalidateFlag);

	///@}


};

}; /* namespace rtps */
}; /* namespace eprosima */

#endif /* CDRMESSAGECREATOR_H_ */
