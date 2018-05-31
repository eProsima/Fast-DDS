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
        std::cout << "Bad sent size..." << std::endl;
    }
    //std::cout << "[RTCP] Sent " << send << " bytes" << std::endl;

    return send;
}

bool RTCPMessageManager::sendData(TCPSocketInfo *pSocketInfo,
        TCPCPMKind kind, const TCPTransactionId &transactionId,
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

    size_t send = sendMessage(pSocketInfo, msg);
    return send > 0;
}

static uint32_t& addToCRC(uint32_t &crc, octet data)
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

bool RTCPMessageManager::CheckCRC(const TCPHeader &header, const octet *data, uint32_t size)
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = addToCRC(crc, data[i]);
    }
    return crc == header.crc;
}

void RTCPMessageManager::CalculateCRC(TCPHeader &header, const octet *data, uint32_t size)
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = addToCRC(crc, data[i]);
    }
    header.crc = crc;
}

void RTCPMessageManager::fillHeaders(TCPCPMKind kind, const TCPTransactionId &transactionId,
        TCPControlMsgHeader &retCtrlHeader,  TCPHeader &header, const SerializedPayload_t *payload,
        const ResponseCode *respCode)
{
    retCtrlHeader.kind = kind;
    retCtrlHeader.length = static_cast<uint16_t>(TCPControlMsgHeader::getSize());
    retCtrlHeader.length += static_cast<uint16_t>((payload != nullptr) ? (payload->length + 6) : 0);
    retCtrlHeader.length += static_cast<uint16_t>((respCode != nullptr) ? 4 : 0);
    retCtrlHeader.transactionId = transactionId;

    switch(kind)
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
    //std::cout << "[RTCP] Send (CRC= " << header.crc << ")" << std::endl;

    // LOG
    /*
    switch(kind)
    {
        case BIND_CONNECTION_REQUEST:
            std::cout << "[RTCP] Send [BIND_CONNECTION_REQUEST] " << retCtrlHeader.transactionId << std::endl;
            break;
        case OPEN_LOGICAL_PORT_REQUEST:
            std::cout << "[RTCP] Send [OPEN_LOGICAL_PORT_REQUEST] " << retCtrlHeader.transactionId << std::endl;
            break;
        case CHECK_LOGICAL_PORT_REQUEST:
            std::cout << "[RTCP] Send [CHECK_LOGICAL_PORT_REQUEST]: " << retCtrlHeader.length << " - " << retCtrlHeader.transactionId << std::endl;
            break;
        case KEEP_ALIVE_REQUEST:
            std::cout << "[RTCP] Send [KEEP_ALIVE_REQUEST] " << retCtrlHeader.transactionId << std::endl;
            break;
        case LOGICAL_PORT_IS_CLOSED_REQUEST:
            std::cout << "[RTCP] Send [LOGICAL_PORT_IS_CLOSED_REQUEST] " << retCtrlHeader.transactionId << std::endl;
            break;
        case BIND_CONNECTION_RESPONSE:
            std::cout << "[RTCP] Send [BIND_CONNECTION_RESPONSE] " << retCtrlHeader.transactionId << std::endl;
            break;
        case OPEN_LOGICAL_PORT_RESPONSE:
            std::cout << "[RTCP] Send [OPEN_LOGICAL_PORT_RESPONSE] " << retCtrlHeader.transactionId << std::endl;
            break;
        case CHECK_LOGICAL_PORT_RESPONSE:
            std::cout << "[RTCP] Send [CHECK_LOGICAL_PORT_RESPONSE] " << retCtrlHeader.transactionId << std::endl;
            break;
        case KEEP_ALIVE_RESPONSE:
            std::cout << "[RTCP] Send [KEEP_ALIVE_RESPONSE] " << retCtrlHeader.transactionId << std::endl;
            break;
        case UNBIND_CONNECTION_REQUEST:
            std::cout << "[RTCP] Send [UNBIND_CONNECTION_REQUEST] " << retCtrlHeader.transactionId << std::endl;
            break;
    }
    */
}

void RTCPMessageManager::sendConnectionRequest(TCPSocketInfo *pSocketInfo, uint16_t localLogicalPort)
{
    ConnectionRequest_t request;
    Locator_t locator;
    EndpointToLocator(pSocketInfo->getSocket()->local_endpoint(), locator);
    locator.set_logical_port(localLogicalPort);
    request.transportLocator(locator);

    SerializedPayload_t payload(static_cast<uint32_t>(ConnectionRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);

    sendData(pSocketInfo, BIND_CONNECTION_REQUEST, getTransactionId(), &payload);
/*
    sendData(pSocketInfo, BIND_CONNECTION_REQUEST, getTransactionId(), (octet*)&request,
        ConnectionRequest_t::getCdrSerializedSize(request));
*/
    pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eWaitingForBindResponse);
}

void RTCPMessageManager::sendOpenLogicalPortRequest(TCPSocketInfo *pSocketInfo, uint16_t port)
{
    OpenLogicalPortRequest_t request;
    request.logicalPort(port);
    sendOpenLogicalPortRequest(pSocketInfo, request);
}

void RTCPMessageManager::sendOpenLogicalPortRequest(TCPSocketInfo *pSocketInfo,
    OpenLogicalPortRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(OpenLogicalPortRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    sendData(pSocketInfo, OPEN_LOGICAL_PORT_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendCheckLogicalPortsRequest(TCPSocketInfo *pSocketInfo,
    std::vector<uint16_t> &ports)
{
    CheckLogicalPortsRequest_t request;
    request.logicalPortsRange(ports);
    sendCheckLogicalPortsRequest(pSocketInfo, request);
}

void RTCPMessageManager::sendCheckLogicalPortsRequest(TCPSocketInfo *pSocketInfo,
    CheckLogicalPortsRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(CheckLogicalPortsRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    sendData(pSocketInfo, CHECK_LOGICAL_PORT_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendKeepAliveRequest(TCPSocketInfo *pSocketInfo, KeepAliveRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(KeepAliveRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
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
    SerializedPayload_t payload(static_cast<uint32_t>(LogicalPortIsClosedRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    sendData(pSocketInfo, LOGICAL_PORT_IS_CLOSED_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendLogicalPortIsClosedRequest(TCPSocketInfo *pSocketInfo,
        uint16_t port)
{
    LogicalPortIsClosedRequest_t request;
    request.logicalPort(port);
    sendLogicalPortIsClosedRequest(pSocketInfo, request);
}

void RTCPMessageManager::sendUnbindConnectionRequest(TCPSocketInfo *pSocketInfo)
{
    sendData(pSocketInfo, UNBIND_CONNECTION_REQUEST, getTransactionId());
}

void RTCPMessageManager::processConnectionRequest(TCPSocketInfo *pSocketInfo,
        const ConnectionRequest_t &request, const TCPTransactionId &transactionId, Locator_t &localLocator)
{
    BindConnectionResponse_t response;
    localLocator.set_logical_port(request.transportLocator().get_logical_port());
    response.locator(localLocator);

    SerializedPayload_t payload(static_cast<uint32_t>(BindConnectionResponse_t::getBufferCdrSerializedSize(response)));
    response.serialize(&payload);

    // TODO More options!
    if (pSocketInfo->mConnectionStatus == TCPSocketInfo::eConnectionStatus::eWaitingForBind)
    {
        {
            std::unique_lock<std::recursive_mutex> scope(pSocketInfo->mPendingLogicalMutex);
            pSocketInfo->mPendingLogicalOutputPorts.push_back(request.transportLocator().get_logical_port());
            transport->BindInputSocket(localLocator, pSocketInfo);
            //std::cout << "## Bound as Input " << localLocator << std::endl;
        }
        sendData(pSocketInfo, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_OK);
        //std::cout << "[RTCP] Connection established (Req) (physical: " << pSocketInfo->mLocator.get_physical_port() << ")" << std::endl;
        pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eEstablished);
    }
    else
    {
        if (pSocketInfo->mConnectionStatus == TCPSocketInfo::eConnectionStatus::eEstablished)
        {
            sendData(pSocketInfo, BIND_CONNECTION_RESPONSE, transactionId, &payload,
                RETCODE_EXISTING_CONNECTION);
        }
        else
        {
            sendData(pSocketInfo, BIND_CONNECTION_RESPONSE, transactionId, &payload,
                RETCODE_SERVER_ERROR);
        }
    }
}


void RTCPMessageManager::processOpenLogicalPortRequest(TCPSocketInfo *pSocketInfo,
        const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId)
{
    for (uint16_t port : pSocketInfo->mLogicalInputPorts)
    {
        if (port == request.logicalPort())
        {
            //std::cout << "[RTCP] OpenLogicalPortRequest [OK]: " << request.logicalPort() << std::endl;
            sendData(pSocketInfo, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_OK);
            return;
        }
    }
    //std::cout << "[RTCP] OpenLogicalPortRequest [FAILED]: " << request.logicalPort() << std::endl;
    sendData(pSocketInfo, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_INVALID_PORT);
}

void RTCPMessageManager::processCheckLogicalPortsRequest(TCPSocketInfo *pSocketInfo,
        const CheckLogicalPortsRequest_t &request, const TCPTransactionId &transactionId)
{
    CheckLogicalPortsResponse_t response;

    //std::cout << "[RTCP] CheckOpenedLogicalPort [STARTING]: " << request.logicalPortsRange().size() << std::endl;

    //for (uint16_t port : request.logicalPortsRange())
    for (size_t i = 0; i < request.logicalPortsRange().size(); ++i)
    {
        uint16_t port = request.logicalPortsRange()[i];
        /*
        if (std::find(pSocketInfo->mLogicalInputPorts.begin(),
                pSocketInfo->mLogicalInputPorts.end(), port)
                != pSocketInfo->mLogicalInputPorts.end())
        */
        for (uint16_t opened_port : pSocketInfo->mLogicalInputPorts)
        {
            if (opened_port == port)
            {
                //std::cout << "[RTCP]FoundOpenedLogicalPort: " << port << std::endl;
                response.availableLogicalPorts().emplace_back(port);
            }
        }
    }

    SerializedPayload_t payload(static_cast<uint32_t>(CheckLogicalPortsResponse_t::getBufferCdrSerializedSize(response)));
    response.serialize(&payload);
    //std::cout << "[RTCP] INFO: " << payload.length << std::endl;
    //std::cout << "[RTCP] INFO: " << response.availableLogicalPorts().size() << std::endl;
    sendData(pSocketInfo, CHECK_LOGICAL_PORT_RESPONSE, transactionId, &payload, RETCODE_OK);
}

void RTCPMessageManager::processKeepAliveRequest(TCPSocketInfo *pSocketInfo,
        const KeepAliveRequest_t &request, const TCPTransactionId &transactionId)
{
    if (pSocketInfo->GetLocator().get_logical_port() == request.locator().get_logical_port())
    {
        sendData(pSocketInfo, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_OK);
    }
    else
    {
        sendData(pSocketInfo, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_UNKNOWN_LOCATOR);
    }
}

void RTCPMessageManager::processLogicalPortIsClosedRequest(TCPSocketInfo* /*pSocketInfo*/,
        const LogicalPortIsClosedRequest_t &/*request*/, const TCPTransactionId &/*transactionId*/)
{
    // TODO?
}

bool RTCPMessageManager::processBindConnectionResponse(TCPSocketInfo *pSocketInfo,
        const BindConnectionResponse_t &/*response*/, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        //std::cout << "[RTCP]Connection established (Resp) (physical: " << pSocketInfo->mLocator.get_physical_port() << ")" << std::endl;
        pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eEstablished);
        mUnconfirmedTransactions.erase(it);
        return true;
    }
    else
    {
        logWarning(RTPS_MSG_IN, "Received response for BindConnection with an unexpected transactionId: " << transactionId);
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
            //std::cout << "[RTCP] NegotiatingLogicalPort: " << pSocketInfo->mCheckingLogicalPort << std::endl;
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
        logWarning(RTPS_MSG_IN, "Received response for CheckLogicalPorts with an unexpected transactionId: " << transactionId);
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
            //std::cout << "[RTCP]OpenLogicalPort failed: " << pSocketInfo->mCheckingLogicalPort << std::endl;
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

bool RTCPMessageManager::processOpenLogicalPortResponse(TCPSocketInfo *pSocketInfo,
        ResponseCode respCode, const TCPTransactionId &transactionId, Locator_t &remoteLocator)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        switch(respCode)
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

                    //std::cout << "[RTCP] OpenedAndRoutedLogicalPort " << pSocketInfo->mNegotiatingLogicalPort << "->" << pSocketInfo->mPendingLogicalPort << std::endl;

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
                    //std::cout << "[RTCP]OpenedLogicalPort " << pSocketInfo->mPendingLogicalPort << std::endl;
                }

                pSocketInfo->mLogicalOutputPorts.emplace_back(*(pSocketInfo->mPendingLogicalOutputPorts.begin()));
                pSocketInfo->mPendingLogicalOutputPorts.erase(pSocketInfo->mPendingLogicalOutputPorts.begin());
                pSocketInfo->mPendingLogicalPort = 0;
                transport->mBoundOutputSockets[remoteLocator] = pSocketInfo;
            }
            break;
            case RETCODE_INVALID_PORT:
            {
                prepareAndSendCheckLogicalPortsRequest(pSocketInfo);
            }
            break;
            default:
                logWarning(RTPS_MSG_IN,
                    "Received response for OpenLogicalPort with error code: " << ((respCode == RETCODE_BAD_REQUEST) ? "BAD_REQUEST" : "SERVER_ERROR"));
            break;
        }
        mUnconfirmedTransactions.erase(it);
        return true;
    }
    else
    {
        logWarning(RTPS_MSG_IN, "Received response for OpenLogicalPort with an unexpected transactionId: " << transactionId);
        return false;
    }
}

bool RTCPMessageManager::processKeepAliveResponse(TCPSocketInfo *pSocketInfo,
        ResponseCode respCode, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        // TODO Notify transport in each case
        switch (respCode)
        {
        case RETCODE_OK:
            pSocketInfo->mWaitingForKeepAlive = false;
            break;
        case RETCODE_UNKNOWN_LOCATOR:
            break;
        case RETCODE_BAD_REQUEST:
            break;
        case RETCODE_SERVER_ERROR:
            break;
        default:
            break;
        }
        mUnconfirmedTransactions.erase(it);
        return true;
    }
    else
    {
        logWarning(RTPS_MSG_IN, "Received response for KeepAlive with an unexpected transactionId: " << transactionId);
        return false;
    }
}

void RTCPMessageManager::processRTCPMessage(TCPSocketInfo *socketInfo, octet* receiveBuffer)
{
    TCPControlMsgHeader controlHeader;
    memcpy(&controlHeader, receiveBuffer, TCPControlMsgHeader::getSize());
    size_t dataSize = controlHeader.length - TCPControlMsgHeader::getSize();
    size_t bufferSize = dataSize + 4;

    switch (controlHeader.kind)
    {
    case BIND_CONNECTION_REQUEST:
    {
        //std::cout << "[RTCP] Receive [BIND_CONNECTION_REQUEST] " << controlHeader.transactionId << std::endl;
        ConnectionRequest_t request;
        Locator_t myLocator;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        EndpointToLocator(socketInfo->getSocket()->local_endpoint(), myLocator);

        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);

        processConnectionRequest(socketInfo, request, controlHeader.transactionId, myLocator);
    }
    break;
    case BIND_CONNECTION_RESPONSE:
    {
        //std::cout << "[RTCP] Receive [BIND_CONNECTION_RESPONSE] " << controlHeader.transactionId << std::endl;
        ResponseCode respCode;
        BindConnectionResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(dataSize - 4));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t

        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize - 4);
        response.deserialize(&payload);
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
            // TODO Manage errors
        }
    }
    break;
    case OPEN_LOGICAL_PORT_REQUEST:
    {
        //std::cout << "[RTCP] Receive [OPEN_LOGICAL_PORT_REQUEST] " << controlHeader.transactionId << std::endl;
        OpenLogicalPortRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        processOpenLogicalPortRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case CHECK_LOGICAL_PORT_REQUEST:
    {
        //std::cout << "[RTCP] Receive [CHECK_LOGICAL_PORT_REQUEST]: " << controlHeader.length << " - " << controlHeader.transactionId <<  std::endl;
        CheckLogicalPortsRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        processCheckLogicalPortsRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case CHECK_LOGICAL_PORT_RESPONSE:
    {
        //std::cout << "[RTCP] Receive [CHECK_LOGICAL_PORT_RESPONSE] " << controlHeader.transactionId << std::endl;
        ResponseCode respCode;
        CheckLogicalPortsResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(dataSize - 4));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize - 4);
        response.deserialize(&payload);
        processCheckLogicalPortsResponse(socketInfo, response, controlHeader.transactionId);
    }
    break;
    case KEEP_ALIVE_REQUEST:
    {
        //std::cout << "[RTCP] Receive [KEEP_ALIVE_REQUEST] " << controlHeader.transactionId << std::endl;
        KeepAliveRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        processKeepAliveRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case LOGICAL_PORT_IS_CLOSED_REQUEST:
    {
        //std::cout << "[RTCP] Receive [LOGICAL_PORT_IS_CLOSED_REQUEST] " << controlHeader.transactionId << std::endl;
        LogicalPortIsClosedRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        processLogicalPortIsClosedRequest(socketInfo, request, controlHeader.transactionId);
    }
    break;
    case UNBIND_CONNECTION_REQUEST:
    {
        // TODO Close socket
        //std::cout << "[RTCP] Receive [UNBIND_CONNECTION_REQUEST] " << controlHeader.transactionId << std::endl;
    }
    break;
    case OPEN_LOGICAL_PORT_RESPONSE:
    {
        //std::cout << "[RTCP] Receive [OPEN_LOGICAL_PORT_RESPONSE] " << controlHeader.transactionId << std::endl;
        ResponseCode respCode;
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4);
        Locator_t remoteLocator;
        EndpointToLocator(socketInfo->getSocket()->remote_endpoint(), remoteLocator);
        if (!processOpenLogicalPortResponse(socketInfo, respCode, controlHeader.transactionId, remoteLocator))
        {
            // TODO unexpected transactionId
        }
    }
    break;
    case KEEP_ALIVE_RESPONSE:
    {
        //std::cout << "[RTCP] Receive [KEEP_ALIVE_RESPONSE] " << controlHeader.transactionId << std::endl;
        ResponseCode respCode;
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4);
        if (!processKeepAliveResponse(socketInfo, respCode, controlHeader.transactionId))
        {
            // TODO unexpected transactionId
        }
    }
    break;
    }
}
} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
