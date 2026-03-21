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
 * @file TimedEventImpl.h
 *
 */

#ifndef _RTPS_RESOURCES_TIMEDEVENTIMPL_H_
#define _RTPS_RESOURCES_TIMEDEVENTIMPL_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/Time_t.hpp>
#include <rtps/resources/TimedEvent.h>

#include <atomic>
#include <functional>

namespace eprosima {
namespace fastdds {
namespace rtps {

/*!
 * This class encapsulates a timer.
 * It also manages the state of the event (INACTIVE, READY, WAITING..).
 * @ingroup MANAGEMENT_MODULE
 */
class TimedEventImpl
{
    using Callback = std::function<bool ()>;

public:

    enum StateCode
    {
        INACTIVE = 0, //! The event is inactive. The event service is not waiting for it.
        READY, //! The event is ready for being processed by ResourceEvent and added to the event service.
        WAITING, //! The event is waiting for the event service to be triggered.
    };

    /*!
     * @brief Default constructor.
     * @param callback Callback called when the timer is triggered.
     * @param interval Expiration time in microseconds of the event.
     */
    TimedEventImpl(
            Callback callback,
            std::chrono::microseconds interval);

    /*!
     * @brief Updates the expiration time of the event.
     *
     * When updating the interval, the timer is NOT restarted and the new interval will only be used the next time
     * update() or trigger() are called.
     * @param interval New expiration time.
     * @return true on success
     */
    bool update_interval(
            const dds::Duration_t& interval);

    /*!
     * @brief Updates the expiration time of the event.
     *
     * When updating the interval, the timer is NOT restarted and the new interval will only be used the next time
     * update() or trigger() are called.
     * @param interval New expiration time in milliseconds.
     * @return true on success
     */
    bool update_interval_millisec(
            double interval);

    /*!
     * @brief Returns current expiration time in milliseconds
     * @return Event expiration time in milliseconds
     */
    double getIntervalMsec()
    {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(interval_microsec_.load());
        return static_cast<double>(ms.count());
    }

    /*!
     * @brief Returns the remaining milliseconds for the timer to expire
     * @return Remaining milliseconds for the timer to expire
     */
    double getRemainingTimeMilliSec()
    {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            next_trigger_time_.load() - std::chrono::steady_clock::now());
        return static_cast<double>(ms.count());
    }

    /*!
     * @brief Returns next trigger time as a time point
     * @return Event's next trigger time as a time point
     */
    std::chrono::steady_clock::time_point next_trigger_time()
    {
        return next_trigger_time_;
    }

    /*!
     * @brief Tries to set the event as READY.
     * To achieve it, the event has to be INACTIVE.
     * @return true on success.
     */
    bool go_ready();

    /*!
     * @brief Tries to cancel the event and set it as INACTIVE.
     * To achieve it, the event has to be WAITING.
     * @return true on success.
     */
    bool go_cancel();

    /*!
     * @brief It updates the timer depending on the state of the TimedEventImpl object.
     * @warning This method has to be called from ResourceEvent's internal thread.
     * @return false if the event was canceled, true otherwise.
     */
    bool update(
            std::chrono::steady_clock::time_point current_time,
            std::chrono::steady_clock::time_point cancel_time);

    /*!
     * @brief Triggers the callback action.
     * Also updates next trigger time.
     * @warning This method has to be called from ResourceEvent's internal thread when the timer expires.
     */
    void trigger(
            std::chrono::steady_clock::time_point current_time,
            std::chrono::steady_clock::time_point cancel_time);

private:

    //! Expiration time in microseconds of the event.
    std::atomic<std::chrono::microseconds> interval_microsec_;

    //! Next time this event should be triggered
    std::atomic<std::chrono::steady_clock::time_point> next_trigger_time_;

    //! User function to be called when this event is triggered
    Callback callback_;

    //! Current state of this event
    std::atomic<StateCode> state_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif //_RTPS_RESOURCES_TIMEDEVENTIMPL_H_
