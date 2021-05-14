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

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/SampleIdentity.h>
#include <fastdds/rtps/common/Time_t.h>
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

    /**
     * @brief Report a change on the number of DATA / DATAFRAG submessages sent for a specific sample.
     * @param SampleIdentity of the affected sample.
     * @param Current total number of submessages sent for the affected sample.
     */
    inline void on_sample_datas(
            const fastrtps::rtps::SampleIdentity&,
            size_t)
    {
    }

    /**
     * @brief Report that a HEARTBEAT message is sent
     * @param current count of heartbeats
     */
    inline void on_heartbeat(
            uint32_t)
    {
    }

    /**
     * @brief Report that a DATA / DATA_FRAG message is generated
     * @param number of locators to which the message will be sent
     */
    inline void on_data_generated(
            size_t)
    {
    }

    /// Notify listeners of DATA / DATA_FRAG counts
    inline void on_data_sent()
    {
    }

    /**
     * @brief Reports publication throughtput based on last added sample to writer's history
     * @param size of the message sent
     */
    inline void on_publish_throughput(
            uint32_t)
    {
    }

    /// Report that a GAP message is sent
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

    /**
     * @brief Report that a sample has been notified to the user.
     * @param GUID of the writer from where the sample was received.
     * @param Source timestamp received from the writer for the sample being notified.
     */
    inline void on_data_notify(
            const fastrtps::rtps::GUID_t&,
            const fastrtps::rtps::Time_t&)
    {
    }

    /**
     * @brief Report that an ACKNACK message is sent
     * @param current count of ACKNACKs
     */
    inline void on_acknack(
            int32_t)
    {
    }

    /**
     * @brief Report that a NACKFRAG message is sent
     * @param current count of NACKFRAGs
     */
    inline void on_nackfrag(
            int32_t)
    {
    }

    /**
     * @brief Reports throughtput based on last added sample to history
     * @param size of the message received
     */
    inline void on_subscribe_throughput(
            uint32_t)
    {
    }

};

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
