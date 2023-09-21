// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>

#include <fastdds/rtps/transport/TransportDescriptorInterface.h>

using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

PortBasedTransportDescriptor::PortBasedTransportDescriptor(
        uint32_t maximumMessageSize,
        uint32_t maximumInitialPeersRange)
    : TransportDescriptorInterface(maximumMessageSize, maximumInitialPeersRange)
{
}

bool PortBasedTransportDescriptor::operator ==(
        const PortBasedTransportDescriptor& t) const
{
    return (TransportDescriptorInterface::operator ==(t) &&
           this->default_reception_threads_ ==  t.default_reception_threads() &&
           this->reception_threads_ == t.reception_threads());
}

const ThreadSettings& PortBasedTransportDescriptor::default_reception_threads() const
{
    return default_reception_threads_;
}

const PortBasedTransportDescriptor::ReceptionThreadsConfigMap& PortBasedTransportDescriptor::reception_threads() const
{
    return reception_threads_;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
