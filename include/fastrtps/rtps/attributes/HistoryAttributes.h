// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HistoryAttributes.h
 *
 */

#ifndef HISTORYATTRIBUTES_H_
#define HISTORYATTRIBUTES_H_

#include <cstdint>

#include "../../fastrtps_dll.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * Class HistoryAttributes, to specify the attributes of a WriterHistory or a ReaderHistory.
 * This class is only intended to be used with the RTPS API.
 * The Publsiher-Subscriber API has other fields to define this values (HistoryQosPolicy and ResourceLimitsQosPolicy).
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class RTPS_DllAPI HistoryAttributes
{
public:
	HistoryAttributes():
		payloadInitialSize(500),
		allowPayloadResize(false),
		initialReservedCaches(500),
		maximumReservedCaches(0)
	{}
	/** Constructor
	* @param payload Initial payload size.
	* @param allowResize If true allow the payload to be resized dynamically.
	* @param initial Initial reserved caches.
	* @param maxRes Maximum reserved caches.
	*/
	HistoryAttributes(uint32_t payload,bool allowResize,int32_t initial,int32_t maxRes):
		payloadInitialSize(payload),
		allowPayloadResize(allowResize),
		initialReservedCaches(initial),
		maximumReservedCaches(maxRes)
	{}
	virtual ~HistoryAttributes(){}
	//!Intial payload size of the history, default value 500.
	uint32_t payloadInitialSize;
	//!Allow payload to be resized when it is too small, default to false.
	bool allowPayloadResize;
	//!Number of the initial Reserved Caches, default value 500.
	int32_t initialReservedCaches;
	//!Maximum number of reserved caches. Default value is 0 that indicates to keep reserving until something breaks.
	int32_t maximumReservedCaches;
};

}
}
}

#endif /* HISTORYATTRIBUTES_H_ */
