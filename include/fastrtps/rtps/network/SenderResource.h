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

#ifndef SENDER_RESOURCE_H
#define SENDER_RESOURCE_H

#include <functional>
#include <vector>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSParticipantImpl;
class MessageReceiver;
class ChannelResource;
class TransportInterface;
struct Locator_t;

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

    /**
     * Sends to a destination locator, through the channel managed by this resource.
     * @param data Raw data slice to be sent.
     * @param dataLength Length of the data to be sent. Will be used as a boundary for
     * the previous parameter.
     * @param destination_locator Locator describing the destination endpoint.
     * @return Success of the send operation.
     */
    bool send(const octet* data, uint32_t dataLength, const Locator_t& destination_locator)
    {
        bool returned_value = false;

        if (send_lambda_)
        {
            returned_value = send_lambda_(data, dataLength, destination_locator);
        }

        return returned_value;
    }

    /**
     * Resources can only be transfered through move semantics. Copy, assignment, and
     * construction outside of the factory are forbidden.
     */
    SenderResource(SenderResource&& rValueResource)
    {
        clean_up.swap(rValueResource.clean_up);
        send_lambda_.swap(rValueResource.send_lambda_);
    }

    virtual ~SenderResource() = default;

    int32_t kind() const { return transport_kind_; }

protected:

    SenderResource(int32_t transport_kind) : transport_kind_(transport_kind) {}

    int32_t transport_kind_;

    std::function<void()> clean_up;
    std::function<bool(const octet*, uint32_t, const Locator_t&)> send_lambda_;

private:

    SenderResource()                                 = delete;
    SenderResource(const SenderResource&)            = delete;
    SenderResource& operator=(const SenderResource&) = delete;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
