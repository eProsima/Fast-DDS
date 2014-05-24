/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file rtps_messages.h
 *	Messages structures definition.
 *  Created on: Feb 18, 2014
 *      Author: Gonzalo Rodriguez Canosa
 */

#ifndef RTPS_MESSAGES_H_
#define RTPS_MESSAGES_H_



#include <vector>
#include <iostream>
#include <bitset>

using std::cout;
using std::endl;
using std::bitset;

#include "eprosimartps/qos/ParameterList.h"

using eprosima::dds::ParameterList_t;

namespace eprosima{
namespace rtps{

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/** @defgroup RTPSMESSAGES RTPS Messages structures
  * @ingroup COMMONMODULE
 * Structures to hold different RTPS Messages and submessages structures.
 *  @{
 */












//
//// SUBMESSAGE types definition
//
////!@brief Structure SubmsgData_t, contains the information necessary to create a Data Submessage.
//typedef struct SubmsgData_t{
//	SubmessageHeader_t SubmessageHeader;
//	bool expectsInlineQos;
//	InstanceHandle_t instanceHandle;
//	ChangeKind_t changeKind;
//	EntityId_t readerId;
//	EntityId_t writerId;
//	SequenceNumber_t writerSN;
//	SerializedPayload_t serializedPayload;
//	ParameterList_t inlineQos;
//	void print(){
//		pLongInfo( "DATA SubMsg,flags: " << (bitset<8>) SubmessageHeader.flags << endl);
//		pLongInfo(  "readerId: " << (int)readerId.value[0] << "." << (int)readerId.value[1] << "." << (int)readerId.value[2] << "." << (int)readerId.value[3]);
//		pLongInfo(  " || writerId: " << (int)writerId.value[0] << "." << (int)writerId.value[1] << "." << (int)writerId.value[2] << "." << (int)writerId.value[3] << endl);
//		pLongInfo(  "InlineQos: " << inlineQos.m_parameters.size() << " parameters." << endl);
//		pLongInfo(  "SeqNum: " << writerSN.to64long() << " Payload: enc: " << serializedPayload.encapsulation << " length: " << serializedPayload.length << endl);
//		pLongInfoPrint;
//	}
//}SubmsgData_t;
//
////!@brief Structure SubmsgHeartbeat_t, contains the information necessary to create a Heartbeat Submessage.
//typedef struct{
//	SubmessageHeader_t SubmessageHeader;
//	bool endiannessFlag;
//	bool finalFlag;
//	bool livelinessFlag;
//	EntityId_t readerId;
//	EntityId_t writerId;
//	SequenceNumber_t firstSN;
//	SequenceNumber_t lastSN;
//	Count_t count;
//}SubmsgHeartbeat_t;
//
////!@brief Structure SubmsgAcknack_t, contains the information necessary to create a Acknack Submessage.
//typedef struct{
//	SubmessageHeader_t SubmessageHeader;
//	bool endiannessFlag;
//	bool finalFlag;
//	EntityId_t readerId;
//	EntityId_t writerId;
//	SequenceNumberSet_t readerSNState;
//	Count_t count;
//}SubmsgAcknack_t;
//
////!@brief Structure SubmsgGap_t, contains the information necessary to create a Gap Submessage.
//typedef struct{
//	SubmessageHeader_t SubmessageHeader;
//	bool endiannessFlag;
//	EntityId_t readerId;
//	EntityId_t writerId;
//	SequenceNumber_t gapStart;
//	SequenceNumberSet_t gapList;
//}SubmsgGap_t;
//
//
////!@brief Structure SubmsgInfoTS_t, contains the information necessary to create a InfoTS Submessage.
//typedef struct{
//	SubmessageHeader_t SubmessageHeader;
//	Time_t timestamp;
//}SubmsgInfoTS_t;
//
/////@}
//

#endif

}
}



#endif /* RTPS_MESSAGES_H_ */
