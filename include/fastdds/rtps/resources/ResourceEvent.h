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

#ifndef _FASTDDS_RTPS_RESOURCES_RESOURCEEVENT_H_
#define _FASTDDS_RTPS_RESOURCES_RESOURCEEVENT_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/utils/TimedMutex.hpp>
#include <fastrtps/utils/TimedConditionVariable.hpp>

#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <asio.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class TimedEventImpl;

/**
 * This class centralizes all operations over asio::io_service and its asio::deadlline_timer objects in the same
 * thread.
 * @ingroup MANAGEMENT_MODULE
 */
class ResourceEvent
{
public:

    ResourceEvent();

    virtual ~ResourceEvent();

    /*!
     * @brief Method to initialize the internal thread.
     */
    void init_thread();

    /*!
     * @brief This method removes a TimedEventImpl object in case it is waiting to be processed by ResourceEvent's
     * internal thread.
     *
     * This method has to be called before deleting the TimedEventImpl object.
     * This method cancels any operation of the internal asio::steady_timer.
     * Then it avoids the situation of asio calling the event handler when it was removed previously.
     * @param event TimedEventImpl object that will be deleted and we have to be sure all its operations are cancelled.
     */
    void unregister_timer(
            TimedEventImpl* event);

    /*!
     * @brief This method notifies to ResourceEvent that the TimedEventImpl object has operations to be scheduled.
     *
     * These operations can be the cancellation of the internal asio::steady_timer or starting another async_wait.
     * @param event TimedEventImpl object that has operations to be scheduled.
     */
    void notify(
            TimedEventImpl* event);

    /*!
     * @brief This method notifies to ResourceEvent that the TimedEventImpl object has operations to be scheduled.
     *
     * These operations can be the cancellation of the internal asio::steady_timer or starting another async_wait.
     * @note Non-blocking call version of the method.
     * @param event TimedEventImpl object that has operations to be scheduled.
     * @param timeout Maximum blocking time of the method.
     */
    void notify(
            TimedEventImpl* event,
            const std::chrono::steady_clock::time_point& timeout);

    /*!
     * @brief Returns the internal asio::io_service.
     * @return Associated asio::io_service.
     */
    asio::io_service& get_io_service()
    {
        return io_service_;
    }

private:

    //! Warns the internal thread can stop.
    std::atomic<bool> stop_;

    //! Protects internal data.
    TimedMutex mutex_;

    //! Used to warn there are new TimedEventImpl objects to be processed.
    TimedConditionVariable cv_;

    //! Flag used to allow a thread to delete a TimedEventImpl because the main thread is not using asio::io_service.
    bool allow_to_delete_;

    //! Head of the list of TimedEventImpl objects that have to be processed.
    TimedEventImpl* front_;

    //! Back of the list of TimedEventImpl objects that have to be processed.
    TimedEventImpl* back_;

    //! Thread
    std::thread thread_;

    //! IO service
    asio::io_service io_service_;

    /*!
     * @brief Registers a new TimedEventImpl object in the internal queue to be processed.
     * Non thread safe.
     * @param event Event to be added in the queue.
     * @return True value if the insertion was successful. In other case, it return False.
     */
    bool register_timer_nts(
            TimedEventImpl* event);

    //! Method called by the internal thread.
    void run_io_service();

    std::promise<void> ready;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif //DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif //_FASTDDS_RTPS_RESOURCES_RESOURCEEVENT_H_
