// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_TEST_UDPV4_TRANSPORT_H_
#define _FASTDDS_TEST_UDPV4_TRANSPORT_H_

#include <vector>

#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/messages/RTPS_messages.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <rtps/transport/UDPv4Transport.h>


namespace eprosima {
namespace fastdds {
namespace rtps {

/*
 * This transport acts as a shim over UDPv4, allowing
 * packets to be dropped under certain criteria.
 */

class test_UDPv4Transport : public UDPv4Transport
{
public:

    test_UDPv4Transport(
            const test_UDPv4TransportDescriptor& descriptor);

    virtual bool send(
            const std::vector<NetworkBuffer>& send_buffer,
            uint32_t total_bytes,
            eProsimaUDPSocket& socket,
            LocatorsIterator* destination_locators_begin,
            LocatorsIterator* destination_locators_end,
            bool only_multicast_purpose,
            bool whitelisted,
            const std::chrono::steady_clock::time_point& max_blocking_time_point) override;

    virtual LocatorList NormalizeLocator(
            const Locator& locator) override;

    std::shared_ptr<TestUDPv4TransportOptions> test_transport_options;

protected:

    virtual bool get_ips(
            std::vector<fastdds::rtps::IPFinder::info_IP>& locNames,
            bool return_loopback,
            bool force_lookup) const override;

private:

    struct PercentageData
    {
        PercentageData(
                std::atomic<uint8_t>& percent)
            : percentage(percent)
            , accumulator(0)
        {
        }

        std::atomic<uint8_t>& percentage;
        uint8_t accumulator;
    };

    PercentageData drop_data_messages_percentage_;
    PercentageData drop_participant_builtin_data_messages_percentage_;
    PercentageData drop_publication_builtin_data_messages_percentage_;
    PercentageData drop_subscription_builtin_data_messages_percentage_;
    test_UDPv4TransportDescriptor::filter drop_data_messages_filter_;
    test_UDPv4TransportDescriptor::filter drop_builtin_data_messages_filter_;
    bool drop_participant_builtin_topic_data_;
    bool drop_publication_builtin_topic_data_;
    bool drop_subscription_builtin_topic_data_;
    PercentageData drop_data_frag_messages_percentage_;
    test_UDPv4TransportDescriptor::filter drop_data_frag_messages_filter_;
    PercentageData drop_heartbeat_messages_percentage_;
    test_UDPv4TransportDescriptor::filter drop_heartbeat_messages_filter_;
    PercentageData drop_ack_nack_messages_percentage_;
    test_UDPv4TransportDescriptor::filter drop_ack_nack_messages_filter_;
    PercentageData drop_gap_messages_percentage_;
    test_UDPv4TransportDescriptor::filter drop_gap_messages_filter_;
    test_UDPv4TransportDescriptor::filter sub_messages_filter_;
    PercentageData percentage_of_messages_to_drop_;
    test_UDPv4TransportDescriptor::filter messages_filter_;
    std::vector<fastdds::rtps::SequenceNumber_t> sequence_number_data_messages_to_drop_;
    test_UDPv4TransportDescriptor::DestinationLocatorFilter locator_filter_;

    bool should_drop_locator(
            const Locator& remote_locator);

    bool log_drop(
            const std::vector<NetworkBuffer>& buffer,
            uint32_t size);
    bool packet_should_drop(
            const std::vector<NetworkBuffer>& buffers,
            uint32_t total_bytes);
    bool random_chance_drop();
    bool should_be_dropped(
            PercentageData* percentage);

    bool send(
            const std::vector<NetworkBuffer>& send_buffer,
            uint32_t total_bytes,
            eProsimaUDPSocket& socket,
            const Locator& remote_locator,
            bool only_multicast_purpose,
            bool whitelisted,
            const std::chrono::microseconds& timeout);
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TEST_UDPV4_TRANSPORT_H_
