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

#include <fastrtps/rtps/messages/TCPMessageReceiver.h>
#include <fastrtps/transport/SocketInfo.h>
#include <fastrtps/log/Log.h>

#define IDSTRING "(ID:" << std::this_thread::get_id() <<") "<<

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

TCPMessageReceiver::TCPMessageReceiver()
{
}

TCPMessageReceiver::~TCPMessageReceiver()
{
}

bool TCPMessageReceiver::CheckTCPControlMessage(TCPSocketInfo* pSocketInfo, octet* buffer, uint32_t bufferSize)
{
    //TODO: Check OpenLogicalPortResponse to remove the pending logical ports from socketInfo

    return false;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
