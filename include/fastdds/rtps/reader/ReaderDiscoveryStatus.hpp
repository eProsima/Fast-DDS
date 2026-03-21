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

/**
 * @file ReaderDiscoveryStatus.hpp
 */

#ifndef FASTDDS_RTPS_READER__READERDISCOVERYSTATUS_HPP
#define FASTDDS_RTPS_READER__READERDISCOVERYSTATUS_HPP

#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Enum DISCOVERY_STATUS, four different status for discovered readers.
// *INDENT-OFF* : Does not understand the #if correctly and ends up removing the ;
//                at the end of the enum, which does not build.
#if defined(_WIN32)
enum class FASTDDS_EXPORTED_API ReaderDiscoveryStatus
#else
enum class ReaderDiscoveryStatus
#endif // if defined(_WIN32)
{
    DISCOVERED_READER,
    CHANGED_QOS_READER,
    REMOVED_READER,
    IGNORED_READER
};
// *INDENT-ON*

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_READER__READERDISCOVERYSTATUS_HPP
