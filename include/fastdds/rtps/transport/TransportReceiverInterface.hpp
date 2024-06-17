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

/**
 * @file TransportReceiverInterface.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__TRANSPORTRECEIVERINTERFACE_HPP
#define FASTDDS_RTPS_TRANSPORT__TRANSPORTRECEIVERINTERFACE_HPP

#include <fastdds/rtps/common/Locator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Interface against which to implement a data receiver, decoupled from transport internals.
 * @ingroup TRANSPORT_MODULE
 * */
class FASTDDS_EXPORTED_API TransportReceiverInterface
{
public:

    //! Destructor
    virtual ~TransportReceiverInterface() = default;

    /**
     * Method to be called by the transport when receiving data.
     * @param data Pointer to the received data.
     * @param size Number of bytes received.
     * @param local_locator Locator identifying the local endpoint.
     * @param remote_locator Locator identifying the remote endpoint.
     */
    virtual void OnDataReceived(
            const fastdds::rtps::octet* data,
            const uint32_t size,
            const Locator& local_locator,
            const Locator& remote_locator) = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__TRANSPORTRECEIVERINTERFACE_HPP
