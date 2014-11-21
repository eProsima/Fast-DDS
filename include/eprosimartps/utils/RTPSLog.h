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
enum LOG_CATEGORY: uint32_t
#else
enum LOG_CATEGORY : uint32_t
#endif
{
	RTPS_PDP = 1,
	RTPS_EDP,
	RTPS_LIVELINESS,

	RTPS_QOS_CHECK,
	RTPS_CDR_MSG,
	RTPS_UTILS,
	RTPS_HISTORY,
	RTPS_WRITER,
	RTPS_READER,
	RTPS_MSG_IN,
	RTPS_MSG_OUT,
	RTPS_PROXY_DATA,
	RTPS_PARTICIPANT,
	PUBSUB_PUBLISHER,
	PUBSUB_SUBSCRIBER,
	PUBSUB_PARTICIPANT

};

}

using namespace eprosima;




#endif /* RTPSLOG_H_ */
