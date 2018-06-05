// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file RTCPMessageManager.cpp
 *
 */


#include <fastrtps/transport/tcp/RTCPHeader.h>
#include <fastrtps/transport/tcp/RTCPMessageManager.h>
#include <fastrtps/transport/SocketInfo.h>
#include <fastrtps/log/Log.h>

#define IDSTRING "(ID:" << std::this_thread::get_id() <<") "<<

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static void EndpointToLocator(const asio::ip::tcp::endpoint& endpoint, Locator_t& locator)
{
    if (endpoint.protocol() == asio::ip::tcp::v4())
    {
        locator.kind = LOCATOR_KIND_TCPv4;
        auto ipBytes = endpoint.address().to_v4().to_bytes();
        locator.set_IP4_address(ipBytes.data());
    }
    else if (endpoint.protocol() == asio::ip::tcp::v6())
    {
        locator.kind = LOCATOR_KIND_TCPv6;
        auto ipBytes = endpoint.address().to_v6().to_bytes();
        locator.set_IP6_address(ipBytes.data());
    }
    locator.set_port(endpoint.port());
}

static void readSerializedPayload(SerializedPayload_t &payload, const octet* data, size_t size)
{
    payload.reserve(static_cast<uint32_t>(size));
    memcpy(&payload.encapsulation, data, 2);
    memcpy(&payload.length, &data[2], 4);
    memcpy(payload.data, &data[6], size);
    payload.pos = 0;
}

RTCPMessageManager::~RTCPMessageManager()
{
}

size_t RTCPMessageManager::sendMessage(TCPSocketInfo *pSocketInfo, const CDRMessage_t &msg) const
{
    size_t send = transport->Send(pSocketInfo, msg.buffer, msg.length);
    if (send != msg.length)
    {
        logWarning(RTCP, "Bad sent size..." << send << " bytes of " << msg.length << " bytes.");
    }
    //logInfo(RTCP, "Sent " << send << " bytes");
    return send;
}

bool RTCPMessageManager::sendData(TCPSocketInfo *pSocketInfo, TCPCPMKind kind, const TCPTransactionId &transactionId,
    const SerializedPayload_t *payload, const ResponseCode respCode)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;
    CDRMessage_t msg;
    CDRMessage::initCDRMsg(&msg);
    const ResponseCode* code = (respCode != RETCODE_VOID) ? &respCode : nullptr;

    fillHeaders(kind, transactionId, ctrlHeader, header, payload, code);

    RTPSMessageCreator::addCustomContent(&msg, (octet*)&header, TCPHeader::getSize());
    RTPSMessageCreator::addCustomContent(&msg, (octet*)&ctrlHeader, TCPControlMsgHeader::getSize());
    if (code != nullptr)
    {
        RTPSMessageCreator::addCustomContent(&msg, (octet*)code, 4); // uint32_t
    }
    if (payload != nullptr)
    {
        RTPSMessageCreator::addCustomContent(&msg, (octet*)&payload->encapsulation, 2); // Encapsulation
        RTPSMessageCreator::addCustomContent(&msg, (octet*)&payload->length, 4); // Length
        RTPSMessageCreator::addCustomContent(&msg, payload->data, payload->length); // Data
    }

    return sendMessage(pSocketInfo, msg) > 0;
}

uint32_t& RTCPMessageManager::addToCRC(uint32_t &crc, octet data)
{
    static uint32_t max = 0xffffffff;
    if (crc + data < crc)
    {
        crc -= (max - data);
    }
    else
    {
        crc += data;
    }
    return crc;
}

void RTCPMessageManager::fillHeaders(TCPCPMKind kind, const TCPTransactionId &transactionId,
    TCPControlMsgHeader &retCtrlHeader, TCPHeader &header, const SerializedPayload_t *payload,
    const ResponseCode *respCode)
{
    retCtrlHeader.kind = kind;
    retCtrlHeader.length = static_cast<uint16_t>(TCPControlMsgHeader::getSize());
    retCtrlHeader.length += static_cast<uint16_t>((payload != nullptr) ? (payload->length + 6) : 0);
    retCtrlHeader.length += static_cast<uint16_t>((respCode != nullptr) ? 4 : 0);
    retCtrlHeader.transactionId = transactionId;

    switch (kind)
    {
    case BIND_CONNECTION_REQUEST:
    case OPEN_LOGICAL_PORT_REQUEST:
    case CHECK_LOGICAL_PORT_REQUEST:
    case KEEP_ALIVE_REQUEST:
        retCtrlHeader.setFlags(false, true, true);
        mUnconfirmedTransactions.emplace(retCtrlHeader.transactionId);
        break;
    case LOGICAL_PORT_IS_CLOSED_REQUEST:
    case BIND_CONNECTION_RESPONSE:
    case OPEN_LOGICAL_PORT_RESPONSE:
    case CHECK_LOGICAL_PORT_RESPONSE:
    case KEEP_ALIVE_RESPONSE:
        retCtrlHeader.setFlags(false, true, false);
        break;
    case UNBIND_CONNECTION_REQUEST:
        retCtrlHeader.setFlags(false, false, false);
        break;
    }

    retCtrlHeader.setEndianess(DEFAULT_ENDIAN); // Override "false" endianess set on the switch
    header.logicalPort = 0; // This is a control message
    header.length = static_cast<uint32_t>(retCtrlHeader.length + TCPHeader::getSize());

    // Finally, calculate the CRC
    octet* it = (octet*)&retCtrlHeader;
    uint32_t crc = 0;
    for (size_t i = 0; i < TCPControlMsgHeader::getSize(); ++i)
    {
        crc = addToCRC(crc, it[i]);
    }
    if (respCode != nullptr)
    {
        it = (octet*)respCode;
        for (int i = 0; i < 4; ++i)
        {
            crc = addToCRC(crc, it[i]);
        }
    }
    if (payload != nullptr)
    {
        octet* pay = (octet*)&(payload->encapsulation);
        for (uint32_t i = 0; i < 2; ++i)
        {
            crc = addToCRC(crc, pay[i]);
        }
        pay = (octet*)&(payload->length);
        for (uint32_t i = 0; i < 4; ++i)
        {
            crc = addToCRC(crc, pay[i]);
        }
        for (uint32_t i = 0; i < payload->length; ++i)
        {
            crc = addToCRC(crc, payload->data[i]);
        }
    }
    header.crc = crc;
    //logInfo(RTCP, "Send (CRC= " << header.crc << ")");

    // LOG
    switch (kind)
    {
    case BIND_CONNECTION_REQUEST:
        logInfo(RTCP_SEQ, "Send [BIND_CONNECTION_REQUEST] Seq: " << retCtrlHeader.transactionId);
        break;
    case OPEN_LOGICAL_PORT_REQUEST:
        logInfo(RTCP_SEQ, "Send [OPEN_LOGICAL_PORT_REQUEST] Seq: " << retCtrlHeader.transactionId);
        break;
    case CHECK_LOGICAL_PORT_REQUEST:
        logInfo(RTCP_SEQ, "Send [CHECK_LOGICAL_PORT_REQUEST]: Seq: " << retCtrlHeader.transactionId);
        break;
    case KEEP_ALIVE_REQUEST:
        logInfo(RTCP_SEQ, "Send [KEEP_ALIVE_REQUEST] Seq: " << retCtrlHeader.transactionId);
        break;
    case LOGICAL_PORT_IS_CLOSED_REQUEST:
        logInfo(RTCP_SEQ, "Send [LOGICAL_PORT_IS_CLOSED_REQUEST] Seq: " << retCtrlHeader.transactionId);
        break;
    case BIND_CONNECTION_RESPONSE:
        logInfo(RTCP_SEQ, "Send [BIND_CONNECTION_RESPONSE] Seq: " << retCtrlHeader.transactionId);
        break;
    case OPEN_LOGICAL_PORT_RESPONSE:
        logInfo(RTCP_SEQ, "Send [OPEN_LOGICAL_PORT_RESPONSE] Seq: " << retCtrlHeader.transactionId);
        break;
    case CHECK_LOGICAL_PORT_RESPONSE:
        logInfo(RTCP_SEQ, "Send [CHECK_LOGICAL_PORT_RESPONSE] Seq: " << retCtrlHeader.transactionId);
        break;
    case KEEP_ALIVE_RESPONSE:
        logInfo(RTCP_SEQ, "Send [KEEP_ALIVE_RESPONSE] Seq: " << retCtrlHeader.transactionId);
        break;
    case UNBIND_CONNECTION_REQUEST:
        logInfo(RTCP_SEQ, "Send [UNBIND_CONNECTION_REQUEST] Seq: " << retCtrlHeader.transactionId);
        break;
    }
}

void RTCPMessageManager::sendConnectionRequest(TCPSocketInfo *pSocketInfo, uint16_t localLogicalPort)
{
    ConnectionRequest_t request;
    Locator_t locator;
    EndpointToLocator(pSocketInfo->getSocket()->local_endpoint(), locator);
    locator.set_logical_port(localLogicalPort);
    locator.set_IP4_WAN_address(transport->mConfiguration_.wan_addr[0], transport->mConfiguration_.wan_addr[1],
        transport->mConfiguration_.wan_addr[2], transport->mConfiguration_.wan_addr[3]);
    request.transportLocator(locator);

    SerializedPayload_t payload(static_cast<uint32_t>(ConnectionRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);

    logInfo(RTCP_MSG, "Send [BIND_CONNECTION_REQUEST] PhysicalPort: " << locator.get_physical_port()
        << ", LogicalPort: " << localLogicalPort);
    sendData(pSocketInfo, BIND_CONNECTION_REQUEST, getTransactionId(), &payload);
    pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eWaitingForBindResponse);
}

void RTCPMessageManager::sendOpenLogicalPortRequest(TCPSocketInfo *pSocketInfo, uint16_t port)
{
    OpenLogicalPortRequest_t request;
    request.logicalPort(port);
    sendOpenLogicalPortRequest(pSocketInfo, request);
}

void RTCPMessageManager::sendOpenLogicalPortRequest(TCPSocketInfo *pSocketInfo, OpenLogicalPortRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(OpenLogicalPortRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_REQUEST] LogicalPort: " << request.logicalPort());
    sendData(pSocketInfo, OPEN_LOGICAL_PORT_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendCheckLogicalPortsRequest(TCPSocketInfo *pSocketInfo, std::vector<uint16_t> &ports)
{
    CheckLogicalPortsRequest_t request;
    request.logicalPortsRange(ports);
    sendCheckLogicalPortsRequest(pSocketInfo, request);
}

void RTCPMessageManager::sendCheckLogicalPortsRequest(TCPSocketInfo *pSocketInfo, CheckLogicalPortsRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(CheckLogicalPortsRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [CHECK_LOGICAL_PORT_REQUEST]");
    sendData(pSocketInfo, CHECK_LOGICAL_PORT_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendKeepAliveRequest(TCPSocketInfo *pSocketInfo, KeepAliveRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(KeepAliveRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [KEEP_ALIVE_REQUEST]");
    sendData(pSocketInfo, KEEP_ALIVE_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendKeepAliveRequest(TCPSocketInfo *pSocketInfo)
{
    KeepAliveRequest_t request;
    request.locator(pSocketInfo->GetLocator());
    sendKeepAliveRequest(pSocketInfo, request);
}

void RTCPMessageManager::sendLogicalPortIsClosedRequest(TCPSocketInfo *pSocketInfo,
    LogicalPortIsClosedRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(
        LogicalPortIsClosedRequest_t::getBufferCdrSerializedSize(request)));

    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [LOGICAL_PORT_IS_CLOSED_REQUEST] LogicalPort: " << request.logicalPort());
    sendData(pSocketInfo, LOGICAL_PORT_IS_CLOSED_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendLogicalPortIsClosedRequest(TCPSocketInfo *pSocketInfo, uint16_t port)
{
    LogicalPortIsClosedRequest_t request;
    request.logicalPort(port);
    sendLogicalPortIsClosedRequest(pSocketInfo, request);
}

void RTCPMessageManager::sendUnbindConnectionRequest(TCPSocketInfo *pSocketInfo)
{
    logInfo(RTCP_MSG, "Send [UNBIND_CONNECTION_REQUEST]");
    sendData(pSocketInfo, UNBIND_CONNECTION_REQUEST, getTransactionId());
}

bool RTCPMessageManager::processBindConnectionRequest(TCPSocketInfo *pSocketInfo, const ConnectionRequest_t &request,
    const TCPTransactionId &transactionId, Locator_t &localLocator)
{
    BindConnectionResponse_t response;
    localLocator.set_logical_port(transport->mConfiguration_.metadata_logical_port);
    localLocator.set_IP4_WAN_address(transport->mConfiguration_.wan_addr[0],
        transport->mConfiguration_.wan_addr[1],
        transport->mConfiguration_.wan_addr[2],
        transport->mConfiguration_.wan_addr[3]);

    response.locator(localLocator);

    SerializedPayload_t payload(static_cast<uint32_t>(BindConnectionResponse_t::getBufferCdrSerializedSize(response)));
    response.serialize(&payload);

    if (request.protocolVersion() != c_ProtocolVersion)
    {
        sendData(pSocketInfo, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_INCOMPATIBLE_VERSION);
        return false;
    }
    else if (pSocketInfo->mConnectionStatus == TCPSocketInfo::eConnectionStatus::eWaitingForBind)
    {
        {
            std::unique_lock<std::recursive_mutex> scope(pSocketInfo->mPendingLogicalMutex);
            pSocketInfo->mPendingLogicalOutputPorts.push_back(request.transportLocator().get_logical_port());
            transport->BindInputSocket(request.transportLocator(), pSocketInfo);
        }
        sendData(pSocketInfo, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_OK);
        pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eEstablished);
    }
    else if (pSocketInfo->mConnectionStatus == TCPSocketInfo::eConnectionStatus::eEstablished)
    {
        sendData(pSocketInfo, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_EXISTING_CONNECTION);
    }
    else
    {
        sendData(pSocketInfo, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_SERVER_ERROR);
    }
    return true;
}

bool RTCPMessageManager::processOpenLogicalPortRequest(TCPSocketInfo *pSocketInfo,
    const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId)
{
    if (pSocketInfo->mConnectionStatus != TCPSocketInfo::eConnectionStatus::eEstablished)
    {
        sendData(pSocketInfo, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (std::find(pSocketInfo->mOpenedPorts.begin(), pSocketInfo->mOpenedPorts.end(),
        request.logicalPort()) != pSocketInfo->mOpenedPorts.end())
    {
        //logInfo(RTCP, "OpenLogicalPortRequest [FAILED]: " << request.logicalPort());
        sendData(pSocketInfo, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_INVALID_PORT);
    }
    else
    {
        pSocketInfo->mOpenedPorts.emplace_back(request.logicalPort());
        sendData(pSocketInfo, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_OK);
    }
    return true;
}

void RTCPMessageManager::processCheckLogicalPortsRequest(TCPSocketInfo *pSocketInfo,
    const CheckLogicalPortsRequest_t &request, const TCPTransactionId &transactionId)
{
    CheckLogicalPortsResponse_t response;
    if (pSocketInfo->mConnectionStatus != TCPSocketInfo::eConnectionStatus::eEstablished)
    {
        sendData(pSocketInfo, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else
    {
        for (uint16_t port : request.logicalPortsRange())
        {
            if (std::find(pSocketInfo->mOpenedPorts.begin(), pSocketInfo->mOpenedPorts.end(), port) ==
                pSocketInfo->mOpenedPorts.end())
            {
                logInfo(RTCP, "FoundOpenedLogicalPort: " << port);
                response.availableLogicalPorts().emplace_back(port);
            }
        }

        SerializedPayload_t payload(static_cast<uint32_t>(
            CheckLogicalPortsResponse_t::getBufferCdrSerializedSize(response)));
        response.serialize(&payload);
        sendData(pSocketInfo, CHECK_LOGICAL_PORT_RESPONSE, transactionId, &payload, RETCODE_OK);
    }
}

bool RTCPMessageManager::processKeepAliveRequest(TCPSocketInfo *pSocketInfo, const KeepAliveRequest_t &request,
    const TCPTransactionId &transactionId)
{
    if (pSocketInfo->mConnectionStatus != TCPSocketInfo::eConnectionStatus::eEstablished)
    {
        sendData(pSocketInfo, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (pSocketInfo->GetLocator().get_logical_port() == request.locator().get_logical_port())
    {
        sendData(pSocketInfo, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_OK);
    }
    else
    {
        sendData(pSocketInfo, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_UNKNOWN_LOCATOR);
        return false;
    }
    return true;
}

void RTCPMessageManager::processLogicalPortIsClosedRequest(TCPSocketInfo* pSocketInfo,
    const LogicalPortIsClosedRequest_t &/*request*/, const TCPTransactionId & transactionId)
{
    if (pSocketInfo->mConnectionStatus != TCPSocketInfo::eConnectionStatus::eEstablished)
    {
        sendData(pSocketInfo, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
}

bool RTCPMessageManager::processBindConnectionResponse(TCPSocketInfo *pSocketInfo,
    const BindConnectionResponse_t &/*response*/, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        logInfo(RTCP, "Connection established (Resp) (physical: " << pSocketInfo->mLocator.get_physical_port() << ")");
        pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eEstablished);
        mUnconfirmedTransactions.erase(it);
        return true;
    }
    else
    {
        logWarning(RTPS_MSG_IN, "Received BindConnectionResponse with an invalid transactionId: " << transactionId);
        return false;
    }
}

bool RTCPMessageManager::processCheckLogicalPortsResponse(TCPSocketInfo *pSocketInfo,
    const CheckLogicalPortsResponse_t &response, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        if (response.availableLogicalPorts().empty())
        {
            pSocketInfo->mCheckingLogicalPort += (transport->mConfiguration_.logical_port_range
                * transport->mConfiguration_.logical_port_increment);
            prepareAndSendCheckLogicalPortsRequest(pSocketInfo);
        }
        else
        {
            pSocketInfo->mCheckingLogicalPort = response.availableLogicalPorts()[0];
            pSocketInfo->mPendingLogicalOutputPorts.emplace_back(pSocketInfo->mCheckingLogicalPort);
            //logInfo(RTCP, "NegotiatingLogicalPort: " << pSocketInfo->mCheckingLogicalPort);
            if (pSocketInfo->mNegotiatingLogicalPort == 0)
            {
                logWarning(RTPS_MSG_IN, "Negotiated new logical port wihtout initial port?");
            }
        }

        mUnconfirmedTransactions.erase(it);
        return true;
    }
    else
    {
        logWarning(RTPS_MSG_IN, "Received CheckLogicalPortsResponse with an invalid transactionId: " << transactionId);
        return false;
    }
}

void RTCPMessageManager::prepareAndSendCheckLogicalPortsRequest(TCPSocketInfo *pSocketInfo)
{
    // Dont try again this port
    {
        std::unique_lock<std::recursive_mutex> scopedLock(pSocketInfo->mPendingLogicalMutex);
        if (!pSocketInfo->mPendingLogicalOutputPorts.empty())
        {
            pSocketInfo->mPendingLogicalOutputPorts.erase(pSocketInfo->mPendingLogicalOutputPorts.begin());
        }

        if (pSocketInfo->mNegotiatingLogicalPort == 0) // Keep original logical port being negotiated
        {
            pSocketInfo->mNegotiatingLogicalPort = pSocketInfo->mPendingLogicalPort;
            pSocketInfo->mCheckingLogicalPort = pSocketInfo->mPendingLogicalPort;
            logInfo(RTCP, "OpenLogicalPort failed: " << pSocketInfo->mCheckingLogicalPort);
        }
        pSocketInfo->mPendingLogicalPort = 0;
    }

    std::vector<uint16_t> ports;
    for (uint16_t p = pSocketInfo->mCheckingLogicalPort + transport->mConfiguration_.logical_port_increment;
        p <= pSocketInfo->mCheckingLogicalPort +
        (transport->mConfiguration_.logical_port_range
            * transport->mConfiguration_.logical_port_increment);
        p += transport->mConfiguration_.logical_port_increment)
    {
        if (p <= pSocketInfo->mNegotiatingLogicalPort + transport->mConfiguration_.max_logical_port)
        {
            ports.emplace_back(p);
        }
    }

    if (ports.empty()) // No more available ports!
    {
        logError(RTPS_MSG_IN, "Cannot find an available logical port.");
    }
    else
    {
        sendCheckLogicalPortsRequest(pSocketInfo, ports);
    }
}

bool RTCPMessageManager::processOpenLogicalPortResponse(TCPSocketInfo *pSocketInfo, ResponseCode respCode,
    const TCPTransactionId &transactionId, Locator_t &remoteLocator)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        switch (respCode)
        {
        case RETCODE_OK:
        {
            std::unique_lock<std::recursive_mutex> scopedLock(pSocketInfo->mPendingLogicalMutex);
            if (pSocketInfo->mNegotiatingLogicalPort != 0
                && pSocketInfo->mPendingLogicalPort == pSocketInfo->mCheckingLogicalPort)
            {
                // Add route
                pSocketInfo->mLogicalPortRouting[pSocketInfo->mNegotiatingLogicalPort]
                    = pSocketInfo->mPendingLogicalPort;

                //logInfo(RTCP, "OpenedAndRoutedLogicalPort " << pSocketInfo->mNegotiatingLogicalPort
                // << "->" << pSocketInfo->mPendingLogicalPort);

                // We want the reference to the negotiated port, not the real logical one
                remoteLocator.set_logical_port(pSocketInfo->mNegotiatingLogicalPort);

                // Both, real one and negotiated must be added
                pSocketInfo->mLogicalOutputPorts.emplace_back(pSocketInfo->mNegotiatingLogicalPort);

                pSocketInfo->mNegotiatingLogicalPort = 0;
                pSocketInfo->mCheckingLogicalPort = 0;
            }
            else
            {
                remoteLocator.set_logical_port(pSocketInfo->mPendingLogicalPort);
                logInfo(RTCP, "OpenedLogicalPort " << pSocketInfo->mPendingLogicalPort);
            }

            pSocketInfo->mLogicalOutputPorts.emplace_back(*(pSocketInfo->mPendingLogicalOutputPorts.begin()));
            pSocketInfo->mPendingLogicalOutputPorts.erase(pSocketInfo->mPendingLogicalOutputPorts.begin());
            pSocketInfo->mPendingLogicalPort = 0;
            transport->BindInputSocket(remoteLocator, pSocketInfo);
            /*
            if (transport->mBoundOutputSockets.find(remoteLocator) == transport->mBoundOutputSockets.end())
            {
                //std::cout << "################## MM" << std::endl;
                //std::cout << "LOCATOR KIND: " << remoteLocator.kind << std::endl;
                //std::cout << "LOCATOR Address: " << remoteLocator.to_IP4_string() << std::endl;
                //std::cout << "LOCATOR Physical: " << remoteLocator.get_physical_port() << std::endl;
                //std::cout << "LOCATOR Logical: " << remoteLocator.get_logical_port() << std::endl;
                //std::cout << "ENDPOINT Local: " << pSocketInfo->getSocket()->local_endpoint().port() << std::endl;
                //std::cout << "ENDPOINT Remote: " << pSocketInfo->getSocket()->remote_endpoint().port() << std::endl;
                transport->mBoundOutputSockets[remoteLocator] = pSocketInfo;
                //std::cout << transport->mBoundOutputSockets.size() << std::endl;
                //std::cout << "MM ##################" << std::endl;
            }
            */
        }
        break;
        case RETCODE_INVALID_PORT:
        {
            prepareAndSendCheckLogicalPortsRequest(pSocketInfo);
        }
        break;
        default:
            logWarning(RTPS_MSG_IN, "Received response for OpenLogicalPort with error code: "
                << ((respCode == RETCODE_BAD_REQUEST) ? "BAD_REQUEST" : "SERVER_ERROR"));
            break;
        }
        mUnconfirmedTransactions.erase(it);
    }
    else
    {
        logWarning(RTPS_MSG_IN, "Received OpenLogicalPortResponse with an invalid transactionId: " << transactionId);
    }
    return true;
}

bool RTCPMessageManager::processKeepAliveResponse(TCPSocketInfo *pSocketInfo,
    ResponseCode respCode, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        switch (respCode)
        {
        case RETCODE_OK:
            pSocketInfo->mWaitingForKeepAlive = false;
            break;
        case RETCODE_UNKNOWN_LOCATOR:
            return false;
        default:
            break;
        }
        mUnconfirmedTransactions.erase(it);
    }
    else
    {
        logWarning(RTPS_MSG_IN, "Received response for KeepAlive with an unexpected transactionId: " << transactionId);
    }
    return true;
}

bool RTCPMessageManager::processRTCPMessage(TCPSocketInfo *socketInfo, octet* receiveBuffer, size_t receivedSize)
{
    bool bProcessOk(true);

    TCPControlMsgHeader controlHeader;
    memcpy(&controlHeader, receiveBuffer, TCPControlMsgHeader::getSize());
    size_t dataSize = controlHeader.length - TCPControlMsgHeader::getSize();
    size_t bufferSize = dataSize + 4;

    // Message size checking.
    if (dataSize + TCPControlMsgHeader::getSize() != receivedSize)
    {
        sendData(socketInfo, controlHeader.kind, controlHeader.transactionId, nullptr, RETCODE_BAD_REQUEST);
        return true;
    }

    switch (controlHeader.kind)
    {
    case BIND_CONNECTION_REQUEST:
    {
        logInfo(RTCP_SEQ, "Receive [BIND_CONNECTION_REQUEST] Seq: " << controlHeader.transactionId);
        ConnectionRequest_t request;
        Locator_t myLocator;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        EndpointToLocator(socketInfo->getSocket()->local_endpoint(), myLocator);

        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);

        logInfo(RTCP_MSG, "Receive [BIND_CONNECTION_REQUEST] " <<
            "LogicalPort: " << request.transportLocator().get_logical_port()
            << ", Physical remote: " << request.transportLocator().get_physical_port());

        bProcessOk = processBindConnectionRequest(socketInfo, request, controlHeader.transactionId, myLocator);
    }
    break;
    case BIND_CONNECTION_RESPONSE:
    {
        logInfo(RTCP_SEQ, "Receive [BIND_CONNECTION_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        BindConnectionResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(dataSize - 4));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize - 4);
        response.deserialize(&payload);

        logInfo(RTCP_MSG, "Receive [BIND_CONNECTION_RESPONSE] LogicalPort: " << response.locator().get_logical_port()
            << ", Physical remote: " << response.locator().get_physical_port());

        if (respCode == RETCODE_OK || respCode == RETCODE_EXISTING_CONNECTION)
        {
            std::unique_lock<std::recursive_mutex> scopedLock(socketInfo->mPendingLogicalMutex);
            if (!socketInfo->mPendingLogicalOutputPorts.empty())
            {
                processBindConnectionResponse(socketInfo, response, controlHeader.transactionId);
            }
        }
        else
        {
            // If the bind message fails, close the connection and try again.
            bProcessOk = false;
        }
    }
    break;
    case OPEN_LOGICAL_PORT_REQUEST:
    {
        logInfo(RTCP_SEQ, "Receive [OPEN_LOGICAL_PORT_REQUEST] Seq: " << controlHeader.transactionId);
        OpenLogicalPortRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [OPEN_LOGICAL_PORT_REQUEST] LogicalPort: " << request.logicalPort());
        bProcessOk = processOpenLogicalPortRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case CHECK_LOGICAL_PORT_REQUEST:
    {
        logInfo(RTCP_SEQ, "Receive [CHECK_LOGICAL_PORT_REQUEST] Seq: " << controlHeader.transactionId);
        CheckLogicalPortsRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [CHECK_LOGICAL_PORT_REQUEST]");
        processCheckLogicalPortsRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case CHECK_LOGICAL_PORT_RESPONSE:
    {
        logInfo(RTCP_SEQ, "Receive [CHECK_LOGICAL_PORT_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        CheckLogicalPortsResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(dataSize - 4));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize - 4);
        response.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [CHECK_LOGICAL_PORT_RESPONSE]");
        processCheckLogicalPortsResponse(socketInfo, response, controlHeader.transactionId);
    }
    break;
    case KEEP_ALIVE_REQUEST:
    {
        logInfo(RTCP_SEQ, "Receive [KEEP_ALIVE_REQUEST] Seq: " << controlHeader.transactionId);
        KeepAliveRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [KEEP_ALIVE_REQUEST]");
        bProcessOk = processKeepAliveRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case LOGICAL_PORT_IS_CLOSED_REQUEST:
    {
        logInfo(RTCP_SEQ, "Receive [LOGICAL_PORT_IS_CLOSED_REQUEST] Seq: " << controlHeader.transactionId);
        LogicalPortIsClosedRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [LOGICAL_PORT_IS_CLOSED_REQUEST] LogicalPort: " << request.logicalPort());
        processLogicalPortIsClosedRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case UNBIND_CONNECTION_REQUEST:
    {
        logInfo(RTCP_SEQ, "Receive [UNBIND_CONNECTION_REQUEST] Seq:" << controlHeader.transactionId);
        logInfo(RTCP_MSG, "Receive [UNBIND_CONNECTION_REQUEST]");
        bProcessOk = false;
    }
    break;
    case OPEN_LOGICAL_PORT_RESPONSE:
    {
        logInfo(RTCP_SEQ, "Receive [OPEN_LOGICAL_PORT_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4);
        Locator_t remoteLocator;
        EndpointToLocator(socketInfo->getSocket()->remote_endpoint(), remoteLocator);
        logInfo(RTCP_MSG, "Receive [OPEN_LOGICAL_PORT_RESPONSE]");
        processOpenLogicalPortResponse(socketInfo, respCode, controlHeader.transactionId, remoteLocator);
    }
    break;
    case KEEP_ALIVE_RESPONSE:
    {
        logInfo(RTCP_SEQ, "Receive [KEEP_ALIVE_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4);
        logInfo(RTCP_MSG, "Receive [KEEP_ALIVE_RESPONSE]");
        bProcessOk = processKeepAliveResponse(socketInfo, respCode, controlHeader.transactionId);
    }
    break;
    default:
        sendData(socketInfo, controlHeader.kind, controlHeader.transactionId, nullptr, RETCODE_BAD_REQUEST);
        break;
    }
    return bProcessOk;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
