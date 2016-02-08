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

#include "../common/Time_t.h"
#include "EndpointAttributes.h"
namespace eprosima{
namespace fastrtps{
namespace rtps{


/**
 * Class ReaderTimes, defining the times associated with the Reliable Readers events.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class ReaderTimes
{
public:
	ReaderTimes()
	{
		heartbeatResponseDelay.fraction = 500*1000*1000;
	};
	virtual ~ReaderTimes(){};
	//!Delay to be applied when a hearbeat message is received, default value ~116ms.
	Duration_t heartbeatResponseDelay;
};

/**
 * Class ReaderAttributes, to define the attributes of a RTPSReader.
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
	//!Indicates if the reader expects Inline qos, default value 0.
	bool expectsInlineQos;
};

/**
 * Class RemoteWriterAttributes, to define the attributes of a Remote Writer.
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
	//!Liveliness lease duration, default value c_TimeInfinite.
	Duration_t livelinessLeaseDuration;
	//!Ownership Strength of the associated writer.
	uint16_t ownershipStrength;
};
}
}
}


#endif /* WRITERATTRIBUTES_H_ */
