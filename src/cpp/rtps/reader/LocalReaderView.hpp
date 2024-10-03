// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LocalReaderView.hpp
 */
#ifndef FASTDDS_RTPS_READER__LOCALREADERVIEW_HPP
#define FASTDDS_RTPS_READER__LOCALREADERVIEW_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseReader;

enum class LocalReaderViewStatus
{
    ACTIVE = 0,
    INACTIVE
};

/**
 * @brief Class LocalReaderView, contains the status of a
 * LocalReaderView (local to the process) tracking the number
 * of references to it.
 * @ingroup READER_MODULE
 */
class LocalReaderView
{
public:

    /**
     * @brief Sets the status of the local reader view.
     *
     * @param new_status The new status to set.
     */
    void set_status(
            LocalReaderViewStatus new_status);

    /**
     * @brief Retrieves the current status of the local reader view.
     *
     * @return The current status of the local reader view.
     */
    LocalReaderViewStatus get_status();

    /**
     * @brief Waits for a specified number of references and sets the new status.
     *
     * This method will block until the specified number of references,
     * then it sets the status of the local reader view
     * to the new status.
     *
     * @param references The number of references to wait for before setting the status.
     * @param new_status The new status to set after waiting.
     */
    void wait_for_references_and_set_status(
            size_t references,
            LocalReaderViewStatus new_status);

    /**
     * @brief Increments the reference count of the local reader view.
     *
     * This method should be called whenever a new reference to the
     * local reader view is made.
     */
    void add_reference();

    /**
     * @brief Decrements the reference count of the local reader view.
     *
     * This method should be called to release a reference to the
     * local reader view.
     */
    void dereference();

protected:

    LocalReaderViewStatus status{LocalReaderViewStatus::ACTIVE};
    std::atomic<size_t> references{0};
    std::mutex mutex;
    std::condition_variable cv;

};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // FASTDDS_RTPS_WRITER__LOCALREADERVIEW_HPP