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

#include <fastdds/rtps/transport/TransportDescriptorInterface.hpp>

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

const ThreadSettings& PortBasedTransportDescriptor::get_thread_config_for_port(
        uint32_t port) const
{
    auto search = reception_threads_.find(port);
    if (search != reception_threads_.end())
    {
        return search->second;
    }
    return default_reception_threads_;
}

bool PortBasedTransportDescriptor::set_thread_config_for_port(
        const uint32_t& port,
        const ThreadSettings& thread_settings)
{
    reception_threads_[port] = thread_settings;
    return true;
}

const ThreadSettings& PortBasedTransportDescriptor::default_reception_threads() const
{
    return default_reception_threads_;
}

void PortBasedTransportDescriptor::default_reception_threads(
        const ThreadSettings& default_reception_threads)
{
    default_reception_threads_ = default_reception_threads;
}

const PortBasedTransportDescriptor::ReceptionThreadsConfigMap& PortBasedTransportDescriptor::reception_threads() const
{
    return reception_threads_;
}

bool PortBasedTransportDescriptor::reception_threads(
        const ReceptionThreadsConfigMap& reception_threads)
{
    reception_threads_ = reception_threads;
    return true;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
