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

#ifndef _FASTDDS_RTPS_RESOURCES_TIMEDEVENT_H_
#define _FASTDDS_RTPS_RESOURCES_TIMEDEVENT_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/Time_t.h>

#include <thread>
#include <functional>
#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class TimedEventImpl;
class ResourceEvent;

/*!
 * Implementation of events.
 * This class can be used to launch an event through ResourceEvent's internal thread.
 *
 * In the construct you can set the callback to be called when the expiration time expires.
 * @code
   TimedEvent* event = new TimedEvent(resource_event_,
        [&]() -> bool
        {
            std::cout << "hello" << std::endl;
            return true;
        },
        100);
 * @endcode
 *
 * The signature of the callback is:
 * - The returned value tells if the event has to be scheduled again or not. true value tells to reschedule the event.
 *   false value doesn't.
 *
 * Usually the callback will call a method of the owner of the event. So the callback surely accesses internal data that
 * object. Then you have to be aware of deleting the event before deleting any other attribute of the object. Here are
 * explained two cases:
 *
 * - The event is an attribute of the class. Then it has to be declared as the last member of the class. Then we assure
 *   it will be the last one on being constructed and the first one on being deleted.
 *   @code
   class Owner
   {
    int attr1;

    long attr2;

    TimedEvent event;  // Declared as the last member of the class.
   };
 * @endcode
 *
 * - The class has a pointer to the event (TimedEvent is created in heap). Then the pointer has to be the first one on
 *   being freed in the destructor of the class.
 * @code
   class Owner
   {
     int* attr1;

     TimedEvent* event;

     long attr2,
   };

   Owner::~Owner()
   {
    delete(event); // First pointer to be deleted;

    delete(attr1);
   }
 * @endcode
 *
 * @ingroup MANAGEMENT_MODULE
 * @warning Read carefully the detailed description. This class cannot be used in any way.
 */
class TimedEvent
{
public:

    /*!
     * @brief Default constructor.
     *
     * The event is not created scheduled.
     * @param service ResourceEvent object that will operate with the event.
     * @param callback Callback called when the event expires.
     * @param milliseconds Expiration time in milliseconds.
     */
    TimedEvent(
            ResourceEvent& service,
            std::function<bool()> callback,
            double milliseconds);

    //! Default destructor.
    virtual ~TimedEvent();

    /*!
     * @brief Cancels any previous scheduling of the event.
     */
    void cancel_timer();

    /*!
     * @brief Schedules the event if there is not a previous scheduling.
     */
    void restart_timer();

    /*!
     * @brief Schedules the event if there is not a previous scheduling.
     * @note Non-blocking call version.
     * @param timeout Time point in the future until the method can be blocked.
     */
    void restart_timer(
            const std::chrono::steady_clock::time_point& timeout);

    /**
     * Update event interval.
     * When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
     *
     * @param inter New interval for the timedEvent
     * @return true on success
     */
    bool update_interval(
            const Duration_t& inter);

    /**
     * Update event interval.
     * When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
     *
     * @param time_millisec New interval for the timedEvent
     * @return true on success
     */
    bool update_interval_millisec(
            double time_millisec);

    /**
     * Get the milliseconds interval
     * @return Milliseconds interval
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

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif //DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif //_FASTDDS_RTPS_RESOURCES_TIMEDEVENT_H_
