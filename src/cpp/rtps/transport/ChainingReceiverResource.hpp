// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _RTPS_TRANSPORT_CHAININGRECEIVERRESOURCE_HPP_
#define _RTPS_TRANSPORT_CHAININGRECEIVERRESOURCE_HPP_

#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ChainingTransport;

class ChainingReceiverResource : public TransportReceiverInterface
{
public:

    ChainingReceiverResource(
            ChainingTransport& transport,
            TransportReceiverInterface* low_receiver_resource)
        : transport_(transport)
        , low_receiver_resource_(low_receiver_resource)
    {
    }

    virtual ~ChainingReceiverResource() = default;

    /**
     * Method to be called by the transport when receiving data.
     * @param data Pointer to the received data.
     * @param size Number of bytes received.
     * @param localLocator Locator identifying the local endpoint.
     * @param remote_locator Locator identifying the remote endpoint.
     */
    void OnDataReceived(
            const octet* data,
            const uint32_t size,
            const Locator_t& local_locator,
            const Locator_t& remote_locator) override
    {
        transport_.receive(low_receiver_resource_, data, size, local_locator, remote_locator);
    }

private:

    ChainingTransport& transport_;
    TransportReceiverInterface* low_receiver_resource_ = nullptr;
};


} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_TRANSPORT_CHAININGRECEIVERRESOURCE_HPP_
