// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TEST_UDPV4_TRANSPORT_H
#define TEST_UDPV4_TRANSPORT_H
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/rtps/messages/RTPS_messages.h>
#include <fastrtps/rtps/common/SequenceNumber.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <vector>

#include "test_UDPv4TransportDescriptor.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

/*
 * This transport acts as a shim over UDPv4, allowing
 * packets to be dropped under certain criteria.
 */

class test_UDPv4Transport : public UDPv4Transport
{
public:
    test_UDPv4Transport(const test_UDPv4TransportDescriptor& descriptor);

    virtual bool send(
           const octet* send_buffer,
           uint32_t send_buffer_size,
           eProsimaUDPSocket& socket,
           const Locator_t& remote_locator,
           bool only_multicast_purpose,
           const std::chrono::microseconds& timeout) override;

    RTPS_DllAPI static bool test_UDPv4Transport_ShutdownAllNetwork;
    // Handle to a persistent log of dropped packets. Defaults to length 0 (no logging) to prevent wasted resources.
    RTPS_DllAPI static std::vector<std::vector<octet> > test_UDPv4Transport_DropLog;
    RTPS_DllAPI static uint32_t test_UDPv4Transport_DropLogLength;

private:

    struct PercentageData
    {
        PercentageData(uint8_t percent)
            : percentage(percent)
            , accumulator(0)
        {
        }

        uint8_t percentage;
        uint8_t accumulator;
    };

    PercentageData drop_data_messages_percentage_;
    bool drop_participant_builtin_topic_data_;
    bool drop_publication_builtin_topic_data_;
    bool drop_subscription_builtin_topic_data_;
    PercentageData drop_data_frag_messages_percentage_;
    PercentageData drop_heartbeat_messages_percentage_;
    PercentageData drop_ack_nack_messages_percentage_;
    std::vector<SequenceNumber_t> sequence_number_data_messages_to_drop_;
    PercentageData percentage_of_messages_to_drop_;

    bool log_drop(const octet* buffer, uint32_t size);
    bool packet_should_drop(const octet* send_buffer, uint32_t send_buffer_size);
    bool random_chance_drop();
    bool should_be_dropped(PercentageData* percentage);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
