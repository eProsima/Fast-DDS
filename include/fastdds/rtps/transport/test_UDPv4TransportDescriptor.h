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
 * test_UDPv4 Transport configuration
 * @ingroup TRANSPORT_MODULE
 */
struct test_UDPv4TransportDescriptor : public SocketTransportDescriptor
{

    typedef std::function<bool (fastrtps::rtps::CDRMessage_t& msg)> filter;

    // Test shim parameters
    uint8_t dropDataMessagesPercentage;
    filter drop_data_messages_filter_;
    bool dropParticipantBuiltinTopicData;
    bool dropPublicationBuiltinTopicData;
    bool dropSubscriptionBuiltinTopicData;
    uint8_t dropDataFragMessagesPercentage;
    filter drop_data_frag_messages_filter_;
    uint8_t dropHeartbeatMessagesPercentage;
    filter drop_heartbeat_messages_filter_;
    uint8_t dropAckNackMessagesPercentage;
    filter drop_ack_nack_messages_filter_;
    uint8_t dropGapMessagesPercentage;
    filter drop_gap_messages_filter_;

    // General drop percentage (indescriminate)
    uint8_t percentageOfMessagesToDrop;
    filter messages_filter_;

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
