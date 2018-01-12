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

/*
 * Subscriber.cpp
 *
 */

#include <fastrtps/subscriber/Subscriber.h>
#include "SubscriberImpl.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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

SubscriberAttributes Subscriber::getAttributes() const
{
    return mp_impl->getAttributes();
}

bool Subscriber::isInCleanState() const
{
    return mp_impl->isInCleanState();
}

uint64_t Subscriber::getUnreadCount() const
{
	return mp_impl->getUnreadCount();
}