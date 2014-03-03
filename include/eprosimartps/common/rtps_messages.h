/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * rtps_messages.h
 *
 *  Created on: Feb 18, 2014
 *      Author: Gonzalo Rodriguez Canosa
 */

#ifndef RTPS_MESSAGES_H_
#define RTPS_MESSAGES_H_

#include <vector>
#include <iostream>



namespace eprosima{
namespace rtps{

//!@brief Structure Header_t, RTPS Message Header Structure.
typedef struct Header_t{
	ProtocolVersion_t version;
	VendorId_t vendorId;;
	GuidPrefix_t guidPrefix;
	Header_t(){
		PROTOCOLVERSION(version);
		VENDORID_EPROSIMA(vendorId);
	}
	~Header_t(){
	}
	void print(){
		cout << "RTPS HEADER of Version: " << (int)version.major << "." << (int)version.minor;
		cout << "  || VendorId: " << (int)vendorId[0] << "." <<(int)vendorId[1] << endl;
		cout << "GuidPrefix: ";
		for(int i =0;i<12;i++)
			cout << (int)guidPrefix.value[i] << ".";
		cout << endl;
	}
}Header_t;




// //!@brief Enumeration of the different Submessages types
#define	PAD 0x01
#define	ACKNACK 0x06
#define	HEARTBEAT 0x07
#define	GAP 0x08
#define	INFO_TS 0x09
#define	INFO_SRC 0x0c
#define	INFO_REPLY_IP4 0x0d
#define	INFO_DST 0x0e
#define	INFO_REPLY 0x0f
#define	NACK_FRAG 0x12
#define	HEARTBEAT_FRAG 0x13
#define	DATA 0x15
#define	DATA_FRAG 0x16


//!@brief Structure SubmessageHeader_t.
typedef struct SubmessageHeader_t{
	octet submessageId;
	uint16_t submessageLength;
	SubmessageFlag flags;
	void print (){
		cout << "Submessage Header, ID: " << (int)submessageId;
		cout << " length: " << (int)submessageLength << " flags " << hex << flags <<dec<< endl;
	}

}SubmessageHeader_t;



// SUBMESSAGE types definition

//!@brief Structure SubmsgData_t, contains the information necessary to create a Data Submessage.
typedef struct SubmsgData_t{
	SubmessageHeader_t SubmessageHeader;
	bool endiannessFlag;
	bool inlineQosFlag;
	bool dataFlag;
	bool keyFlag;
	EntityId_t readerId;
	EntityId_t writerId;
	SequenceNumber_t writerSN;
	SerializedPayload_t serializedPayload;
}SubmsgData_t;

//!@brief Structure SubmsgHeartbeat_t, contains the information necessary to create a Heartbeat Submessage.
typedef struct{
	SubmessageHeader_t SubmessageHeader;
	bool endiannessFlag;
	bool finalFlag;
	bool livelinessFlag;
	EntityId_t readerId;
	EntityId_t writerId;
	SequenceNumber_t firstSN;
	SequenceNumber_t lastSN;
	Count_t count;
}SubmsgHeartbeat_t;

//!@brief Structure SubmsgAcknack_t, contains the information necessary to create a Acknack Submessage.
typedef struct{
	SubmessageHeader_t SubmessageHeader;
	bool endiannessFlag;
	bool finalFlag;
	EntityId_t readerId;
	EntityId_t writerId;
	SequenceNumberSet_t readerSNState;
	Count_t count;
}SubmsgAcknack_t;

//!@brief Structure SubmsgGap_t, contains the information necessary to create a Gap Submessage.
typedef struct{
	SubmessageHeader_t SubmessageHeader;
	bool endiannessFlag;
	EntityId_t readerId;
	EntityId_t writerId;
	SequenceNumber_t gapStart;
	SequenceNumberSet_t gapList;
}SubmsgGap_t;


//!@brief Structure SubmsgInfoTS_t, contains the information necessary to create a InfoTS Submessage.
typedef struct{
	SubmessageHeader_t SubmessageHeader;
	Time_t timestamp;
}SubmsgInfoTS_t;



/**
 * @brief Structure CDRMessage_t, contains a serialized message.
 */
typedef struct CDRMessage_t{
	CDRMessage_t(){
		pos = 0;
		length = 0;
		buffer = (octet*)malloc(RTPSMESSAGE_MAX_SIZE);
		max_size = RTPSMESSAGE_MAX_SIZE;

		msg_endian = BIGEND;
	}
	~CDRMessage_t(){
		if(buffer != NULL)
			free(buffer);
	}
	CDRMessage_t(CDRMessage_t& msg){
		pos = msg.pos;
		length = msg.length;
		max_size = msg.max_size;
		msg_endian = msg.msg_endian;
		if(buffer !=NULL)
			free(buffer);
		buffer=(octet*)malloc(msg.length);
	}
	octet* buffer;
	uint16_t pos; //current w_pos in bytes
	uint16_t max_size; // max size of buffer in bytes
	uint16_t length;
	Endianness_t msg_endian;
}CDRMessage_t;



}
}



#endif /* RTPS_MESSAGES_H_ */
