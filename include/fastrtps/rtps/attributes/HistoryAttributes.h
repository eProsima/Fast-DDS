/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HistoryAttributes.h
 *
 */

#ifndef HISTORYATTRIBUTES_H_
#define HISTORYATTRIBUTES_H_

namespace eprosima{
namespace fastrtps{
namespace rtps{


	class RTPS_DllAPI HistoryAttributes
{
public:
	HistoryAttributes(){};
	HistoryAttributes(uint32_t payload,uint16_t initial,int32_t maxRes):
		payloadMaxSize(payload),initialReservedCaches(initial),
		maximumReservedCaches(maxRes){}
	virtual ~HistoryAttributes(){};
	uint32_t payloadMaxSize;
	uint16_t initialReservedCaches;
	int32_t maximumReservedCaches;
};

}
}
}

#endif /* HISTORYATTRIBUTES_H_ */
