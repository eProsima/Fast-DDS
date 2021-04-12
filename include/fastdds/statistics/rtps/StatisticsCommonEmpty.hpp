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
 * @file StatisticsCommonEmpty.hpp
 */

#ifndef _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
#define _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/statistics/IListeners.hpp>

namespace eprosima {

namespace fastrtps {
namespace rtps {

class RTPSMessageGroup;

} // rtps
} // fastrtps

namespace fastdds {
namespace statistics {

class StatisticsWriterImpl
{
protected:

    // TODO: methods for listeners callbacks

    /*
     * @brief Report a HEARTBEAT message is sent
     * @param current count of heartbeats
     */
    void on_heartbeat(
            uint32_t count)
    {
        (void)count;
    }

    //! Report a DATA message is sent
    inline void on_data()
    {
    }

    //! Report a DATA_FRAG message is sent
    inline void on_data_frag()
    {
    }

};

class StatisticsReaderImpl
{
    friend class fastrtps::rtps::RTPSMessageGroup;

protected:

    // TODO: methods for listeners callbacks

    //! Report that an ACKNACK message is sent
    inline void on_acknack(
            int32_t )
    {
    }

};

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
