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
 * @file ReaderDiscoveryInfo.hpp
 *
 */

#ifndef FASTDDS_RTPS_READER__READERDISCOVERYINFO_HPP
#define FASTDDS_RTPS_READER__READERDISCOVERYINFO_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/builtin/data/ReaderProxyData.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class ReaderDiscoveryInfo with discovery information of the reader.
 * @ingroup RTPS_MODULE
 */
struct ReaderDiscoveryInfo
{
public:

    //!Enum DISCOVERY_STATUS, four different status for discovered readers.
    //!@ingroup RTPS_MODULE
    // *INDENT-OFF* : Does not understand the #if correctly and ends up removing the ;
    //                at the end of the enum, which does not build.
#if defined(_WIN32)
    enum FASTDDS_EXPORTED_API DISCOVERY_STATUS
#else
    enum DISCOVERY_STATUS
#endif // if defined(_WIN32)
    {
        DISCOVERED_READER,
        CHANGED_QOS_READER,
        REMOVED_READER,
        IGNORED_READER
    };
    // *INDENT-ON*
    ReaderDiscoveryInfo(
            const ReaderProxyData& data)
        : status(DISCOVERED_READER)
        , info(data)
    {
    }

    virtual ~ReaderDiscoveryInfo()
    {
    }

    //! Status
    DISCOVERY_STATUS status;

    //! Participant discovery info
    const ReaderProxyData& info;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_READER__READERDISCOVERYINFO_HPP
