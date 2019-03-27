// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TimedCallback.cpp
 *
 */

#include <fastrtps/rtps/timedevent//TimedCallback.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

TimedCallback::TimedCallback(
        std::function<void()> callback,
        double milliseconds,
        asio::io_service &service,
        const std::thread &event_thread)
    : TimedEvent(service, event_thread, milliseconds)
    , callback_(callback)
{
}

TimedCallback::~TimedCallback()
{
    destroy();
}

void TimedCallback::event(EventCode code, const char *msg)
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
            logWarning(TimedCallback, "Event successfull but callback is nullptr");
        }
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(TimedCallback, "Aborted");
    }
    else
    {
        logInfo(TimedCallback, "Event message: " << msg);
    }
}

}
} /* namespace rtps */
} /* namespace eprosima */
