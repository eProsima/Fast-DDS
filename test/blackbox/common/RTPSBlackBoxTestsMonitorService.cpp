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

/**
 * Refers to RTPS-MS-API-01 from the test plan.
 *
 * The MS RTPS API create() operation in a RTPSParticipant shall be correctly per-
 * formed. The is_created() RTPS API shall be coherent with the current status of
 * the MS in the RTPSParticipant.
 */
TEST(RTPSMonitorServiceTest, monitor_service_create_is_created)
{

}

/**
 * Refers to RTPS-MS-API-02 from the test plan.
 *
 * The MS RTPS API enable() operation in a RTPSParticipant shall be correctly per-
 * formed. The disable() RTPS API properly finishes on a previously enabled MS.
 */
TEST(RTPSMonitorServiceTest, monitor_service_create_enable_disable)
{

}

