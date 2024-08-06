// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ChainingTransport.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__CHAININGTRANSPORT_HPP
#define FASTDDS_RTPS_TRANSPORT__CHAININGTRANSPORT_HPP

#include <map>
#include <memory>

#include "TransportInterface.hpp"
#include "ChainingTransportDescriptor.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

class ChainingReceiverResource;

/**
 * @brief Deleter for a ChainingReceiverResource
 *
 * @note this is required to create a \c unique_ptr of a \c ChainingReceiverResource as it is
 * an incomplete class for this header.
 */
struct ChainingReceiverResourceDeleter
{
    void operator ()(
            ChainingReceiverResource* p);
};

//! Type of the \c unique_ptr of a \c ChainingReceiverResource .
using ChainingReceiverResourceReferenceType =
        std::unique_ptr<ChainingReceiverResource, ChainingReceiverResourceDeleter>;

/**
 * This is the base class for chaining adapter transports.
 *    - Directly proxies all operations except Send and Receive
 *
 *    - Has a pointer to the low level transport
 * @ingroup TRANSPORT_MODULE
 */
class ChainingTransport : public TransportInterface
{

public:

    //!  Constructor
    FASTDDS_EXPORTED_API ChainingTransport(
            const ChainingTransportDescriptor& t)
        : TransportInterface(0)
        , low_level_transport_(t.low_level_descriptor->create_transport())
    {
        transport_kind_ = low_level_transport_->kind();
    }

    //! Destructor
    FASTDDS_EXPORTED_API virtual ~ChainingTransport() = default;

    /*!
     * Initialize the low-level transport. This method will prepare all the internals of the transport.
     * @param properties Optional policy to specify additional parameters of the created transport.
     * @param max_msg_size_no_frag Optional maximum message size to avoid 65500 KB fragmentation limit.
     * @return True when the transport was correctly initialized.
     */
    FASTDDS_EXPORTED_API bool init(
            const fastdds::rtps::PropertyPolicy* properties = nullptr,
            const uint32_t& max_msg_size_no_frag = 0) override
    {
        return low_level_transport_->init(properties, max_msg_size_no_frag);
    }

    /*!
     * Call the low-level transport `IsInputChannelOpen()`.
     * Must report whether the input channel associated to this locator is open. Channels must either be
     * fully closed or fully open, so that "open" and "close" operations are whole and definitive.
     */
    FASTDDS_EXPORTED_API bool IsInputChannelOpen(
            const fastdds::rtps::Locator_t& loc) const override
    {
        return low_level_transport_->IsInputChannelOpen(loc);
    }

    /*!
     * Call the low-level transport `IsLocatorSupported()`.
     * Must report whether the given locator is supported by this transport (typically inspecting its "kind" value).
     */
    FASTDDS_EXPORTED_API bool IsLocatorSupported(
            const fastdds::rtps::Locator_t& loc) const override
    {
        return low_level_transport_->IsLocatorSupported(loc);
    }

    /*!
     * Call the low-level transport `RemoteToMainLocal()`.
     * Returns the locator describing the main (most general) channel that can write to the provided remote locator.
     */
    FASTDDS_EXPORTED_API fastdds::rtps::Locator_t RemoteToMainLocal(
            const fastdds::rtps::Locator_t& loc) const override
    {
        return low_level_transport_->RemoteToMainLocal(loc);
    }

    /*!
     * Call the low-level transport `OpenInputChannel()`.
     * Opens an input channel to receive incoming connections.
     *   If there is an existing channel it registers the receiver interface.
     */
    FASTDDS_EXPORTED_API bool OpenInputChannel(
            const fastdds::rtps::Locator_t& loc,
            TransportReceiverInterface* receiver_interface,
            uint32_t max_message_size) override;

    /*!
     * Call the low-level transport `OpenOutputChannel()`.
     * Must open the channel that maps to/from the given locator. This method must allocate, reserve and mark
     * any resources that are needed for said channel.
     */
    FASTDDS_EXPORTED_API bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const fastdds::rtps::Locator_t& loc) override;

    /*!
     * Call the low-level transport `CloseInputChannel()`.
     * Must close the channel that maps to/from the given locator.
     * IMPORTANT: It MUST be safe to call this method even during a Receive operation on another thread. You must implement
     * any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage.
     */
    FASTDDS_EXPORTED_API bool CloseInputChannel(
            const fastdds::rtps::Locator_t& loc) override
    {
        return low_level_transport_->CloseInputChannel(loc);
    }

    /*!
     * Call the low-level transport `NormalizeLocator()`.
     * Performs locator normalization (assign valid IP if not defined by user)
     */
    FASTDDS_EXPORTED_API fastdds::rtps::LocatorList_t NormalizeLocator(
            const fastdds::rtps::Locator_t& locator) override
    {
        return low_level_transport_->NormalizeLocator(locator);
    }

    /*!
     * Call the low-level transport `is_local_locator()`.
     * Must report whether the given locator is from the local host
     */
    FASTDDS_EXPORTED_API bool is_local_locator(
            const fastdds::rtps::Locator_t& locator) const override
    {
        return low_level_transport_->is_local_locator(locator);
    }

    /*!
     * Call the low-level transport `is_localhost_allowed()`.
     * Must report whether localhost locator is allowed
     */
    FASTDDS_EXPORTED_API bool is_localhost_allowed() const override
    {
        return low_level_transport_->is_localhost_allowed();
    }

    /*!
     * Call the low-level transport `netmask_filter_info()`.
     * Returns netmask filter information (transport's netmask filter kind and allowlist)
     */
    FASTDDS_EXPORTED_API NetmaskFilterInfo netmask_filter_info() const override
    {
        return low_level_transport_->netmask_filter_info();
    }

    /*!
     * Call the low-level transport `DoInputLocatorsMatch()`.
     * Must report whether two locators map to the same internal channel.
     */
    FASTDDS_EXPORTED_API bool DoInputLocatorsMatch(
            const fastdds::rtps::Locator_t& locator_1,
            const fastdds::rtps::Locator_t& locator_2) const override
    {
        return low_level_transport_->DoInputLocatorsMatch(locator_1, locator_2);
    }

    /*!
     * Call the low-level transport `select_locators()`.
     * Performs the locator selection algorithm for this transport.
     */
    FASTDDS_EXPORTED_API void select_locators(
            fastdds::rtps::LocatorSelector& selector) const override
    {
        return low_level_transport_->select_locators(selector);
    }

    /*!
     * Call the low-level transport `AddDefaultOutputLocator()`.
     * Add default output locator to the locator list
     */
    FASTDDS_EXPORTED_API void AddDefaultOutputLocator(
            fastdds::rtps::LocatorList_t& defaultList) override
    {
        return low_level_transport_->AddDefaultOutputLocator(defaultList);
    }

    /*!
     * Call the low-level transport `getDefaultMetatrafficMulticastLocators()`.
     * Add metatraffic multicast locator with the given port
     */
    FASTDDS_EXPORTED_API bool getDefaultMetatrafficMulticastLocators(
            fastdds::rtps::LocatorList_t& locators,
            uint32_t metatraffic_multicast_port) const override
    {
        return low_level_transport_->getDefaultMetatrafficMulticastLocators(locators, metatraffic_multicast_port);
    }

    /*!
     * Call the low-level transport `getDefaultMetatrafficUnicastLocators()`.
     * Add metatraffic unicast locator with the given port
     */
    FASTDDS_EXPORTED_API bool getDefaultMetatrafficUnicastLocators(
            fastdds::rtps::LocatorList_t& locators,
            uint32_t metatraffic_unicast_port) const override
    {
        return low_level_transport_->getDefaultMetatrafficUnicastLocators(locators, metatraffic_unicast_port);
    }

    /*!
     * Call the low-level transport `getDefaultUnicastLocators()`.
     * Add unicast locator with the given port
     */
    FASTDDS_EXPORTED_API bool getDefaultUnicastLocators(
            fastdds::rtps::LocatorList_t& locators,
            uint32_t unicast_port) const override
    {
        return low_level_transport_->getDefaultUnicastLocators(locators, unicast_port);
    }

    /*!
     * Call the low-level transport `fillMetatrafficMulticastLocator()`.
     * Assign port to the given metatraffic multicast locator if not already defined
     */
    FASTDDS_EXPORTED_API bool fillMetatrafficMulticastLocator(
            fastdds::rtps::Locator_t& locator,
            uint32_t metatraffic_multicast_port) const override
    {
        return low_level_transport_->fillMetatrafficMulticastLocator(locator, metatraffic_multicast_port);
    }

    /*!
     * Call the low-level transport `fillMetatrafficUnicastLocator()`.
     * Assign port to the given metatraffic unicast locator if not already defined
     */
    FASTDDS_EXPORTED_API bool fillMetatrafficUnicastLocator(
            fastdds::rtps::Locator_t& locator,
            uint32_t metatraffic_unicast_port) const override
    {
        return low_level_transport_->fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);
    }

    /*!
     * Call the low-level transport `configureInitialPeerLocator()`.
     * Configure the initial peer locators list
     */
    FASTDDS_EXPORTED_API bool configureInitialPeerLocator(
            fastdds::rtps::Locator_t& locator,
            const fastdds::rtps::PortParameters& port_params,
            uint32_t domainId,
            fastdds::rtps::LocatorList_t& list) const override
    {
        return low_level_transport_->configureInitialPeerLocator(locator, port_params, domainId, list);
    }

    /*!
     * Call the low-level transport `fillUnicastLocator()`.
     * Assign port to the given unicast locator if not already defined
     */
    FASTDDS_EXPORTED_API bool fillUnicastLocator(
            fastdds::rtps::Locator_t& locator,
            uint32_t well_known_port) const override
    {
        return low_level_transport_->fillUnicastLocator(locator, well_known_port);
    }

    //! Call the low-level transport `transform_remote_locator()`.
    //! Transforms a remote locator into a locator optimized for local communications.
    FASTDDS_EXPORTED_API bool transform_remote_locator(
            const fastdds::rtps::Locator_t& remote_locator,
            fastdds::rtps::Locator_t& result_locator) const override
    {
        return low_level_transport_->transform_remote_locator(remote_locator, result_locator);
    }

    /**
     * Call the low-level transport `max_recv_buffer_size()`.
     * @return The maximum datagram size for reception supported by the transport
     */
    FASTDDS_EXPORTED_API uint32_t max_recv_buffer_size() const override
    {
        return low_level_transport_->max_recv_buffer_size();
    }

    /**
     * Blocking Send through the specified channel. It may perform operations on the output buffer.
     * At the end the function must call to the low-level transport's `send()` function.
     * @code{.cpp}
           // Example of calling the low-level transport `send()` function.
           return low_sender_resource->send(buffers, total_bytes, destination_locators_begin,
                       destination_locators_end, timeout);
       @endcode
     * @param low_sender_resource SenderResource generated by the lower transport.
     * @param buffers Vector of buffers to send.
     * @param total_bytes Length of all buffers to be sent. Will be used as a boundary for the previous parameter.
     * It must not exceed the \c sendBufferSize fed to this class during construction.
     * @param destination_locators_begin First iterator of the list of Locators describing the remote destinations
     * we're sending to.
     * @param destination_locators_end End iterator of the list of Locators describing the remote destinations
     * we're sending to.
     * @param timeout Maximum blocking time.
     */
    FASTDDS_EXPORTED_API virtual bool send(
            fastdds::rtps::SenderResource* low_sender_resource,
            const std::vector<NetworkBuffer>& buffers,
            uint32_t total_bytes,
            fastdds::rtps::LocatorsIterator* destination_locators_begin,
            fastdds::rtps::LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& timeout) = 0;

    /*!
     * Blocking Receive from the specified channel. It may perform operations on the input buffer.
     * At the end the function must call to the `next_receiver`'s `OnDataReceived` function.
     * @code{.cpp}
           // Example of calling the `next_receiver`'s `OnDataReceived` function.
           next_receiver->OnDataReceived(receive_buffer, receive_buffer_size, local_locator, remote_locator);
       @endcode
     * @param next_receiver Next resource receiver to be called.
     * @param receive_buffer vector with enough capacity (not size) to accommodate a full receive buffer. That
     * capacity must not be less than the \c receiveBufferSize supplied to this class during construction.
     * @param receive_buffer_size Size of the raw data. It will be used as bounds check for the previous argument.
     * It must not exceed the \c receiveBufferSize fed to this class during construction.
     * @param local_locator Locator mapping to the local channel we're listening to.
     * @param [out] remote_locator Locator describing the remote destination we received a packet from.
     */
    FASTDDS_EXPORTED_API virtual void receive(
            TransportReceiverInterface* next_receiver,
            const fastdds::rtps::octet* receive_buffer,
            uint32_t receive_buffer_size,
            const fastdds::rtps::Locator_t& local_locator,
            const fastdds::rtps::Locator_t& remote_locator) = 0;

    FASTDDS_EXPORTED_API void update_network_interfaces() override
    {
        low_level_transport_->update_network_interfaces();
    }

    //! Call the low-level transport `transform_remote_locator()`.
    //! Transforms a remote locator into a locator optimized for local communications,
    //! if allowed by both local and remote transports.
    FASTDDS_EXPORTED_API bool transform_remote_locator(
            const fastdds::rtps::Locator_t& remote_locator,
            fastdds::rtps::Locator_t& result_locator,
            bool allowed_remote_localhost,
            bool allowed_local_localhost) const override
    {
        return low_level_transport_->transform_remote_locator(remote_locator, result_locator, allowed_remote_localhost,
                       allowed_local_localhost);
    }

    /*!
     * Call the low-level transport `is_locator_allowed()`.
     * Must report whether the given locator is allowed by this transport.
     */
    FASTDDS_EXPORTED_API bool is_locator_allowed(
            const fastdds::rtps::Locator_t& locator) const override
    {
        return low_level_transport_->is_locator_allowed(locator);
    }

    /*!
     * Call the low-level transport `is_locator_reachable()`.
     * Must report whether the given locator is reachable by this transport.
     */
    FASTDDS_EXPORTED_API bool is_locator_reachable(
            const fastdds::rtps::Locator_t& locator) override
    {
        return low_level_transport_->is_locator_reachable(locator);
    }

protected:

    std::unique_ptr<TransportInterface> low_level_transport_;

private:

    std::map<fastdds::rtps::Locator_t, ChainingReceiverResourceReferenceType> receiver_resources_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__CHAININGTRANSPORT_HPP
