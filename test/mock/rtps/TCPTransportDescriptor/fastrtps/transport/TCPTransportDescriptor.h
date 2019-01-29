// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TCP_TRANSPORT_DESCRIPTOR_H
#define TCP_TRANSPORT_DESCRIPTOR_H

#include <fastrtps/transport/SocketTransportDescriptor.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
* Transport configuration
* @ingroup TRANSPORT_MODULE
*/
typedef struct TCPTransportDescriptor : public SocketTransportDescriptor
{
    struct TLSConfig
    {
        enum TLSOptions : uint32_t
        {
            DEFAULT_WORKAROUNDS     = 1 << 0, // 0000 0000 0001
            NO_COMPRESSION          = 1 << 1, // 0000 0000 0010
            NO_SSLV2                = 1 << 2, // 0000 0000 0100
            NO_SSLV3                = 1 << 3, // 0000 0000 1000
            NO_TLSV1                = 1 << 4, // 0000 0001 0000
            NO_TLSV1_1              = 1 << 5, // 0000 0010 0000
            NO_TLSV1_2              = 1 << 6, // 0000 0100 0000
            NO_TLSV1_3              = 1 << 7, // 0000 1000 0000
            SINGLE_DH_USE           = 1 << 8  // 0001 0000 0000
        };

        enum TLSVerifyMode : uint8_t
        {
            VERIFY_NONE                 = 1 << 0, // 0000 0001
            VERIFY_PEER                 = 1 << 1, // 0000 0010
            VERIFY_FAIL_IF_NO_PEER_CERT = 1 << 2, // 0000 0100
            VERIFY_CLIENT_ONCE          = 1 << 3  // 0000 1000
        };

        std::string password;
        uint32_t options;
        std::string cert_chain_file;
        std::string private_key_file;
        std::string tmp_dh_file;
        std::string verify_file;
        TLSVerifyMode verify_mode;

        void add_option(const TLSOptions option)
        {
            options |= option;
        }

        bool get_option(const TLSOptions option)
        {
            return options & option;
        }
    };

    std::vector<uint16_t> listening_ports;
    uint32_t keep_alive_frequency_ms;
    uint32_t keep_alive_timeout_ms;
    uint16_t max_logical_port;
    uint16_t logical_port_range;
    uint16_t logical_port_increment;
    uint16_t metadata_logical_port;
    uint32_t tcp_negotiation_timeout;
    bool enable_tcp_nodelay;
    bool wait_for_tcp_negotiation;
    bool calculate_crc;
    bool check_crc;

    TLSConfig tls_config;

    void add_listener_port(uint16_t port)
    {
        listening_ports.push_back(port);
    }

    void set_metadata_logical_port(uint16_t port)
    {
        metadata_logical_port = port;
    }

    RTPS_DllAPI TCPTransportDescriptor()
        : SocketTransportDescriptor(65550, 4)
    {

    }

    RTPS_DllAPI TCPTransportDescriptor(const TCPTransportDescriptor& /*t*/)
        : SocketTransportDescriptor(65550, 4)
    {

    }

    virtual ~TCPTransportDescriptor() {}

} TCPTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // TCP_TRANSPORT_DESCRIPTOR_H
