#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

struct UDPMessageSender
{
    asio::io_context context;
    asio::ip::udp::socket socket;

    UDPMessageSender()
        : context()
        , socket(context)
    {
        socket.open(asio::ip::udp::v4());
    }

    void send(
            const CDRMessage_t& msg,
            const Locator_t& destination)
    {
        std::string addr = IPLocator::toIPv4string(destination);
        unsigned short port = static_cast<unsigned short>(destination.port);
        auto remote = asio::ip::udp::endpoint(asio::ip::make_address(addr), port);
        asio::error_code ec;

        socket.send_to(asio::buffer(msg.buffer, msg.length), remote, 0, ec);
    }

};
