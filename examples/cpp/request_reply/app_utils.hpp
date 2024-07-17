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

#ifndef FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__APP_UTILS_HPP
#define FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__APP_UTILS_HPP

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
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>

#include "types/Calculator.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

class RemoteServerMatchedStatus
{
public:

    void match_request_reader(
            const rtps::GuidPrefix_t& guid_prefix,
            const bool& status)
    {
        matched_status_[guid_prefix].set(request_reader_position, status);
    }

    void match_reply_writer(
            const rtps::GuidPrefix_t& guid_prefix,
            const bool& status)
    {
        matched_status_[guid_prefix].set(reply_writer_position, status);
    }

    bool is_matched(
            const rtps::GuidPrefix_t& guid_prefix)
    {
        bool is_server_matched = false;

        auto status = matched_status_.find(guid_prefix);

        if (status != matched_status_.end())
        {
            is_server_matched = status->second.all();
        }

        return is_server_matched;
    }

    bool is_any_server_matched()
    {
        bool any_server_matched = false;
        for (const auto& status : matched_status_)
        {
            if (status.second.all())
            {
                any_server_matched = true;
                break;
            }
        }
        return any_server_matched;
    }

    void clear()
    {
        matched_status_.clear();
    }

private:

    std::map<rtps::GuidPrefix_t, std::bitset<2>> matched_status_;

    static const size_t request_reader_position = 0;

    static const size_t reply_writer_position = 1;
};

class RemoteClientMatchedStatus
{
public:

    void match_request_writer(
            const rtps::GuidPrefix_t& guid_prefix,
            const bool& status)
    {
        matched_status_[guid_prefix].set(request_writer_position, status);
    }

    void match_reply_reader(
            const rtps::GuidPrefix_t& guid_prefix,
            const bool& status)
    {
        matched_status_[guid_prefix].set(reply_reader_position, status);
    }

    bool is_matched(
            const rtps::GuidPrefix_t& guid_prefix)
    {
        bool is_client_matched = false;

        auto status = matched_status_.find(guid_prefix);

        if (status != matched_status_.end())
        {
            is_client_matched = status->second.all();
        }

        return is_client_matched;
    }

    bool is_fully_unmatched(
            const rtps::GuidPrefix_t& guid_prefix)
    {
        bool is_client_unmatched = false;

        auto status = matched_status_.find(guid_prefix);

        if (status != matched_status_.end())
        {
            is_client_unmatched = !status->second.none();
        }

        return is_client_unmatched;
    }

    bool no_client_matched()
    {
        bool no_client_matched = true;

        for (const auto& status : matched_status_)
        {
            if (status.second.any())
            {
                no_client_matched = false;
                break;
            }
        }

        return no_client_matched;
    }

    void clear()
    {
        matched_status_.clear();
    }

private:

    std::map<rtps::GuidPrefix_t, std::bitset<2>> matched_status_;

    static const size_t request_writer_position = 0;

    static const size_t reply_reader_position = 1;
};

struct TypeConverter
{
    static std::string to_string(
            const CalculatorRequestType& request)
    {
        std::ostringstream request_ss;
        request_ss << request.x() << " " << to_string(request.operation()) << " " << request.y();
        return request_ss.str();
    }

    static std::string to_string(
            const CalculatorOperationType& operation)
    {
        std::string operation_str = "Unknown";
        switch (operation)
        {
            case CalculatorOperationType::ADDITION:
                operation_str = "+";
                break;
            case CalculatorOperationType::SUBTRACTION:
                operation_str = "-";
                break;
            case CalculatorOperationType::MULTIPLICATION:
                operation_str = "*";
                break;
            case CalculatorOperationType::DIVISION:
                operation_str = "/";
                break;
            default:
                break;
        }
        return operation_str;
    }

    static std::string to_string(
            const rtps::GuidPrefix_t& guid_prefix)
    {
        std::ostringstream client_id;
        client_id << guid_prefix;
        return client_id.str();
    }

    static std::string to_string(
            const rtps::ParticipantDiscoveryStatus& info)
    {
        std::string info_str = "Unknown";

        switch (info)
        {
            case rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT:
                info_str = "discovered";
                break;
            case rtps::ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT:
                info_str = "changed QoS";
                break;
            case rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT:
                info_str = "removed";
                break;
            case rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT:
                info_str = "dropped";
                break;
            case rtps::ParticipantDiscoveryStatus::IGNORED_PARTICIPANT:
                info_str = "ignored";
                break;
            default:
                break;
        }

        return info_str;
    }

};

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
#define request_reply_debug(context, message) \
    std::cout << C_B_WHITE << Timestamp::now() << C_B_BLUE << " [DEBUG] " << C_B_WHITE \
              << "[" << context << "] " << C_DEF << message << std::endl
#else
#define request_reply_debug(context, message)
#endif // ifdef NDEGUB

#define request_reply_info(context, message) \
    std::cout << C_B_WHITE << Timestamp::now() << C_B_GREEN << " [INFO] " << C_B_WHITE \
              << "[" << context << "] " << C_DEF << message << std::endl

#define request_reply_error(context, message) \
    std::cerr << C_B_WHITE << Timestamp::now() << C_B_RED << " [ERROR] " << C_B_WHITE \
              << "[" << context << "] " << C_DEF << message << std::endl

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__APP_UTILS_HPP
