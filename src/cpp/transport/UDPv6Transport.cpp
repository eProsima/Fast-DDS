
#include <fastrtps/transport/UDPv6Transport.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/RTPSLog.h>

using namespace std;
using namespace boost::asio;
using namespace boost::interprocess;

namespace eprosima{
namespace fastrtps{
namespace rtps{

static const uint32_t maximumUDPSocketSize = 65536;
static const char* const CLASS_NAME = "UDPv6Transport";

UDPv6Transport::UDPv6Transport(const UDPv6TransportDescriptor& descriptor):
    mSendBufferSize(descriptor.sendBufferSize),
    mReceiveBufferSize(descriptor.receiveBufferSize),
    mGranularMode(descriptor.granularMode)
    {
        auto ioServiceFunction = [&]()
        {
            io_service::work work(mService);
            mService.run();
        };
        ioServiceThread.reset(new boost::thread(ioServiceFunction));

        for (const auto& interface : descriptor.interfaceWhiteList)
           mInterfaceWhiteList.emplace_back(ip::address_v6::from_string(interface));
    }

UDPv6TransportDescriptor::UDPv6TransportDescriptor():
    sendBufferSize(maximumUDPSocketSize),
    receiveBufferSize(maximumUDPSocketSize),
    granularMode(false)
    {}

UDPv6Transport::~UDPv6Transport()
{
    mService.stop();
    ioServiceThread->join();
}

bool UDPv6Transport::IsInputChannelOpen(const Locator_t& locator) const
{
    boost::unique_lock<boost::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.port) != mInputSockets.end());
}

bool UDPv6Transport::IsOutputChannelOpen(const Locator_t& locator) const
{
    boost::unique_lock<boost::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    if (mGranularMode)
        return mGranularOutputSockets.find(locator) != mGranularOutputSockets.end();
    else 
        return mOutputSockets.find(locator.port) != mOutputSockets.end();
}

bool UDPv6Transport::OpenOutputChannel(const Locator_t& locator)
{
    if (IsOutputChannelOpen(locator) ||
            !IsLocatorSupported(locator))
        return false;   

    if (mGranularMode)   
        return OpenAndBindGranularOutputSocket(locator);
    else
        return OpenAndBindOutputSockets(locator.port);
}

static bool IsMulticastAddress(const Locator_t& locator)
{
    return locator.address[0] == 0xFF;
}

bool UDPv6Transport::OpenInputChannel(const Locator_t& locator)
{
    if (!IsLocatorSupported(locator))
        return false;   

    bool success = false;

    if (!IsInputChannelOpen(locator))
        success = OpenAndBindInputSockets(locator.port);

    if (IsMulticastAddress(locator))
    {
        // The multicast group will be joined silently, because we do not
        // want to return another resource.
        auto& socket = mInputSockets.at(locator.port);
        socket.set_option(ip::multicast::join_group(ip::address_v6::from_string(locator.to_IP6_string())));
    }

    return success;
}

bool UDPv6Transport::CloseOutputChannel(const Locator_t& locator)
{
    if (!IsOutputChannelOpen(locator))
        return false;   

    boost::unique_lock<boost::recursive_mutex> scopedLock(mOutputMapMutex);
    if (mGranularMode)
    {
        auto& socket = mGranularOutputSockets.at(locator);
        socket.cancel();
        socket.close();
        mGranularOutputSockets.erase(locator);
    }
    else
    {
        auto& sockets = mOutputSockets.at(locator.port);
        for (auto& socket : sockets)
        {
            socket.cancel();
            socket.close();
        }

        mOutputSockets.erase(locator.port);
    }

    return true;
}

bool UDPv6Transport::CloseInputChannel(const Locator_t& locator)
{
    if (!IsInputChannelOpen(locator))
        return false;   

    boost::unique_lock<boost::recursive_mutex> scopedLock(mInputMapMutex);

    auto& socket = mInputSockets.at(locator.port);
    socket.cancel();
    socket.close();

    mInputSockets.erase(locator.port);
    return true;
}

static void GetIP6s(vector<IPFinder::info_IP>& locNames)
{
    IPFinder::getIPs(&locNames);
    // Controller out IP4
    auto newEnd = remove_if(locNames.begin(), 
            locNames.end(),
            [](IPFinder::info_IP ip){return ip.type != IPFinder::IP6;});
    locNames.erase(newEnd, locNames.end());
}

bool UDPv6Transport::IsInterfaceAllowed(const ip::address_v6& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v6::any())
        return true;

    return  find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}


bool UDPv6Transport::OpenAndBindOutputSockets(uint32_t port)
{
    const char* const METHOD_NAME = "OpenAndBindOutputSockets";

    boost::unique_lock<boost::recursive_mutex> scopedLock(mOutputMapMutex);

    try 
    {
        // If there is no whitelist, we can simply open a generic output socket
        // and gain efficiency.
        if (mInterfaceWhiteList.empty())
            mOutputSockets[port].push_back(OpenAndBindUnicastOutputSocket(ip::address_v6::any(), port));
        else
        {
            std::vector<IPFinder::info_IP> locNames;
            GetIP6s(locNames);
            for (const auto& infoIP : locNames)
            {
                auto ip = boost::asio::ip::address_v6::from_string(infoIP.name);
                if (IsInterfaceAllowed(ip))
                    mOutputSockets[port].push_back(OpenAndBindUnicastOutputSocket(ip, port));
            }
        }

    }
    catch (boost::system::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << port << ")" << " with boost msg: "<<e.what() , C_YELLOW);
        mOutputSockets.erase(port);
        return false;
    }

    return true;
}

bool UDPv6Transport::OpenAndBindGranularOutputSocket(const Locator_t& locator)
{
    const char* const METHOD_NAME = "OpenAndBindGranularOutputSocket";
    auto ip = boost::asio::ip::address_v6::from_string(locator.to_IP6_string());
    if (!IsInterfaceAllowed(ip))
        return false;

    boost::unique_lock<boost::recursive_mutex> scopedLock(mOutputMapMutex);

    try 
    {
        mGranularOutputSockets.insert(std::pair<Locator_t, boost::asio::ip::udp::socket>(locator, 
                    OpenAndBindUnicastOutputSocket(boost::asio::ip::address_v6::from_string(locator.to_IP6_string()), locator.port)));
    }
    catch (boost::system::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << locator.port << ")" << " with boost msg: "<<e.what() , C_YELLOW);
        mGranularOutputSockets.erase(locator);
        return false;
    }

    return true;
}

bool UDPv6Transport::OpenAndBindInputSockets(uint32_t port)
{
    const char* const METHOD_NAME = "OpenAndBindInputSockets";

    boost::unique_lock<boost::recursive_mutex> scopedLock(mInputMapMutex);

    try 
    {
        mInputSockets.emplace(port, OpenAndBindInputSocket(port));
    }
    catch (boost::system::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << port << ")" << " with boost msg: "<<e.what() , C_YELLOW);
        mInputSockets.erase(port);
        return false;
    }

    return true;
}

boost::asio::ip::udp::socket UDPv6Transport::OpenAndBindUnicastOutputSocket(const ip::address_v6& ipAddress, uint32_t port)
{
    ip::udp::socket socket(mService);
    socket.open(ip::udp::v6());
    socket.set_option(socket_base::send_buffer_size(mSendBufferSize));

    ip::udp::endpoint endpoint(ipAddress, static_cast<uint16_t>(port));
    socket.bind(endpoint);

    return socket;
}

boost::asio::ip::udp::socket UDPv6Transport::OpenAndBindInputSocket(uint32_t port)
{
    ip::udp::socket socket(mService);
    socket.open(ip::udp::v6());
    socket.set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    socket.set_option(ip::udp::socket::reuse_address( true ) );
    socket.set_option(ip::multicast::enable_loopback( true ) );
    auto anyIP = ip::address_v6::any();

    ip::udp::endpoint endpoint(anyIP, static_cast<uint16_t>(port));
    socket.bind(endpoint);

    return socket;
}

bool UDPv6Transport::DoLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    if (mGranularMode)
        return left == right;
    else
        return left.port == right.port;
}

bool UDPv6Transport::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == LOCATOR_KIND_UDPv6;
}

Locator_t UDPv6Transport::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    // All remotes are equally mapped to from the local [0:0:0:0:0:0:0:0]:port (main output channel).
    Locator_t mainLocal(remote);
    memset(mainLocal.address, 0x00, sizeof(mainLocal.address));
    return remote;
}

bool UDPv6Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
    if (!IsOutputChannelOpen(localLocator) ||
            sendBufferSize > mSendBufferSize)
        return false;

    boost::unique_lock<boost::recursive_mutex> scopedLock(mOutputMapMutex);
    bool success = false;

    if (mGranularMode)
    {
        auto& socket = mGranularOutputSockets.at(localLocator);
        success |= SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, socket);
    }
    else
    {
        auto& sockets = mOutputSockets.at(localLocator.port);
        for (auto& socket : sockets)
            success |= SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, socket);
    }

    return success;
}

static Locator_t EndpointToLocator(ip::udp::endpoint& endpoint)
{
    Locator_t locator;

    locator.port = endpoint.port();
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    memcpy(&locator.address[0], ipBytes.data(), sizeof(ipBytes));

    return locator;
}

bool UDPv6Transport::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
        const Locator_t& localLocator, Locator_t& remoteLocator)
{
    const char* const METHOD_NAME = "Receive";

    if (!IsInputChannelOpen(localLocator) ||
            receiveBufferCapacity < mReceiveBufferSize)
        return false;

    interprocess_semaphore receiveSemaphore(0);
    bool success = false;

    auto handler = [&receiveBuffer, &receiveBufferSize,  &success, METHOD_NAME, &receiveSemaphore](const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        if(error != boost::system::errc::success)
        {
            logInfo(RTPS_MSG_IN, "Error while listening to socket...",C_BLUE);
            receiveBufferSize = 0;
        }
        else 
        {
            logInfo(RTPS_MSG_IN,"Msg processed (" << bytes_transferred << " bytes received), Socket async receive put again to listen ",C_BLUE);
            receiveBufferSize = static_cast<uint32_t>(bytes_transferred);
            success = true;
        }

        receiveSemaphore.post();
    };

    ip::udp::endpoint senderEndpoint;

    { // lock scope
        boost::unique_lock<boost::recursive_mutex> scopedLock(mInputMapMutex);

        auto& socket = mInputSockets.at(localLocator.port);
        socket.async_receive_from(boost::asio::buffer(receiveBuffer, receiveBufferCapacity),
                senderEndpoint,
                handler);
    }

    receiveSemaphore.wait();
    if (success)
        remoteLocator = EndpointToLocator(senderEndpoint);

    return success;
}

bool UDPv6Transport::SendThroughSocket(const octet* sendBuffer,
        uint32_t sendBufferSize,
        const Locator_t& remoteLocator,
        boost::asio::ip::udp::socket& socket)
{
    const char* const METHOD_NAME = "SendThroughSocket";

    boost::asio::ip::address_v6::bytes_type remoteAddress;
    memcpy(&remoteAddress, &remoteLocator.address[0], sizeof(remoteAddress));
    auto destinationEndpoint = ip::udp::endpoint(boost::asio::ip::address_v6(remoteAddress), static_cast<uint16_t>(remoteLocator.port));
    size_t bytesSent = 0;
    logInfo(RTPS_MSG_OUT,"UDPv6: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << socket.local_endpoint(), C_YELLOW);

    try 
    {
        bytesSent = socket.send_to(boost::asio::buffer(sendBuffer, sendBufferSize), destinationEndpoint);
    }
    catch (const std::exception& error) 
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what(), C_YELLOW);
        return false;
    }

    (void) bytesSent;
    logInfo (RTPS_MSG_OUT,"SENT " << bytesSent,C_YELLOW);
    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
