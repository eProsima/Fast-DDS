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
 * @file DeadlineTimer.cpp
 *
 */

#include <fastrtps/rtps/resources/DeadlineTimer.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

DeadlineTimer::DeadlineTimer(
        std::function<void()> callback,
        Duration_t period,
        asio::io_service &service,
        const std::thread &event_thread)
    : TimedEvent(service, event_thread, period.to_ns() * 1e-6)
    , callback_(callback)
{
}

DeadlineTimer::~DeadlineTimer()
{
}

void DeadlineTimer::event(EventCode code, const char *msg)
{
    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        if (callback_ != nullptr)
        {
            callback_();
        }
        else
        {
            logWarning(DEADLINETIMER, "Event successfull but callback is nullptr");
        }
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(DEADLINETIMER, "Aborted");
    }
    else
    {
        logInfo(DEADLINETIMER, "Event message: " << msg);
    }
}

}
} /* namespace rtps */
} /* namespace eprosima */
