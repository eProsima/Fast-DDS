/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_ETH_TRANSPORT__SRC__ETHERNETSENDERRESOURCE_HPP_
#define FASTDDS_ETH_TRANSPORT__SRC__ETHERNETSENDERRESOURCE_HPP_

#if defined(__linux__)

#include <cstdint>
#include <map>
#include <string>

#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

#include <fastdds/rtps/transport/ethernet/EthernetLocator.hpp>
#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>

#include <rtps/transport/ethernet/EthernetPacket.hpp>
#include <statistics/rtps/messages/OutputTrafficManager.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct EthernetSenderResource : public SenderResource
{
    /**
     * @brief Default constructor for EthernetSenderResource.
     *
     * @param configuration  Configuration object containing transport settings.
     */
    EthernetSenderResource(
            const EthernetTransportDescriptor& configuration);

    /**
     * Add locators representing the local endpoints managed by this sender resource.
     *
     * @param [in,out] locators  List where locators will be added.
     */
    void add_locators_to_list(
            LocatorList& locators) const override;

private:

    int socket_ = -1;
    int if_index_ = -1;
    EthernetPacketHeader eth_header_;
    EthernetPacketPrefix eth_prefix_;
    const std::map<int32_t, EthernetTransportDescriptor::PriorityMapping>& priority_mapping_;
    eprosima::fastdds::statistics::rtps::OutputTrafficManager statistics_info_;

    bool fill_prefix_from_priority(
            int32_t transport_priority,
            EthernetPacketPrefix& prefix);

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#else

#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>
#include <fastdds/rtps/common/Locator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct EthernetSenderResource : public SenderResource
{
    explicit EthernetSenderResource(
            const EthernetTransportDescriptor&)
        : SenderResource(LOCATOR_KIND_ETHERNET)
    {
    }

    void add_locators_to_list(
            eprosima::fastdds::rtps::LocatorList&) const override
    {
    }

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // if defined(__linux__)

#endif  // FASTDDS_ETH_TRANSPORT__SRC__ETHERNETSENDERRESOURCE_HPP_
