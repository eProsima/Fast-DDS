/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterAttributes.h
 *
 */
#ifndef WRITERATTRIBUTES_H_
#define WRITERATTRIBUTES_H_

#include "../common/Time_t.h"
#include "../common/Guid.h"
#include "EndpointAttributes.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{


typedef enum RTPSWriterPublishMode : octet
{
    SYNCHRONOUS_WRITER,
    ASYNCHRONOUS_WRITER
} RTPSWriterPublishMode;


/**
 * Class WriterTimes, defining the times associated with the Reliable Writers events.
 * @ingroup RTPS_ATTRIBUTES_MODULE
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
	//! Periodic HB period, default value 3s.
	Duration_t heartbeatPeriod;
	//!Delay to apply to the response of a ACKNACK message, default value ~45ms.
	Duration_t nackResponseDelay;
	//!This time allows the RTPSWriter to ignore nack messages too soon after the data as sent, default value 0s.
	Duration_t nackSupressionDuration;
};

/**
 * Class WriterAttributes, defining the attributes of a RTPSWriter.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class  WriterAttributes
{
public:
	WriterAttributes() : mode(SYNCHRONOUS_WRITER)
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
	//!Indicates if the Writer is synchronous or asynchronous
	RTPSWriterPublishMode mode;
};

/**
 * Class RemoteReaderAttributes, to define the attributes of a Remote Reader.
 * @ingroup RTPS_ATTRIBUTES_MODULE
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
