/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterAttributes.h
 *
 */

#ifndef READERATTRIBUTES_H_
#define READERATTRIBUTES_H_

#include "eprosimartps/rtps/common/Time_t.h"
#include "eprosimartps/rtps/attributes/EndpointAttributes.h"
namespace eprosima{
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
	};
	virtual ~ReaderAttributes();
	EndpointAttributes endpoint;
	ReaderTimes times;
};

class RemoteWriterAttributes
{
public:
	RemoteWriterAttributes()
	{
		endpoint.endpointKind = WRITER;
	};
	virtual ~RemoteWriterAttributes()
	{

	};
	EndpointAttributes endpoint;
	GUID_t guid;
};

}
}


#endif /* WRITERATTRIBUTES_H_ */
