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
 * @file ReaderDiscoveryInfo.h
 *
 */

#ifndef _FASTDDS_RTPS_READER_READERDISCOVERYINFO_H__
#define _FASTDDS_RTPS_READER_READERDISCOVERYINFO_H__

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>

namespace eprosima {
namespace fastrtps {
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
#if defined(_WIN32)
        enum RTPS_DllAPI DISCOVERY_STATUS
#else
        enum DISCOVERY_STATUS
#endif
        {
            DISCOVERED_READER,
            CHANGED_QOS_READER,
            REMOVED_READER
        };

        ReaderDiscoveryInfo(const ReaderProxyData& data)
            : status(DISCOVERED_READER)
            , info(data)
        {}

        virtual ~ReaderDiscoveryInfo() {}

        //! Status
        DISCOVERY_STATUS status;

        //! Participant discovery info
        const ReaderProxyData& info;
};

}
}
}

#endif // _FASTDDS_RTPS_READER_READERDISCOVERYINFO_H__
