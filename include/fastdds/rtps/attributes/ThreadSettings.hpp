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

/**
 * @file ThreadSettings.hpp
 */

#include <cstdint>
#include <limits>

#include <fastdds/fastdds_dll.hpp>

#ifndef FASTDDS_RTPS_ATTRIBUTES_THREADSETTINGS_HPP
#define FASTDDS_RTPS_ATTRIBUTES_THREADSETTINGS_HPP

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Struct ThreadSettings to specify various thread settings.
 * This class is used to define attributes across a wide set of Qos and APIs.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
struct FASTDDS_EXPORTED_API ThreadSettings
{
    /**
     * @brief The scheduling policy used for this thread.
     *
     * Configures the scheduling policy used for the thread.
     * A value of -1 indicates system default.
     *
     * This value is platform specific and it is used as-is to configure the specific platform thread.
     * It is ignored on Windows platforms.
     * Setting this value to something other than the default one may require different privileges
     * on different platforms.
     */
    int32_t scheduling_policy = -1;

    /**
     * @brief The thread's priority.
     *
     * Configures the thread's priority.
     * A value of -2^31 indicates system default.
     *
     * This value is platform specific and it is used as-is to configure the specific platform thread.
     * Setting this value to something other than the default one may require different privileges
     * on different platforms.
     */
    int32_t priority = std::numeric_limits<int32_t>::min();

    /**
     * @brief The thread's affinity.
     *
     * On some systems (Windows, Linux), this is a bit mask for setting the threads affinity to each core individually.
     * On MacOS, this sets the affinity tag for the thread, and the OS tries to share the L2 cache between threads
     * with the same affinity.
     * A value of 0 indicates no particular affinity.
     *
     * This value is platform specific and it is used as-is to configure the specific platform thread.
     * Setting this value to something other than the default one may require different privileges
     * on different platforms.
     */
    uint64_t affinity = 0;

    /**
     * @brief The thread's stack size in bytes.
     *
     * Configures the thread's stack size.
     * A value of -1 indicates system default.
     *
     * This value is platform specific and it is used as-is to configure the specific platform thread.
     * Setting this value to something other than the default one may require different privileges
     * on different platforms.
     */
    int32_t stack_size = -1;

    /**
     * Compare the left hand side (LHS) ThreadSetting with another one for equality.
     *
     * @param rhs The ThreadSettings instance to compare with the LHS one.
     */
    bool operator ==(
            const ThreadSettings& rhs) const;

    /**
     * Compare the left hand side (LHS) ThreadSetting with another one for inequality.
     *
     * @param rhs The ThreadSettings instance to compare with the LHS one.
     */
    bool operator !=(
            const ThreadSettings& rhs) const;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_ATTRIBUTES_THREADSETTINGS_HPP
