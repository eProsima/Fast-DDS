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
#include <fastrtps/transport/TCPv4Transport.h>

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

bool test_RTCPMessageManager::processOpenLogicalPortRequest(TCPChannelResource *pChannelResource,
    const OpenLogicalPortRequest_t &request, const TCPTransactionId &transactionId)
{
    if (std::find(mLogicalPortsBlocked.begin(), mLogicalPortsBlocked.end(), request.logicalPort()) !=
        mLogicalPortsBlocked.end())
    {
        sendData(pChannelResource, OPEN_LOGICAL_PORT_RESPONSE, transactionId, nullptr, RETCODE_INVALID_PORT);
    }
    else
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
    }
    return true;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
