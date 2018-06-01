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


#include <fastrtps/transport/tcp/test_RTCPMessageManager.h>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

test_RTCPMessageManager::test_RTCPMessageManager(TCPv4Transport* tcpv4_transport)
    : RTCPMessageManager(tcpv4_transport)
    , mInvalidTransactionPercentage(0)
{
}

test_RTCPMessageManager::~test_RTCPMessageManager()
{
}

TCPTransactionId test_RTCPMessageManager::getTransactionId()
{
    if (mInvalidTransactionPercentage <= (rand() % 100))
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mutex);
        return myTransId++;
    }
    return TCPTransactionId();
}

void test_RTCPMessageManager::processOpenLogicalPortRequest(TCPSocketInfo *pSocketInfo,
    const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId)
{
    if (std::find(mLogicalPortsBlocked.begin(), mLogicalPortsBlocked.end(), request.logicalPort()) !=
        mLogicalPortsBlocked.end())
    {
        sendData(pSocketInfo, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_INVALID_PORT);
    }
    else
    {
        if (std::find(pSocketInfo->mLogicalInputPorts.begin(), pSocketInfo->mLogicalInputPorts.end(),
            request.logicalPort()) != pSocketInfo->mLogicalInputPorts.end())
        {
            sendData(pSocketInfo, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_OK);
            return;
        }
        sendData(pSocketInfo, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_INVALID_PORT);
    }
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
