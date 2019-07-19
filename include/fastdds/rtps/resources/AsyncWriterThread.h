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
 * @file AsyncWriterThread.h
 *
 */
#ifndef _FASTDDS_RTPS_RESOURCES_ASYNCWRITERTHREAD_H_
#define _FASTDDS_RTPS_RESOURCES_ASYNCWRITERTHREAD_H_

#include <thread>
#include <atomic>
#include <list>

#include <fastdds/rtps/resources/AsyncInterestTree.h>
#include <fastrtps/utils/TimedMutex.hpp>
#include <fastrtps/utils/TimedConditionVariable.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSWriter;

/**
 * @brief This static class owns a thread that manages asynchronous writes.
 * Asynchronous writes happen directly (when using an async writer) and
 * indirectly (when responding to a NACK).
 * @ingroup COMMON_MODULE
 */
class AsyncWriterThread
{
public:

    AsyncWriterThread() = default;

    ~AsyncWriterThread();

    /*!
     * @brief Unregister a writer if it is waiting to be processed.
     * @param writer Asynchronous writer to be removed.
     * @return Result of the operation.
     * @note Always call this function from writer's destructor.
     */
    void unregister_writer(
        RTPSWriter* writer);

    /*!
     * Wakes the thread up and starts processing async writers.
     * @param interested_writer The writer interested in an async write.
     */
    void wake_up(
        RTPSWriter* interested_writer);

    /*!
     * Wakes the thread up and starts processing async writers.
     * @param interested_writer The writer interested in an async write.
     * @param max_blocking_time Time point until the function must be blocked.
     * @note This method is blocked for a period of time.
     */
    void wake_up(
        RTPSWriter* interested_writer,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

private:

    AsyncWriterThread(const AsyncWriterThread&) = delete;
    const AsyncWriterThread& operator=(const AsyncWriterThread&) = delete;

    //! @brief runs main method
    void run();

    std::thread* thread_ = nullptr;
    RecursiveTimedMutex condition_variable_mutex_;

    //! List of asynchronous writers.
    AsyncInterestTree interestTree_;

    bool running_ = false;
    bool run_scheduled_ = false;
    TimedConditionVariable cv_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_RESOURCES_ASYNCWRITERTHREAD_H_
