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
 * @file LocalReaderView.cpp
 */

#include <rtps/reader/LocalReaderView.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

void LocalReaderView::set_status(
        LocalReaderViewStatus new_status)
{
    std::lock_guard<std::mutex> lock(mutex);
    status = new_status;
}

LocalReaderViewStatus LocalReaderView::get_status()
{
    std::lock_guard<std::mutex> lock(mutex);
    return status;
}

void LocalReaderView::add_reference()
{
    references.fetch_add(1);
}

void LocalReaderView::dereference()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (references.load() != 0u)
        {
            references.fetch_sub(1);
        }
    }

    cv.notify_one();
}

void LocalReaderView::wait_for_references_and_set_status(
        size_t references,
        LocalReaderViewStatus new_status)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (this->references.load() != references)
    {
        cv.wait(lock, [&]()
                {
                    return this->references.load() == references;
                });
    }
    status = new_status;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
