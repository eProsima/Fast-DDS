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

#ifndef _FASTDDS_RTPS_RESOURCES_ASYNC_INTEREST_TREE_H_
#define _FASTDDS_RTPS_RESOURCES_ASYNC_INTEREST_TREE_H_

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <mutex>
#include <set>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/*!
 * Used by AsyncWriterThread to manage a double queue.
 * One queue is being processed by AsyncWriterThread's internal thread while in the other one other threads can register
 * RTPSWriter pointers that need to send samples asynchronously.
 */
class AsyncInterestTree
{
    friend class AsyncWriterThread;

public:

    /*!
     * @brief Registers a writer in a hidden queue.
     * @param writer Pointer to the writer.
     * @return true if the writer was queued or false if it already is queued.
     */
    bool register_interest(
        RTPSWriter* writer);

    /*!
     * @brief Registers a writer in a hidden queue.
     * @param writer Pointer to the writer.
     * @param max_blocking_time Time point until the function must be blocked.
     * @return true if the writer was queued or false if it already is queued.
     * @note This method is blocked for a period of time.
     */
    bool register_interest(
        RTPSWriter* writer,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    /*!
     * @brief Unregister a writer from both queues.
     * @param writer Pointer to the writer.
     * @return true if both queues remain empty.
     */
    bool unregister_interest(
        RTPSWriter* writer);

    /*!
     * @brief Clears the visible queue and swaps with the hidden set.
     */
    void swap();

    /*!
     * @brief Remove next writer from visible queue and returns it.
     * @return Next writer.
     */
    RTPSWriter* next_active_nts();

private:

    bool register_interest_nts(
        RTPSWriter* writer);

    mutable std::timed_mutex mMutexActive, mMutexHidden;

    RTPSWriter* active_front_ = nullptr;

    RTPSWriter* hidden_front_ = nullptr;

    int active_pos_ = 0;

    int hidden_pos_ = 1;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // _FASTDDS_RTPS_RESOURCES_ASYNC_INTEREST_TREE_H_
