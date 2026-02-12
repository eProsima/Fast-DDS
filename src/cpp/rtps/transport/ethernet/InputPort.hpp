/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_ETH_TRANSPORT__SRC__INPUTPORT_HPP_
#define FASTDDS_ETH_TRANSPORT__SRC__INPUTPORT_HPP_

#if defined(__linux__)

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

#include <rtps/transport/ethernet/EthernetPacket.hpp>
#include <rtps/transport/ethernet/InputPortData.hpp>
#include <utils/thread.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Class to process incoming Ethernet packets on a specific port.
 */
struct InputPort
{
    /**
     * @brief Constructor for InputPort.
     *
     * Creates an InputPort that processes incoming Ethernet packets on a specific port.
     *
     * @param logical_port      The logical port number to listen on.
     * @param receiver          Pointer to the TransportReceiverInterface that will handle received data.
     * @param max_message_size  Maximum size of messages that can be processed.
     * @param thread_config     Configuration for the processing thread.
     */
    InputPort(
            uint16_t logical_port,
            TransportReceiverInterface* receiver,
            uint32_t max_message_size,
            const ThreadSettings& thread_config);

    /**
     * @brief Destructor for InputPort.
     *
     * Will stop the processing thread and clean up resources.
     */
    ~InputPort();

    /**
     * @brief Process an incoming Ethernet packet.
     *
     * This method is called when an Ethernet packet is received on the associated port.
     * It extracts the RTPS payload and forwards it to the TransportReceiverInterface.
     *
     * @param data  The received Ethernet packet along with its payload size.
     */
    void process_data(
            InputPortData data);

private:

    /**
     * @brief Enqueue an incoming packet for processing.
     *
     * @param data  The packet data to enqueue.
     */
    void enqueue(
            InputPortData data);

    /**
     * @brief Main loop for the processing thread.
     */
    void processing_loop();

    /// Pointer to the receiver interface to handle incoming data.
    TransportReceiverInterface* receiver_;
    /// Maximum size of messages that can be processed.
    uint32_t max_message_size_;
    /// Queue for incoming packets to be processed.
    std::queue<InputPortData> packet_queue_;
    /// Mutex to protect access to the packet queue.
    std::mutex queue_mutex_;
    /// Condition variable to signal the processing thread of new packets.
    std::condition_variable queue_cv_;
    /// Thread for processing incoming packets.
    eprosima::thread processing_thread_;
    /// Flag to indicate if the processing thread should stop.
    bool stop_thread_ = false;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // if defined(__linux__)

#endif  // FASTDDS_ETH_TRANSPORT__SRC__INPUTPORT_HPP_
