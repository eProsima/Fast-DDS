// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file InputFeedProcessor.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_RPC__INPUT_FEED_PROCESSOR_HPP
#define FASTDDS_EXAMPLES_CPP_RPC__INPUT_FEED_PROCESSOR_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <limits>

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc {

class InputFeedProcessor
{
public:

    enum class Status
    {
        VALID_INPUT,
        INVALID_INPUT,
        FEED_CLOSED
    };

    static std::pair<Status, int32_t> get_input()
    {
        long long value = 0;

        if (!provided_input_feed)
        {
            std::string line;
            if (!std::getline(std::cin, line))
            {
                EPROSIMA_LOG_ERROR(InputFeedProcessor, "An error occurred while reading the input.");
                return std::make_pair(Status::INVALID_INPUT, 0);
            }

            if (line.empty())
            {
                EPROSIMA_LOG_INFO(InputFeedProcessor, "Empty input received. Closing feed.");
                return std::make_pair(Status::FEED_CLOSED, 0);
            }

            try
            {
                value = std::stoll(line);
            }
            catch (const std::invalid_argument&)
            {
                EPROSIMA_LOG_ERROR(InputFeedProcessor, "Invalid input: " << line);
                return std::make_pair(Status::INVALID_INPUT, 0);
            }
            catch (const std::out_of_range&)
            {
                EPROSIMA_LOG_ERROR(InputFeedProcessor, "Input out of range: " << line);
                return std::make_pair(Status::INVALID_INPUT, 0);
            }

            if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max())
            {
                return std::make_pair(Status::INVALID_INPUT, 0);
            }
        }
        else
        {
            try
            {
                if (provided_input_feed->good() && provided_input_feed->rdbuf()->in_avail())
                {
                    std::string substr;
                    getline(*provided_input_feed, substr, ',');
                    try
                    {
                        value = std::stoi(substr);
                        if (value < std::numeric_limits<int32_t>::min() ||
                                value > std::numeric_limits<int32_t>::max())
                        {
                            return std::make_pair(Status::INVALID_INPUT, 0);
                        }
                    }
                    catch (...)
                    {
                        return std::make_pair(Status::INVALID_INPUT, 0);
                    }
                }
                else
                {
                    return std::make_pair(Status::FEED_CLOSED, 0);
                }
            }
            catch (const std::invalid_argument&)
            {
                return std::make_pair(Status::INVALID_INPUT, 0);
            }
        }

        return std::make_pair(Status::VALID_INPUT, static_cast<int32_t>(value));
    }

    static void print_help()
    {
        std::cout << "Input feed help:" << std::endl;
        std::cout << "  - Enter a number to process it." << std::endl;
        std::cout << "  - Press Enter without typing anything to close the input feed." << std::endl;
    }

    static std::unique_ptr<std::stringstream> provided_input_feed;
};

} // namespace rpc
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RPC__INPUT_FEED_PROCESSOR_HPP
