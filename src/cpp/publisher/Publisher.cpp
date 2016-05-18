/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Publisher.cpp
 *
 */

#include <fastrtps/publisher/Publisher.h>
#include "PublisherImpl.h"

#include <fastrtps/utils/RTPSLog.h>

namespace eprosima {
namespace fastrtps {

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
	logInfo(PUBLISHER,"Writing new data");
	return mp_impl->create_new_change(ALIVE,Data);
}

bool Publisher::write(void* Data, WriteParams &wparams) {
	const char* const METHOD_NAME = "write";
	logInfo(PUBLISHER,"Writing new data with WriteParams");
	return mp_impl->create_new_change_with_params(ALIVE, Data, wparams);
}

bool Publisher::dispose(void* Data)
{
	const char* const METHOD_NAME = "dispose";
	logInfo(PUBLISHER,"Disposing of Data");
	return mp_impl->create_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool Publisher::unregister(void* Data) {
	const char* const METHOD_NAME = "unregister";
	//Convert data to serialized Payload
	logInfo(PUBLISHER,"Unregistering of Data");
	return mp_impl->create_new_change(NOT_ALIVE_UNREGISTERED,Data);
}

bool Publisher::dispose_and_unregister(void* Data) {
	//Convert data to serialized Payload
	const char* const METHOD_NAME = "dispose_and_unregister";
	logInfo(PUBLISHER,"Disposing and Unregistering Data");
	return mp_impl->create_new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,Data);
}

bool Publisher::removeAllChange(size_t* removed )
{
	const char* const METHOD_NAME = "removeAllChange";
	logInfo(PUBLISHER,"Removing all data from history");
	return mp_impl->removeAllChange(removed);
}

bool Publisher::wait_for_all_acked(const Time_t& max_wait)
{
	const char* const METHOD_NAME = "wait_for_all_acked";
	logInfo(PUBLISHER,"Waiting for all samples acknowledged");
	return mp_impl->wait_for_all_acked(max_wait);
}

const GUID_t& Publisher::getGuid()
{
	return mp_impl->getGuid();
}

} /* namespace pubsub */
} /* namespace eprosima */
