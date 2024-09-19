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

#ifndef FASTDDS_RTPS_NETWORK__NETWORKFACTORY_HPP
#define FASTDDS_RTPS_NETWORK__NETWORKFACTORY_HPP

#include <memory>
#include <vector>

#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

#include <rtps/messages/MessageReceiver.h>
#include <rtps/network/ReceiverResource.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantAttributes;

/**
 * Provides the Fast DDS library with abstract resources, which
 * in turn manage the SEND and RECEIVE operations over some transport.
 * Once a transport is registered, it becomes invisible to the library
 * and is abstracted away for good.
 * @ingroup NETWORK_MODULE.
 */
class NetworkFactory
{
public:

    NetworkFactory(
            const RTPSParticipantAttributes&)
    {
    }

    bool transform_remote_locator(
            const Locator_t& remote_locator,
            Locator_t& result_locator,
            const NetworkConfigSet_t&) const
    {
        result_locator = remote_locator;
        return true;
    }

    bool transform_remote_locator(
            const Locator_t& remote_locator,
            Locator_t& result_locator,
            const NetworkConfigSet_t&,
            bool) const
    {
        result_locator = remote_locator;
        return true;
    }

    bool is_locator_supported(
            const Locator_t&) const
    {
        return true;
    }

    bool is_locator_allowed(
            const Locator_t&) const
    {
        return true;
    }

    bool is_locator_remote_or_allowed(
            const Locator_t&) const
    {
        return true;
    }

    bool is_locator_remote_or_allowed(
            const Locator_t&,
            bool) const
    {
        return true;
    }

    bool is_locator_reachable(
            const Locator_t&)
    {
        return true;
    }

    uint32_t get_min_send_buffer_size()
    {
        return 65536;
    }

    bool is_local_locator(
            const Locator_t&) const
    {
        return true;
    }

    std::vector<fastdds::rtps::TransportNetmaskFilterInfo> netmask_filter_info() const
    {
        return {};
    }

    NetworkConfigSet_t network_configuration() const
    {
        return NetworkConfigSet_t{};
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_NETWORK__NETWORKFACTORY_HPP
