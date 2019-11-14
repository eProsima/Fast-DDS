// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_TRANSPORT_RECEIVER_INTERFACE_H
#define _FASTDDS_TRANSPORT_RECEIVER_INTERFACE_H

#include <fastdds/rtps/common/Locator.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Interface against which to implement a data receiver, decoupled from transport internals.
 * @ingroup TRANSPORT_MODULE
 * */
class TransportReceiverInterface
{
public:

    virtual ~TransportReceiverInterface() = default;

    /**
     * Method to be called by the transport when receiving data.
     * @param data Pointer to the received data.
     * @param size Number of bytes received.
     * @param localLocator Locator identifying the local endpoint.
     * @param remote_locator Locator identifying the remote endpoint.
     */
    virtual void OnDataReceived(const fastrtps::rtps::octet* data, const uint32_t size,
        const fastrtps::rtps::Locator_t& localLocator, const fastrtps::rtps::Locator_t& remote_locator) = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TRANSPORT_RECEIVER_INTERFACE_H
