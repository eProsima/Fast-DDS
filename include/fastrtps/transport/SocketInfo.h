#ifndef RTPS_SOCKET_INFO_
#define RTPS_SOCKET_INFO_

#include <asio.hpp>
#include <memory>

namespace eprosima{
namespace fastrtps{
namespace rtps{

#if defined(ASIO_HAS_MOVE)
	typedef asio::ip::tcp::socket eProsimaSocket;
	inline eProsimaSocket* getSocketPtr(eProsimaSocket &socket)
    {
        return &socket;
    }
    inline eProsimaSocket&& moveSocket(eProsimaSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaSocket&& createSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::tcp::socket(io_service));
    }
#else
	typedef std::shared_ptr<asio::ip::tcp::socket> eProsimaSocket;
    inline eProsimaSocket getSocketPtr(eProsimaSocket &socket)
    {
        return socket;
    }
    inline eProsimaSocket& moveSocket(eProsimaSocket &socket)
    {
        return socket;
    }
    inline eProsimaSocket&& createSocket(asio::io_service& io_service)
    {
        return std::move(std::make_shared<asio::ip::tcp::socket>(io_service));
    }
#endif

class SocketInfo
    {
        public:
            SocketInfo(eProsimaSocket& socket) :
                socket_(moveSocket(socket))
            {
            }

            SocketInfo(SocketInfo&& socketInfo) :
                socket_(moveSocket(socketInfo.socket_))
            {
            }

            SocketInfo& operator=(SocketInfo&& socketInfo)
            {
                socket_ = moveSocket(socketInfo.socket_);
                return *this;
            }

            inline const eProsimaSocket* getSocket()
            {
                return getSocketPtr(socket_);
            }

        private:
            eProsimaSocket socket_;
            SocketInfo(const SocketInfo&) = delete;
            SocketInfo& operator=(const SocketInfo&) = delete;
    };

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_PORT_MANAGER_