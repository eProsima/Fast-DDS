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
 * @file app_utils.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_RPC__APP_UTILS_HPP
#define FASTDDS_EXAMPLES_CPP_RPC__APP_UTILS_HPP

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "types/calculator.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc {

struct Timestamp
{
    static std::string now()
    {
        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        // Convert to tm struct for local time
        std::tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_now, &time_t_now);
#else
        localtime_r(&time_t_now, &tm_now);
#endif // if defined(_WIN32) || defined(_WIN64)

        // Format date and time
        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%S");

        // Add milliseconds
        auto duration = now.time_since_epoch();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        oss << '.' << std::setfill('0') << std::setw(3) << milliseconds;

        return oss.str();
    }

};

#ifndef NDEGUB
#define client_server_debug(context, message) \
    std::cout << C_B_WHITE << Timestamp::now() << C_B_BLUE << " [DEBUG] " << C_B_WHITE \
              << "[" << context << "] " << C_DEF << message << std::endl
#else
#define client_server_debug(context, message)
#endif // ifdef NDEGUB

#define client_server_info(context, message) \
    std::cout << C_B_WHITE << Timestamp::now() << C_B_GREEN << " [INFO] " << C_B_WHITE \
              << "[" << context << "] " << C_DEF << message << std::endl

#define client_server_error(context, message) \
    std::cerr << C_B_WHITE << Timestamp::now() << C_B_RED << " [ERROR] " << C_B_WHITE \
              << "[" << context << "] " << C_DEF << message << std::endl

} // namespace rpc
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RPC__APP_UTILS_HPP
