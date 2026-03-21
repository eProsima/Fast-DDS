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

#include "./DatagramInjectionTransport.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

DatagramInjectionTransportDescriptor::DatagramInjectionTransportDescriptor(
        std::shared_ptr<TransportDescriptorInterface> low_level)
    : ChainingTransportDescriptor(low_level)
{
}

TransportInterface* DatagramInjectionTransportDescriptor::create_transport() const
{
    return new DatagramInjectionTransport(const_cast<DatagramInjectionTransportDescriptor*>(this));
}

void DatagramInjectionTransportDescriptor::add_receiver(
        TransportReceiverInterface* receiver_interface)
{
    std::lock_guard<std::mutex> guard(mtx_);
    receivers_.insert(receiver_interface);
}

std::set<TransportReceiverInterface*> DatagramInjectionTransportDescriptor::get_receivers()
{
    std::lock_guard<std::mutex> guard(mtx_);
    return receivers_;
}

void DatagramInjectionTransportDescriptor::update_send_resource_list(
        const SendResourceList& send_resource_list)
{
    std::lock_guard<std::mutex> guard(mtx_);

    send_resource_list_.clear();
    for (const auto& resource : send_resource_list)
    {
        send_resource_list_.insert(resource.get());
    }
}

std::set<SenderResource*> DatagramInjectionTransportDescriptor::get_send_resource_list()
{
    std::lock_guard<std::mutex> guard(mtx_);
    return send_resource_list_;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
