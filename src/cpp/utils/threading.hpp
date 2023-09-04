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

namespace eprosima {

/**
 * @brief Give a name to the thread calling this function.
 *
 * @param[in]  name  A null-terminated string with the name to give to the calling thread.
 *                   The implementation for certain platforms may truncate the final thread
 *                   name if there is a limit on the length of the name of a thread.
 */
void set_name_to_current_thread(
        const char* name);

/**
 * @brief Give a name to the thread calling this function.
 *
 * @param[in]  fmt   A null-terminated string to be used as the format argument of
 *                   a `snprintf` like function, in order to accomodate the restrictions of
 *                   the OS. Those restrictions may truncate the final thread name if there
 *                   is a limit on the length of the name of a thread.
 * @param[in]  arg   Single variadic argument passed to the formatting function.
 */
void set_name_to_current_thread(
        const char* fmt,
        uint32_t arg);

/**
 * @brief Give a name to the thread calling this function.
 *
 * @param[in]  fmt   A null-terminated string to be used as the format argument of
 *                   a `snprintf` like function, in order to accomodate the restrictions of
 *                   the OS. Those restrictions may truncate the final thread name if there
 *                   is a limit on the length of the name of a thread.
 * @param[in]  arg1  First variadic argument passed to the formatting function.
 * @param[in]  arg2  Second variadic argument passed to the formatting function.
 */
void set_name_to_current_thread(
        const char* fmt,
        uint32_t arg1,
        uint32_t arg2);

} // eprosima

#endif  // UTILS__THREADING_HPP_
