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
 * @file TimedEvent.h
 *
 */

#ifndef _RTPS_RESOURCES_TIMEDEVENT_H_
#define _RTPS_RESOURCES_TIMEDEVENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "../common/Time_t.h"

#include <thread>
#include <functional>
#include <cstdint>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class TimedEventImpl;
class ResourceEvent;

/**
 * Timed Event class used to define any timed events.
 * @ingroup MANAGEMENT_MODULE
 */
class TimedEvent
{
    public:

        /**
         * Enum representing event statuses
         */
        enum EventCode
        {
            EVENT_SUCCESS,
            EVENT_ABORT
        };

        /**
         * @param service IO service to run the event.
         * @param event_thread starting thread for identification.
         * @param milliseconds Interval of the timedEvent.
         * @param autodestruction Self-destruct mode flag.
         */
        TimedEvent(
                ResourceEvent& service,
                std::function<bool(EventCode)> callback,
                double milliseconds);

        virtual ~TimedEvent();

        void cancel_timer();

        //!Method to restart the timer.
        void restart_timer();

        void restart_timer(const std::chrono::steady_clock::time_point& timeout);

        /**
         * Update event interval.
         * When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
         *
         * @param inter New interval for the timedEvent
         * @return true on success
         */
        bool update_interval(const Duration_t& inter);

        /**
         * Update event interval.
         * When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
         *
         * @param time_millisec New interval for the timedEvent
         * @return true on success
         */
        bool update_interval_millisec(double time_millisec);

        /**
         * Get the milliseconds interval
         * @return Mulliseconds interval
         */
        double getIntervalMilliSec();

        /**
         * Get the remaining milliseconds for the timer to expire
         * @return Remaining milliseconds for the timer to expire
         */
        double getRemainingTimeMilliSec();

    private:

        ResourceEvent& service_;

        TimedEventImpl* impl_;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif

#endif //_RTPS_RESOURCES_TIMEDEVENT_H_
