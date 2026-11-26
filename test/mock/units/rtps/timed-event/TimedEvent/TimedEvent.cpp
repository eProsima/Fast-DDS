// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of TimedEvent: replaces src/cpp/rtps/resources/TimedEvent.cpp and forwards every method to
// the TimedEventMock companion of the instance.

#include <rtps/resources/TimedEvent.h>

#include <mock/rtps/resources/TimedEventMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<TimedEvent>;

TimedEvent::TimedEvent(
        ResourceEvent& service,
        std::function<bool()>,
        double)
    : service_(service)
    , impl_(nullptr)
{
    Companion::attach(this);
}

TimedEvent::TimedEvent(
        ResourceEvent& service,
        std::function<bool()>,
        std::chrono::microseconds)
    : service_(service)
    , impl_(nullptr)
{
    Companion::attach(this);
}

TimedEvent::~TimedEvent()
{
    Companion::detach(this);
}

void TimedEvent::cancel_timer()
{
    Companion::get(this).cancel_timer();
}

void TimedEvent::restart_timer()
{
    Companion::get(this).restart_timer();
}

void TimedEvent::restart_timer(
        const std::chrono::steady_clock::time_point& timeout)
{
    Companion::get(this).restart_timer(timeout);
}

void TimedEvent::recreate_timer()
{
    Companion::get(this).recreate_timer();
}

bool TimedEvent::update_interval(
        const dds::Duration_t& inter)
{
    return Companion::get(this).update_interval(inter);
}

bool TimedEvent::update_interval_millisec(
        double time_millisec)
{
    return Companion::get(this).update_interval_millisec(time_millisec);
}

double TimedEvent::getIntervalMilliSec()
{
    return Companion::get(this).getIntervalMilliSec();
}

double TimedEvent::getRemainingTimeMilliSec()
{
    return Companion::get(this).getRemainingTimeMilliSec();
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
