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


	class  WriterTimes
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
	EndpointAttributes endpoint;
	WriterTimes times;
};

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
	EndpointAttributes endpoint;
	GUID_t guid;
	bool expectsInlineQos;
};
}
}
}


#endif /* WRITERATTRIBUTES_H_ */
