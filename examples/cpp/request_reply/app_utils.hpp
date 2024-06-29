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
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "CLIParser.hpp"
#include "types/Calculator.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

struct RequestInput
{
    RequestInput(
            const CLIParser::config& config)
        : operation(config.operation)
        , x(config.x)
        , y(config.y)
    {
    }

    CLIParser::Operation operation = CLIParser::Operation::UNDEFINED;

    std::int16_t x = 0;

    std::int16_t y = 0;
};

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
    static CalculatorOperationType to_calculator_type(
            const CLIParser::Operation& operation)
    {
        CalculatorOperationType calculator_operation;

        switch (operation)
        {
            case CLIParser::Operation::ADDITION:
                calculator_operation = CalculatorOperationType::ADDITION;
                break;
            case CLIParser::Operation::SUBTRACTION:
                calculator_operation = CalculatorOperationType::SUBTRACTION;
                break;
            case CLIParser::Operation::MULTIPLICATION:
                calculator_operation = CalculatorOperationType::MULTIPLICATION;
                break;
            case CLIParser::Operation::DIVISION:
                calculator_operation = CalculatorOperationType::DIVISION;
                break;
            default:
                throw std::invalid_argument("Invalid operation");
                break;
        }

        return calculator_operation;
    }

    static std::string to_calculator_type(
            const rtps::GuidPrefix_t& guid_prefix)
    {
        std::ostringstream client_id;
        client_id << guid_prefix;
        return client_id.str();
    }

};

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__APP_UTILS_HPP
