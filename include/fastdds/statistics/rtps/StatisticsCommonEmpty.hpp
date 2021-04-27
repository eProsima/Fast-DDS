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
     * @brief Report that a HEARTBEAT message is sent
     * @param current count of heartbeats
     */
    void on_heartbeat(
            uint32_t)
    {
    }

    //! Report that a DATA message is sent
    inline void on_data()
    {
    }

    //! Report that a DATA_FRAG message is sent
    inline void on_data_frag()
    {
    }

    //! Report that a GAP message is sent
    inline void on_gap()
    {
    }

    /*
     * @brief Report that several changes are marked for redelivery
     * @param number of changes to redeliver
     */
    inline void on_resent_data(
            uint32_t)
    {
    }

};

class StatisticsReaderImpl
{
    friend class fastrtps::rtps::RTPSMessageGroup;

protected:

    // TODO: methods for listeners callbacks

    /*
     * @brief Report that an ACKNACK message is sent
     * @param current count of ACKNACKs
     */
    inline void on_acknack(
            int32_t)
    {
    }

    /*
     * @brief Report that a NACKFRAG message is sent
     * @param current count of NACKFRAGs
     */
    inline void on_nackfrag(
            int32_t)
    {
    }

};

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
