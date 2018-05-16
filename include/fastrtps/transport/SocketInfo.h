#ifndef RTPS_SOCKET_INFO_
#define RTPS_SOCKET_INFO_

#include <asio.hpp>
#include <memory>

namespace eprosima{
namespace fastrtps{
namespace rtps{

#if defined(ASIO_HAS_MOVE)
    // Typedefs
	typedef asio::ip::udp::socket eProsimaUDPSocket;
	typedef asio::ip::tcp::socket eProsimaTCPSocket;
    // UDP
	inline eProsimaUDPSocket* getSocketPtr(eProsimaUDPSocket &socket)
    {
        return &socket;
    }
    inline eProsimaUDPSocket moveSocket(eProsimaUDPSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::udp::socket(io_service));
    }
    inline eProsimaUDPSocket& getRefFromPtr(eProsimaUDPSocket* socket)
    {
        return *socket;
    }
    // TCP
	inline eProsimaTCPSocket* getSocketPtr(eProsimaTCPSocket &socket)
    {
        return &socket;
    }
    inline eProsimaTCPSocket moveSocket(eProsimaTCPSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaTCPSocket createTCPSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::tcp::socket(io_service));
    }
    inline eProsimaTCPSocket& getRefFromPtr(eProsimaTCPSocket* socket)
    {
        return *socket;
    }
#else
    // Typedefs
	typedef std::shared_ptr<asio::ip::udp::socket> eProsimaUDPSocket;
	typedef std::shared_ptr<asio::ip::tcp::socket> eProsimaTCPSocket;
    // UDP
    inline eProsimaUDPSocket getSocketPtr(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket& moveSocket(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::udp::socket>(io_service);
    }
    inline eProsimaUDPSocket& getRefFromPtr(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    // TCP
    inline eProsimaTCPSocket getSocketPtr(eProsimaTCPSocket &socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket& moveSocket(eProsimaTCPSocket &socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket createTCPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::tcp::socket>(io_service);
    }
    inline eProsimaTCPSocket& getRefFromPtr(eProsimaTCPSocket &socket)
    {
        return socket;
    }
#endif

class UDPSocketInfo
    {
        public:
            UDPSocketInfo(eProsimaUDPSocket& socket) :
                socket_(moveSocket(socket))
            {
            }

            UDPSocketInfo(UDPSocketInfo&& socketInfo) :
                socket_(moveSocket(socketInfo.socket_))
            {
            }

            UDPSocketInfo& operator=(UDPSocketInfo&& socketInfo)
            {
                socket_ = moveSocket(socketInfo.socket_);
                return *this;
            }

            void only_multicast_purpose(const bool value)
            {
                only_multicast_purpose_ = value;
            };

            bool& only_multicast_purpose()
            {
                return only_multicast_purpose_;
            }

            bool only_multicast_purpose() const
            {
                return only_multicast_purpose_;
            }

#if defined(ASIO_HAS_MOVE)
            inline eProsimaUDPSocket* getSocket()
#else
            inline eProsimaUDPSocket getSocket()
#endif
            {
                return getSocketPtr(socket_);
            }

        private:
            eProsimaUDPSocket socket_;
            bool only_multicast_purpose_;
            UDPSocketInfo(const UDPSocketInfo&) = delete;
            UDPSocketInfo& operator=(const UDPSocketInfo&) = delete;
    };

class TCPSocketInfo
{
    public:
        TCPSocketInfo(eProsimaTCPSocket& socket) :
            socket_(moveSocket(socket))
        {
            mMutex = std::make_shared<std::recursive_mutex>();
        }

        TCPSocketInfo(TCPSocketInfo&& socketInfo) :
            socket_(moveSocket(socketInfo.socket_))
        {
            mMutex = std::make_shared<std::recursive_mutex>();
        }

        TCPSocketInfo& operator=(TCPSocketInfo&& socketInfo)
        {
            socket_ = moveSocket(socketInfo.socket_);
            return *this;
        }

        void only_multicast_purpose(const bool value)
        {
            only_multicast_purpose_ = value;
        };

        bool& only_multicast_purpose()
        {
            return only_multicast_purpose_;
        }

        bool only_multicast_purpose() const
        {
            return only_multicast_purpose_;
        }

#if defined(ASIO_HAS_MOVE)
        inline eProsimaTCPSocket* getSocket()
#else
        inline eProsimaTCPSocket getSocket()
#endif
        {
            return getSocketPtr(socket_);
        }

        std::recursive_mutex& GetMutex() const
        {
            return *mMutex;
        }

    private:
        std::shared_ptr<std::recursive_mutex> mMutex;
        eProsimaTCPSocket socket_;
        bool only_multicast_purpose_;
        TCPSocketInfo(const TCPSocketInfo&) = delete;
        TCPSocketInfo& operator=(const TCPSocketInfo&) = delete;
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_PORT_MANAGER_