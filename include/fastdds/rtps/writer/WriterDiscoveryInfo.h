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
 * @file WriterDiscoveryInfo.h
 */
#ifndef _RTPS_WRITER_WRITERDISCOVERYINFO_H_
#define _RTPS_WRITER_WRITERDISCOVERYINFO_H_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
* Class WriterDiscoveryInfo with discovery information of the writer.
* @ingroup RTPS_MODULE
*/
struct WriterDiscoveryInfo
{
    public:

        //!Enum DISCOVERY_STATUS, four different status for discovered writers.
        //!@ingroup RTPS_MODULE
#if defined(_WIN32)
        enum RTPS_DllAPI DISCOVERY_STATUS
#else
        enum  DISCOVERY_STATUS
#endif
        {
            DISCOVERED_WRITER,
            CHANGED_QOS_WRITER,
            REMOVED_WRITER
        };

        WriterDiscoveryInfo(const WriterProxyData& data)
            : status(DISCOVERED_WRITER)
            , info(data)
        {}

        virtual ~WriterDiscoveryInfo() {}

        //! Status
        DISCOVERY_STATUS status;

        //! Participant discovery info
        const WriterProxyData& info;
};

}
}
}

#endif // _RTPS_WRITER_WRITERDISCOVERYINFO_H_
