// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BuiltinTransports.hpp
 */

#ifndef FASTDDS_RTPS_ATTRIBUTES__BUILTINTRANSPORTS_HPP
#define FASTDDS_RTPS_ATTRIBUTES__BUILTINTRANSPORTS_HPP

#include <ostream>
#include <cstdint>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {


/**
 * @brief Options for configuring the built-in transports when using LARGE_DATA mode.
 */
struct FASTDDS_EXPORTED_API BuiltinTransportsOptions
{
    //! Whether to use non-blocking send operation.
    bool non_blocking_send = false;

    /**
     * @brief The maximum message size to be used.
     *
     * It specifies the maximum message size that will be used by the Network Factory
     * to register every transport.
     *
     */
    uint32_t maxMessageSize = fastdds::rtps::s_maximumMessageSize;

    /**
     * @brief The value used to configure the send and receive fuffer sizes of the sockets.
     *
     * It specifies the value that will be used to configure the send and receive buffer sizes of the sockets
     * used by the transports created with the builtin transports.
     * Zero value indicates to use default system buffer size.
     *
     */
    uint32_t sockets_buffer_size = 0;

    /**
     * @brief Time to wait for logical port negotiation (ms).
     *
     * It specifies the value that will be used to configure the honomym attribute of the TCPTransportDescriptor used.
     * It only takes effect if the LARGE_DATA mode is used.
     * Zero value means no waiting (default).
     *
     */
    uint32_t tcp_negotiation_timeout = 0;
};

/**
 * @brief Equal to operator.
 *
 * @param bto1 Left hand side BuiltinTransportsOptions being compared.
 * @param bto2 Right hand side BuiltinTransportsOptions being compared.
 * @return true if \c bto1 is equal to  \c bto2.
 * @return false otherwise.
 */
inline bool operator ==(
        const BuiltinTransportsOptions& bto1,
        const BuiltinTransportsOptions& bto2)
{
    if (bto1.non_blocking_send != bto2.non_blocking_send)
    {
        return false;
    }
    if (bto1.maxMessageSize != bto2.maxMessageSize)
    {
        return false;
    }
    if (bto1.sockets_buffer_size != bto2.sockets_buffer_size)
    {
        return false;
    }
    if (bto1.tcp_negotiation_timeout != bto2.tcp_negotiation_timeout)
    {
        return false;
    }
    return true;
}

/**
 * Defines the kind of transports automatically instantiated upon the creation of a participant
 */
enum class BuiltinTransports : uint16_t
{
    NONE = 0,          //< No transport will be instantiated
    DEFAULT = 1,       //< Default value that will instantiate UDPv4 and SHM transports
    DEFAULTv6 = 2,     //< Instantiate UDPv6 and SHM transports
    SHM = 3,           //< Instantiate SHM transport only
    UDPv4 = 4,         //< Instantiate UDPv4 transport only
    UDPv6 = 5,         //< Instantiate UDPv6 transport only
    LARGE_DATA = 6,    //< Instantiate SHM, UDPv4 and TCPv4 transports, but UDPv4 is only used for bootstrapping discovery
    LARGE_DATAv6 = 7,  //< Instantiate SHM, UDPv6 and TCPv6 transports, but UDPv6 is only used for bootstrapping discovery
    P2P = 8            //< Instantiate SHM, UDPv4 (unicast) and TCPv4 transports, shall only be used along with ROS2_EASY_MODE=<ip>
};

inline std::ostream& operator <<(
        std::ostream& output,
        BuiltinTransports transports)
{
    switch (transports)
    {
        case BuiltinTransports::NONE:
            output << "NONE";
            break;
        case BuiltinTransports::DEFAULT:
            output << "DEFAULT";
            break;
        case BuiltinTransports::DEFAULTv6:
            output << "DEFAULTv6";
            break;
        case BuiltinTransports::SHM:
            output << "SHM";
            break;
        case BuiltinTransports::UDPv4:
            output << "UDPv4";
            break;
        case BuiltinTransports::UDPv6:
            output << "UDPv6";
            break;
        case BuiltinTransports::LARGE_DATA:
            output << "LARGE_DATA";
            break;
        case BuiltinTransports::LARGE_DATAv6:
            output << "LARGE_DATAv6";
            break;
        case BuiltinTransports::P2P:
            output << "P2P";
            break;
        default:
            output << "UNKNOWN";
            break;
    }
    return output;
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_ATTRIBUTES__BUILTINTRANSPORTS_HPP
