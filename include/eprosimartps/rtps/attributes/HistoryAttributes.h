/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HistoryAttributes.h
 *
 */

#ifndef HISTORYATTRIBUTES_H_
#define HISTORYATTRIBUTES_H_

namespace eprosima{
namespace rtps{


class HistoryAttributes
{
public:
	HistoryAttributes();
	virtual ~HistoryAttributes();
	uint32_t payloadMaxSize;
	uint16_t initialReservedCaches;
	int32_t maximumReservedCaches;
};


}
}

#endif /* HISTORYATTRIBUTES_H_ */
