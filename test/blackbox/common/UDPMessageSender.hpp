#include <asio/io_service.hpp>
#include <asio/ip/udp.hpp>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/utils/IPLocator.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

struct UDPMessageSender
{
    asio::io_service service;
    asio::ip::udp::socket socket;

    UDPMessageSender()
        : service()
        , socket(service)
    {
        socket.open(asio::ip::udp::v4());
    }

    void send(
            const CDRMessage_t& msg,
            const Locator_t& destination)
    {
        std::string addr = IPLocator::toIPv4string(destination);
        unsigned short port = static_cast<unsigned short>(destination.port);
        auto remote = asio::ip::udp::endpoint(asio::ip::address::from_string(addr), port);
        asio::error_code ec;

        socket.send_to(asio::buffer(msg.buffer, msg.length), remote, 0, ec);
    }

};
