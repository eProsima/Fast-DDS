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

bool RTCPMessageManager::sendData(TCPChannelResource *pChannelResource, TCPCPMKind kind, const TCPTransactionId &transactionId,
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

void RTCPMessageManager::sendConnectionRequest(TCPChannelResource *pChannelResource, uint16_t localLogicalPort)
{
    ConnectionRequest_t request;
    Locator_t locator;
    EndpointToLocator(pChannelResource->getSocket()->local_endpoint(), locator);
    locator.set_logical_port(localLogicalPort);

    if (locator.kind == LOCATOR_KIND_TCPv4)
    {
        const TCPv4TransportDescriptor* pTCPv4Desc = (TCPv4TransportDescriptor*)mTransport->get_configuration();
        locator.set_IP4_WAN_address(pTCPv4Desc->wan_addr[0], pTCPv4Desc->wan_addr[1], pTCPv4Desc->wan_addr[2],
            pTCPv4Desc->wan_addr[3]);
    }
    request.transportLocator(locator);

    SerializedPayload_t payload(static_cast<uint32_t>(ConnectionRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);

    logInfo(RTCP_MSG, "Send [BIND_CONNECTION_REQUEST] PhysicalPort: " << locator.get_physical_port()
        << ", LogicalPort: " << localLogicalPort);
    sendData(pChannelResource, BIND_CONNECTION_REQUEST, getTransactionId(), &payload);
    pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eWaitingForBindResponse);
}

void RTCPMessageManager::sendOpenLogicalPortRequest(TCPChannelResource *pChannelResource, uint16_t port)
{
    OpenLogicalPortRequest_t request;
    request.logicalPort(port);
    sendOpenLogicalPortRequest(pChannelResource, request);
}

void RTCPMessageManager::sendOpenLogicalPortRequest(TCPChannelResource *pChannelResource, OpenLogicalPortRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(OpenLogicalPortRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_REQUEST] LogicalPort: " << request.logicalPort());
    sendData(pChannelResource, OPEN_LOGICAL_PORT_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource, std::vector<uint16_t> &ports)
{
    CheckLogicalPortsRequest_t request;
    request.logicalPortsRange(ports);
    sendCheckLogicalPortsRequest(pChannelResource, request);
}

void RTCPMessageManager::sendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource, CheckLogicalPortsRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(CheckLogicalPortsRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [CHECK_LOGICAL_PORT_REQUEST]");
    sendData(pChannelResource, CHECK_LOGICAL_PORT_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendKeepAliveRequest(TCPChannelResource *pChannelResource, KeepAliveRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(KeepAliveRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [KEEP_ALIVE_REQUEST]");
    sendData(pChannelResource, KEEP_ALIVE_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendKeepAliveRequest(TCPChannelResource *pChannelResource)
{
    KeepAliveRequest_t request;
    request.locator(pChannelResource->GetLocator());
    sendKeepAliveRequest(pChannelResource, request);
}

void RTCPMessageManager::sendLogicalPortIsClosedRequest(TCPChannelResource *pChannelResource,
    LogicalPortIsClosedRequest_t &request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(
        LogicalPortIsClosedRequest_t::getBufferCdrSerializedSize(request)));

    request.serialize(&payload);
    logInfo(RTCP_MSG, "Send [LOGICAL_PORT_IS_CLOSED_REQUEST] LogicalPort: " << request.logicalPort());
    sendData(pChannelResource, LOGICAL_PORT_IS_CLOSED_REQUEST, getTransactionId(), &payload);
}

void RTCPMessageManager::sendLogicalPortIsClosedRequest(TCPChannelResource *pChannelResource, uint16_t port)
{
    LogicalPortIsClosedRequest_t request;
    request.logicalPort(port);
    sendLogicalPortIsClosedRequest(pChannelResource, request);
}

void RTCPMessageManager::sendUnbindConnectionRequest(TCPChannelResource *pChannelResource)
{
    logInfo(RTCP_MSG, "Send [UNBIND_CONNECTION_REQUEST]");
    sendData(pChannelResource, UNBIND_CONNECTION_REQUEST, getTransactionId());
}

bool RTCPMessageManager::processBindConnectionRequest(TCPChannelResource *pChannelResource, const ConnectionRequest_t &request,
    const TCPTransactionId &transactionId, Locator_t &localLocator)
{
    BindConnectionResponse_t response;

    if (localLocator.kind == LOCATOR_KIND_TCPv4)
    {
        const TCPv4TransportDescriptor* pTCPv4Desc = (TCPv4TransportDescriptor*)mTransport->get_configuration();
        localLocator.set_logical_port(pTCPv4Desc->metadata_logical_port);
        localLocator.set_IP4_WAN_address(pTCPv4Desc->wan_addr[0], pTCPv4Desc->wan_addr[1], pTCPv4Desc->wan_addr[2],
            pTCPv4Desc->wan_addr[3]);
    }
    else if (localLocator.kind == LOCATOR_KIND_TCPv6)
    {
        const TCPv6TransportDescriptor* pTCPv6Desc = (TCPv6TransportDescriptor*)mTransport->get_configuration();
        localLocator.set_logical_port(pTCPv6Desc->metadata_logical_port);
    }
    else
    {
        assert(false);
    }

    response.locator(localLocator);

    SerializedPayload_t payload(static_cast<uint32_t>(BindConnectionResponse_t::getBufferCdrSerializedSize(response)));
    response.serialize(&payload);

    if (request.protocolVersion() != c_ProtocolVersion)
    {
        sendData(pChannelResource, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_INCOMPATIBLE_VERSION);
        return false;
    }
    else if (pChannelResource->mConnectionStatus == TCPChannelResource::eConnectionStatus::eWaitingForBind)
    {
        {
            std::unique_lock<std::recursive_mutex> scope(pChannelResource->mPendingLogicalMutex);
            pChannelResource->EnqueueLogicalPort(request.transportLocator().get_logical_port());
            mTransport->BindSocket(request.transportLocator(), pChannelResource);
        }
        sendData(pChannelResource, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_OK);
        pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eEstablished);
    }
    else if (pChannelResource->mConnectionStatus == TCPChannelResource::eConnectionStatus::eEstablished)
    {
        sendData(pChannelResource, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_EXISTING_CONNECTION);
    }
    else
    {
        sendData(pChannelResource, BIND_CONNECTION_RESPONSE, transactionId, &payload, RETCODE_SERVER_ERROR);
    }
    return true;
}

bool RTCPMessageManager::processOpenLogicalPortRequest(TCPChannelResource *pChannelResource,
    const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId)
{
    if (pChannelResource->mConnectionStatus != TCPChannelResource::eConnectionStatus::eEstablished)
    {
        sendData(pChannelResource, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (std::find(pChannelResource->mOpenedPorts.begin(), pChannelResource->mOpenedPorts.end(),
        request.logicalPort()) != pChannelResource->mOpenedPorts.end())
    {
        //logInfo(RTCP, "OpenLogicalPortRequest [FAILED]: " << request.logicalPort());
        sendData(pChannelResource, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_INVALID_PORT);
    }
    else
    {
        pChannelResource->mOpenedPorts.emplace_back(request.logicalPort());
        sendData(pChannelResource, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_OK);
    }
    return true;
}

void RTCPMessageManager::processCheckLogicalPortsRequest(TCPChannelResource *pChannelResource,
    const CheckLogicalPortsRequest_t &request, const TCPTransactionId &transactionId)
{
    CheckLogicalPortsResponse_t response;
    if (pChannelResource->mConnectionStatus != TCPChannelResource::eConnectionStatus::eEstablished)
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
				if (std::find(pChannelResource->mOpenedPorts.begin(), pChannelResource->mOpenedPorts.end(), port) ==
					pChannelResource->mOpenedPorts.end())
				{
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

bool RTCPMessageManager::processKeepAliveRequest(TCPChannelResource *pChannelResource, const KeepAliveRequest_t &request,
    const TCPTransactionId &transactionId)
{
    if (pChannelResource->mConnectionStatus != TCPChannelResource::eConnectionStatus::eEstablished)
    {
        sendData(pChannelResource, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (pChannelResource->GetLocator().get_logical_port() == request.locator().get_logical_port())
    {
        sendData(pChannelResource, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_OK);
    }
    else
    {
        sendData(pChannelResource, KEEP_ALIVE_RESPONSE, transactionId, nullptr, RETCODE_UNKNOWN_LOCATOR);
        return false;
    }
    return true;
}

void RTCPMessageManager::processLogicalPortIsClosedRequest(TCPChannelResource* pChannelResource,
    const LogicalPortIsClosedRequest_t &/*request*/, const TCPTransactionId & transactionId)
{
    if (pChannelResource->mConnectionStatus != TCPChannelResource::eConnectionStatus::eEstablished)
    {
        sendData(pChannelResource, CHECK_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_SERVER_ERROR);
    }
}

bool RTCPMessageManager::processBindConnectionResponse(TCPChannelResource *pChannelResource,
    const BindConnectionResponse_t &/*response*/, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        logInfo(RTCP, "Connection established (Resp) (physical: " << pChannelResource->mLocator.get_physical_port() << ")");
        pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eEstablished);
        mUnconfirmedTransactions.erase(it);
        return true;
    }
    else
    {
        logWarning(RTCP, "Received BindConnectionResponse with an invalid transactionId: " << transactionId);
        return false;
    }
}

bool RTCPMessageManager::processCheckLogicalPortsResponse(TCPChannelResource *pChannelResource,
    const CheckLogicalPortsResponse_t &response, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        if (response.availableLogicalPorts().empty())
        {
            pChannelResource->mCheckingLogicalPort += (mTransport->GetLogicalPortRange()
                * mTransport->GetLogicalPortIncrement());
            prepareAndSendCheckLogicalPortsRequest(pChannelResource);
        }
        else
        {
            pChannelResource->mCheckingLogicalPort = response.availableLogicalPorts()[0];
            pChannelResource->EnqueueLogicalPort(pChannelResource->mCheckingLogicalPort);
            //logInfo(RTCP, "NegotiatingLogicalPort: " << pChannelResource->mCheckingLogicalPort);
            if (pChannelResource->mNegotiatingLogicalPort == 0)
            {
                logWarning(RTCP, "Negotiated new logical port wihtout initial port?");
            }
        }

        mUnconfirmedTransactions.erase(it);
        return true;
    }
    else
    {
        logWarning(RTCP, "Received CheckLogicalPortsResponse with an invalid transactionId: " << transactionId);
        return false;
    }
}

void RTCPMessageManager::prepareAndSendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource)
{
    // Dont try again this port
    {
        std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->mPendingLogicalMutex);
        if (!pChannelResource->mPendingLogicalOutputPorts.empty())
        {
            pChannelResource->mPendingLogicalOutputPorts.erase(pChannelResource->mPendingLogicalOutputPorts.begin());
        }

        if (pChannelResource->mNegotiatingLogicalPort == 0) // Keep original logical port being negotiated
        {
            pChannelResource->mNegotiatingLogicalPort = pChannelResource->mPendingLogicalPort;
            pChannelResource->mCheckingLogicalPort = pChannelResource->mPendingLogicalPort;
            logInfo(RTCP, "OpenLogicalPort failed: " << pChannelResource->mCheckingLogicalPort);
        }
        pChannelResource->mPendingLogicalPort = 0;
    }

    std::vector<uint16_t> ports;
    for (uint16_t p = pChannelResource->mCheckingLogicalPort + mTransport->GetLogicalPortIncrement();
        p <= pChannelResource->mCheckingLogicalPort + (mTransport->GetLogicalPortRange()
            * mTransport->GetLogicalPortIncrement());
        p += mTransport->GetLogicalPortIncrement())
    {
        if (p <= pChannelResource->mNegotiatingLogicalPort + mTransport->GetMaxLogicalPort())
        {
            ports.emplace_back(p);
        }
    }

    if (ports.empty()) // No more available ports!
    {
        logError(RTCP, "Cannot find an available logical port.");
    }
	else
	{
		sendCheckLogicalPortsRequest(pChannelResource, ports);
	}
}

bool RTCPMessageManager::processOpenLogicalPortResponse(TCPChannelResource *pChannelResource, ResponseCode respCode,
    const TCPTransactionId &transactionId, Locator_t &remoteLocator)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        switch (respCode)
        {
        case RETCODE_OK:
        {
            std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->mPendingLogicalMutex);
            if (pChannelResource->mNegotiatingLogicalPort != 0
                && pChannelResource->mPendingLogicalPort == pChannelResource->mCheckingLogicalPort)
            {
                // Add route
                pChannelResource->mLogicalPortRouting[pChannelResource->mNegotiatingLogicalPort]
                    = pChannelResource->mPendingLogicalPort;

                //logInfo(RTCP, "OpenedAndRoutedLogicalPort " << pChannelResource->mNegotiatingLogicalPort
                // << "->" << pChannelResource->mPendingLogicalPort);

                // We want the reference to the negotiated port, not the real logical one
                remoteLocator.set_logical_port(pChannelResource->mNegotiatingLogicalPort);

                // Both, real one and negotiated must be added
                pChannelResource->mLogicalOutputPorts.emplace_back(pChannelResource->mNegotiatingLogicalPort);

                pChannelResource->mNegotiatingLogicalPort = 0;
                pChannelResource->mCheckingLogicalPort = 0;
            }
            else
            {
                remoteLocator.set_logical_port(pChannelResource->mPendingLogicalPort);
                logInfo(RTCP, "OpenedLogicalPort " << pChannelResource->mPendingLogicalPort);
            }

            pChannelResource->mLogicalOutputPorts.emplace_back(*(pChannelResource->mPendingLogicalOutputPorts.begin()));
            pChannelResource->mPendingLogicalOutputPorts.erase(pChannelResource->mPendingLogicalOutputPorts.begin());
            pChannelResource->mPendingLogicalPort = 0;
            mTransport->BindSocket(remoteLocator, pChannelResource);
        }
        break;
        case RETCODE_INVALID_PORT:
        {
            prepareAndSendCheckLogicalPortsRequest(pChannelResource);
        }
        break;
        default:
            logWarning(RTCP, "Received response for OpenLogicalPort with error code: "
                << ((respCode == RETCODE_BAD_REQUEST) ? "BAD_REQUEST" : "SERVER_ERROR"));
            break;
        }
        mUnconfirmedTransactions.erase(it);
    }
    else
    {
        logWarning(RTCP, "Received OpenLogicalPortResponse with an invalid transactionId: " << transactionId);
    }
    return true;
}

bool RTCPMessageManager::processKeepAliveResponse(TCPChannelResource *pChannelResource,
    ResponseCode respCode, const TCPTransactionId &transactionId)
{
    auto it = mUnconfirmedTransactions.find(transactionId);
    if (it != mUnconfirmedTransactions.end())
    {
        switch (respCode)
        {
        case RETCODE_OK:
            pChannelResource->mWaitingForKeepAlive = false;
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
        logWarning(RTCP, "Received response for KeepAlive with an unexpected transactionId: " << transactionId);
    }
    return true;
}

bool RTCPMessageManager::processRTCPMessage(TCPChannelResource *pChannelResource, octet* receiveBuffer, size_t receivedSize)
{
    bool bProcessOk(true);

    TCPControlMsgHeader controlHeader;
    memcpy(&controlHeader, receiveBuffer, TCPControlMsgHeader::getSize());
    size_t dataSize = controlHeader.length - TCPControlMsgHeader::getSize();
    size_t bufferSize = dataSize + 4;

    // Message size checking.
    if (dataSize + TCPControlMsgHeader::getSize() != receivedSize)
    {
        sendData(pChannelResource, controlHeader.kind, controlHeader.transactionId, nullptr, RETCODE_BAD_REQUEST);
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
        EndpointToLocator(pChannelResource->getSocket()->local_endpoint(), myLocator);

        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize()]), dataSize);
        request.deserialize(&payload);

        logInfo(RTCP_MSG, "Receive [BIND_CONNECTION_REQUEST] " <<
            "LogicalPort: " << request.transportLocator().get_logical_port()
            << ", Physical remote: " << request.transportLocator().get_physical_port());

        bProcessOk = processBindConnectionRequest(pChannelResource, request, controlHeader.transactionId, myLocator);
    }
    break;
    case BIND_CONNECTION_RESPONSE:
    {
        logInfo(RTCP_SEQ, "Receive [BIND_CONNECTION_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        BindConnectionResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(dataSize));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize);
        response.deserialize(&payload);

        logInfo(RTCP_MSG, "Receive [BIND_CONNECTION_RESPONSE] LogicalPort: " << response.locator().get_logical_port()
            << ", Physical remote: " << response.locator().get_physical_port());

        if (respCode == RETCODE_OK || respCode == RETCODE_EXISTING_CONNECTION)
        {
            std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->mPendingLogicalMutex);
            if (!pChannelResource->mPendingLogicalOutputPorts.empty())
            {
                processBindConnectionResponse(pChannelResource, response, controlHeader.transactionId);
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
        bProcessOk = processOpenLogicalPortRequest(pChannelResource, request, controlHeader.transactionId);
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
        processCheckLogicalPortsRequest(pChannelResource, request, controlHeader.transactionId);
    }
    break;
    case CHECK_LOGICAL_PORT_RESPONSE:
    {
        logInfo(RTCP_SEQ, "Receive [CHECK_LOGICAL_PORT_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        CheckLogicalPortsResponse_t response;
        SerializedPayload_t payload(static_cast<uint32_t>(dataSize));
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4); // uint32_t
        readSerializedPayload(payload, &(receiveBuffer[TCPControlMsgHeader::getSize() + 4]), dataSize);
        response.deserialize(&payload);
        logInfo(RTCP_MSG, "Receive [CHECK_LOGICAL_PORT_RESPONSE]");
        processCheckLogicalPortsResponse(pChannelResource, response, controlHeader.transactionId);
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
        bProcessOk = processKeepAliveRequest(pChannelResource, request, controlHeader.transactionId);
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
        processLogicalPortIsClosedRequest(pChannelResource, request, controlHeader.transactionId);
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
        EndpointToLocator(pChannelResource->getSocket()->remote_endpoint(), remoteLocator);
        logInfo(RTCP_MSG, "Receive [OPEN_LOGICAL_PORT_RESPONSE]");
        processOpenLogicalPortResponse(pChannelResource, respCode, controlHeader.transactionId, remoteLocator);
    }
    break;
    case KEEP_ALIVE_RESPONSE:
    {
        logInfo(RTCP_SEQ, "Receive [KEEP_ALIVE_RESPONSE] Seq: " << controlHeader.transactionId);
        ResponseCode respCode;
        memcpy(&respCode, &(receiveBuffer[TCPControlMsgHeader::getSize()]), 4);
        logInfo(RTCP_MSG, "Receive [KEEP_ALIVE_RESPONSE]");
        bProcessOk = processKeepAliveResponse(pChannelResource, respCode, controlHeader.transactionId);
    }
    break;
    default:
        sendData(pChannelResource, controlHeader.kind, controlHeader.transactionId, nullptr, RETCODE_BAD_REQUEST);
        break;
    }
    return bProcessOk;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
