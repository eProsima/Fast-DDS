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

/**
 * @file SenderResource.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__SENDERRESOURCE_HPP
#define FASTDDS_RTPS_TRANSPORT__SENDERRESOURCE_HPP

#include <fastdds/rtps/common/Types.hpp>

#include <functional>
#include <vector>
#include <list>
#include <chrono>

#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorsIterator.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class MessageReceiver;

/**
 * RAII object that encapsulates the Send operation over one chanel in an unknown transport.
 * A Sender resource is always univocally associated to a transport channel; the
 * act of constructing a Sender Resource opens the channel and its destruction
 * closes it.
 * @ingroup NETWORK_MODULE
 */
class SenderResource
{
public:

    using NetworkBuffer = eprosima::fastdds::rtps::NetworkBuffer;

    /**
     * Sends to a destination locator, through the channel managed by this resource.
     * @param buffers Vector of buffers to send.
     * @param total_bytes Length of all buffers to be sent. Will be used as a boundary for
     * the previous parameter.
     * @param destination_locators_begin destination endpoint Locators iterator begin.
     * @param destination_locators_end destination endpoint Locators iterator end.
     * @param max_blocking_time_point If transport supports it then it will use it as maximum blocking time.
     * @return Success of the send operation.
     */
    bool send(
            const std::vector<NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            LocatorsIterator* destination_locators_begin,
            LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& max_blocking_time_point)
    {
        return send_buffers_lambda_(buffers, total_bytes, destination_locators_begin, destination_locators_end,
                       max_blocking_time_point);
    }

    /**
     * Resources can only be transfered through move semantics. Copy, assignment, and
     * construction outside of the factory are forbidden.
     */
    SenderResource(
            SenderResource&& rValueResource)
    {
        clean_up.swap(rValueResource.clean_up);
        send_buffers_lambda_.swap(rValueResource.send_buffers_lambda_);
    }

    virtual ~SenderResource() = default;

    int32_t kind() const
    {
        return transport_kind_;
    }

    /**
     * Add locators representing the local endpoints managed by this sender resource.
     *
     * @param [in,out] locators  List where locators will be added.
     */
    virtual void add_locators_to_list(
            LocatorList_t& locators) const
    {
        (void)locators;
    }

protected:

    SenderResource(
            int32_t transport_kind)
        : transport_kind_(transport_kind)
    {
    }

    int32_t transport_kind_;

    std::function<void()> clean_up;

    std::function<bool(
                const std::vector<NetworkBuffer>&,
                uint32_t,
                LocatorsIterator* destination_locators_begin,
                LocatorsIterator* destination_locators_end,
                const std::chrono::steady_clock::time_point&)> send_buffers_lambda_;

private:

    SenderResource()                                 = delete;
    SenderResource(
            const SenderResource&)            = delete;
    SenderResource& operator =(
            const SenderResource&) = delete;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__SENDERRESOURCE_HPP
