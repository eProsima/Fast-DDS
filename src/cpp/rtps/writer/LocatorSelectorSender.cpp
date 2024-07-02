#include <rtps/writer/BaseWriter.hpp>

#include <rtps/writer/LocatorSelectorSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool LocatorSelectorSender::send(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        std::chrono::steady_clock::time_point max_blocking_time_point) const
{
    return writer_.send_nts(buffers, total_bytes, *this, max_blocking_time_point);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
