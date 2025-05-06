#ifndef FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_FEED__INPUT_FEED_PROCESSOR_HPP
#define FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_FEED__INPUT_FEED_PROCESSOR_HPP

#include <iostream>
#include <string>
#include <limits>

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc_client_server {

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

        long long value = 0;

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

        return std::make_pair(Status::VALID_INPUT, static_cast<int32_t>(value));
    }

    static void print_help()
    {
        std::cout << "Input feed help:" << std::endl;
        std::cout << "  - Enter a number to process it." << std::endl;
        std::cout << "  - Press Enter without typing anything to close the input feed." << std::endl;
    }
};

} // namespace rpc_client_server
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_FEED__INPUT_FEED_PROCESSOR_HPP
