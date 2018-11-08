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
 * @file RTCPMessageManager.h
 */



#ifndef TEST_RTCP_MESSAGEMANAGER_H_
#define TEST_RTCP_MESSAGEMANAGER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/transport/tcp/RTCPMessageManager.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class TCPTransportInterface;

/**
 * Class test_RTCPMessageManager, process the received TCP messages.
 * @ingroup MANAGEMENT_MODULE
 */
class test_RTCPMessageManager : public RTCPMessageManager
{
public:

    test_RTCPMessageManager(TCPTransportInterface* transport);
    virtual ~test_RTCPMessageManager();

    void SetInvalidTransactionPercentage(uint8_t value) { mInvalidTransactionPercentage = value; }
    void SetLogicalPortsBlocked(std::vector<uint16_t> list) { mLogicalPortsBlocked = list; }

    virtual ResponseCode processOpenLogicalPortRequest(TCPChannelResource *pChannelResource,
        const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId) override;

protected:
    TCPTransactionId getTransactionId();

private:
    std::vector<uint16_t> mLogicalPortsBlocked;
    uint8_t mInvalidTransactionPercentage;
};
} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* TEST_RTCP_MESSAGEMANAGER_H_ */
