
#ifndef RTPS_TCP_MANAGER_
#define RTPS_TCP_MANAGER_

#include <fastrtps/rtps/common/Locator.h>

#include <asio.hpp>
#include <memory>
#include <map>
#include <functional>

#if defined(ASIO_HAS_MOVE)
	typedef asio::ip::tcp::socket TCPSocket;
#else
	typedef std::shared_ptr<asio::ip::tcp::socket> TCPSocket;
#endif

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TCPSocketListener
{
public:
    virtual void onReceive(octet* receiveBuffer, uint32_t receiveBufferSize, 
            const Locator_t& localLocator, Locator_t& remoteLocator);
};

class TCPManager
{
public:
    virtual bool IsOpenSocket(const Locator_t &locator);
    virtual bool OpenInputSocket(Locator_t &locator, const TCPSocketListener &listener);
    virtual bool OpenOutputSocket(Locator_t &locator);
    virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator);
    virtual void SocketAccepted(uint32_t port, uint32_t receiveBufferSize, const asio::error_code& error);
    virtual void SocketConnected(uint32_t port, uint32_t id, uint32_t sendBufferSize, const asio::error_code& error);
protected:
private:
    std::map<Locator_t, TCPSocket> m_LocatorInputSockets;
    std::map<Locator_t, TCPSocket> m_LocatorOutputSockets;
};


class TCPManagerAccepter
{
public:
	asio::ip::tcp::acceptor m_acceptor;
	uint16_t m_port;
	uint32_t m_receiveBufferSize;
	TCPSocket m_socket;

	TCPManagerAccepter(asio::io_service& io_service, uint16_t port, uint32_t receiveBufferSize);

	void Accept(TCPManager *manager);
	void RetryAccept(asio::io_service& io_service, TCPManager *manager);
};

class TCPManagerConnector
{
public:
	uint32_t m_port;
	uint32_t m_id;
	const asio::ip::address_v4 m_ipAddress;
	uint32_t m_sendBufferSize;
    TCPSocket m_socket;

	TCPManagerConnector(asio::io_service& io_service, const asio::ip::address_v4& ipAddress, uint16_t port, uint32_t id, uint32_t sendBufferSize);

	void Connect(TCPManager *manager, uint32_t& port);
	void RetryConnect(asio::io_service& io_service, TCPManager *manager);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_MANAGER_