/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * $rtps_common.h
 *
 *  Created on: $Feb 18, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      		grcanosa@gmail.com
 */


#include <cstdlib>
#include <cstring>
#include <math.h>
#include <vector>
#include <iostream>
 #include <bitset>
 #include <string>
#include <sstream>

#ifndef RTPS_COMMON_H_
#define RTPS_COMMON_H_


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





//Pre define data encapsulation schemes
#define CDR_BE 0x0000
#define CDR_LE 0x0001
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003


//!@brief Structure SerializedPayload_t.
typedef struct SerializedPayload_t{
	uint16_t encapsulation;
	uint16_t length;
	octet* data;
	SerializedPayload_t(){
		length = 0;
		data = NULL;
		encapsulation = CDR_BE;
	}
	SerializedPayload_t(short len){
		encapsulation = CDR_BE;
		length = len;
		data = (octet*)malloc(length);
	}
	~SerializedPayload_t(){
//		if(data!=NULL)
//			free(data);
	}
	bool copy(SerializedPayload_t* serdata){
		if(data !=NULL)
			free(data);
		length = serdata->length;
		data = (octet*)malloc(length);
		encapsulation = serdata->encapsulation;
		memcpy(data,serdata->data,length);
		return true;
	}
}SerializedPayload_t;


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
	READER,
	WRITER
}HistoryKind_t;


typedef Time_t Duration_t;

/**
 * Structure WriterParams_t, writer parameters passed on creation.
 */
typedef struct WriterParams_t{
	bool pushMode;
	Duration_t heartbeatPeriod;
	Duration_t nackResponseDelay;
	Duration_t nackSupressionDuration;
	Duration_t resendDataPeriod;
	int16_t historySize;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	ReliabilityKind_t reliabilityKind;
	TopicKind_t topicKind;
	StateKind_t stateKind;
	std::string topicName;
	std::string topicDataType;
	WriterParams_t(){
		pushMode = true;
		TIME_ZERO(heartbeatPeriod);
		TIME_ZERO(nackResponseDelay);
		TIME_ZERO(nackSupressionDuration);
		TIME_ZERO(resendDataPeriod);
		historySize = DEFAULT_HISTORY_SIZE;
		unicastLocatorList.clear();
		multicastLocatorList.clear();
		reliabilityKind = BEST_EFFORT;
		topicKind = NO_KEY;
		stateKind = STATELESS;
	}
}WriterParams_t;

typedef struct ReaderParams_t{
	bool expectsInlineQos;
	Duration_t heartbeatResponseDelay;
	Duration_t heartbeatSupressionDuration;
	int16_t historySize;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	ReliabilityKind_t reliabilityKind;
	TopicKind_t topicKind;
	StateKind_t stateKind;
	std::string topicName;
	std::string topicDataType;
	ReaderParams_t(){
		expectsInlineQos = false;
		TIME_ZERO(heartbeatResponseDelay);
		TIME_ZERO(heartbeatSupressionDuration);
		historySize = DEFAULT_HISTORY_SIZE;
		unicastLocatorList.clear();
		multicastLocatorList.clear();
		reliabilityKind = BEST_EFFORT;
		topicKind = NO_KEY;
		stateKind = STATELESS;
	}
}ReaderParams_t;


}
}




#endif /* RTPS_COMMON_H_ */
