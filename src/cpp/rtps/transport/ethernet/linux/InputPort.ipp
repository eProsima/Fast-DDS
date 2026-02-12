/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>

#include <arpa/inet.h>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

#include <fastdds/rtps/transport/ethernet/EthernetLocator.hpp>

#include <rtps/transport/ethernet/EthernetPacket.hpp>
#include <rtps/transport/ethernet/InputPortData.hpp>
#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

InputPort::InputPort(
        uint16_t logical_port,
        TransportReceiverInterface* receiver,
        uint32_t max_message_size,
        const ThreadSettings& thread_config)
    : receiver_(receiver)
    , max_message_size_(max_message_size)
{
    auto fn = [this]()
            {
                processing_loop();
            };
    uint32_t port = logical_port;
    processing_thread_ = create_thread(fn, thread_config, "dds.eth.%u", port);
}

InputPort::~InputPort()
{
    // Stop processing thread
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stop_thread_ = true;
        queue_cv_.notify_one();
    }
    // Wait for thread to finish
    if (processing_thread_.joinable())
    {
        processing_thread_.join();
    }
    // Clear remaining packets in the queue
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!packet_queue_.empty())
    {
        // TODO: Return packets to pool
        packet_queue_.pop();
    }
}

void InputPort::process_data(
        InputPortData data)
{
    if (data.payload_size > max_message_size_)
    {
        // Message too large, discard
        // TODO: Return packet to pool
        return;
    }

    // Enqueue packet for processing in a separate thread
    enqueue(data);
}

void InputPort::enqueue(
        InputPortData data)
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!stop_thread_)
    {
        packet_queue_.push(data);
        queue_cv_.notify_one();
    }
}

void InputPort::processing_loop()
{
    while (true)
    {
        // Wait for data to be available in the queue
        InputPortData data;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this]
                    {
                        return !packet_queue_.empty() || stop_thread_;
                    });
            if (stop_thread_)
            {
                break;
            }
            data = packet_queue_.front();
            packet_queue_.pop();
        }

        // Process the packet
        Locator local_locator = EthernetLocator::create_locator(
            data.packet->header.h_dest,
            ntohs(data.data->dest_port), 0, 0);
        Locator remote_locator = EthernetLocator::create_locator(
            data.packet->header.h_source,
            ntohs(data.data->source_port), 0, 0);

        receiver_->OnDataReceived(data.data->payload, data.payload_size, local_locator, remote_locator);

        // TODO: Return packet to pool
    }
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
