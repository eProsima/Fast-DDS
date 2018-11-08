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
 * @file RTCPMessageManager.h
 */
#ifndef RTCP_MESSAGEMANAGER_H_
#define RTCP_MESSAGEMANAGER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../../qos/ParameterList.h"
#include <fastrtps/rtps/common/all_common.h>
#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/transport/tcp/TCPControlMessage.h>
#include <fastrtps/transport/tcp/RTCPHeader.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class TCPChannelResource;
class TCPTransportInterface;

const ProtocolVersion_t c_rtcpProtocolVersion = {1, 0};

/**
 * Class RTCPMessageManager, process the received TCP messages.
 * @ingroup MANAGEMENT_MODULE
 */
class RTCPMessageManager
{
public:

    RTCPMessageManager(TCPTransportInterface* pTransport) : mTransport(pTransport) {}
    virtual ~RTCPMessageManager();

    /** @name Send RTCP Message Methods.
    * These methods create RTPS messages for different types
    */
    TCPTransactionId sendConnectionRequest(TCPChannelResource *pChannelResource);

    TCPTransactionId sendOpenLogicalPortRequest(TCPChannelResource *pChannelResource,
        OpenLogicalPortRequest_t &request);

    TCPTransactionId sendOpenLogicalPortRequest(TCPChannelResource *pChannelResource, uint16_t port);

    TCPTransactionId sendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource,
        CheckLogicalPortsRequest_t &request);

    TCPTransactionId sendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource, std::vector<uint16_t> &ports);

    TCPTransactionId sendKeepAliveRequest(TCPChannelResource *pChannelResource, KeepAliveRequest_t &request);

    TCPTransactionId sendKeepAliveRequest(TCPChannelResource *pChannelResource);

    TCPTransactionId sendLogicalPortIsClosedRequest(TCPChannelResource *pChannelResource,
        LogicalPortIsClosedRequest_t &request);

    TCPTransactionId sendLogicalPortIsClosedRequest(TCPChannelResource *pChannelResource, uint16_t port);

    TCPTransactionId sendUnbindConnectionRequest(TCPChannelResource *pChannelResource);

    /** @name Process RTCP Message Methods.
    * These methods create RTPS messages for different types
    */
    ResponseCode processBindConnectionRequest(TCPChannelResource *pChannelResource, const ConnectionRequest_t &request,
        const TCPTransactionId &transactionId, Locator_t &localLocator);

    virtual ResponseCode processOpenLogicalPortRequest(TCPChannelResource *pChannelResource,
        const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId);

    void processCheckLogicalPortsRequest(TCPChannelResource *pChannelResource,
        const CheckLogicalPortsRequest_t &request, const TCPTransactionId &transactionId);

    ResponseCode processKeepAliveRequest(TCPChannelResource *pChannelResource, const KeepAliveRequest_t &request,
        const TCPTransactionId &transactionId);

    void processLogicalPortIsClosedRequest(TCPChannelResource *pChannelResource,
        const LogicalPortIsClosedRequest_t &request, const TCPTransactionId &transactionId);

    ResponseCode processBindConnectionResponse(TCPChannelResource *pChannelResource,
        const BindConnectionResponse_t &response, const TCPTransactionId &transactionId);

    ResponseCode processCheckLogicalPortsResponse(TCPChannelResource *pChannelResource,
        const CheckLogicalPortsResponse_t &response, const TCPTransactionId &transactionId);

    ResponseCode processOpenLogicalPortResponse(TCPChannelResource *pChannelResource, ResponseCode respCode,
        const TCPTransactionId &transactionId, Locator_t &remoteLocator);

    ResponseCode processKeepAliveResponse(TCPChannelResource *pChannelResource, ResponseCode respCode,
        const TCPTransactionId &transactionId);

    ResponseCode processRTCPMessage(TCPChannelResource *ChannelResource, octet* receiveBuffer, size_t receivedSize);

    static uint32_t& addToCRC(uint32_t &crc, octet data);

protected:
    TCPTransportInterface* mTransport;
    std::set<TCPTransactionId> mUnconfirmedTransactions;
    TCPTransactionId myTransId;
    std::recursive_mutex mutex;

    TCPTransactionId getTransactionId()
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mutex);
        return myTransId++;
    }

    bool findTransactionId(const TCPTransactionId& transactionId)
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mutex);
        auto it = mUnconfirmedTransactions.find(transactionId);
        return it != mUnconfirmedTransactions.end();
    }

    void addTransactionId(const TCPTransactionId& transactionId)
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mutex);
        mUnconfirmedTransactions.emplace(transactionId);
    }

    bool removeTransactionId(const TCPTransactionId& transactionId)
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mutex);
        auto it = mUnconfirmedTransactions.find(transactionId);
        if (it != mUnconfirmedTransactions.end())
        {
            mUnconfirmedTransactions.erase(it);
            return true;
        }
        else
        {
            return false;
        }
    }

    //void prepareAndSendCheckLogicalPortsRequest(TCPChannelResource *pChannelResource);

    size_t sendMessage(TCPChannelResource *pChannelResource, const CDRMessage_t &msg) const;

    bool sendData(TCPChannelResource *pChannelResource, TCPCPMKind kind, const TCPTransactionId &transactionId,
        const SerializedPayload_t *payload = nullptr, const ResponseCode respCode = RETCODE_VOID);

    void fillHeaders(TCPCPMKind kind, const TCPTransactionId &transactionId, TCPControlMsgHeader &retCtrlHeader,
        TCPHeader &header, const SerializedPayload_t *payload = nullptr, const ResponseCode *respCode = nullptr);

    bool isCompatibleProtocol(const ProtocolVersion_t &protocol) const;
};
} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* RTCP_MESSAGEMANAGER_H_ */
