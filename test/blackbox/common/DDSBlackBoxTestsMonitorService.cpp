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
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DDSMonitorServiceTest : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

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
 * Refers to DDS-MS-API-01 from the test plan.
 *
 * Check enable() disable() operations
 */
TEST(DDSMonitorServiceTest, monitor_service_enable_disable_api)
{

}

/**
 * Refers to DDS-MS-API-02 from the test plan.
 *
 * Checks fastdds.enable_monitor_service property
 */
TEST(DDSMonitorServiceTest, monitor_service_property)
{

}

/**
 * Refers to DDS-MS-API-03 from the test plan.
 *
 * Checks that appending MONITOR_SERVICE_TOPIC reserved name
 * in FASTDDS_STATISTICS enviroment variable properly initializes
 * the service.
 */
TEST(DDSMonitorServiceTest, monitor_service_environment_variable)
{

}

/**
 * Refers to DDS-MS-SIMPLE-01 from the test plan.
 *
 * A MSC correctly shall receive the corresponding proxy update in the MST after creating
 * an endpoint in a MSP
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_proxy)
{

}

/**
 * Refers to DDS-MS-SIMPLE-02 from the test plan.
 *
 * A MSC correctly shall receive the corresponding connection list update in the MST
 * after creating two MSP with a pair of matched endpoints.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_connection_list)
{

}

/**
 * Refers to DDS-MS-SIMPLE-03 from the test plan.
 *
 * MSC correctly receives a QoS incompatibility after adding a pair of reader/writer
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_qos_incompatibility_status)
{

}

/**
 * Refers to DDS-MS-SIMPLE-04 from the test plan.
 *
 * To implement when InconsistentTopciStatus is fully suported

TEST_P(DDSMonitorServiceTest, monitor_service_simple_inconsistent_topic)
{

}*/

/**
 * Refers to DDS-MS-SIMPLE-05 from the test plan.
 *
 * The lease duration of a writer created in the MSP expires and the liveliness lost status
 * is correctly notified to the MSC
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_liveliness_lost_status)
{

}

/**
 * Refers to DDS-MS-SIMPLE-06 from the test plan.
 *
 * In a MSP, the liveliness of a reader changes and the status is correctly notified to the
 * MSC.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_liveliness_changed_status)
{

}

/**
 * Refers to DDS-MS-SIMPLE-07 from the test plan.
 *
 * The deadline is forced to be missed in an entity of the MSP and the status is correctly
 * notified to the MSC.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_deadline_lost_status)
{

}

/**
 * Refers to DDS-MS-SIMPLE-08 from the test plan.
 *
 * Making use of the test transport, force loosing a sample in the MSP so that the lost
 * sample status is correctly notified to the MSC.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_sample_lost_status)
{

}

/**
 * Refers to DDS-MS-SIMPLE-09 from the test plan.
 *
 * Removing a previously created endpoint in the MSP makes MSC to receive the corre-
 * sponding instance disposals.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_instance_disposals)
{

}

/**
 * Refers to DDS-MS-SIMPLE-10 from the test plan.
 *
 * MSC late joins a MSP with an already enabled MS.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_late_joiner)
{

}

/**
 * Refers to DDS-MS-SIMPLE-11 from the test plan.
 *
 * Enabling the MS, disabling it, making some updates and re-enabling it shall correctly
 * behave.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_enable_disable_enable)
{

}

/**
 * Refers to DDS-MS-ADV-01 from the test plan.
 *
 * A MSC shall correctly receive the corresponding proxies from different MSPs.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_advanced_proxy)
{

}

/**
 * Refers to DDS-MS-ADV-02 from the test plan.
 *
 * A MSC shall correctly receive the corresponding instance disposals after deleting one
 * of the MSP.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_advanced_instance_disposals)
{

}

/**
 * Refers to DDS-MS-ADV-03 from the test plan.
 *
 * Multiple MSC shall correctly receive the updates after late joining.
 */
TEST_P(DDSMonitorServiceTest, monitor_service_advanced_late_joiners)
{

}


#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSMonitorServiceTest,
        DDSMonitorServiceTest,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<DDSMonitorServiceTest::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }
        });
