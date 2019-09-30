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

#ifndef _FASTDDS_RTPS_NETWORK_FACTORY_HPP
#define _FASTDDS_RTPS_NETWORK_FACTORY_HPP

#include <fastrtps/transport/TransportInterface.h>
#include <fastrtps/rtps/common/LocatorSelector.hpp>
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <vector>
#include <memory>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSParticipantAttributes;

/**
 * Provides the FastRTPS library with abstract resources, which
 * in turn manage the SEND and RECEIVE operations over some transport.
 * Once a transport is registered, it becomes invisible to the library
 * and is abstracted away for good.
 * @ingroup NETWORK_MODULE.
 */
class NetworkFactory
{
    public:

        NetworkFactory() {}

        bool transform_remote_locator(
                const Locator_t& remote_locator,
                Locator_t& result_locator) const
        {
            result_locator = remote_locator;
            return true;
        }

        uint32_t get_min_send_buffer_size() { return 65536; }
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
