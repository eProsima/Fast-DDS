/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file rtps_common.h
 *	Common definitions.
 *  Created on: Feb 18, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      		grcanosa@gmail.com
 */




#ifndef RTPS_COMMON_H_
#define RTPS_COMMON_H_
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <vector>
#include <iostream>
 #include <bitset>
 #include <string>
#include <sstream>

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima{


namespace rtps{



/**
 * @brief Structure CDRMessage_t, contains a serialized message.
 */
typedef struct CDRMessage_t{
	CDRMessage_t(){
		pos = 0;
		length = 0;
		buffer = (octet*)malloc(RTPSMESSAGE_MAX_SIZE);
		max_size = RTPSMESSAGE_MAX_SIZE;

		msg_endian = EPROSIMA_ENDIAN;
	}
	~CDRMessage_t()
	{
//		cout << "CDRMessage destructor" << endl;
		if(buffer != NULL)
			free(buffer);
//		cout << "CDRMessage destructor" << endl;
	}
//	CDRMessage_t(CDRMessage_t& msg)
//	{
//		pos = msg.pos;
//		length = msg.length;
//		max_size = msg.max_size;
//		msg_endian = msg.msg_endian;
//		if(buffer !=NULL)
//			free(buffer);
//		buffer=(octet*)malloc(msg.length);
//	}
	CDRMessage_t(uint16_t size)
	{
		pos = 0;
		length = 0;
		buffer = (octet*)malloc(size);
		max_size = size;
		msg_endian = EPROSIMA_ENDIAN;
	}
	//!Pointer to the buffer where the data is stored.
	octet* buffer;
	//!Read or write position.
	uint16_t pos;
	//!Max size of the message.
	uint16_t max_size;
	//!Current length of the message.
	uint16_t length;
	//!Endianness of the message.
	Endianness_t msg_endian;
}CDRMessage_t;





//Pre define data encapsulation schemes
#define CDR_BE 0x0000
#define CDR_LE 0x0001
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003


//!@brief Structure SerializedPayload_t.
typedef struct SerializedPayload_t{
	//!Encapsulation of the data as suggested in the RTPS 2.1 specification chapter 10.
	uint16_t encapsulation;
	//!Actual length of the data
	uint16_t length;
	//!Pointer to the data.
	octet* data;
	SerializedPayload_t(){
		length = 0;
		data = NULL;
		encapsulation = CDR_BE;
	}
	SerializedPayload_t(short len){
		encapsulation = CDR_BE;
		length = 0;
		data = (octet*)malloc(length);
	}
	~SerializedPayload_t(){
	}
	/*!
	 * Copy another structure (including allocating new space for the data.)
	 * @param[in] serData Pointer to the structure to copy
	 * @return True if correct
	 */
	bool copy(SerializedPayload_t* serData){
		length = serData->length;
		encapsulation = serData->encapsulation;
		if(data == NULL)
			data = (octet*)malloc(length);
		memcpy(data,serData->data,length);
		return true;
	}
}SerializedPayload_t;


typedef struct InstanceHandle_t{
	octet value[16];
	InstanceHandle_t()
	{
		for(uint8_t i=0;i<16;i++)
			value[i] = 0;
	}
	InstanceHandle_t& operator=(const InstanceHandle_t& ihandle){

		for(uint8_t i =0;i<16;i++)
		{
			value[i] = ihandle.value[i];
		}
		return *this;
	}
}InstanceHandle_t;




//!@brief Enum TopicKind_t.
typedef enum TopicKind_t{
	NO_KEY=1,
	WITH_KEY=2
}TopicKind_t;


typedef uint32_t Count_t;

//!Structure Time_t, used to describe times.
typedef struct Time_t{
	int32_t seconds;
	uint32_t fraction;
	int64_t to64time(){
		return (int64_t)seconds+((int64_t)(fraction/pow(2.0,32)));
	}
	Time_t()
	{
		seconds = 0;
		fraction = 0;
	}
}Time_t;

#define TIME_ZERO(t){t.seconds=0;t.fraction=0;}
#define TIME_INVALID(t){t.seconds=-1;t.fraction=0xffffffff;}
#define TIME_INFINITE(t){t.seconds=0x7fffffff;t.fraction=0xffffffff;}

/**
 * Enum ReliabilityKind_t, reliability kind for reader or writer.
 */
typedef enum ReliabilityKind_t{
	BEST_EFFORT,//!< BEST_EFFORT
	RELIABLE    //!< RELIABLE
}ReliabilityKind_t;

/**
 * Enum StateKind_t, type of Writer or reader.
 */
typedef enum StateKind_t{
	STATELESS,//!< STATELESS
	STATEFUL  //!< STATEFUL
}StateKind_t;

//!Enum HistoryKind_t, indicates whether the HistoryCache belongs to a reader or a writer
typedef enum HistoryKind_t{
	UDEF,
	READER,
	WRITER
}HistoryKind_t;


typedef Time_t Duration_t;


struct DDS_Reliability_t{
	ReliabilityKind_t kind;
	Duration_t heartbeatPeriod;
	Duration_t nackResponseDelay;
	Duration_t nackSupressionDuration;
	Duration_t resendDataPeriod;
	Duration_t heartbeatResponseDelay;
	Duration_t heartbeatSupressionDuration;
	uint8_t hb_per_max_samples;
	DDS_Reliability_t()
	{
		TIME_ZERO(heartbeatPeriod);
		heartbeatPeriod.seconds = 3;
		TIME_ZERO(nackResponseDelay);
		nackResponseDelay.fraction = 200*1000*1000;
		TIME_ZERO(nackSupressionDuration);
		TIME_ZERO(resendDataPeriod);
		TIME_ZERO(heartbeatResponseDelay);
		heartbeatResponseDelay.fraction = 500*1000*1000;
		TIME_ZERO(heartbeatSupressionDuration);
		hb_per_max_samples= 5;
		kind = BEST_EFFORT;
	}
};


/**
 * Structure WriterParams_t, writer parameters passed on creation.
 */
typedef struct WriterParams_t{
	bool pushMode;
	uint16_t historySize;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	DDS_Reliability_t reliablility;
	TopicKind_t topicKind;
	StateKind_t stateKind;
	std::string topicName;
	std::string topicDataType;
	WriterParams_t(){
		pushMode = true;
		historySize = DEFAULT_HISTORY_SIZE;
		unicastLocatorList.clear();
		multicastLocatorList.clear();
		topicKind = NO_KEY;
		stateKind = STATELESS;
	}
}WriterParams_t;

typedef struct ReaderParams_t{
	bool expectsInlineQos;

	uint16_t historySize;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	DDS_Reliability_t reliablility;
	TopicKind_t topicKind;
	StateKind_t stateKind;
	std::string topicName;
	std::string topicDataType;
	ReaderParams_t(){
		expectsInlineQos = false;
		historySize = DEFAULT_HISTORY_SIZE;
		unicastLocatorList.clear();
		multicastLocatorList.clear();
		topicKind = NO_KEY;
		stateKind = STATELESS;
	}
}ReaderParams_t;

typedef struct ParticipantParams_t{
	std::vector<Locator_t> defaultUnicastLocatorList;
	std::vector<Locator_t> defaultMulticastLocatorList;
	uint32_t defaultSendPort;
	ParticipantParams_t(){
		defaultSendPort = 10042;
		Locator_t defUni;
		defUni.kind = LOCATOR_KIND_UDPv4;
		LOCATOR_ADDRESS_INVALID(defUni.address);
		defUni.address[12] = 127;
		defUni.address[13] = 0;
		defUni.address[14] = 0;
		defUni.address[15] = 1;
		defUni.port = 10043;
		defaultUnicastLocatorList.push_back(defUni);
	}
}ParticipantParams_t;




}
}




#endif /* RTPS_COMMON_H_ */
