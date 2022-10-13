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

#ifndef _FASTDDS_TEST_UDPV4_TRANSPORT_DESCRIPTOR_
#define _FASTDDS_TEST_UDPV4_TRANSPORT_DESCRIPTOR_

#include <functional>

#include <fastdds/rtps/transport/SocketTransportDescriptor.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/messages/CDRMessage.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * UDP v4 Test Transport configuration
 * @ingroup TRANSPORT_MODULE
 */
struct test_UDPv4TransportDescriptor : public SocketTransportDescriptor
{
    //! Custom message filtering functions
    typedef std::function<bool (fastrtps::rtps::CDRMessage_t& msg)> filter;
    //! Locator filtering function
    typedef std::function<bool (const Locator& destination)> DestinationLocatorFilter;

    //! Test shim parameters
    //! Percentage of data messages being dropped
    uint8_t dropDataMessagesPercentage;
    //! Filtering function for dropping data messages
    filter drop_data_messages_filter_;
    //! Flag to enable dropping of discovery Participant DATA(P) messages
    bool dropParticipantBuiltinTopicData;
    //! Flag to enable dropping of discovery Writer DATA(W) messages
    bool dropPublicationBuiltinTopicData;
    //! Flag to enable dropping of discovery Reader DATA(R) messages
    bool dropSubscriptionBuiltinTopicData;
    //! Percentage of data fragments being dropped
    uint8_t dropDataFragMessagesPercentage;
    //! Filtering function for dropping data fragments messages
    filter drop_data_frag_messages_filter_;
    //! Percentage of heartbeats being dropped
    uint8_t dropHeartbeatMessagesPercentage;
    //! Filtering function for dropping heartbeat messages
    filter drop_heartbeat_messages_filter_;
    //! Percentage of AckNacks being dropped
    uint8_t dropAckNackMessagesPercentage;
    //! Filtering function for dropping AckNacks
    filter drop_ack_nack_messages_filter_;
    //! Percentage of gap messages being dropped
    uint8_t dropGapMessagesPercentage;
    //! Filtering function for dropping gap messages
    filter drop_gap_messages_filter_;

    // General drop percentage (indiscriminate)
    uint8_t percentageOfMessagesToDrop;
    // General filtering function for all kind of messages (indiscriminate)
    filter messages_filter_;

    //! Filtering function for dropping messages to specific destinations
    DestinationLocatorFilter locator_filter_;

    //! Vector containing the message's sequence numbers being dropped
    std::vector<fastrtps::rtps::SequenceNumber_t> sequenceNumberDataMessagesToDrop;

    //! Log dropped packets
    uint32_t dropLogLength;

    //! Constructor
    RTPS_DllAPI test_UDPv4TransportDescriptor();

    //! Destructor
    virtual ~test_UDPv4TransportDescriptor() = default;

    //! Create transport using the parameters defined within the Descriptor
    virtual TransportInterface* create_transport() const override;

    //! Copy constructor
    RTPS_DllAPI test_UDPv4TransportDescriptor(
            const test_UDPv4TransportDescriptor& t) = default;

    //! Copy assignment
    RTPS_DllAPI test_UDPv4TransportDescriptor& operator =(
            const test_UDPv4TransportDescriptor& t) = default;

    //! Comparison operator
    // Filters are not included
    RTPS_DllAPI bool operator ==(
            const test_UDPv4TransportDescriptor& t) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_UDPV4_TRANSPORT_DESCRIPTOR_
