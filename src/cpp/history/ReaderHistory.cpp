/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.cpp
 *
 */

#include "eprosimartps/history/ReaderHistory.h"

#include "eprosimartps/Endpoint.h"
#include "eprosimartps/common/CacheChange.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/reader/RTPSReader.h"

namespace eprosima {
namespace rtps {

ReaderHistory::ReaderHistory(Endpoint* endp,
		uint32_t payload_max_size):
		History(endp,endp->getTopic().historyQos,endp->getTopic().resourceLimitsQos,payload_max_size)

{
	// TODO Auto-generated constructor stub

}

ReaderHistory::~ReaderHistory()
{
	// TODO Auto-generated destructor stub
}

} /* namespace rtps */
} /* namespace eprosima */
