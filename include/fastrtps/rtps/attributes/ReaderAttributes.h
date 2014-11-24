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

#ifndef READERATTRIBUTES_H_
#define READERATTRIBUTES_H_

#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/attributes/EndpointAttributes.h"
namespace eprosima{
namespace fastrtps{
namespace rtps{


class ReaderTimes
{
public:
	ReaderTimes()
	{
		heartbeatResponseDelay.fraction = 500*1000*1000;
	};
	virtual ~ReaderTimes(){};
	Duration_t heartbeatResponseDelay;
};

class ReaderAttributes
{
public:
	ReaderAttributes()
	{
		endpoint.endpointKind = READER;
		endpoint.durabilityKind = VOLATILE;
		endpoint.reliabilityKind = BEST_EFFORT;
		expectsInlineQos = false;
	};
	virtual ~ReaderAttributes();
	EndpointAttributes endpoint;
	ReaderTimes times;
	bool expectsInlineQos;
};

class RemoteWriterAttributes
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
	EndpointAttributes endpoint;
	GUID_t guid;
	Duration_t livelinessLeaseDuration;
	uint16_t ownershipStrength;
};
}
}
}


#endif /* WRITERATTRIBUTES_H_ */
