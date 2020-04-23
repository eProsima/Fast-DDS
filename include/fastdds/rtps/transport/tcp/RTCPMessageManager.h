// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#ifndef _FASTDDS_RTCP_MESSAGEMANAGER_H_
#define _FASTDDS_RTCP_MESSAGEMANAGER_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/all_common.h>
#include <fastdds/rtps/writer/StatelessWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/transport/tcp/TCPControlMessage.h>
#include <fastdds/rtps/transport/tcp/RTCPHeader.h>

#include<memory>
#include <atomic>

namespace eprosima {
namespace fastdds{
namespace rtps {

class TCPChannelResource;
class TCPTransportInterface;

const fastrtps::rtps::ProtocolVersion_t c_rtcpProtocolVersion = {1, 0};

/**
 * Class RTCPMessageManager, process the received TCP messages.
 * @ingroup MANAGEMENT_MODULE
 */
class RTCPMessageManager
{
    std::atomic<bool> alive_;
public:

    RTCPMessageManager(TCPTransportInterface* pTransport) : alive_(true), mTransport(pTransport) {}
    virtual ~RTCPMessageManager();

    /** @name Send RTCP Message Methods.
    * These methods create RTPS messages for different types
    */
    TCPTransactionId sendConnectionRequest(std::shared_ptr<TCPChannelResource>& channel);

    TCPTransactionId sendOpenLogicalPortRequest(
            TCPChannelResource* channel,
            OpenLogicalPortRequest_t &request);

    TCPTransactionId sendOpenLogicalPortRequest(
            TCPChannelResource* channel,
            uint16_t port);

    TCPTransactionId sendCheckLogicalPortsRequest(
            TCPChannelResource* channel,
            CheckLogicalPortsRequest_t &request);

    TCPTransactionId sendCheckLogicalPortsRequest(
            TCPChannelResource* channel,
            std::vector<uint16_t> &ports);

    TCPTransactionId sendKeepAliveRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            KeepAliveRequest_t &request);

    TCPTransactionId sendKeepAliveRequest(std::shared_ptr<TCPChannelResource>& channel);

    TCPTransactionId sendLogicalPortIsClosedRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            LogicalPortIsClosedRequest_t& request);

    TCPTransactionId sendLogicalPortIsClosedRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            uint16_t port);

    TCPTransactionId sendUnbindConnectionRequest(std::shared_ptr<TCPChannelResource>& channel);

    /** @name Process RTCP Message Methods.
    * These methods create RTPS messages for different types
    */
    ResponseCode processBindConnectionRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            const ConnectionRequest_t &request,
            const TCPTransactionId &transactionId,
            fastrtps::rtps::Locator_t &localLocator);

    virtual ResponseCode processOpenLogicalPortRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            const OpenLogicalPortRequest_t &request,
            const TCPTransactionId &transactionId);

    void processCheckLogicalPortsRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            const CheckLogicalPortsRequest_t &request,
            const TCPTransactionId &transactionId);

    ResponseCode processKeepAliveRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            const KeepAliveRequest_t &request,
            const TCPTransactionId &transactionId);

    void processLogicalPortIsClosedRequest(
            std::shared_ptr<TCPChannelResource>& channel,
            const LogicalPortIsClosedRequest_t &request,
            const TCPTransactionId &transactionId);

    ResponseCode processBindConnectionResponse(
            std::shared_ptr<TCPChannelResource>& channel,
            const BindConnectionResponse_t &response,
            const TCPTransactionId &transactionId);

    ResponseCode processCheckLogicalPortsResponse(
            std::shared_ptr<TCPChannelResource>& channel,
            const CheckLogicalPortsResponse_t &response,
            const TCPTransactionId &transactionId);

    ResponseCode processOpenLogicalPortResponse(
            std::shared_ptr<TCPChannelResource>& channel,
            ResponseCode respCode,
            const TCPTransactionId &transactionId);

    ResponseCode processKeepAliveResponse(
            std::shared_ptr<TCPChannelResource>& channel,
            ResponseCode respCode,
            const TCPTransactionId &transactionId);

    ResponseCode processRTCPMessage(
            std::shared_ptr<TCPChannelResource>& channel,
            fastrtps::rtps::octet* receive_buffer,
            size_t receivedSize);

    static uint32_t& addToCRC(uint32_t &crc, fastrtps::rtps::octet data);

    void dispose()
    {
        alive_.store(false);
    }

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

    //void prepareAndSendCheckLogicalPortsRequest(TCPChannelResource *p_channel_resource);

    size_t sendMessage(
            TCPChannelResource* channel,
            const fastrtps::rtps::CDRMessage_t &msg) const;

    bool sendData(
            std::shared_ptr<TCPChannelResource>& channel,
            TCPCPMKind kind,
            const TCPTransactionId &transactionId,
            const fastrtps::rtps::SerializedPayload_t *payload = nullptr,
            const ResponseCode respCode = RETCODE_VOID);

    bool sendData(
            TCPChannelResource* channel,
            TCPCPMKind kind,
            const TCPTransactionId &transactionId,
            const fastrtps::rtps::SerializedPayload_t *payload = nullptr,
            const ResponseCode respCode = RETCODE_VOID);

    void fillHeaders(TCPCPMKind kind, const TCPTransactionId &transactionId, TCPControlMsgHeader &retCtrlHeader,
        TCPHeader &header, const fastrtps::rtps::SerializedPayload_t *payload = nullptr, const ResponseCode *respCode = nullptr);

    bool isCompatibleProtocol(const fastrtps::rtps::ProtocolVersion_t &protocol) const;

    inline bool alive() const
    {
        return alive_.load();
    }
};
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTDDS_RTCP_MESSAGEMANAGER_H_ */
