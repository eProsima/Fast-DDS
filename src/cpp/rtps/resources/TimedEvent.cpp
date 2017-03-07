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
 * @file TimedEvent.cpp
 *
 */

#include <fastrtps/rtps/resources/TimedEvent.h>
#include "TimedEventImpl.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

    TimedEvent::TimedEvent(asio::io_service &service, const std::thread& event_thread, double milliseconds, TimedEvent::AUTODESTRUCTION_MODE autodestruction)
{
	mp_impl = new TimedEventImpl(this, service, event_thread, std::chrono::microseconds((int64_t)(milliseconds*1000)), autodestruction);
}

TimedEvent::~TimedEvent()
{
	delete(mp_impl);
}

void TimedEvent::cancel_timer()
{
	mp_impl->cancel_timer();
}


void TimedEvent::restart_timer()
{
	mp_impl->restart_timer();
}

bool TimedEvent::update_interval(const Duration_t& inter)
{
	return mp_impl->update_interval(inter);
}

bool TimedEvent::update_interval_millisec(double time_millisec)
{
	return mp_impl->update_interval_millisec(time_millisec);
}

double TimedEvent::getIntervalMilliSec()
{
	return mp_impl->getIntervalMsec();
}

double TimedEvent::getRemainingTimeMilliSec()
{
	return mp_impl->getRemainingTimeMilliSec();
}

void TimedEvent::destroy()
{
    mp_impl->destroy();
}

}
} /* namespace rtps */
} /* namespace eprosima */
