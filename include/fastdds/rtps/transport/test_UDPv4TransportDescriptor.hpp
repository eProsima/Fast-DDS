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

/**
 * @file test_UDPv4TransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__TEST_UDPV4TRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT__TEST_UDPV4TRANSPORTDESCRIPTOR_HPP

#include <functional>
#include <atomic>

#include <fastdds/rtps/transport/SocketTransportDescriptor.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CDRMessage_t;
struct TestUDPv4TransportOptions;

/**
 * UDP v4 Test Transport configuration
 * @ingroup TRANSPORT_MODULE
 */
struct test_UDPv4TransportDescriptor : public SocketTransportDescriptor
{
    //! Custom message filtering functions
    typedef std::function<bool (eprosima::fastdds::rtps::CDRMessage_t& msg)> filter;
    //! Locator filtering function
    typedef std::function<bool (const Locator& destination)> DestinationLocatorFilter;

    //! Test transport options
    std::shared_ptr<TestUDPv4TransportOptions> test_transport_options = std::make_shared<TestUDPv4TransportOptions>();

    //! Test shim parameters
    //! Percentage of data messages being dropped
    mutable std::atomic<uint8_t> dropDataMessagesPercentage{0};
    //! Percentage of Data[P] messages being dropped
    mutable std::atomic<uint8_t> dropParticipantBuiltinDataMessagesPercentage{0};
    //! Percentage of Data[W] messages being dropped
    mutable std::atomic<uint8_t> dropPublicationBuiltinDataMessagesPercentage{0};
    //! Percentage of Data[R] messages being dropped
    mutable std::atomic<uint8_t> dropSubscriptionBuiltinDataMessagesPercentage{0};
    //! Filtering function for dropping data messages
    filter drop_data_messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };
    //! Filtering function for dropping builtin data messages
    filter drop_builtin_data_messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };
    //! Flag to enable dropping of discovery Participant DATA(P) messages
    bool dropParticipantBuiltinTopicData = false;
    //! Flag to enable dropping of discovery Writer DATA(W) messages
    bool dropPublicationBuiltinTopicData = false;
    //! Flag to enable dropping of discovery Reader DATA(R) messages
    bool dropSubscriptionBuiltinTopicData = false;
    //! Percentage of data fragments being dropped
    mutable std::atomic<uint8_t> dropDataFragMessagesPercentage{0};
    //! Filtering function for dropping data fragments messages
    filter drop_data_frag_messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };
    //! Percentage of heartbeats being dropped
    mutable std::atomic<uint8_t> dropHeartbeatMessagesPercentage{0};
    //! Filtering function for dropping heartbeat messages
    filter drop_heartbeat_messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };
    //! Percentage of AckNacks being dropped
    mutable std::atomic<uint8_t> dropAckNackMessagesPercentage{0};
    //! Filtering function for dropping AckNacks
    filter drop_ack_nack_messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };
    //! Percentage of gap messages being dropped
    mutable std::atomic<uint8_t> dropGapMessagesPercentage{0};
    //! Filtering function for dropping gap messages
    filter drop_gap_messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };
    // General filtering function for all kind of sub-messages (indiscriminate)
    filter sub_messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };

    // General drop percentage (indiscriminate)
    mutable std::atomic<uint8_t> percentageOfMessagesToDrop{0};
    // General filtering function for all kind of messages (indiscriminate)
    filter messages_filter_ = [](fastdds::rtps::CDRMessage_t&)
            {
                return false;
            };

    //! Filtering function for dropping messages to specific destinations
    DestinationLocatorFilter locator_filter_ = [](const Locator&)
            {
                return false;
            };

    //! Vector containing the message's sequence numbers being dropped
    std::vector<fastdds::rtps::SequenceNumber_t> sequenceNumberDataMessagesToDrop{};

    //! Log dropped packets
    uint32_t dropLogLength = 0;

    //! Constructor
    FASTDDS_EXPORTED_API test_UDPv4TransportDescriptor();

    //! Destructor
    virtual ~test_UDPv4TransportDescriptor() = default;

    //! Create transport using the parameters defined within the Descriptor
    virtual TransportInterface* create_transport() const override;

    //! Copy constructor
    FASTDDS_EXPORTED_API test_UDPv4TransportDescriptor(
            const test_UDPv4TransportDescriptor& t) = delete;

    //! Copy assignment
    FASTDDS_EXPORTED_API test_UDPv4TransportDescriptor& operator =(
            const test_UDPv4TransportDescriptor& t) = delete;

    //! Move constructor
    FASTDDS_EXPORTED_API test_UDPv4TransportDescriptor(
            test_UDPv4TransportDescriptor&& t) = delete;

    //! Move assignment
    FASTDDS_EXPORTED_API test_UDPv4TransportDescriptor& operator =(
            test_UDPv4TransportDescriptor&& t) = delete;

    //! Comparison operator
    // Filters are not included
    FASTDDS_EXPORTED_API bool operator ==(
            const test_UDPv4TransportDescriptor& t) const;
};

struct TestUDPv4TransportOptions
{
    FASTDDS_EXPORTED_API TestUDPv4TransportOptions() = default;
    ~TestUDPv4TransportOptions() = default;

    std::atomic<bool> test_UDPv4Transport_ShutdownAllNetwork{false};
    // Handle to a persistent log of dropped packets. Defaults to length 0 (no logging) to prevent wasted resources.
    std::vector<std::vector<fastdds::rtps::octet>> test_UDPv4Transport_DropLog{};
    std::atomic<uint32_t> test_UDPv4Transport_DropLogLength{0};
    std::atomic<bool> always_drop_participant_builtin_topic_data{false};
    std::atomic<bool> simulate_no_interfaces{false};
    test_UDPv4TransportDescriptor::DestinationLocatorFilter locator_filter = [](const Locator&)
            {
                return false;
            };
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__TEST_UDPV4TRANSPORTDESCRIPTOR_HPP
