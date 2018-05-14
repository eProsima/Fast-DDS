
#ifndef RTPS_TCP_MANAGER_
#define RTPS_TCP_MANAGER_

#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/utils/Semaphore.h>
#include "TCPPortManager.h"

#include <asio.hpp>
#include <memory>
#include <map>
#include <functional>
#include <mutex>

#if defined(ASIO_HAS_MOVE)
	typedef asio::ip::tcp::socket TCPSocket;
#else
	typedef std::shared_ptr<asio::ip::tcp::socket> TCPSocket;
#endif

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TCPManager
{
public:
    virtual bool IsOpenSocket(const Locator_t &locator);
    virtual bool OpenInputSocket(Locator_t &locator, const TCPInputSocketListener* listener);
    virtual bool OpenOutputSocket(Locator_t &locator, const TCPOutputSocketListener* listener);
    virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator);
    virtual void SocketAccepted(const Locator_t& locator, uint32_t receiveBufferSize, const asio::error_code& error);
    virtual void SocketConnected(const Locator_t& locator, uint32_t id, uint32_t sendBufferSize, const asio::error_code& error);
protected:
    uint32_t mSendBufferSize;
    uint32_t mReceiveBufferSize;
    asio::io_service mService;
    mutable std::recursive_mutex mOutputMapMutex;
    mutable std::recursive_mutex mInputMapMutex;
    //! Checks for TCPv4 kind.
    virtual bool IsLocatorSupported(const Locator_t&) const;
    bool OpenAndBindInputSockets(Locator_t &locator, const TCPInputSocketListener &listener);
    void OpenAndBindInputSocket(Locator_t &locator, const TCPInputSocketListener &listener);
private:
    std::map<Locator_t, TCPSocket> m_LocatorInputSockets;
    std::map<Locator_t, TCPSocket> m_LocatorOutputSockets;
    std::map<Locator_t, Semaphore*> mInputSemaphores;
    std::map<Locator_t, Semaphore*> mOutputSemaphores;
    std::map<Locator_t, std::vector<TCPInputSocketListener>> mReceiveListeners;
};



} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_MANAGER_