/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
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

/**
 * Attributes of a WriterHistory or a ReaderHistory.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class RTPS_DllAPI HistoryAttributes
{
public:
	HistoryAttributes():
		payloadMaxSize(50),
		initialReservedCaches(100),
		maximumReservedCaches(-1)
	{};
	/** Constructor
	* @param payload Maximum payload size.
	* @param initial Initial reserved caches.
	* @param maxRes Maximum reserved caches.
	*/
	HistoryAttributes(uint32_t payload,uint32_t initial,int32_t maxRes):
		payloadMaxSize(payload),initialReservedCaches(initial),
		maximumReservedCaches(maxRes){}
	virtual ~HistoryAttributes(){};
	//!Maximum payload size of the history.
	uint32_t payloadMaxSize;
	//!Number of the initial Reserved Caches.
	uint32_t initialReservedCaches;
	//!Maximum number of reserved caches. Default value is -1 that indicates to keep reserving until something breaks.
	int32_t maximumReservedCaches;
};

}
}
}

#endif /* HISTORYATTRIBUTES_H_ */
