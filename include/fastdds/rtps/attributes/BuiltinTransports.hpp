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

#ifndef _FASTDDS_RTPS_ATTRIBUTES__BUILTINTRANSPORTS_HPP_
#define _FASTDDS_RTPS_ATTRIBUTES__BUILTINTRANSPORTS_HPP_

#include <ostream>
#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Defines the kind of transports automatically instantiated upon the creation of a participant
 */
enum class BuiltinTransports : uint16_t
{
    NONE = 0,      //< No transport will be instantiated
    DEFAULT = 1,   //< Default value that will instantiate UDPv4 and SHM transports
    DEFAULTv6 = 2,     //< Instantiate UDPv6 and SHM transports
    SHM = 3,           //< Instantiate SHM transport only
    UDPv4 = 4,         //< Instantiate UDPv4 transport only
    UDPv6 = 5,         //< Instantiate UDPv6 transport only
    LARGE_DATA = 6,    //< Instantiate SHM, UDPv4 and TCPv4 transports, but UDPv4 is only used for bootstrapping discovery
    LARGE_DATAv6 = 7   //< Instantiate SHM, UDPv6 and TCPv6 transports, but UDPv6 is only used for bootstrapping discovery
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
        default:
            output << "UNKNOWN";
            break;
    }
    return output;
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_RTPS_ATTRIBUTES__BUILTINTRANSPORTS_HPP_
