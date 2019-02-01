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

#ifndef TEST_TCPV4_TRANSPORT_H
#define TEST_TCPV4_TRANSPORT_H
#include <fastrtps/transport/TCPv4Transport.h>
#include <fastrtps/rtps/messages/RTPS_messages.h>
#include <fastrtps/rtps/common/SequenceNumber.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <vector>

#include "test_TCPv4TransportDescriptor.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

// Handle to a persistent log of dropped packets. Defaults to length 0 (no logging) to prevent wasted resources.
static std::vector<std::vector<octet> > g_test_TCPv4Transport_DropLog;
static uint32_t g_test_TCPv4Transport_DropLogLength;
static bool g_test_TCPv4Transport_ShutdownAllNetwork;
static bool g_test_TCPv4Transport_CloseSocketConnection;
/*
 * This transport acts as a shim over TCPv4, allowing
 * packets to be dropped under certain criteria.
 */
class test_TCPv4Transport : public TCPv4Transport
{
    uint8_t invalid_crcs_percentage_;
    uint8_t close_socket_on_send_percentage_;
    uint8_t drop_data_messages_percentage_;
    bool drop_participant_builtin_topic_data_;
    bool drop_publication_builtin_topic_data_;
    bool drop_subscription_builtin_topic_data_;
    uint8_t drop_data_frag_messages_percentage_;
    uint8_t drop_heartbeat_messages_percentage_;
    uint8_t drop_ack_nack_messages_percentage_;
    std::vector<SequenceNumber_t> sequence_number_data_messages_to_drop_;
    uint8_t percentage_of_messages_to_drop_;

    bool log_drop(
        const octet* buffer,
        uint32_t size);

    bool packet_should_drop(
        const octet* send_buffer,
        uint32_t send_buffer_size);

    bool random_chance_drop();

protected:
    void calculate_crc(
        TCPHeader &header,
        const octet *data,
        uint32_t size);

public:
   RTPS_DllAPI test_TCPv4Transport(const test_TCPv4TransportDescriptor& descriptor);

   virtual bool send(
       const octet* send_buffer,
       uint32_t send_buffer_size,
       const Locator_t& local_locator,
       const Locator_t& remote_locator) override;

   virtual bool send(
       const octet* send_buffer,
       uint32_t send_buffer_size,
       const Locator_t& local_locator,
       const Locator_t& remote_locator,
       ChannelResource* p_channel_resource) override;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
