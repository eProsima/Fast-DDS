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

#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <fastrtps/transport/TransportDescriptorInterface.h>
#include <fastrtps/transport/TransportReceiverInterface.h>

#include <fastdds/rtps/transport/TransportInterface.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

using SendResourceList = fastdds::rtps::SendResourceList;
using TransportInterface = fastdds::rtps::TransportInterface;

static const uint32_t s_maximumMessageSize = fastdds::rtps::s_maximumMessageSize;
static const uint32_t s_maximumInitialPeersRange = fastdds::rtps::s_maximumInitialPeersRange;
static const uint32_t s_minimumSocketBuffer = fastdds::rtps::s_minimumSocketBuffer;
static const std::string s_IPv4AddressAny = fastdds::rtps::s_IPv4AddressAny;
static const std::string s_IPv6AddressAny = fastdds::rtps::s_IPv6AddressAny;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
