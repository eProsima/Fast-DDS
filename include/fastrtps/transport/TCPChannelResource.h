// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TCP_CHANNEL_RESOURCE_INFO_
#define TCP_CHANNEL_RESOURCE_INFO_

#include <asio.hpp>
#include <fastrtps/transport/TransportReceiverInterface.h>
#include <fastrtps/transport/ChannelResource.h>
#include <fastrtps/transport/tcp/RTCPMessageManager.h>
#include <fastrtps/rtps/common/Locator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TCPConnector;
class TCPTransportInterface;

enum eSocketErrorCodes
{
    eNoError,
    eBrokenPipe,
    eAsioError,
    eSystemError,
    eException,
    eConnectionAborted = 125
};

#if defined(ASIO_HAS_MOVE)
    // Typedefs
	typedef asio::ip::tcp::socket eProsimaTCPSocket;
    typedef eProsimaTCPSocket& eProsimaTCPSocketRef;

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
        return asio::ip::tcp::socket(io_service);
    }
	inline eProsimaTCPSocket& getTCPSocketRef(eProsimaTCPSocket& socket)
	{
		return socket;
	}
#else
    // Typedefs
	typedef std::shared_ptr<asio::ip::tcp::socket> eProsimaTCPSocket;
    typedef eProsimaTCPSocket eProsimaTCPSocketRef;

    // TCP
    inline eProsimaTCPSocket getSocketPtr(eProsimaTCPSocket socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket moveSocket(eProsimaTCPSocket socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket createTCPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::tcp::socket>(io_service);
    }
	inline asio::ip::tcp::socket& getTCPSocketRef(eProsimaTCPSocket socket)
    {
        return *socket;
    }
#endif

class TCPChannelResource : public ChannelResource
{
enum eConnectionStatus
{
    eDisconnected = 0,
    eConnecting,                // Output -> Trying connection.
    eConnected,                 // Output -> Send bind message.
    eWaitingForBind,            // Input -> Waiting for the bind message.
    eWaitingForBindResponse,    // Output -> Waiting for the bind response message.
    eEstablished,
    eUnbinding
};

public:
    // Constructor called when trying to connect to a remote server
    TCPChannelResource(TCPTransportInterface* parent, RTCPMessageManager* rtcpManager,
        asio::io_service& service, const Locator_t& locator, uint32_t maxMsgSize);

    // Constructor called when local server accepted connection
    TCPChannelResource(TCPTransportInterface* parent, RTCPMessageManager* rtcpManager,
        asio::io_service& service, eProsimaTCPSocketRef socket, uint32_t maxMsgSize);

    virtual ~TCPChannelResource();

    bool operator==(const TCPChannelResource& channelResource) const
    {
        return &mSocket == &(channelResource.mSocket);
    }

    void fillLogicalPorts(std::vector<Locator_t>& outVector);

    void AddLogicalPort(uint16_t port);

    void SetLogicalPortPending(uint16_t port);

    bool RemoveLogicalPort(uint16_t port);

	virtual void Disable() override;

	uint32_t GetMsgSize() const;

#if defined(ASIO_HAS_MOVE)
    inline eProsimaTCPSocket* getSocket()
#else
    inline eProsimaTCPSocket getSocket()
#endif
    {
        return getSocketPtr(mSocket);
    }

    std::recursive_mutex& GetReadMutex()
    {
        return mReadMutex;
    }

    std::recursive_mutex& GetWriteMutex()
    {
        return mWriteMutex;
    }

    inline void SetRTCPThread(std::thread* pThread)
    {
        mRTCPThread = pThread;
    }

    std::thread* ReleaseRTCPThread();

    inline bool GetIsInputSocket() const
    {
        return m_inputSocket;
    }

    inline void SetIsInputSocket(bool bInput)
    {
        m_inputSocket = bInput;
    }

	bool IsLogicalPortOpened(uint16_t port);

    bool IsLogicalPortAdded(uint16_t port);

    bool IsConnectionEstablished()
    {
        std::unique_lock<std::mutex> scoped(mStatusMutex);
        return mConnectionStatus == eConnectionStatus::eEstablished;
    }

    inline const Locator_t& GetLocator() const
    {
        return mLocator;
    }

    void InputPortClosed(uint16_t port);

    void Connect();
    ResponseCode ProcessBindRequest(const Locator_t& locator);
    void Disconnect();

    bool WaitUntilPortIsOpenOrConnectionIsClosed(uint16_t port);

protected:
    inline bool ChangeStatus(eConnectionStatus s)
    {
        std::unique_lock<std::mutex> scoped(mStatusMutex);
        if (mConnectionStatus != s)
        {
        	mConnectionStatus = s;
	        if (mConnectionStatus == eEstablished)
	        {
	            SendPendingOpenLogicalPorts();
	        }
            mNegotiationCondition.notify_all();
	        return true;
	    }
	    return false;
    }

    void AddLogicalPortResponse(const TCPTransactionId &id, bool success);
    void ProcessCheckLogicalPortsResponse(const TCPTransactionId &transactionId,
        const std::vector<uint16_t> &availablePorts);

    friend class TCPTransportInterface;
    friend class RTCPMessageManager;
    friend class test_RTCPMessageManager;

private:
    TCPTransportInterface * mParent;
    RTCPMessageManager* mRTCPManager;
    Locator_t mLocator;
    bool m_inputSocket;
    bool mWaitingForKeepAlive;
    std::map<TCPTransactionId, uint16_t> mNegotiatingLogicalPorts; // Must be accessed after lock mPendingLogicalMutex
    std::map<TCPTransactionId, uint16_t> mLastCheckedLogicalPort;
    std::thread* mRTCPThread;
    std::vector<uint16_t> mPendingLogicalOutputPorts; // Must be accessed after lock mPendingLogicalMutex
    std::vector<uint16_t> mLogicalOutputPorts;
    std::recursive_mutex mReadMutex;
    std::recursive_mutex mWriteMutex;
    std::recursive_mutex mPendingLogicalMutex;
    std::condition_variable mNegotiationCondition;
    asio::io_service& mService;
    eProsimaTCPSocket mSocket;
    eConnectionStatus mConnectionStatus;
    std::mutex mStatusMutex;

    void PrepareAndSendCheckLogicalPortsRequest(uint16_t closedPort);
    void SendPendingOpenLogicalPorts();
    void CopyPendingPortsFrom(TCPChannelResource* from);
    void SetAllPortsAsPending();

    TCPChannelResource(const TCPChannelResource&) = delete;
    TCPChannelResource& operator=(const TCPChannelResource&) = delete;
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // TCP_CHANNEL_RESOURCE_INFO_
