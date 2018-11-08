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

/**
 * @file RTCPMessageManager.cpp
 *
 */
#include <fastrtps/transport/tcp/RTCPHeader.h>
#include <fastrtps/transport/tcp/RTCPMessageManager.h>
#include <fastrtps/transport/TCPChannelResource.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/System.h>
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>


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
        IPLocator::setIPv4(locator, ipBytes.data());
    }
    else if (endpoint.protocol() == asio::ip::tcp::v6())
    {
        locator.kind = LOCATOR_KIND_TCPv6;
        auto ipBytes = endpoint.address().to_v6().to_bytes();
        IPLocator::setIPv6(locator, ipBytes.data());
    }
    IPLocator::setPhysicalPort(locator, endpoint.port());
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

size_t RTCPMessageManager::sendMessage(TCPChannelResource *pChannelResource, const CDRMessage_t &msg) const
{
    size_t send = mTransport->Send(pChannelResource, msg.buffer, msg.length);
    if (send != msg.length)
    {
        logWarning(RTCP, "Bad sent size..." << send << " bytes of " << msg.length << " bytes.");
    }
    //logInfo(RTCP, "Sent " << send << " bytes");
    return send;
}

bool RTCPMessageManager::sendData(TCPChannelResource *pChannelResource, TCPCPMKind kind,
        const TCPTransactionId &transactionId, const SerializedPayload_t *payload, const ResponseCode respCode)
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

    return sendMessage(pChannelResource, msg) > 0;
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
        addTransactionId(retCtrlHeader.transactionId);
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
    /*
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
    */
}

TCPTransactionId RTCPMessageManager::sendConnectionRequest(TCPChannelResource *pChannelResource)
{
    ConnectionRequest_t request;
    Locator_t locator;
    mTransport->EndpointToLocator(pChannelResource->getSocket()->local_endpoint(), locator);

    auto config = mTransport->GetConfiguration();
    if (!config->listening_ports.empty())
    {
        IPLocator::setPhysicalPort(locator, *(config->listening_ports.begin()));
    }
    else
    {
        IPLocator::setPhysicalPort(locator, static_cast<uint16_t>(System::GetPID()));
    }

    if (locator.kind == LOCATOR_KIND_TCPv4)
    {
        const TCPv4TransportDescriptor* pTCPv4Desc = static_cast<TCPv4TransportDescriptor*>(config);
        IPLocator::setWan(locator, pTCPv4Desc->wan_addr[0], pTCPv4Desc->wan_addr[1], pTCPv4Desc->wan_addr[2],
            pTCPv4Desc->wan_addr[3]);
    }
    request.protocolVersion(c_rtcpProtocolVersion);
    request.transportLocator(locator);

    SerializedPayload_t payload(static_cast<uint32_t>(ConnectionRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);

    logInfo(RTCP_MSG, "Send [BIND_CONNECTION_REQUEST] PhysicalPort: " << IPLocator::getPhysicalPort(locator));
    TCPTransactionId id = getTransactionId();
    sendData(pChannelResource, BIND_CONNECTION_REQUEST, id, &payload);
    pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eWaitingForBindResponse);
    return id;
}

TCPTransactionId RTCPMessageManager::sendOpenLogicalPortRequest(TCPChannelResource *pChannelResource, uint16_t port)
{
    OpenLogicalPortRequest_t request;
    request.logicalPort(port);
    return sendOpenLogicalPortRequest(pChannelResource, request);
}

TCPTransactionId RTCPMessageManager::sendOpenLogicalPortRequest(TCPChannelResource *pChannelResource,
        OpenLogicalPortRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(OpenLogicalPortRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_REQUEST] LogicalPort: " << request.logicalPort());
    TCPTransactionId id = getTransactionId();
    sendData(pChannelResource, OPEN_LOGICAL_PORT_REQUEST, id, &payload);
    return id;
}

TCPTransactionId RTCPMessageManager::sendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource,
        std::vector<uint16_t> &ports)
{
    CheckLogicalPortsRequest_t request;
    request.logicalPortsRange(ports);
    return sendCheckLogicalPortsRequest(pChannelResource, request);
}

TCPTransactionId RTCPMessageManager::sendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource,
        CheckLogicalPortsRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(CheckLogicalPortsRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [CHECK_LOGICAL_PORT_REQUEST]");
    TCPTransactionId id = getTransactionId();
    sendData(pChannelResource, CHECK_LOGICAL_PORT_REQUEST, id, &payload);
    return id;
}

TCPTransactionId RTCPMessageManager::sendKeepAliveRequest(TCPChannelResource *pChannelResource,
        KeepAliveRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(KeepAliveRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [KEEP_ALIVE_REQUEST]");
    TCPTransactionId id = getTransactionId();
    sendData(pChannelResource, KEEP_ALIVE_REQUEST, id, &payload);
    return id;
}

TCPTransactionId RTCPMessageManager::sendKeepAliveRequest(TCPChannelResource *pChannelResource)
{
    KeepAliveRequest_t request;
    request.locator(pChannelResource->GetLocator());
    return sendKeepAliveRequest(pChannelResource, request);
}

TCPTransactionId RTCPMessageManager::sendLogicalPortIsClosedRequest(TCPChannelResource *pChannelResource,
    LogicalPortIsClosedRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(
        LogicalPortIsClosedRequest_t::getBufferCdrSerializedSize(request)));

    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [LOGICAL_PORT_IS_CLOSED_REQUEST] LogicalPort: " << request.logicalPort());
    TCPTransactionId id = getTransactionId();
    sendData(pChannelResource, LOGICAL_PORT_IS_CLOSED_REQUEST, id, &payload);
    return id;
}

TCPTransactionId RTCPMessageManager::sendLogicalPortIsClosedRequest(TCPChannelResource *pChannelResource, uint16_t port)
{
    LogicalPortIsClosedRequest_t request;
    request.logicalPort(port);
    return sendLogicalPortIsClosedRequest(pChannelResource, request);
}

TCPTransactionId RTCPMessageManager::sendUnbindConnectionRequest(TCPChannelResource *pChannelResource)
{
    logInfo(RTCP_MSG, "Send [UNBIND_CONNECTION_REQUEST]");
    TCPTransactionId id = getTransactionId();
    sendData(pChannelResource, UNBIND_CONNECTION_REQUEST, id);
    return id;
}

ResponseCode RTCPMessageManager::processBindConnectionRequest(TCPChannelResource *pChannelResource,
        const ConnectionRequest_t &request, const TCPTransactionId &transactionId, Locator_t &localLocator)
{
    BindConnectionResponse_t response;

    if (localLocator.kind == LOCATOR_KIND_TCPv4)
    {
        const TCPv4TransportDescriptor* pTCPv4Desc = (TCPv4TransportDescriptor*)mTransport->get_configuration();
        IPLocator::setWan(localLocator, pTCPv4Desc->wan_addr[0], pTCPv4Desc->wan_addr[1], pTCPv4Desc->wan_addr[2],
            pTCPv4Desc->wan_addr[3]);
    }
    else if (localLocator.kind == LOCATOR_KIND_TCPv6)
    {
    }
    else
    {
        assert(false);
    }

    response.locator(localLocator);

    SerializedPayload_t payload(static_cast<uint32_t>(BindConnectionResponse_t::getBufferCdrSerializedSize(response)));
    response.serialize(&payload);

    if (!isCompatibleProtocol(request.protocolVersion()))
    {
        sendData(pChannelResource, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_INCOMPATIBLE_VERSION);
        logWarning(RTCP, "Rejected client due to INCOMPATIBLE_VERSION: Expected: " << c_rtcpProtocolVersion
            << " but received " << request.protocolVersion());
        return RETCODE_INCOMPATIBLE_VERSION;
    }

    ResponseCode code = pChannelResource->ProcessBindRequest(request.transportLocator());
    sendData(pChannelResource, BIND_CONNECTION_RESPONSE, transactionId, &payload, code);

    return RETCODE_OK;
}

ResponseCode RTCPMessageManager::processOpenLogicalPortRequest(TCPChannelResource *pChannelResource,
    const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId)
{
    if (!pChannelResource->IsConnectionEstablished())
    {
        sendData(pChannelResource, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (request.logicalPort() == 0 || !mTransport->IsInputPortOpen(request.logicalPort()))
    {
        logInfo(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_RESPONSE] Not found: " << request.logicalPort());
        sendData(pChannelResource, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_INVALID_PORT);
    }
    else
    {
        logInfo(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_RESPONSE] Found: " << request.logicalPort());
        sendData(pChannelResource, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_OK);
    }
    return RETCODE_OK;
}

void RTCPMessageManager::processCheckLogicalPortsRequest(TCPChannelResource *pChannelResource,
    const CheckLogicalPortsRequest_t &request, const TCPTransactionId &transactionId)
{
    CheckLogicalPortsResponse_t response;
    if (!pChannelResource->IsConnectionEstablished())
    {
        sendData(pChannelResource, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else
    {
        if (request.logicalPortsRange().empty())
        {
            logWarning(RTCP, "No available logical ports.");
        }
        else
        {
            for (uint16_t port : request.logicalPortsRange())
            {
                if (mTransport->IsInputPortOpen(port))
                {
                    if (port == 0)
                    {
                        logInfo(RTCP, "FoundOpenedLogicalPort 0, but will not be considered");
                    }
                    logInfo(RTCP, "FoundOpenedLogicalPort: " << port);
                    response.availableLogicalPorts().emplace_back(port);
                }
            }
        }

        SerializedPayload_t payload(static_cast<uint32_t>(
            CheckLogicalPortsResponse_t::getBufferCdrSerializedSize(response)));
        response.serialize(&payload);
        sendData(pChannelResource, CHECK_LOGICAL_PORT_RESPONSE, transactionId, &payload, RETCODE_OK);
    }
}

ResponseCode RTCPMessageManager::processKeepAliveRequest(TCPChannelResource *pChannelResource,
        const KeepAliveRequest_t &request, const TCPTransactionId &transactionId)
{
    if (!pChannelResource->IsConnectionEstablished())
    {
        sendData(pChannelResource, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (IPLocator::getLogicalPort(pChannelResource->GetLocator()) == IPLocator::getLogicalPort(request.locator()))
    {
        sendData(pChannelResource, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_OK);
    }
    else
    {
        sendData(pChannelResource, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_UNKNOWN_LOCATOR);
        return RETCODE_UNKNOWN_LOCATOR;
    }
    return RETCODE_OK;
}

void RTCPMessageManager::processLogicalPortIsClosedRequest(TCPChannelResource* pChannelResource,
        const LogicalPortIsClosedRequest_t &request, const TCPTransactionId & transactionId)
{
    if (!pChannelResource->IsConnectionEstablished())
    {
        sendData(pChannelResource, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else
    {
        pChannelResource->SetLogicalPortPending(request.logicalPort());
    }
}

ResponseCode RTCPMessageManager::processBindConnectionResponse(TCPChannelResource *pChannelResource,
        const BindConnectionResponse_t &/*response*/, const TCPTransactionId &transactionId)
{
    if (findTransactionId(transactionId))
    {
        logInfo(RTCP, "Connection established (Resp) (physical: "
                << IPLocator::getPhysicalPort(pChannelResource->mLocator) << ")");
        pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eEstablished);
        removeTransactionId(transactionId);
        return RETCODE_OK;
    }
    else
    {
        logWarning(RTCP, "Received BindConnectionResponse with an invalid transactionId: " << transactionId);
        return RETCODE_VOID;
    }
}

ResponseCode RTCPMessageManager::processCheckLogicalPortsResponse(TCPChannelResource *pChannelResource,
        const CheckLogicalPortsResponse_t &response, const TCPTransactionId &transactionId)
{
    if (findTransactionId(transactionId))
    {
        pChannelResource->ProcessCheckLogicalPortsResponse(transactionId, response.availableLogicalPorts());
        removeTransactionId(transactionId);
        return RETCODE_OK;
    }
    else
    {
        logWarning(RTCP, "Received CheckLogicalPortsResponse with an invalid transactionId: " << transactionId);
        return RETCODE_VOID;
    }
}

ResponseCode RTCPMessageManager::processOpenLogicalPortResponse(TCPChannelResource *pChannelResource,
        ResponseCode respCode, const TCPTransactionId &transactionId, Locator_t &/*remoteLocator*/)
{
    if (findTransactionId(transactionId))
    {
        switch (respCode)
        {
        case RETCODE_OK:
        {
            pChannelResource->AddLogicalPortResponse(transactionId, true);
        }
        break;
        case RETCODE_INVALID_PORT:
        {
            pChannelResource->AddLogicalPortResponse(transactionId, false);
        }
        break;
        default:
            logWarning(RTCP, "Received response for OpenLogicalPort with error code: "
                << ((respCode == RETCODE_BAD_REQUEST) ? "BAD_REQUEST" : "SERVER_ERROR"));
            break;
        }
        removeTransactionId(transactionId);
    }
    else
    {
        logWarning(RTCP, "Received OpenLogicalPortResponse with an invalid transactionId: " << transactionId);
    }
    return RETCODE_OK;
}

ResponseCode RTCPMessageManager::processKeepAliveResponse(TCPChannelResource *pChannelResource,
        ResponseCode respCode, const TCPTransactionId &transactionId)
{
    if (findTransactionId(transactionId))
    {
        switch (respCode)
        {
        case RETCODE_OK:
            pChannelResource->mWaitingForKeepAlive = false;
            break;
        case RETCODE_UNKNOWN_LOCATOR:
            return RETCODE_UNKNOWN_LOCATOR;
        default:
            break;
        }
        removeTransactionId(transactionId);
    }
    else
    {
        logWarning(RTCP, "Received response for KeepAlive with an unexpected transactionId: " << transactionId);
    }
    return RETCODE_OK;
}

ResponseCode RTCPMessageManager::processRTCPMessage(TCPChannelResource *pChannelResource, octet* receiveBuffer,
        size_t receivedSize)
{
    ResponseCode responseCode(RETCODE_OK);

    TCPControlMsgHeader controlHeader = *(reinterpret_cast<TCPControlMsgHeader*>(receiveBuffer));
    //memcpy(&controlHeader, receiveBuffer, TCPControlMsgHeader::getSize());
    size_t dataSize = controlHeader.length - TCPControlMsgHeader::getSize();
    size_t bufferSize = dataSize + 4;

    // Message size checking.
    if (dataSize + TCPControlMsgHeader::getSize() != receivedSize)
    {
        sendData(pChannelResource, controlHeader.kind, controlHeader.transactionId, nullptr, RETCODE_BAD_REQUEST);
        return RETCODE_OK;
    }

    switch (controlHeader.kind)
    {
    case BIND_CONNECTION_REQUEST:
    {
        //logInfo(RTCP_SEQ, "Receive [BIND_CONNECTION_REQUEST] Seq: " << controlHeader.transactionId);
        ConnectionRequest_t request;
        Locator_t myLocator;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        EndpointToLocator(pChannelResource->getSocket()->local_endpoint(), myLocator);

        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);

        logInfo(RTCP_MSG, "Receive [BIND_CONNECTION_REQUEST] " <<
            "LogicalPort: " << IPLocator::getLogicalPort(request.transportLocator())
            << ", Physical remote: " << IPLocator::getPhysicalPort(request.transportLocator()));

        responseCode = processBindConnectionRequest(pChannelResource, request, controlHeader.transactionId, myLocator);
    }
    break;
    case BIND_CONNECTION_RESPONSE:
    {
        //logInfo(RTCP_SEQ, "Receive [BIND_CONNECTION_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        BindConnectionResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize);
        response.deserialize(&payload);

        logInfo(RTCP_MSG, "Receive [BIND_CONNECTION_RESPONSE] LogicalPort: " \
            << IPLocator::getLogicalPort(response.locator()) << ", Physical remote: " \
            << IPLocator::getPhysicalPort(response.locator()));

        if (respCode == RETCODE_OK || respCode == RETCODE_EXISTING_CONNECTION)
        {
            std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->mPendingLogicalMutex);
            if (!pChannelResource->mPendingLogicalOutputPorts.empty())
            {
                responseCode = processBindConnectionResponse(pChannelResource, response, controlHeader.transactionId);
            }
        }
        else
        {
            // If the bind message fails, close the connection and try again.
            if (respCode == RETCODE_INCOMPATIBLE_VERSION)
            {
                logError(RTCP, "Received RETCODE_INCOMPATIBLE_VERSION from server.");
            }
            responseCode = respCode;
        }
    }
    break;
    case OPEN_LOGICAL_PORT_REQUEST:
    {
        //logInfo(RTCP_SEQ, "Receive [OPEN_LOGICAL_PORT_REQUEST] Seq: " << controlHeader.transactionId);
        OpenLogicalPortRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [OPEN_LOGICAL_PORT_REQUEST] LogicalPort: " << request.logicalPort());
        responseCode = processOpenLogicalPortRequest(pChannelResource, request, controlHeader.transactionId);
    }
    break;
    case CHECK_LOGICAL_PORT_REQUEST:
    {
        //logInfo(RTCP_SEQ, "Receive [CHECK_LOGICAL_PORT_REQUEST] Seq: " << controlHeader.transactionId);
        CheckLogicalPortsRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [CHECK_LOGICAL_PORT_REQUEST]");
        processCheckLogicalPortsRequest(pChannelResource, request, controlHeader.transactionId);
    }
    break;
    case CHECK_LOGICAL_PORT_RESPONSE:
    {
        //logInfo(RTCP_SEQ, "Receive [CHECK_LOGICAL_PORT_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        CheckLogicalPortsResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize);
        response.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [CHECK_LOGICAL_PORT_RESPONSE]");
        processCheckLogicalPortsResponse(pChannelResource, response, controlHeader.transactionId);
    }
    break;
    case KEEP_ALIVE_REQUEST:
    {
        //logInfo(RTCP_SEQ, "Receive [KEEP_ALIVE_REQUEST] Seq: " << controlHeader.transactionId);
        KeepAliveRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [KEEP_ALIVE_REQUEST]");
        responseCode = processKeepAliveRequest(pChannelResource, request, controlHeader.transactionId);
    }
    break;
    case LOGICAL_PORT_IS_CLOSED_REQUEST:
    {
        //logInfo(RTCP_SEQ, "Receive [LOGICAL_PORT_IS_CLOSED_REQUEST] Seq: " << controlHeader.transactionId);
        LogicalPortIsClosedRequest_t request;
        SerializedPayload_t payload(static_cast<uint32_t>(bufferSize));
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [LOGICAL_PORT_IS_CLOSED_REQUEST] LogicalPort: " << request.logicalPort());
        processLogicalPortIsClosedRequest(pChannelResource, request, controlHeader.transactionId);
    }
    break;
    case UNBIND_CONNECTION_REQUEST:
    {
        //logInfo(RTCP_SEQ, "Receive [UNBIND_CONNECTION_REQUEST] Seq:" << controlHeader.transactionId);
        logInfo(RTCP_MSG, "Receive [UNBIND_CONNECTION_REQUEST]");
        mTransport->CloseTCPSocket(pChannelResource);
        responseCode = RETCODE_OK;
    }
    break;
    case OPEN_LOGICAL_PORT_RESPONSE:
    {
        //logInfo(RTCP_SEQ, "Receive [OPEN_LOGICAL_PORT_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4);
        Locator_t remoteLocator;
        EndpointToLocator(pChannelResource->getSocket()->remote_endpoint(), remoteLocator);
        logInfo(RTCP_MSG, "Receive [OPEN_LOGICAL_PORT_RESPONSE]");
        processOpenLogicalPortResponse(pChannelResource, respCode, controlHeader.transactionId, remoteLocator);
    }
    break;
    case KEEP_ALIVE_RESPONSE:
    {
        //logInfo(RTCP_SEQ, "Receive [KEEP_ALIVE_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4);
        logInfo(RTCP_MSG, "Receive [KEEP_ALIVE_RESPONSE]");
        responseCode = processKeepAliveResponse(pChannelResource, respCode, controlHeader.transactionId);
    }
    break;
    default:
        sendData(pChannelResource, controlHeader.kind, controlHeader.transactionId, nullptr, RETCODE_BAD_REQUEST);
        break;
    }
    return responseCode;
}

bool RTCPMessageManager::isCompatibleProtocol(const ProtocolVersion_t &protocol) const
{
    return protocol == c_rtcpProtocolVersion;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
