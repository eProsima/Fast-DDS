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

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"

#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>

using namespace eprosima::fastdds::rtps;

/*
 * Abbreviations
 * +--------+----------------------------+
 * | Abbr   |  Description               |
 * +--------+----------------------------+
 * | MS     | Monitor Service            |
 * +--------+----------------------------+
 * | MSC    | Monitor Service Consumer   |
 * +--------+----------------------------+
 * | MSP    | Monitor Service Participant|
 * +--------+----------------------------+
 * | MSP    | Monitor Service Topic      |
 * +--------+----------------------------+
 */


class MonitorServiceRTPSParticipant
{

public:

    MonitorServiceRTPSParticipant()
    {

    }

#ifdef FASTDDS_STATISTICS

    void init()
    {
        rtps_participant_ = RTPSDomain::createParticipant(
            static_cast<uint32_t>(GET_PID()) % 230, RTPSParticipantAttributes());

        ASSERT_NE(rtps_participant_, nullptr);
    }

    bool enable_monitor_service()
    {
        return rtps_participant_->enable_monitor_service();
    }

    bool disable_monitor_service()
    {
        return rtps_participant_->disable_monitor_service();
    }

    bool is_monitor_service_created()
    {
        return rtps_participant_->is_monitor_service_created();
    }

    bool create_monitor_service()
    {
        return rtps_participant_->create_monitor_service();
    }

#endif //FASTDDS_STATISTICS

protected:

    RTPSParticipant* rtps_participant_;
};


/**
 * Refers to RTPS-MS-API-01 from the test plan.
 *
 * The MS RTPS API create() operation in a RTPSParticipant shall be correctly per-
 * formed. The is_created() RTPS API shall be coherent with the current status of
 * the MS in the RTPSParticipant.
 */
TEST(RTPSMonitorServiceTest, monitor_service_create_is_created)
{
    #ifdef FASTDDS_STATISTICS

    //! Setup
    MonitorServiceRTPSParticipant MSRTPS;

    //! Procedure
    MSRTPS.init();

    //! Assertions
    ASSERT_FALSE(MSRTPS.is_monitor_service_created());
    ASSERT_TRUE(MSRTPS.create_monitor_service());
    ASSERT_TRUE(MSRTPS.is_monitor_service_created());

    #endif //FASTDDS_STATISTICS
}

/**
 * Refers to RTPS-MS-API-02 from the test plan.
 *
 * The MS RTPS API enable() operation in a RTPSParticipant shall be correctly per-
 * formed. The disable() RTPS API properly finishes on a previously enabled MS.
 */
TEST(RTPSMonitorServiceTest, monitor_service_create_enable_disable)
{
    #ifdef FASTDDS_STATISTICS

    //! Setup
    MonitorServiceRTPSParticipant MSRTPS;

    //! Procedure
    MSRTPS.init();

    //! Assertions
    ASSERT_TRUE(MSRTPS.create_monitor_service());
    ASSERT_TRUE(MSRTPS.enable_monitor_service());
    ASSERT_TRUE(MSRTPS.disable_monitor_service());

    #endif //FASTDDS_STATISTICS
}

