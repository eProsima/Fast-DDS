// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_SOCKET_TRANSPORT_INTERFACE_HPP_
#define _FASTDDS_SOCKET_TRANSPORT_INTERFACE_HPP_

#include <vector>

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorWithMask.hpp>
#include <fastdds/rtps/transport/network/AllowedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/SocketTransportDescriptor.h>
#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastrtps/utils/IPFinder.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class SocketTransportInterface : public TransportInterface
{
public:

    enum class SocketKind
    {
        IPV4,
        IPV6
    };

    ~SocketTransportInterface() override;

    bool IsLocatorSupported(
            const Locator&) const override;

    LocatorList NormalizeLocator(
            const Locator& locator) override;

    bool is_local_locator(
            const Locator& locator) const override;

    bool is_localhost_allowed() const override;

    /**
     * Transforms a remote locator into a locator optimized for local communications.
     *
     * If the remote locator corresponds to one of the local interfaces, it is converted
     * to the corresponding local address if allowed by both local and remote transports.
     *
     * @param [in]  remote_locator Locator to be converted.
     * @param [out] result_locator Converted locator.
     * @param [in]  allowed_remote_localhost Whether localhost is allowed (and hence used) in the remote transport.
     * @param [in]  allowed_local_localhost Whether localhost is allowed locally (by this or other transport).
     *
     * @return false if the input locator is not supported/allowed by this transport, true otherwise.
     */
    bool transform_remote_locator(
            const Locator& remote_locator,
            Locator& result_locator,
            bool allowed_remote_localhost,
            bool allowed_local_localhost) const override;

    NetmaskFilterInfo netmask_filter_info() const override;

protected:

    SocketKind socket_kind_;
    NetmaskFilterKind netmask_filter_;
    std::vector<AllowedNetworkInterface> allowed_interfaces_;

    SocketTransportInterface(
            int32_t transport_kind);

    SocketTransportInterface(
            int32_t transport_kind,
            const SocketTransportDescriptor& descriptor);

    bool is_ipv4() const;

    virtual bool compare_locator_ip(
            const Locator& lh,
            const Locator& rh) const;

    virtual bool compare_locator_ip_and_port(
            const Locator& lh,
            const Locator& rh) const;

    virtual void fill_local_ip(
            Locator& loc) const;

    virtual bool get_ips(
            std::vector<fastrtps::rtps::IPFinder::info_IP>& locNames,
            bool return_loopback,
            bool force_lookup) const;

    virtual bool get_ips_unique_interfaces(
            std::vector<fastrtps::rtps::IPFinder::info_IP>& locNames,
            bool return_loopback,
            bool force_lookup) const;

    virtual const std::string& localhost_name();

    virtual void fill_interface_whitelist_() = 0;

    //! Checks if the interfaces whitelist is empty.
    virtual bool is_interface_whitelist_empty() const = 0;

    //! Checks if the given interface is allowed by the whitelist.
    virtual bool is_interface_allowed(
            const std::string& iface) const = 0;

    //! Checks if the given locator is allowed by the whitelist.
    virtual bool is_interface_allowed(
            const Locator& loc) const = 0;

    /**
     * Method to get a list of interfaces to bind the socket associated to the given locator.
     * @return Vector of interfaces in string format.
     */
    virtual std::vector<std::string> get_binding_interfaces_list() = 0;

    //! Checks if two IP addresses are the same, without taking into account the scope in the IPv6 case
    bool compare_ips(
            const std::string& ip1,
            const std::string& ip2) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SOCKET_TRANSPORT_INTERFACE_HPP_
