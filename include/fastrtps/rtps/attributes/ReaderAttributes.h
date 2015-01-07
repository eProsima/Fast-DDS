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

#ifndef READERATTRIBUTES_H_
#define READERATTRIBUTES_H_

#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/attributes/EndpointAttributes.h"
namespace eprosima{
namespace fastrtps{
namespace rtps{


/**
 * Times associated with a Reliable Reader.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class   ReaderTimes
{
public:
	ReaderTimes()
	{
		heartbeatResponseDelay.fraction = 500*1000*1000;
	};
	virtual ~ReaderTimes(){};
	//!Delay to be applied when a hearbeat message is received.
	Duration_t heartbeatResponseDelay;
};

/**
 * Attributes of a RTPSReader.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class  ReaderAttributes
{
public:
	ReaderAttributes()
	{
		endpoint.endpointKind = READER;
		endpoint.durabilityKind = VOLATILE;
		endpoint.reliabilityKind = BEST_EFFORT;
		expectsInlineQos = false;
	};
	virtual ~ReaderAttributes(){};
	//!Attributes of the associated endpoint.
	EndpointAttributes endpoint;
	//!Times associated with this reader.
	ReaderTimes times;
	//!Indicates if the reader expects Inline qos.
	bool expectsInlineQos;
};

/**
 * Attributes that define a remote Writer.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class  RemoteWriterAttributes
{
public:
	RemoteWriterAttributes()
	{
		endpoint.endpointKind = WRITER;
		livelinessLeaseDuration = c_TimeInfinite;
		ownershipStrength = 0;
	};
	virtual ~RemoteWriterAttributes()
	{

	};
	//!Attributes of the associated endpoint.
	EndpointAttributes endpoint;
	//!GUID_t of the writer, can be unknown if the reader is best effort.
	GUID_t guid;
	//!Liveliness lease duration.
	Duration_t livelinessLeaseDuration;
	//!Ownership Strength of the associated writer.
	uint16_t ownershipStrength;
};
}
}
}


#endif /* WRITERATTRIBUTES_H_ */
