/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Publisher.cpp
 *
 */

#include "eprosimartps/pubsub/publisher/Publisher.h"
#include "eprosimartps/pubsub/publisher/PublisherImpl.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace pubsub {

static const char* const CLASS_NAME = "Publisher";

Publisher::Publisher(PublisherImpl* pimpl):
		mp_impl(pimpl)
{
	// TODO Auto-generated constructor stub

}

Publisher::~Publisher() {
	// TODO Auto-generated destructor stub
}

bool Publisher::write(void* Data) {
	const char* const METHOD_NAME = "write";
	logInfo(PUBSUB_PUBLISHER,"Writing new data");
	return mp_impl->create_new_change(ALIVE,Data);
}

bool Publisher::dispose(void* Data)
{
	const char* const METHOD_NAME = "dispose";
	logInfo(PUBSUB_PUBLISHER,"Disposing of Data");
	return mp_impl->create_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool Publisher::unregister(void* Data) {
	const char* const METHOD_NAME = "unregister";
	//Convert data to serialized Payload
	logInfo(PUBSUB_PUBLISHER,"Unregistering of Data");
	return mp_impl->create_new_change(NOT_ALIVE_UNREGISTERED,Data);
}

bool Publisher::dispose_and_unregister(void* Data) {
	//Convert data to serialized Payload
	const char* const METHOD_NAME = "dispose_and_unregister";
	logInfo(PUBSUB_PUBLISHER,"Disposing and Unregistering Data");
	return mp_impl->create_new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,Data);
}

} /* namespace pubsub */
} /* namespace eprosima */
