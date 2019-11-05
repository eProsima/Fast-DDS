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

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <chrono>
#include <thread>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * @brief Class RTPSDomainImpl, contains the private implementation of the RTPSDomain
 * @ingroup RTPS_MODULE
 */
class RTPSDomainImpl
{
public:

    /***
     * @returns A pointer to a local reader given its endpoint guid, or nullptr if not found.
     */
    static RTPSReader* find_local_reader(
            const GUID_t& reader_guid)
    {
        for (auto participant : RTPSDomain::m_RTPSParticipants)
        {
            // Reader found
            if (auto reader = participant.second->find_local_reader(reader_guid))
            {
                return reader;
            }
        }

        return nullptr;
    }

    /***
     * @returns A pointer to a local writer given its endpoint guid, or nullptr if not found.
     */
    static RTPSReader* find_local_writer(
            const GUID_t& writer_guid)
    {
        for (auto participant : RTPSDomain::m_RTPSParticipants)
        {
            // Writer found
            if (auto writer = participant.second->find_local_writer(writer_guid))
            {
                return writer;
            }
        }

        return nullptr;
    }
};

}
}
}
