/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool InputChannelManager::init(
        const EthernetTransportDescriptor& /*configuration*/)
{
    EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
            "Ethernet transport is not supported on this platform");
    return false;
}

void InputChannelManager::stop()
{
}

EthernetAddress InputChannelManager::get_interface_address() const
{
    return EthernetAddress{};
}

bool InputChannelManager::is_open(
        const EthernetAddress& /*address*/,
        uint16_t /*logical_port*/) const
{
    return false;
}

bool InputChannelManager::open(
        const EthernetAddress& /*address*/,
        uint16_t /*logical_port*/,
        TransportReceiverInterface* /*receiver_interface*/,
        uint32_t /*max_message_size*/)
{
    return false;
}

bool InputChannelManager::close(
        const EthernetAddress& /*address*/,
        uint16_t /*logical_port*/)
{
    return false;
}

bool InputChannelManager::locators_match(
        const Locator& /*locator1*/,
        const Locator& /*locator2*/) const
{
    return false;
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
