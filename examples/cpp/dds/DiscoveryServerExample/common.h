// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file common.h
 *
 */

#pragma once

#ifndef EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_COMMON_H_
#define EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_COMMON_H_

#include <fastdds/rtps/attributes/ServerAttributes.h>

enum class TransportKind
{
    UDPv4,
    TCPv4,
    UDPv6,
    TCPv6,
    SHM,
};

inline eprosima::fastrtps::rtps::GuidPrefix_t get_discovery_server_guid_from_id(unsigned short id)
{
    eprosima::fastrtps::rtps::GuidPrefix_t result;

    // Get default DS guid and modify the one value expected to be changed
    std::istringstream(eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_GUIDPREFIX) >> result;
    result.value[2] =
        static_cast<eprosima::fastrtps::rtps::octet>(id); // This is done like this in Fast

    return result;
}

#endif /* EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_COMMON_H_ */
