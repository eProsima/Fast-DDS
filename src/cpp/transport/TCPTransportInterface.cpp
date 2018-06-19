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

#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/tcp/RTCPMessageManager.h>
#include <fastrtps/transport/timedevent/CleanTCPSocketsEvent.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include "asio.hpp"
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/transport/TCPChannelResource.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static const int s_default_keep_alive_frequency = 50000; // 50 SECONDS
static const int s_default_keep_alive_timeout = 10000; // 10 SECONDS
static const int s_clean_deleted_sockets_pool_timeout = 100; // 100 MILLISECONDS
static const int s_default_tcp_negotitation_timeout = 5000; // 5 Seconds

TCPTransportDescriptor::TCPTransportDescriptor()
    : TransportDescriptorInterface(s_maximumMessageSize)
    , keep_alive_frequency_ms(s_default_keep_alive_frequency)
    , keep_alive_timeout_ms(s_default_keep_alive_timeout)
    , max_logical_port(100)
    , logical_port_range(20)
    , logical_port_increment(2)
    , metadata_logical_port(0)
	, tcp_negotiation_timeout(s_default_tcp_negotitation_timeout)
	, wait_for_tcp_negotiation(true)
{
}

TCPTransportDescriptor::TCPTransportDescriptor(const TCPTransportDescriptor& t)
    : TransportDescriptorInterface(t)
    , listening_ports(t.listening_ports)
    , keep_alive_frequency_ms(t.keep_alive_frequency_ms)
    , keep_alive_timeout_ms(t.keep_alive_timeout_ms)
    , max_logical_port(t.max_logical_port)
    , logical_port_range(t.logical_port_range)
    , logical_port_increment(t.logical_port_increment)
    , metadata_logical_port(t.metadata_logical_port)
	, tcp_negotiation_timeout(t.tcp_negotiation_timeout)
	, wait_for_tcp_negotiation(t.wait_for_tcp_negotiation)
{
}

TCPTransportInterface::TCPTransportInterface()
{
}

TCPTransportInterface::~TCPTransportInterface()
{
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
