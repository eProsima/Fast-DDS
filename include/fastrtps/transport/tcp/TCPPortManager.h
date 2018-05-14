#ifndef RTPS_TCP_PORT_MANAGER_
#define RTPS_TCP_PORT_MANAGER_

#include "TCPSocketListener.h"
#include "RemoteLocator.h"
#include <fastrtps/transport/SocketInfo.h>
#include <fastrtps/rtps/common/Locator.h>
#include <asio.hpp>
#include <vector>
#include <mutex>
#include <map>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * Manager class that keep control for locator ports.
 * Locator ports can be local input ports or remote output ports.
 * */
class TCPPortManager
{
public:
    TCPPortManager() = delete;
    TCPPortManager(uint16_t port) : m_Port(port), m_socket(nullptr), m_Accepter(nullptr) {}
    virtual bool IsOpenInput();
    virtual bool IsOpenOutput(const Locator_t& remoteLocator);
    virtual bool OpenInput(Locator_t &locator, const TCPInputSocketListener* listener);
    virtual bool OpenOutput(Locator_t &locator, const TCPOutputSocketListener* listener);
    virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator);
    virtual void SocketAccepted(const Locator_t& locator, uint32_t receiveBufferSize, 
        const asio::error_code& error);
    virtual void SocketConnected(const Locator_t& locator, uint32_t id, 
        uint32_t sendBufferSize, const asio::error_code& error);
    ~TCPPortManager()
    {
        delete m_Accepter;
    }
protected:
    mutable std::recursive_mutex mOutputMapMutex;
    mutable std::recursive_mutex mInputMapMutex;

    class TCPAccepter
    {
    public:
        asio::ip::tcp::acceptor m_acceptor;
        uint32_t m_receiveBufferSize;

        TCPAccepter(asio::io_service& io_service, uint16_t port, uint32_t receiveBufferSize);

        void Accept();
    };

    /*
    class TCPConnector
    {
    public:
        uint32_t m_id;
        Locator_t* m_Locator;
        uint32_t m_sendBufferSize;
        SocketInfo m_socket;

        TCPConnector(asio::io_service& io_service, Locator_t &locator, uint32_t id, uint32_t sendBufferSize);

        void Connect(Locator_t &locator);
        void RetryConnect(asio::io_service& io_service);
    };
    */
    virtual void Connect(Locator_t &locator);
    virtual void RetryConnect(asio::io_service& io_service);
    virtual bool IsLocatorSupported(const Locator_t&) const; // Checks for TCP kind.
    bool OpenAndBindInputSockets(Locator_t &locator, const TCPInputSocketListener *listener);
    void OpenAndBindInputSocket(Locator_t &locator, const TCPInputSocketListener *listener);
    void RegisterListener(const TCPInputSocketListener *listener);
private:
    const uint16_t m_Port;                                                  // Local bound physical port
    SocketInfo* m_socket;                                                   // Input socket
    std::map<uint16_t, const TCPInputSocketListener*> m_InputListeners;     // Logical ports map
    std::map<Locator_t, const TCPOutputSocketListener*> m_OutputListeners;  // Logical ports map
    std::vector<const TCPInputSocketListener*> m_AcceptListeners;           // Listeners for no logical port cases
    std::vector<SocketInfo*> m_Connections;                                 // Already bound connections
    std::vector<SocketInfo*> m_UnboundConnections;                          // Not yet bound connections
    TCPAccepter* m_Accepter;                                                // For input connections
    std::map<RemoteLocator_t, std::vector<Locator_t>> m_LocatorToRemote;    // Concrete Locators
    std::map<RemoteLocator_t, SocketInfo*> m_OutputConnections;             // Key is remote locator
    //std::map<Locator_t, Semaphore*> mInputSemaphores;
    //std::map<Locator_t, Semaphore*> mOutputSemaphores;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_PORT_MANAGER_