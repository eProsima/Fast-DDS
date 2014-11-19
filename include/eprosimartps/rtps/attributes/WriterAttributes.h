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

#ifndef WRITERATTRIBUTES_H_
#define WRITERATTRIBUTES_H_

#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/rtps/attributes/EndpointAttributes.h"
namespace eprosima{
namespace rtps{


class WriterTimes
{
public:
	WriterTimes()
	{
		heartbeatPeriod.seconds = 3;
		nackResponseDelay.fraction = 200*1000*1000;
	};
	virtual ~WriterTimes(){};
	Duration_t heartbeatPeriod;
	Duration_t nackResponseDelay;
	Duration_t nackSupressionDuration;
};

class WriterAttributes
{
public:
	WriterAttributes()
	{
		endpoint.endpointKind = WRITER;
		endpoint.durabilityKind = VOLATILE;
	};
	virtual ~WriterAttributes();
	EndpointAttributes endpoint;
	WriterTimes times;
};

class RemoteReaderAttributes
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
	EndpointAttributes endpoint;
	GUID_t guid;
	bool expectsInlineQos;
};

}
}


#endif /* WRITERATTRIBUTES_H_ */
