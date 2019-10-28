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

#ifndef UDPV6_TRANSPORT_H
#define UDPV6_TRANSPORT_H

#include <fastrtps/transport/UDPTransportInterface.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>

#include <fastdds/rtps/transport/UDPv6Transport.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

using UDPv6Transport = fastdds::rtps::UDPv6Transport;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
