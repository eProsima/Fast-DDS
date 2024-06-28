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

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <map>

#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "CLIParser.hpp"

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
        return matched_status_[guid_prefix].all();
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
        return matched_status_[guid_prefix].all();
    }

private:

    std::map<rtps::GuidPrefix_t, std::bitset<2>> matched_status_;

    static const size_t request_writer_position = 0;

    static const size_t reply_reader_position = 1;
};

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__APP_UTILS_HPP
