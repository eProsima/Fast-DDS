/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_ETH_TRANSPORT__SRC__INPUTCHANNELMANAGER_HPP_
#define FASTDDS_ETH_TRANSPORT__SRC__INPUTCHANNELMANAGER_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

#include <fastdds/rtps/transport/ethernet/EthernetLocator.hpp>
#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>

#include <rtps/transport/ethernet/InputPort.hpp>
#include <utils/thread.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Class to manage the Input channels of an Ethernet transport.
 */
struct InputChannelManager
{
    /**
     * @brief Initialize the input manager, and bind it to an interface.
     *
     * @param configuration  Configuration object containing transport settings.
     *
     * @return True when a listening socket is correctly bound to the specified interface, false otherwise.
     */
    bool init(
            const EthernetTransportDescriptor& configuration);

    /**
     * @brief Stop the input manager, closing all open channels and the listening socket.
     */
    void stop();

    /**
     * @brief Get a copy of the address of the interface where the manager is listening.
     *
     * This method should only be called on a correctly initialized manager.
     * Otherwise, the returned address would be a default initialized one.
     *
     * @return A copy of the address of the interface where the manager is listening.
     */
    EthernetAddress get_interface_address() const;

    /**
     * @brief Check if an input channel is open for the given Ethernet address and port.
     *
     * @param address       The Ethernet address of the input channel.
     * @param logical_port  The port of the input channel.
     *
     * @return True if the input channel is open, false otherwise.
     */
    bool is_open(
            const EthernetAddress& address,
            uint16_t logical_port) const;

    /**
     * @brief Open an input channel for the given Ethernet address and port.
     *
     * @param address             The Ethernet address of the input channel.
     * @param logical_port        The port of the input channel.
     * @param receiver_interface  The receiver interface to handle incoming messages.
     * @param max_message_size    The maximum size of messages that can be received.
     *
     * @return True if the input channel was successfully opened, false otherwise.
     */
    bool open(
            const EthernetAddress& address,
            uint16_t logical_port,
            TransportReceiverInterface* receiver_interface,
            uint32_t max_message_size);

    /**
     * @brief Close an input channel for the given Ethernet address and port.
     *
     * @param address       The Ethernet address of the input channel.
     * @param logical_port  The port of the input channel.
     *
     * @return True if the input channel was successfully closed, false otherwise.
     */
    bool close(
            const EthernetAddress& address,
            uint16_t logical_port);

    /**
     * @brief Check if two locators correspond to the same input channel.
     *
     * @param locator1  The first locator to compare.
     * @param locator2  The second locator to compare.
     *
     * @return True if the locators match, false otherwise.
     */
    bool locators_match(
            const Locator& locator1,
            const Locator& locator2) const;

private:

#if defined(__linux__)

    /**
     * @brief Check whether a socket is listening on certain ethernet address.
     *
     * @param address The ethernet address to check.
     *
     * @return true if there is a socket listening on the specified address, false otherwise.
     */
    bool is_listening_on_address(
            const EthernetAddress& address) const;

    /**
     * @brief Check whether the manager has a thread processing data for a logical port.
     *
     * @param port The logical port to check.
     *
     * @return true if there is a thread processing data for a logical port, false otherwise.
     */
    bool is_listening_on_port(
            uint16_t port) const;

    /**
     * @brief Start listening on certain address.
     *
     * @param address The ethernet address to listen.
     *                Should be either multicast or the address of a local interface.
     *
     * @return true if listening on the specified address was achieved, false otherwise.
     */
    bool listen_on_address(
            const EthernetAddress& address);

    /**
     * @brief Start thread for processing data from a logical port.
     *
     * @param port                The logical port for which to start the thread.
     * @param receiver_interface  The receiver interface to handle incoming messages.
     * @param max_message_size    The maximum size of messages that can be received.
     *
     * @return True when a thread for the specified port is started.
     * @return True when a thread for the specified port already exists.
     * @return False otherwise.
     */
    bool listen_on_port(
            uint16_t port,
            TransportReceiverInterface* receiver_interface,
            uint32_t max_message_size);

    /**
     * @brief Stop the thread processing data for the specified port (if present).
     *
     * @param port The logical port for which to stop the thread.
     */
    void close_port(
            uint16_t port);

    /**
     * @brief Main function for the listening thread.
     */
    void process_listening();

    mutable std::mutex mtx_;
    int socket_ = -1;
    int if_index_ = -1;
    EthernetTransportDescriptor configuration_;
    std::set<EthernetAddress> mcast_addresses_;
    EthernetAddress local_address_;
    eprosima::thread listening_thread_;
    std::map<uint16_t, std::unique_ptr<InputPort>> port_listeners_;
#endif  // if defined(__linux__)
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_ETH_TRANSPORT__SRC__INPUTCHANNELMANAGER_HPP_
