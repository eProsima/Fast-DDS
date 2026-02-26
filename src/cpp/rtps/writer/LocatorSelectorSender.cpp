#include <rtps/writer/BaseWriter.hpp>

#include <rtps/writer/LocatorSelectorSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool LocatorSelectorSender::send(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        std::chrono::steady_clock::time_point max_blocking_time_point)
{
    locator_selector.initial_allow_to_send(true); // Reset initial allow to send flag
    return writer_.send_nts(buffers, total_bytes, *this, max_blocking_time_point);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
