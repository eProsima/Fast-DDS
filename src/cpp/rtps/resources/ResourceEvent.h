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
 * @file ResourceEvent.h
 *
 */

#ifndef FASTDDS_RTPS_RESOURCES__RESOURCEEVENT_H
#define FASTDDS_RTPS_RESOURCES__RESOURCEEVENT_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <fastdds/utils/TimedConditionVariable.hpp>

namespace eprosima {

class thread;

namespace fastdds {
namespace rtps {

class TimedEventImpl;

/**
 * This class centralizes all operations over timed events in the same thread.
 * @ingroup MANAGEMENT_MODULE
 */
class ResourceEvent
{
public:

    ResourceEvent();

    ~ResourceEvent();

    /*!
     * @brief Method to initialize the internal thread.
     *
     * @param [in]  thread_cfg  Settings to apply to the created thread.
     * @param [in]  name_fmt    A null-terminated string to be used as the format argument of
     *                         a `snprintf` like function, taking `thread_id` as additional
     *                         argument, and used to give a name to the created thread.
     * @param [in]  thread_id   Single variadic argument passed to the formatting function.
     */
    void init_thread(
            const fastdds::rtps::ThreadSettings& thread_cfg = {},
            const char* name_fmt = "event %u",
            uint32_t thread_id = 0);

    void stop_thread();

    /*!
     * @brief This method informs that a TimedEventImpl has been created.
     *
     * This method has to be called when creating a TimedEventImpl object.
     * @param event TimedEventImpl object that has been created.
     */
    void register_timer(
            TimedEventImpl* event);

    /*!
     * @brief This method removes a TimedEventImpl object in case it is waiting to be processed by ResourceEvent's
     * internal thread.
     *
     * This method has to be called before deleting the TimedEventImpl object.
     * This method cancels any operation of the timer.
     * Then it avoids the situation of the execution thread calling the event handler when it was previously removed.
     * @param event TimedEventImpl object that will be deleted and we have to be sure all its operations are cancelled.
     */
    void unregister_timer(
            TimedEventImpl* event);

    /*!
     * @brief This method notifies to ResourceEvent that the TimedEventImpl object has operations to be scheduled.
     *
     * These operations can be the cancellation of the timer or starting another async_wait.
     * @param event TimedEventImpl object that has operations to be scheduled.
     */
    void notify(
            TimedEventImpl* event);

    /*!
     * @brief This method notifies to ResourceEvent that the TimedEventImpl object has operations to be scheduled.
     *
     * These operations can be the cancellation of the timer or starting another async_wait.
     * @note Non-blocking call version of the method.
     * @param event TimedEventImpl object that has operations to be scheduled.
     * @param timeout Maximum blocking time of the method.
     */
    void notify(
            TimedEventImpl* event,
            const std::chrono::steady_clock::time_point& timeout);

private:

    //! Warns the internal thread can stop.
    std::atomic<bool> stop_{ false };

    //! Protects internal data.
    TimedMutex mutex_;

    //! Used to warn about changes on allow_vector_manipulation_.
    TimedConditionVariable cv_manipulation_;

    //! Flag used to allow a thread to manipulate the timer collections when the execution thread is not using them.
    bool allow_vector_manipulation_ = true;

    //! Used to warn there are new TimedEventImpl objects to be processed.
    TimedConditionVariable cv_;

    //! The total number of created timers.
    size_t timers_count_ = 0;

    //! Collection of events pending update action.
    std::vector<TimedEventImpl*> pending_timers_;

    //! Collection of registered events waiting completion.
    std::vector<TimedEventImpl*> active_timers_;

    //! Prevents iterator invalidation when active_timers are manipulated inside loops
    std::atomic<bool> skip_checking_active_timers_;

    //! Current time as seen by the execution thread.
    std::chrono::steady_clock::time_point current_time_;

    //! Execution thread.
    std::unique_ptr<eprosima::thread> thread_;

    /*!
     * @brief Registers a new TimedEventImpl object in the internal queue to be processed.
     * Non thread safe.
     * @param event Event to be added in the queue.
     * @return True value if the insertion was successful. In other case, it return False.
     */
    bool register_timer_nts(
            TimedEventImpl* event);

    //! Method called by the internal thread.
    void event_service();

    //! Sorts waiting timers in ascending order of trigger time.
    void sort_timers();

    //! Updates internal register of current time.
    void update_current_time();

    //! Method called by the internal thread to process due actions.
    void do_timer_actions();

    //! Ensures internal collections can accommodate current total number of timers.
    void resize_collections()
    {
        pending_timers_.reserve(timers_count_);
        active_timers_.reserve(timers_count_);
    }

};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif //FASTDDS_RTPS_RESOURCES__RESOURCEEVENT_H
