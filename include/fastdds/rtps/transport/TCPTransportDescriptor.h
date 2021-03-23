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

#ifndef _FASTDDS_TCP_TRANSPORT_DESCRIPTOR_H_
#define _FASTDDS_TCP_TRANSPORT_DESCRIPTOR_H_

#include <fastdds/rtps/transport/SocketTransportDescriptor.h>
#include <fastrtps/fastrtps_dll.h>
#include <iostream>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * TCP Transport configuration
 *
 * - listening_ports: list of ports to listen as server.
 *
 * - keep_alive_frequency_ms: frequency of RTCP keep alive requests (in ms).
 *
 * - keep_alive_timeout_ms: time since sending the last keep alive request to consider a connection as broken (in ms).
 *
 * - max_logical_port: maximum number of logical ports to try during RTCP negotiation.
 *
 * - logical_port_range: maximum number of logical ports per request to try during RTCP negotiation.
 *
 * - logical_port_increment: increment between logical ports to try during RTCP negotiation.
 *
 * - enable_tcp_nodelay: enables the TCP_NODELAY socket option.
 *
 * - calculate_crc: true to calculate and send CRC on message headers.
 *
 * - check_crc: true to check the CRC of incoming message headers.
 *
 * - apply_security: true to use TLS (Transport Layer Security).
 *
 * - tls_config: Configuration for TLS.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct TCPTransportDescriptor : public SocketTransportDescriptor
{
    /**
     * TLS Configuration
     *
     * - password: password of the private_key_file or rsa_private_key_file.
     *
     * - private_key_file: path to the private key certificate file.
     *
     * - rsa_private_key_file: path to the private key RSA certificate file.
     *
     * - cert_chain_file: path to the public certificate chain file.
     *
     * - tmp_dh_file: path to the Diffie-Hellman parameters file.
     *
     * - verify_file: path to the CA (Certification-Authority) file.
     *
     * - verify_mode: establishes the verification mode mask.
     *
     * - options: establishes the SSL Context options mask.
     *
     * - verify_paths: paths where the system will look for verification files.
     *
     * - default_verify_path: look for verification files on the default paths.
     *
     * - handshake_role: role that the transport will take on handshaking.
     *
     */
    struct TLSConfig
    {
        /**
         * Supported TLS features.
         * Several options can be combined in the same TransportDescriptor using the add_option() member function.
         *
         * - DEFAULT_WORKAROUNDS: implement various bug workarounds.
         *
         * - NO_COMPRESSION: disable compression.
         *
         * - NO_SSLV2: disable SSL v2.
         *
         * - NO_SSLV3: disable SSL v3.
         *
         * - NO_TLSV1: disable TLS v1.
         *
         * - NO_TLSV1_1: disable TLS v1.1.
         *
         * - NO_TLSV1_2: disable TLS v1.2.
         *
         * - NO_TLSV1_3: disable TLS v1.3.
         *
         * - SINGLE_DH_USE: always create a new key using Diffie-Hellman parameters.
         */
        enum TLSOptions : uint32_t
        {
            NONE                    = 0,      // 0000 0000 0000
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

        /**
         * Peer node verification options.
         * Several verification options can be combined in the same TransportDescriptor using the add_verify_mode()
         * member function.
         *
         * - VERIFY_NONE: perform no verification.
         *
         * - VERIFY_PEER: perform verification of the peer.
         *
         * - VERIFY_FAIL_IF_NO_PEER_CERT: fail verification if the peer has no certificate. Ignored unless VERIFY_PEER
         * is also set.
         *
         * - VERIFY_CLIENT_ONCE: do not request client certificate on renegotiation. Ignored unless VERIFY_PEER is also
         * set.
         */
        enum TLSVerifyMode : uint8_t
        {
            UNUSED                      = 0,      // 0000 0000
            VERIFY_NONE                 = 1 << 0, // 0000 0001
            VERIFY_PEER                 = 1 << 1, // 0000 0010
            VERIFY_FAIL_IF_NO_PEER_CERT = 1 << 2, // 0000 0100
            VERIFY_CLIENT_ONCE          = 1 << 3  // 0000 1000
        };

        /**
         * Role that the transport will take on handshaking.
         *
         * - DEFAULT: configured as client if connector, and as server if acceptor.
         *
         * - CLIENT: configured as client.
         *
         * - SERVER: configured as server.
         */
        enum TLSHandShakeRole : uint8_t
        {
            DEFAULT                     = 0,      // 0000 0000
            CLIENT                      = 1 << 0, // 0000 0001
            SERVER                      = 1 << 1  // 0000 0010
        };

        //! Password of the private_key_file or rsa_private_key_file
        std::string password;
        //! SSL context options mask
        uint32_t options = TLSOptions::NONE;
        //! Path to the public certificate chain file
        std::string cert_chain_file;
        //! Path to the private key certificate file
        std::string private_key_file;
        //! Path to the Diffie-Hellman parameters file
        std::string tmp_dh_file;
        //! Path to the CA (Certification-Authority) file.
        std::string verify_file;
        //! Verification mode mask
        uint8_t verify_mode = TLSVerifyMode::UNUSED;
        //! Paths where the system will look for verification files
        std::vector<std::string> verify_paths;
        //! Look for verification files on the default paths. Do not invoque
        bool default_verify_path = false;
        //! Maximum allowed depth for verifying intermediate certificates. Do not override
        int32_t verify_depth = -1;
        //! Path to the private key RSA certificate file
        std::string rsa_private_key_file;
        //! Role that the transport will take on handshaking
        TLSHandShakeRole handshake_role = TLSHandShakeRole::DEFAULT;

        //! Add verification modes to the verification mode mask
        void add_verify_mode(
                const TLSVerifyMode verify)
        {
            verify_mode |= verify;
        }

        //! Get the verification mode mask
        bool get_verify_mode(
                const TLSVerifyMode verify) const
        {
            return (verify_mode & verify) == verify;
        }

        //! Add TLS features to the SSL Context options mask
        void add_option(
                const TLSOptions option)
        {
            options |= option;
        }

        //! Get the SSL Context options mask
        bool get_option(
                const TLSOptions option) const
        {
            return (options & option) == option;
        }

        //! Comparison operator
        bool operator ==(
                const TLSConfig& t) const
        {
            return (this->password == t.password &&
                   this->options == t.options &&
                   this->cert_chain_file == t.cert_chain_file &&
                   this->private_key_file == t.private_key_file &&
                   this->tmp_dh_file == t.tmp_dh_file &&
                   this->verify_file == t.verify_file &&
                   this->verify_mode == t.verify_mode &&
                   this->verify_paths == t.verify_paths &&
                   this->default_verify_path == t.default_verify_path &&
                   this->verify_depth == t.verify_depth &&
                   this->rsa_private_key_file == t.rsa_private_key_file &&
                   this->handshake_role == t.handshake_role);
        }

    };

    //! List of ports to listen as server
    std::vector<uint16_t> listening_ports;
    //! Frequency of RTCP keep alive requests (ms)
    uint32_t keep_alive_frequency_ms;
    //! Time since sending the last keep alive request to consider a connection as broken (ms)
    uint32_t keep_alive_timeout_ms;
    //! Maximum number of logical ports to try during RTCP negotiation
    uint16_t max_logical_port;
    //! Maximum number of logical ports per request to try during RTCP negotiation
    uint16_t logical_port_range;
    //! Increment between logical ports to try during RTCP negotiation
    uint16_t logical_port_increment;

    FASTDDS_TODO_BEFORE(3, 0, "Eliminate tcp_negotiation_timeout, variable not in use.")
    uint32_t tcp_negotiation_timeout;

    //! Enables the TCP_NODELAY socket option
    bool enable_tcp_nodelay;

    FASTDDS_TODO_BEFORE(3, 0, "Eliminate wait_for_tcp_negotiation, variable not in use.")
    bool wait_for_tcp_negotiation;

    //! Enables the calculation and sending of CRC on message headers
    bool calculate_crc;
    //! Enables checking the CRC of incoming message headers
    bool check_crc;
    //! Enables the use of TLS (Transport Layer Security)
    bool apply_security;

    //! Configuration of the TLS (Transport Layer Security)
    TLSConfig tls_config;

    //! Add listener port to the listening_ports list
    void add_listener_port(
            uint16_t port)
    {
        listening_ports.push_back(port);
    }

    //! Constructor
    RTPS_DllAPI TCPTransportDescriptor();

    //! Copy constructor
    RTPS_DllAPI TCPTransportDescriptor(
            const TCPTransportDescriptor& t);

    //! Copy assignment
    RTPS_DllAPI TCPTransportDescriptor& operator =(
            const TCPTransportDescriptor& t);

    //! Destructor
    virtual ~TCPTransportDescriptor() = default;

    //! Comparison operator
    RTPS_DllAPI bool operator ==(
            const TCPTransportDescriptor& t) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TCP_TRANSPORT_DESCRIPTOR_H_
