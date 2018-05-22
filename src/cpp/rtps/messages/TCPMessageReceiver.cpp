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
 * @file TCPMessageReceiver.cpp
 *
 */

#include <fastrtps/transport/tcp/RTCPHeader.h>
#include <fastrtps/rtps/messages/TCPMessageReceiver.h>
#include <fastrtps/transport/SocketInfo.h>
#include <fastrtps/log/Log.h>

#define IDSTRING "(ID:" << std::this_thread::get_id() <<") "<<

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

TCPMessageReceiver::~TCPMessageReceiver()
{
}

bool TCPMessageReceiver::sendData(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader,
        const octet *data, const uint32_t size)
{
    CDRMessage_t msg;
    CDRMessage::initCDRMsg(&msg);
    RTPSMessageCreator::addCustomContent(&msg, header.getAddress(), TCPHeader::GetSize());
    RTPSMessageCreator::addCustomContent(&msg, (octet*)(&ctrlHeader), sizeof(TCPControlMsgHeader));
    RTPSMessageCreator::addCustomContent(&msg, data, size);

    return transport->Send(pSocketInfo, msg.buffer, msg.length) > 0;
    //return pSocketInfo->getSocket()->write_some(asio::buffer(msg.buffer, msg.length)) > 0;
}

bool TCPMessageReceiver::sendData(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader,
        const octet *data, const uint32_t size, const ResponseCode respCode)
{
    CDRMessage_t msg;
    CDRMessage::initCDRMsg(&msg);
    RTPSMessageCreator::addCustomContent(&msg, header.getAddress(), TCPHeader::GetSize());
    RTPSMessageCreator::addCustomContent(&msg, (octet*)(&ctrlHeader), sizeof(TCPControlMsgHeader));
    RTPSMessageCreator::addCustomContent(&msg, (octet*)(&respCode), 4); // uint32_t
    RTPSMessageCreator::addCustomContent(&msg, data, size);

    return transport->Send(pSocketInfo, msg.buffer, msg.length) > 0;
    //return pSocketInfo->getSocket()->write_some(asio::buffer(msg.buffer, msg.length)) > 0;
}

bool TCPMessageReceiver::sendData(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader,
        const ResponseCode respCode)
{
    CDRMessage_t msg;
    CDRMessage::initCDRMsg(&msg);
    RTPSMessageCreator::addCustomContent(&msg, header.getAddress(), TCPHeader::GetSize());
    RTPSMessageCreator::addCustomContent(&msg, (octet*)(&ctrlHeader), sizeof(TCPControlMsgHeader));
    RTPSMessageCreator::addCustomContent(&msg, (octet*)(&respCode), 4); // uint32_t

    return transport->Send(pSocketInfo, msg.buffer, msg.length) > 0;
    //return pSocketInfo->getSocket()->write_some(asio::buffer(msg.buffer, msg.length)) > 0;
}

bool TCPMessageReceiver::sendData(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader)
{
    CDRMessage_t msg;
    CDRMessage::initCDRMsg(&msg);
    RTPSMessageCreator::addCustomContent(&msg, header.getAddress(), TCPHeader::GetSize());
    RTPSMessageCreator::addCustomContent(&msg, (octet*)(&ctrlHeader), sizeof(TCPControlMsgHeader));

    return transport->Send(pSocketInfo, msg.buffer, msg.length) > 0;
    //return pSocketInfo->getSocket()->write_some(asio::buffer(msg.buffer, msg.length)) > 0;
}

void TCPMessageReceiver::sendConnectionRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const Locator_t &transportLocator)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;
    ConnectionRequest_t request;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + request.GetSize();
    ctrlHeader.kind = BIND_CONNECTION_REQUEST;
    ctrlHeader.setFlags(false, true, true);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();
    request.transportLocator(transportLocator);

    sendData(pSocketInfo, header, ctrlHeader, (octet*)&request, request.GetSize());

    pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eWaitingForBindResponse);
}

void TCPMessageReceiver::sendOpenLogicalPortRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, uint16_t port)
{
    OpenLogicalPortRequest_t request;
    request.logicalPort(port);
    sendOpenLogicalPortRequest(pSocketInfo, request);
}

void TCPMessageReceiver::sendOpenLogicalPortRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, OpenLogicalPortRequest_t &request)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + request.GetSize();
    ctrlHeader.kind = OPEN_LOGICAL_PORT_REQUEST;
    ctrlHeader.setFlags(false, true, true);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();

    sendData(pSocketInfo, header, ctrlHeader, (octet*)&request, request.GetSize());
}

void TCPMessageReceiver::sendCheckLogicalPortsRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, std::vector<uint16_t> &ports)
{
    CheckLogicalPortsRequest_t request;
    request.logicalPortsRange(ports);
    sendCheckLogicalPortsRequest(pSocketInfo, request);
}

void TCPMessageReceiver::sendCheckLogicalPortsRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, CheckLogicalPortsRequest_t &request)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + request.GetSize();
    ctrlHeader.kind = CHECK_LOGICAL_PORT_REQUEST;
    ctrlHeader.setFlags(false, true, true);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();

    sendData(pSocketInfo, header, ctrlHeader, (octet*)&request, request.GetSize());
}

void TCPMessageReceiver::sendKeepAliveRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, KeepAliveRequest_t &request)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + request.GetSize();
    ctrlHeader.kind = KEEP_ALIVE_REQUEST;
    ctrlHeader.setFlags(false, true, true);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();
    request.locator(pSocketInfo->m_locator);

    sendData(pSocketInfo, header, ctrlHeader, (octet*)&request, request.GetSize());
}

void TCPMessageReceiver::sendKeepAliveRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo)
{
    KeepAliveRequest_t request;
    request.locator(pSocketInfo->m_locator);
    sendKeepAliveRequest(pSocketInfo, request);
}

void TCPMessageReceiver::sendLogicalPortIsClosedRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        LogicalPortIsClosedRequest_t &request)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + request.GetSize();
    ctrlHeader.kind = LOGICAL_PORT_IS_CLOSED_REQUEST;
    ctrlHeader.setFlags(false, true, false);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();

    sendData(pSocketInfo, header, ctrlHeader, (octet*)&request, request.GetSize());
}

void TCPMessageReceiver::sendLogicalPortIsClosedRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        uint16_t port)
{
    LogicalPortIsClosedRequest_t request;
    request.logicalPort(port);
    sendLogicalPortIsClosedRequest(pSocketInfo, request);
}

void TCPMessageReceiver::sendUnbindConnectionRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize();
    ctrlHeader.kind = UNBIND_CONNECTION_REQUEST;
    ctrlHeader.setFlags(false, false, false);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();

    sendData(pSocketInfo, header, ctrlHeader);
}

void TCPMessageReceiver::processConnectionRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const ConnectionRequest_t &/*request*/, Locator_t &localLocator)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;
    BindConnectionResponse_t response;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + response.GetSize() + 4; // RetCode
    ctrlHeader.kind = BIND_CONNECTION_RESPONSE;
    ctrlHeader.setFlags(false, true, false);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();
    response.locator(localLocator);
 
    // TODO More options!
    if (pSocketInfo->mConnectionStatus == TCPSocketInfo::eConnectionStatus::eWaitingForBind)
    {

        sendData(pSocketInfo, header, ctrlHeader, (octet*)&response, response.GetSize(), RETCODE_OK);
        pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eEstablished);
    }
    else
    {
        if (pSocketInfo->mConnectionStatus == TCPSocketInfo::eConnectionStatus::eEstablished)
        {
            sendData(pSocketInfo, header, ctrlHeader, (octet*)&response, response.GetSize(), RETCODE_EXISTING_CONNECTION);
        }
        else
        {
            sendData(pSocketInfo, header, ctrlHeader, (octet*)&response, response.GetSize(), RETCODE_SERVER_ERROR);
        }
    }
}

void TCPMessageReceiver::processOpenLogicalPortRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const OpenLogicalPortRequest_t &request)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + 4; // RetCode
    ctrlHeader.kind = OPEN_LOGICAL_PORT_RESPONSE;
    ctrlHeader.setFlags(false, true, false);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();
 
    // TODO More options!
    for (uint16_t port : pSocketInfo->mLogicalInputPorts)
    {
        if (port == request.logicalPort())
        {
            sendData(pSocketInfo, header, ctrlHeader, RETCODE_OK);
            return;
        }
    }
    sendData(pSocketInfo, header, ctrlHeader, RETCODE_INVALID_PORT);
}

void TCPMessageReceiver::processCheckLogicalPortsRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const CheckLogicalPortsRequest_t &request)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;
    CheckLogicalPortsResponse_t response;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.kind = OPEN_LOGICAL_PORT_RESPONSE;
    ctrlHeader.setFlags(false, true, false);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();

    for (uint16_t port : request.logicalPortsRange())
    {
        if (std::find(pSocketInfo->mLogicalInputPorts.begin(), 
                pSocketInfo->mLogicalInputPorts.end(), port) 
                != pSocketInfo->mLogicalInputPorts.end())
        {
            response.availableLogicalPorts().emplace_back(port);
        }
    }

    ctrlHeader.length = TCPControlMsgHeader::GetSize() + response.GetSize() + 4; // RetCode
    sendData(pSocketInfo, header, ctrlHeader, (octet*)&response, response.GetSize(), RETCODE_OK);
}

void TCPMessageReceiver::processKeepAliveRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const KeepAliveRequest_t &request)
{
    TCPHeader header;
    TCPControlMsgHeader ctrlHeader;

    header.logicalPort = 0; // This is a control message
    ctrlHeader.length = TCPControlMsgHeader::GetSize() + 4; // RetCode
    ctrlHeader.kind = KEEP_ALIVE_RESPONSE;
    ctrlHeader.setFlags(false, true, false);
    ctrlHeader.setEndianess(DEFAULT_ENDIAN);
    header.length = ctrlHeader.length + TCPHeader::GetSize();
 
    if (pSocketInfo->m_locator.get_logical_port() == request.locator().get_logical_port())
    {
        sendData(pSocketInfo, header, ctrlHeader, RETCODE_OK);
        return;
    }
    sendData(pSocketInfo, header, ctrlHeader, RETCODE_UNKNOWN_LOCATOR);
}

void TCPMessageReceiver::processLogicalPortIsClosedRequest(std::shared_ptr<TCPSocketInfo> &/*pSocketInfo*/, 
        const LogicalPortIsClosedRequest_t &/*request*/)
{
    // TODO?
}

void TCPMessageReceiver::processBindConnectionResponse(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const BindConnectionResponse_t &/*response*/, const uint16_t logicalPort)
{
    OpenLogicalPortRequest_t request;
    request.logicalPort(logicalPort);
    sendOpenLogicalPortRequest(pSocketInfo, request);
}

void TCPMessageReceiver::processCheckLogicalPortsResponse(std::shared_ptr<TCPSocketInfo> &/*pSocketInfo*/, 
        const CheckLogicalPortsResponse_t &/*response*/)
{
    // TODO? Not here I guess...
}


} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
