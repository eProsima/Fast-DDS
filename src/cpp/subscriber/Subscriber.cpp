/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.cpp
 *
 */

#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SubscriberImpl.h"

namespace eprosima {
namespace fastrtps {

const GUID_t& Subscriber::getGuid()
{
	return mp_impl->getGuid();
}


void Subscriber::waitForUnreadMessage()
{
	return mp_impl->waitForUnreadMessage();
}


bool Subscriber::readNextData(void* data,SampleInfo_t* info)
{
	return mp_impl->readNextData(data,info);
}
bool Subscriber::takeNextData(void* data,SampleInfo_t* info)
{
	return mp_impl->takeNextData(data,info);
}

bool Subscriber::updateAttributes(SubscriberAttributes& att)
{
	return mp_impl->updateAttributes(att);
}

SubscriberAttributes Subscriber::getAttributes()
{
	return mp_impl->getAttributes();
}




} /* namespace fastrtps */
} /* namespace eprosima */


