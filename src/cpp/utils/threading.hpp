// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UTILS__THREADING_HPP_
#define UTILS__THREADING_HPP_

#include <array>

#include "./thread.hpp"

namespace eprosima {

// Forward declare dependencies
namespace fastdds {
namespace rtps {
struct ThreadSettings;
} // namespace rtps
} // namespace fastdds

/**
 * @brief Give a name to the thread calling this function.
 *
 * @param [in, out]  thread_name_buffer  Buffer to store the name of the thread.
 * @param [in]       name  A null-terminated string with the name to give to the calling thread.
 *                        The implementation for certain platforms may truncate the final thread
 *                         name if there is a limit on the length of the name of a thread.
 */
void set_name_to_current_thread(
        std::array<char, 16>& thread_name_buffer,
        const char* name);

/**
 * @brief Give a name to the thread calling this function.
 *
 * @param [in, out]  thread_name_buffer  Buffer to store the name of the thread.
 * @param [in]       fmt   A null-terminated string to be used as the format argument of
 *                        a `snprintf` like function, in order to accomodate the restrictions of
 *                        the OS. Those restrictions may truncate the final thread name if there
 *                        is a limit on the length of the name of a thread.
 * @param [in]       arg   Single variadic argument passed to the formatting function.
 */
void set_name_to_current_thread(
        std::array<char, 16>& thread_name_buffer,
        const char* fmt,
        uint32_t arg);

/**
 * @brief Give a name to the thread calling this function.
 *
 * @param [in, out]  thread_name_buffer  Buffer to store the name of the thread.
 * @param [in]       fmt   A null-terminated string to be used as the format argument of
 *                        a `snprintf` like function, in order to accomodate the restrictions of
 *                        the OS. Those restrictions may truncate the final thread name if there
 *                        is a limit on the length of the name of a thread.
 * @param [in]       arg1  First variadic argument passed to the formatting function.
 * @param [in]       arg2  Second variadic argument passed to the formatting function.
 */
void set_name_to_current_thread(
        std::array<char, 16>& thread_name_buffer,
        const char* fmt,
        uint32_t arg1,
        uint32_t arg2);

/**
 * @brief Apply thread settings to the thread calling this function.
 *
 * @param [in]  thread_name  Name of the thread.
 * @param [in]  settings  Thread settings to apply.
 */
void apply_thread_settings_to_current_thread(
        const char* thread_name,
        const fastdds::rtps::ThreadSettings& settings);

/**
 * @brief Create and start a thread with custom settings and name.
 *
 * This wrapper will create a thread on which the incoming functor will be called after
 * giving it a custom name and applying the thread settings.
 *
 * @param [in]  func      Functor with the logic to be run on the created thread.
 * @param [in]  settings  Thread settings to apply to the created thread.
 * @param [in]  name      Name (format) for the created thread.
 * @param [in]  args      Additional arguments to complete the thread name.
 *                       See @ref set_name_to_current_thread for details.
 */
template<typename Functor, typename ... Args>
eprosima::thread create_thread(
        Functor func,
        const fastdds::rtps::ThreadSettings& settings,
        const char* name,
        Args... args)
{
    return eprosima::thread(settings.stack_size, [=]()
                   {
                       std::array<char, 16> thread_name_buffer;
                       set_name_to_current_thread(thread_name_buffer, name, args ...);
                       apply_thread_settings_to_current_thread(thread_name_buffer.data(), settings);
                       func();
                   });
}

} // eprosima

#endif  // UTILS__THREADING_HPP_
