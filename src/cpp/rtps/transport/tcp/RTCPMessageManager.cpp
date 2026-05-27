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

#include <rtps/transport/tcp/RTCPMessageManager.h>

#include <limits>
#include <thread>

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <rtps/transport/tcp/RTCPHeader.h>
#include <rtps/transport/TCPChannelResource.h>
#include <rtps/transport/TCPTransportInterface.h>

#include <utils/SystemInfo.hpp>

#define IDSTRING "(ID:" << std::this_thread::get_id() << ") " <<

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

static void endpoint_to_locator(
        const asio::ip::tcp::endpoint& endpoint,
        Locator& locator)
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

static bool readResponseCode(
        ResponseCode& respCode,
        CDRMessage_t* msg)
{
    return CDRMessage::readUInt32(msg, (uint32_t*)&respCode);
}

static bool readSerializedPayload(
        SerializedPayload_t& payload,
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::readUInt16(msg, &payload.encapsulation);
    valid = valid && CDRMessage::readUInt32(msg, &payload.length);
    if (valid)
    {
        uint64_t next_pos = static_cast<uint64_t>(msg->pos) + static_cast<uint64_t>(payload.length);
        valid = (next_pos == static_cast<uint64_t>(msg->length));
    }
    if (valid)
    {
        payload.data = &msg->buffer[msg->pos];
        msg->pos += payload.length;
    }
    else
    {
        payload.data = nullptr;
        payload.length = 0;
    }

    return valid;
}

RTCPMessageManager::~RTCPMessageManager()
{
    dispose();
}

size_t RTCPMessageManager::sendMessage(
        TCPChannelResource* channel,
        const CDRMessage_t& msg) const
{
    if (!alive())
    {
        return 0;
    }

    asio::error_code ec;
    size_t send = channel->send(nullptr, 0, msg.buffer, msg.length, ec);
    if (send != msg.length || ec)
    {
        EPROSIMA_LOG_WARNING(RTCP,
                "Bad sent size..." << send << " bytes of " << msg.length << " bytes: " << ec.message());
        send = 0;
    }

    //EPROSIMA_LOG_INFO(RTCP, "Sent " << send << " bytes");
    return send;
}

bool RTCPMessageManager::sendData(
        std::shared_ptr<TCPChannelResource>& channel,
        TCPCPMKind kind,
        const TCPTransactionId& transaction_id,
        const SerializedPayload_t* payload,
        const ResponseCode respCode)
{
    if (sendData(channel.get(), kind, transaction_id, payload, respCode))
    {
        return true;
    }

    if (TCPChannelResource::TCPConnectionType::TCP_CONNECT_TYPE == channel->tcp_connection_type() &&
            TCPChannelResource::eConnectionStatus::eDisconnected == channel->connection_status())
    {
        channel->set_all_ports_pending();
        channel->connect(channel);
    }

    return false;
}

bool RTCPMessageManager::sendData(
        TCPChannelResource* channel,
        TCPCPMKind kind,
        const TCPTransactionId& transaction_id,
        const SerializedPayload_t* payload,
        const ResponseCode respCode)
{
    if (!alive())
    {
        return 0;
    }

    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;
    CDRMessage_t msg(this->mTransport->get_configuration()->max_message_size());
    fastdds::rtps::CDRMessage::initCDRMsg(&msg);
    const ResponseCode* code = (respCode != RETCODE_VOID) ? &respCode : nullptr;

    fillHeaders(kind, transaction_id, ctrlHeader, header, payload, code);

    RTPSMessageCreator::addCustomContent(&msg, (octet*)&header, TCPHeader::size());
    RTPSMessageCreator::addCustomContent(&msg, (octet*)&ctrlHeader, TCPControlMsgHeader::size());
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

    return sendMessage(channel, msg) > 0;
}

uint32_t& RTCPMessageManager::addToCRC(
        uint32_t& crc,
        octet data)
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

void RTCPMessageManager::fillHeaders(
        TCPCPMKind kind,
        const TCPTransactionId& transaction_id,
        TCPControlMsgHeader& retCtrlHeader,
        TCPHeader& header,
        const SerializedPayload_t* payload,
        const ResponseCode* respCode)
{
    retCtrlHeader.kind(kind);
    retCtrlHeader.length() = static_cast<uint16_t>(TCPControlMsgHeader::size());
    retCtrlHeader.length() += static_cast<uint16_t>((payload != nullptr) ? (payload->length + 6) : 0);
    retCtrlHeader.length() += static_cast<uint16_t>((respCode != nullptr) ? 4 : 0);
    retCtrlHeader.transaction_id() = transaction_id;

    switch (kind)
    {
        case BIND_CONNECTION_REQUEST:
        case OPEN_LOGICAL_PORT_REQUEST:
        case CHECK_LOGICAL_PORT_REQUEST:
        case KEEP_ALIVE_REQUEST:
            retCtrlHeader.flags(false, true, true);
            addTransactionId(retCtrlHeader.transaction_id());
            break;
        case LOGICAL_PORT_IS_CLOSED_REQUEST:
        case BIND_CONNECTION_RESPONSE:
        case OPEN_LOGICAL_PORT_RESPONSE:
        case CHECK_LOGICAL_PORT_RESPONSE:
        case KEEP_ALIVE_RESPONSE:
            retCtrlHeader.flags(false, true, false);
            break;
        case UNBIND_CONNECTION_REQUEST:
            retCtrlHeader.flags(false, false, false);
            break;
    }

    retCtrlHeader.endianess(fastdds::rtps::DEFAULT_ENDIAN); // Override "false" endianess set on the switch
    header.logical_port = 0; // This is a control message
    header.length = static_cast<uint32_t>(retCtrlHeader.length() + TCPHeader::size());

    // Finally, calculate the CRC

    uint32_t crc = 0;
    if (alive() && mTransport->configuration()->calculate_crc)
    {
        octet* it = (octet*)&retCtrlHeader;
        for (size_t i = 0; i < TCPControlMsgHeader::size(); ++i)
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
    }
    header.crc = crc;
    //EPROSIMA_LOG_INFO(RTCP, "Send (CRC= " << header.crc << ")");

    // LOG
    /*
       switch (kind)
       {
       case BIND_CONNECTION_REQUEST:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [BIND_CONNECTION_REQUEST] Seq: " << retCtrlHeader.transaction_id());
        break;
       case OPEN_LOGICAL_PORT_REQUEST:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [OPEN_LOGICAL_PORT_REQUEST] Seq: " << retCtrlHeader.transaction_id());
        break;
       case CHECK_LOGICAL_PORT_REQUEST:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [CHECK_LOGICAL_PORT_REQUEST]: Seq: " << retCtrlHeader.transaction_id());
        break;
       case KEEP_ALIVE_REQUEST:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [KEEP_ALIVE_REQUEST] Seq: " << retCtrlHeader.transaction_id());
        break;
       case LOGICAL_PORT_IS_CLOSED_REQUEST:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [LOGICAL_PORT_IS_CLOSED_REQUEST] Seq: " << retCtrlHeader.transaction_id());
        break;
       case BIND_CONNECTION_RESPONSE:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [BIND_CONNECTION_RESPONSE] Seq: " << retCtrlHeader.transaction_id());
        break;
       case OPEN_LOGICAL_PORT_RESPONSE:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [OPEN_LOGICAL_PORT_RESPONSE] Seq: " << retCtrlHeader.transaction_id());
        break;
       case CHECK_LOGICAL_PORT_RESPONSE:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [CHECK_LOGICAL_PORT_RESPONSE] Seq: " << retCtrlHeader.transaction_id());
        break;
       case KEEP_ALIVE_RESPONSE:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [KEEP_ALIVE_RESPONSE] Seq: " << retCtrlHeader.transaction_id());
        break;
       case UNBIND_CONNECTION_REQUEST:
        EPROSIMA_LOG_INFO(RTCP_SEQ, "Send [UNBIND_CONNECTION_REQUEST] Seq: " << retCtrlHeader.transaction_id());
        break;
       }
     */
}

TCPTransactionId RTCPMessageManager::sendConnectionRequest(
        std::shared_ptr<TCPChannelResource>& channel)
{
    ConnectionRequest_t request;
    Locator locator;
    mTransport->endpoint_to_locator(channel->local_endpoint(), locator);

    mTransport->fill_local_physical_port(locator);

    if (locator.kind == LOCATOR_KIND_TCPv4)
    {
        auto config = mTransport->configuration();
        const TCPv4TransportDescriptor* pTCPv4Desc = static_cast<TCPv4TransportDescriptor*>(config);
        IPLocator::setWan(locator, pTCPv4Desc->wan_addr[0], pTCPv4Desc->wan_addr[1], pTCPv4Desc->wan_addr[2],
                pTCPv4Desc->wan_addr[3]);
    }
    request.protocolVersion(c_rtcpProtocolVersion);
    request.transportLocator(locator);

    SerializedPayload_t payload(static_cast<uint32_t>(ConnectionRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);

    EPROSIMA_LOG_INFO(RTCP_MSG, "Send [BIND_CONNECTION_REQUEST] PhysicalPort: " << IPLocator::getPhysicalPort(locator));
    //EPROSIMA_LOG_ERROR(DEBUG, "Sending Connection Request with locator: " << IPLocator::to_string(request.transportLocator()));
    channel->change_status(TCPChannelResource::eConnectionStatus::eWaitingForBindResponse);
    TCPTransactionId id = getTransactionId();
    bool success = sendData(channel, BIND_CONNECTION_REQUEST, id, &payload);
    if (!success)
    {
        EPROSIMA_LOG_ERROR(RTCP, "Failed sending Connection Request");
    }
    return id;
}

TCPTransactionId RTCPMessageManager::sendOpenLogicalPortRequest(
        TCPChannelResource* channel,
        uint16_t port)
{
    OpenLogicalPortRequest_t request;
    request.logicalPort(port);
    return sendOpenLogicalPortRequest(channel, request);
}

TCPTransactionId RTCPMessageManager::sendOpenLogicalPortRequest(
        TCPChannelResource* channel,
        OpenLogicalPortRequest_t& request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(OpenLogicalPortRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    EPROSIMA_LOG_INFO(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_REQUEST] LogicalPort: " << request.logicalPort());
    TCPTransactionId id = getTransactionId();
    sendData(channel, OPEN_LOGICAL_PORT_REQUEST, id, &payload);
    return id;
}

TCPTransactionId RTCPMessageManager::sendCheckLogicalPortsRequest(
        TCPChannelResource* channel,
        std::vector<uint16_t>& ports)
{
    CheckLogicalPortsRequest_t request;
    request.logicalPortsRange(ports);
    return sendCheckLogicalPortsRequest(channel, request);
}

TCPTransactionId RTCPMessageManager::sendCheckLogicalPortsRequest(
        TCPChannelResource* channel,
        CheckLogicalPortsRequest_t& request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(CheckLogicalPortsRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    EPROSIMA_LOG_INFO(RTCP_MSG, "Send [CHECK_LOGICAL_PORT_REQUEST]");
    TCPTransactionId id = getTransactionId();
    sendData(channel, CHECK_LOGICAL_PORT_REQUEST, id, &payload);
    return id;
}

TCPTransactionId RTCPMessageManager::sendKeepAliveRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        KeepAliveRequest_t& request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(KeepAliveRequest_t::getBufferCdrSerializedSize(request)));
    request.serialize(&payload);
    EPROSIMA_LOG_INFO(RTCP_MSG, "Send [KEEP_ALIVE_REQUEST]");
    TCPTransactionId id = getTransactionId();
    sendData(channel, KEEP_ALIVE_REQUEST, id, &payload, RETCODE_VOID);
    return id;
}

TCPTransactionId RTCPMessageManager::sendKeepAliveRequest(
        std::shared_ptr<TCPChannelResource>& channel)
{
    KeepAliveRequest_t request;
    request.locator(channel->locator());
    return sendKeepAliveRequest(channel, request);
}

TCPTransactionId RTCPMessageManager::sendLogicalPortIsClosedRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        LogicalPortIsClosedRequest_t& request)
{
    SerializedPayload_t payload(static_cast<uint32_t>(
                LogicalPortIsClosedRequest_t::getBufferCdrSerializedSize(request)));

    request.serialize(&payload);
    EPROSIMA_LOG_INFO(RTCP_MSG, "Send [LOGICAL_PORT_IS_CLOSED_REQUEST] LogicalPort: " << request.logicalPort());
    TCPTransactionId id = getTransactionId();
    sendData(channel, LOGICAL_PORT_IS_CLOSED_REQUEST, id, &payload);
    return id;
}

TCPTransactionId RTCPMessageManager::sendLogicalPortIsClosedRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        uint16_t port)
{
    LogicalPortIsClosedRequest_t request;
    request.logicalPort(port);
    return sendLogicalPortIsClosedRequest(channel, request);
}

TCPTransactionId RTCPMessageManager::sendUnbindConnectionRequest(
        std::shared_ptr<TCPChannelResource>& channel)
{
    EPROSIMA_LOG_INFO(RTCP_MSG, "Send [UNBIND_CONNECTION_REQUEST]");
    TCPTransactionId id = getTransactionId();
    sendData(channel, UNBIND_CONNECTION_REQUEST, id);
    return id;
}

ResponseCode RTCPMessageManager::processBindConnectionRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        const ConnectionRequest_t& request,
        const TCPTransactionId& transaction_id,
        Locator& localLocator)
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
        sendData(channel, BIND_CONNECTION_RESPONSE, transaction_id, &payload, RETCODE_INCOMPATIBLE_VERSION);
        EPROSIMA_LOG_WARNING(RTCP, "Rejected client due to INCOMPATIBLE_VERSION: Expected: " << c_rtcpProtocolVersion
                                                                                             << " but received "
                                                                                             << request.protocolVersion());
        return RETCODE_INCOMPATIBLE_VERSION;
    }

    //EPROSIMA_LOG_ERROR(DEBUG, "Receive Connection Request with locator: " << IPLocator::to_string(request.transportLocator())
    //    << " and will respond with our locator: " << response.locator());

    ResponseCode code = channel->process_bind_request(request.transportLocator());

    if (RETCODE_OK == code)
    {
        code = mTransport->bind_socket(channel);
    }

    sendData(channel, BIND_CONNECTION_RESPONSE, transaction_id, &payload, code);

    // Add pending logical ports to the channel
    mTransport->send_channel_pending_logical_ports(channel);

    return RETCODE_OK;
}

ResponseCode RTCPMessageManager::processOpenLogicalPortRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        const OpenLogicalPortRequest_t& request,
        const TCPTransactionId& transaction_id)
{
    // A server can send an OpenLogicalPortRequest to a client before the BindConnectionResponse is processed.
    if (!channel->connection_established() &&
            channel->connection_status_ != TCPChannelResource::eConnectionStatus::eWaitingForBindResponse)
    {
        EPROSIMA_LOG_ERROR(RTCP, "Trying to send [OPEN_LOGICAL_PORT_RESPONSE] without connection established.");
        sendData(channel, CHECK_LOGICAL_PORT_RESPONSE, transaction_id, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (request.logicalPort() == 0 || !mTransport->is_input_port_open(request.logicalPort()))
    {
        EPROSIMA_LOG_INFO(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_RESPONSE] Not found: " << request.logicalPort());
        sendData(channel, OPEN_LOGICAL_PORT_RESPONSE, transaction_id, nullptr, RETCODE_INVALID_PORT);
    }
    else
    {
        EPROSIMA_LOG_INFO(RTCP_MSG, "Send [OPEN_LOGICAL_PORT_RESPONSE] Found: " << request.logicalPort());
        sendData(channel, OPEN_LOGICAL_PORT_RESPONSE, transaction_id, nullptr, RETCODE_OK);
    }
    return RETCODE_OK;
}

void RTCPMessageManager::processCheckLogicalPortsRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        const CheckLogicalPortsRequest_t& request,
        const TCPTransactionId& transaction_id)
{
    CheckLogicalPortsResponse_t response;
    if (!channel->connection_established())
    {
        sendData(channel, CHECK_LOGICAL_PORT_RESPONSE, transaction_id, nullptr, RETCODE_SERVER_ERROR);
    }
    else
    {
        if (request.logicalPortsRange().empty())
        {
            EPROSIMA_LOG_WARNING(RTCP, "No available logical ports.");
        }
        else
        {
            for (uint16_t port : request.logicalPortsRange())
            {
                if (mTransport->is_input_port_open(port))
                {
                    if (port == 0)
                    {
                        EPROSIMA_LOG_INFO(RTCP, "FoundOpenedLogicalPort 0, but will not be considered");
                    }
                    EPROSIMA_LOG_INFO(RTCP, "FoundOpenedLogicalPort: " << port);
                    response.availableLogicalPorts().emplace_back(port);
                }
            }
        }

        SerializedPayload_t payload(static_cast<uint32_t>(
                    CheckLogicalPortsResponse_t::getBufferCdrSerializedSize(response)));
        response.serialize(&payload);
        sendData(channel, CHECK_LOGICAL_PORT_RESPONSE, transaction_id, &payload, RETCODE_OK);
    }
}

ResponseCode RTCPMessageManager::processKeepAliveRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        const KeepAliveRequest_t& request,
        const TCPTransactionId& transaction_id)
{
    if (!channel->connection_established())
    {
        sendData(channel, KEEP_ALIVE_RESPONSE, transaction_id, nullptr, RETCODE_SERVER_ERROR);
    }
    else if (IPLocator::getLogicalPort(channel->locator()) == IPLocator::getLogicalPort(request.locator()))
    {
        sendData(channel, KEEP_ALIVE_RESPONSE, transaction_id, nullptr, RETCODE_OK);
    }
    else
    {
        sendData(channel, KEEP_ALIVE_RESPONSE, transaction_id, nullptr, RETCODE_UNKNOWN_LOCATOR);
        return RETCODE_UNKNOWN_LOCATOR;
    }
    return RETCODE_OK;
}

void RTCPMessageManager::processLogicalPortIsClosedRequest(
        std::shared_ptr<TCPChannelResource>& channel,
        const LogicalPortIsClosedRequest_t& request,
        const TCPTransactionId& transaction_id)
{
    if (!channel->connection_established())
    {
        sendData(channel, CHECK_LOGICAL_PORT_RESPONSE, transaction_id, nullptr, RETCODE_SERVER_ERROR);
    }
    else
    {
        channel->set_logical_port_pending(request.logicalPort());
    }
}

ResponseCode RTCPMessageManager::processBindConnectionResponse(
        std::shared_ptr<TCPChannelResource>& channel,
        const BindConnectionResponse_t&,
        const TCPTransactionId& transaction_id)
{
    if (findTransactionId(transaction_id))
    {
        EPROSIMA_LOG_INFO(RTCP, "Connection established (Resp) (physical: "
                << IPLocator::getPhysicalPort(channel->locator()) << ")");
        channel->change_status(TCPChannelResource::eConnectionStatus::eEstablished, this);
        removeTransactionId(transaction_id);
        //EPROSIMA_LOG_ERROR(DEBUG, "Received Connection Response with locator: " << response.locator());
        return RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTCP,
                "Received BindConnectionResponse with an invalid transaction_id: " << transaction_id);
        return RETCODE_VOID;
    }
}

ResponseCode RTCPMessageManager::processCheckLogicalPortsResponse(
        std::shared_ptr<TCPChannelResource>& channel,
        const CheckLogicalPortsResponse_t& response,
        const TCPTransactionId& transaction_id)
{
    if (findTransactionId(transaction_id))
    {
        channel->process_check_logical_ports_response(transaction_id, response.availableLogicalPorts(), this);
        removeTransactionId(transaction_id);
        return RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTCP,
                "Received CheckLogicalPortsResponse with an invalid transaction_id: " << transaction_id);
        return RETCODE_VOID;
    }
}

ResponseCode RTCPMessageManager::processOpenLogicalPortResponse(
        std::shared_ptr<TCPChannelResource>& channel,
        ResponseCode respCode,
        const TCPTransactionId& transaction_id)
{
    if (findTransactionId(transaction_id))
    {
        switch (respCode)
        {
            case RETCODE_OK:
            {
                channel->add_logical_port_response(transaction_id, true, this);
            }
            break;
            case RETCODE_INVALID_PORT:
            {
                channel->add_logical_port_response(transaction_id, false, this);
            }
            break;
            default:
                EPROSIMA_LOG_WARNING(RTCP, "Received response for OpenLogicalPort with error code: "
                        << ((respCode == RETCODE_BAD_REQUEST) ? "BAD_REQUEST" : "SERVER_ERROR"));
                break;
        }
        removeTransactionId(transaction_id);
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTCP,
                "Received OpenLogicalPortResponse with an invalid transaction_id: " << transaction_id);
    }
    return RETCODE_OK;
}

ResponseCode RTCPMessageManager::processKeepAliveResponse(
        std::shared_ptr<TCPChannelResource>& /*channel*/,
        ResponseCode respCode,
        const TCPTransactionId& transaction_id)
{
    if (findTransactionId(transaction_id))
    {
        switch (respCode)
        {
            case RETCODE_OK:
                break;
            case RETCODE_UNKNOWN_LOCATOR:
                return RETCODE_UNKNOWN_LOCATOR;
            default:
                break;
        }
        removeTransactionId(transaction_id);
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTCP,
                "Received response for KeepAlive with an unexpected transaction_id: " << transaction_id);
    }
    return RETCODE_OK;
}

ResponseCode RTCPMessageManager::processRTCPMessage(
        std::shared_ptr<TCPChannelResource>& channel,
        octet* receive_buffer,
        size_t receivedSize,
        fastdds::rtps::Endianness_t msg_endian)
{
    if (receivedSize < TCPControlMsgHeader::size())
    {
        EPROSIMA_LOG_WARNING(RTCP, "Received message with size smaller than control header size: " << receivedSize);
        return RETCODE_BAD_REQUEST;
    }

    ResponseCode responseCode(RETCODE_OK);

    size_t dataSize = 0;

    TCPControlMsgHeader controlHeader = *(reinterpret_cast<TCPControlMsgHeader*>(receive_buffer));
    controlHeader.valid_endianness(msg_endian);
    // Control header size checking.
    const size_t header_size = TCPControlMsgHeader::size();
    const size_t control_length = static_cast<size_t>(controlHeader.length());
    bool valid = (control_length >= header_size) && (control_length == receivedSize);
    if (valid)
    {
        dataSize = control_length - header_size;
    }

    if (!valid)
    {
        sendData(channel, controlHeader.kind(), controlHeader.transaction_id(), nullptr, RETCODE_BAD_REQUEST);
        return RETCODE_OK;
    }

    const TCPTransactionId& transaction_id = controlHeader.transaction_id();

    SerializedPayload_t payload(0);
    CDRMessage_t message(0);
    message.init(&(receive_buffer[TCPControlMsgHeader::size()]), static_cast<uint32_t>(dataSize));
    message.msg_endian = msg_endian;
    message.length = static_cast<uint32_t>(dataSize);

    switch (controlHeader.kind())
    {
        case BIND_CONNECTION_REQUEST:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [BIND_CONNECTION_REQUEST] Seq: " << transaction_id);
            ConnectionRequest_t request;
            valid = readSerializedPayload(payload, &message);
            valid = valid && request.deserialize(&payload);
            if (valid)
            {
                Locator myLocator;
                endpoint_to_locator(channel->local_endpoint(), myLocator);

                EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [BIND_CONNECTION_REQUEST] "
                        << "LogicalPort: " << IPLocator::getLogicalPort(
                            request.transportLocator())
                        << ", Physical remote: " << IPLocator::getPhysicalPort(
                            request.transportLocator()));

                responseCode = processBindConnectionRequest(channel, request, transaction_id, myLocator);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to deserialize BindConnectionRequest");
                responseCode = RETCODE_BAD_REQUEST;
            }
        }
        break;

        case BIND_CONNECTION_RESPONSE:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [BIND_CONNECTION_RESPONSE] Seq: " << transaction_id);
            ResponseCode respCode;
            BindConnectionResponse_t response;
            valid = readResponseCode(respCode, &message);
            valid = valid && readSerializedPayload(payload, &message);
            valid = valid && response.deserialize(&payload);

            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [BIND_CONNECTION_RESPONSE] LogicalPort: " \
                        << IPLocator::getLogicalPort(response.locator()) << ", Physical remote: " \
                        << IPLocator::getPhysicalPort(response.locator()));

                if (respCode == RETCODE_OK || respCode == RETCODE_EXISTING_CONNECTION)
                {
                    std::unique_lock<std::recursive_mutex> scopedLock(channel->pending_logical_mutex_);
                    if (!channel->pending_logical_output_ports_.empty())
                    {
                        responseCode = processBindConnectionResponse(channel, response, transaction_id);
                    }
                }
                else
                {
                    // If the bind message fails, close the connection and try again.
                    if (respCode == RETCODE_INCOMPATIBLE_VERSION)
                    {
                        EPROSIMA_LOG_ERROR(RTCP, "Received RETCODE_INCOMPATIBLE_VERSION from server.");
                    }
                    responseCode = respCode;
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to deserialize BindConnectionResponse");
                // Ignore invalid responses
                responseCode = RETCODE_OK;
            }
        }
        break;

        case OPEN_LOGICAL_PORT_REQUEST:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [OPEN_LOGICAL_PORT_REQUEST] Seq: " << transaction_id);
            OpenLogicalPortRequest_t request;
            valid = readSerializedPayload(payload, &message);
            valid = valid && request.deserialize(&payload);
            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG,
                        "Receive [OPEN_LOGICAL_PORT_REQUEST] LogicalPort: " << request.logicalPort());
                responseCode = processOpenLogicalPortRequest(channel, request, transaction_id);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to deserialize OpenLogicalPortRequest");
                responseCode = RETCODE_BAD_REQUEST;
            }
        }
        break;

        case CHECK_LOGICAL_PORT_REQUEST:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [CHECK_LOGICAL_PORT_REQUEST] Seq: " << transaction_id);
            CheckLogicalPortsRequest_t request;
            valid = readSerializedPayload(payload, &message);
            valid = valid && request.deserialize(&payload);
            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [CHECK_LOGICAL_PORT_REQUEST]");
                processCheckLogicalPortsRequest(channel, request, transaction_id);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to deserialize CheckLogicalPortsRequest");
                responseCode = RETCODE_BAD_REQUEST;
            }
        }
        break;

        case CHECK_LOGICAL_PORT_RESPONSE:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [CHECK_LOGICAL_PORT_RESPONSE] Seq: " << transaction_id);
            ResponseCode respCode;
            CheckLogicalPortsResponse_t response;
            valid = readResponseCode(respCode, &message);
            valid = valid && readSerializedPayload(payload, &message);
            valid = valid && response.deserialize(&payload);
            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [CHECK_LOGICAL_PORT_RESPONSE]");
                processCheckLogicalPortsResponse(channel, response, transaction_id);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to deserialize CheckLogicalPortsResponse");
                // Ignore invalid responses
                responseCode = RETCODE_OK;
            }
        }
        break;

        case KEEP_ALIVE_REQUEST:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [KEEP_ALIVE_REQUEST] Seq: " << transaction_id);
            KeepAliveRequest_t request;
            valid = readSerializedPayload(payload, &message);
            valid = valid && request.deserialize(&payload);
            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [KEEP_ALIVE_REQUEST]");
                responseCode = processKeepAliveRequest(channel, request, transaction_id);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to deserialize KeepAliveRequest");
                responseCode = RETCODE_BAD_REQUEST;
            }
        }
        break;

        case LOGICAL_PORT_IS_CLOSED_REQUEST:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [LOGICAL_PORT_IS_CLOSED_REQUEST] Seq: " << transaction_id);
            LogicalPortIsClosedRequest_t request;
            valid = readSerializedPayload(payload, &message);
            valid = valid && request.deserialize(&payload);
            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG,
                        "Receive [LOGICAL_PORT_IS_CLOSED_REQUEST] LogicalPort: " << request.logicalPort());
                processLogicalPortIsClosedRequest(channel, request, transaction_id);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to deserialize LogicalPortIsClosedRequest");
                responseCode = RETCODE_BAD_REQUEST;
            }
        }
        break;

        case UNBIND_CONNECTION_REQUEST:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [UNBIND_CONNECTION_REQUEST] Seq:" << transaction_id);
            EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [UNBIND_CONNECTION_REQUEST]");
            if (alive())
            {
                mTransport->close_tcp_socket(channel);
            }
            responseCode = RETCODE_OK;
        }
        break;

        case OPEN_LOGICAL_PORT_RESPONSE:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [OPEN_LOGICAL_PORT_RESPONSE] Seq: " << transaction_id);
            ResponseCode respCode;
            valid = readResponseCode(respCode, &message);
            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [OPEN_LOGICAL_PORT_RESPONSE]");
                processOpenLogicalPortResponse(channel, respCode, transaction_id);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to read response code from OpenLogicalPortResponse");
                // Ignore invalid responses
                responseCode = RETCODE_OK;
            }
        }
        break;

        case KEEP_ALIVE_RESPONSE:
        {
            //EPROSIMA_LOG_INFO(RTCP_SEQ, "Receive [KEEP_ALIVE_RESPONSE] Seq: " << transaction_id);
            ResponseCode respCode;
            valid = readResponseCode(respCode, &message);
            if (valid)
            {
                EPROSIMA_LOG_INFO(RTCP_MSG, "Receive [KEEP_ALIVE_RESPONSE]");
                responseCode = processKeepAliveResponse(channel, respCode, transaction_id);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP, "Failed to read response code from KeepAliveResponse");
                // Ignore invalid responses
                responseCode = RETCODE_OK;
            }
        }
        break;

        default:
            EPROSIMA_LOG_INFO(RTCP, "Received message with unknown control kind: " << controlHeader.kind());
            responseCode = RETCODE_BAD_REQUEST;
            break;
    }

    // Avoid freeing memory not owned by payload
    payload.data = nullptr;
    payload.length = 0;

    if (responseCode == RETCODE_BAD_REQUEST)
    {
        sendData(channel, controlHeader.kind(), transaction_id, nullptr, RETCODE_BAD_REQUEST);
        responseCode = RETCODE_OK; // Ignore bad request errors after sending the response
    }

    return responseCode;
}

bool RTCPMessageManager::isCompatibleProtocol(
        const ProtocolVersion_t& protocol) const
{
    return protocol == c_rtcpProtocolVersion;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
