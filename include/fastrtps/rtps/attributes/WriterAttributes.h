/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterAttributes.h
 *
 */

#ifndef WRITERATTRIBUTES_H_
#define WRITERATTRIBUTES_H_

#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/attributes/EndpointAttributes.h"
namespace eprosima{
namespace fastrtps{
namespace rtps{


/**
 * Times associated with Reliable RTPSWriter events.
 */
class  WriterTimes
{
public:
	WriterTimes()
	{
		heartbeatPeriod.seconds = 3;
		nackResponseDelay.fraction = 200*1000*1000;
	};
	virtual ~WriterTimes(){};
	//! Periodic HB period
	Duration_t heartbeatPeriod;
	//!Delay to apply to the response of a ACKNACK message.
	Duration_t nackResponseDelay;
	//!This time allows the RTPSWriter to ignore nack messages too soon after the data as sent.
	Duration_t nackSupressionDuration;
};

/**
 * Attributes of a RTPSWriter.
 */
class  WriterAttributes
{
public:
	WriterAttributes()
	{
		endpoint.endpointKind = WRITER;
		endpoint.durabilityKind = TRANSIENT_LOCAL;
		endpoint.reliabilityKind = RELIABLE;
	};
	virtual ~WriterAttributes(){};
	//!Attributes of the associated endpoint.
	EndpointAttributes endpoint;
	//!Writer Times (only used for RELIABLE).
	WriterTimes times;
};

/**
 * Attributes that define RemoteReader.
 */
class  RemoteReaderAttributes
{
public:
	RemoteReaderAttributes()
	{
		endpoint.endpointKind = READER;
		expectsInlineQos = false;
	};
	virtual ~RemoteReaderAttributes()
	{

	};
	//!Attributes of the associated endpoint.
	EndpointAttributes endpoint;
	//!GUID_t of the reader.
	GUID_t guid;
	//!Expects inline QOS.
	bool expectsInlineQos;
};
}
}
}


#endif /* WRITERATTRIBUTES_H_ */
