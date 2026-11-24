// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef MOCK_RECEIVER_STUFF_H
#define MOCK_RECEIVER_STUFF_H

#include <atomic>
#include <functional>
#include <mutex>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Receives the data of the transport channel it opens, and hands it to the MockMessageReceiver created with
 * CreateMessageReceiver().
 */
class MockMessageReceiver
{
public:

    /**
     * Stores the received buffer and calls the callback, if any.
     * @param buffer Received data.
     */
    void processCDRMsg(
            octet* buffer);

    /**
     * Sets the callback called on every received message.
     * @param cb Callback, or nullptr to remove it.
     */
    void setCallback(
            std::function<void()> cb);

    std::atomic<octet*> data;
    std::mutex callback_mutex;
    std::function<void()> callback;
};

class MockReceiverResource : public TransportReceiverInterface
{
public:

    /**
     * Opens an input channel on the transport for the given locator.
     * @param transport Transport where the input channel is opened.
     * @param locator Locator of the input channel.
     */
    MockReceiverResource(
            TransportInterface& transport,
            const Locator_t& locator);

    /**
     * Closes the input channel, if it was opened.
     */
    ~MockReceiverResource();

    /**
     * @return Whether the input channel could be opened.
     */
    bool is_valid() const
    {
        return valid_;
    }

    void OnDataReceived(
            const octet* data,
            const uint32_t size,
            const Locator_t& local_locator,
            const Locator_t& remote_locator) override;

    /**
     * Creates, on the first call, the MockMessageReceiver which receives the data.
     * @return The MockMessageReceiver of this resource.
     */
    MockMessageReceiver* CreateMessageReceiver();

    std::atomic<MockMessageReceiver*> msg_receiver;

private:

    std::function<void()> cleanup_;
    bool valid_ = false;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //MOCK_RECEIVER_STUFF_H
