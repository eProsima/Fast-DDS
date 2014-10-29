/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSLog.h
 *
 */

#ifndef RTPSLOG_H_
#define RTPSLOG_H_

#include "eProsima_cpp/log/Log.h"

namespace eprosima{


#if defined(_WIN32)

#else
enum LOG_CATEGORY : uint32_t
#endif
{
	RTPS_DISCOVERY = 1,
	RTPS_LIVELINESS,

	RTPS_QOS_CHECK,
	RTPS_CDR_MSG,
	RTPS_UTILS,
	RTPS_HISTORY,
	RTPS_WRITER,
	RTPS_PROXY_DATA,

};

}

using namespace eprosima;




#endif /* RTPSLOG_H_ */
