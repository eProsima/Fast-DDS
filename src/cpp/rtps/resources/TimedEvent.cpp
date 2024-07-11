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

#include <rtps/resources/TimedEvent.h>
#include <rtps/resources/ResourceEvent.h>

#include "TimedEventImpl.h"

namespace eprosima {
namespace fastdds {
namespace rtps {

TimedEvent::TimedEvent(
        ResourceEvent& service,
        std::function<bool()> callback,
        double milliseconds)
    : service_(service)
    , impl_(nullptr)
{
    impl_ = new TimedEventImpl(
        callback,
        std::chrono::microseconds(static_cast<int64_t>(milliseconds * 1000)));
    service_.register_timer(impl_);
}

TimedEvent::~TimedEvent()
{
    service_.unregister_timer(impl_);
    delete(impl_);
}

void TimedEvent::cancel_timer()
{
    if (impl_->go_cancel())
    {
        service_.notify(impl_);
    }
}

void TimedEvent::restart_timer()
{
    if (impl_->go_ready())
    {
        service_.notify(impl_);
    }
}

void TimedEvent::restart_timer(
        const std::chrono::steady_clock::time_point& timeout)
{
    if (impl_->go_ready())
    {
        service_.notify(impl_, timeout);
    }
}

void TimedEvent::recreate_timer()
{
    service_.unregister_timer(impl_);
    impl_->go_cancel();
    service_.register_timer(impl_);
}

bool TimedEvent::update_interval(
        const dds::Duration_t& inter)
{
    return impl_->update_interval(inter);
}

bool TimedEvent::update_interval_millisec(
        double time_millisec)
{
    return impl_->update_interval_millisec(time_millisec);
}

double TimedEvent::getIntervalMilliSec()
{
    return impl_->getIntervalMsec();
}

double TimedEvent::getRemainingTimeMilliSec()
{
    return impl_->getRemainingTimeMilliSec();
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
