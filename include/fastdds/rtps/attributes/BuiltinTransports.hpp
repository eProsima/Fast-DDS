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

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Defines the kind of transports automatically instantiated upon the creation of a participant
 */
enum class BuiltinTransports
{
    NONE = 0,      //< No transport will be instantiated
    DEFAULT = 1,   //< Default value that will instantiate UDPv4 and SHM transports
    DEFAULTv6,     //< Instantiate UDPv6 and SHM transports
    SHM,           //< Instantiate SHM transport only
    UDPv4,         //< Instantiate UDPv4 transport only
    UDPv6,         //< Instantiate UDPv6 transport only
    LARGE_DATA,    //< Instantiate SHM, UDPv4 and TCPv4 transports, but UDPv4 is only used for bootstrapping discovery
    LARGE_DATAv6   //< Instantiate SHM, UDPv6 and TCPv6 transports, but UDPv6 is only used for bootstrapping discovery
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_RTPS_ATTRIBUTES__BUILTINTRANSPORTS_HPP_
