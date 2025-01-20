// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <climits>

#include <gtest/gtest.h>

#include <fastdds/rtps/common/PortParameters.hpp>

using namespace eprosima::fastdds::rtps;

/*!
 * @fn TEST(PortParameters, Limit_Unicast_Domain_OK)
 * @brief This test checks the maximum valid domain with default parameters.
 */
TEST(PortParameters, Limit_Unicast_Domain_OK)
{
    PortParameters params;

    uint32_t port = params.getUnicastPort(232, 0);

    ASSERT_TRUE(port > 0);
}

/*!
 * @fn TEST(PortParameters, Limit_Unicast_Domain_Participant_OK)
 * @brief This test checks the maximum valid domain + participant with default parameters.
 */
TEST(PortParameters, Limit_Unicast_Domain_Participant_OK)
{
    PortParameters params;

    uint32_t port = params.getUnicastPort(232, 62);

    ASSERT_TRUE(port > 0);
}

/*!
 * @fn TEST(PortParametersDeathTest, Limit_Unicast_Domain_FAIL)
 * @brief This test checks the minimum invalid domain with default parameters.
 */
TEST(PortParametersDeathTest, Limit_Unicast_Domain_FAIL)
{
    PortParameters params;
    ASSERT_EXIT( { params.getUnicastPort(233, 0); }, ::testing::ExitedWithCode(EXIT_FAILURE), "");
}

/*!
 * @fn TEST(PortParametersDeathTest, Limit_Unicast_Domain_Participant_FAIL)
 * @brief This test checks the minimum invalid domain+participant with default parameters.
 */
TEST(PortParametersDeathTest, Limit_Unicast_Domain_Participant_FAIL)
{
    PortParameters params;
    ASSERT_EXIT( { params.getUnicastPort(232, 63); }, ::testing::ExitedWithCode(EXIT_FAILURE), "");
}

/*!
 * @fn TEST(PortParameters, Limit_Multicast_Domain_OK)
 * @brief This test checks the maximum valid domain with default parameters.
 */
TEST(PortParameters, Limit_Multicast_Domain_OK)
{
    PortParameters params;

    uint32_t port = params.getMulticastPort(232);

    ASSERT_TRUE(port > 0);
}

/*!
 * @fn TEST(PortParametersDeathTest, Limit_Multicast_Domain_FAIL)
 * @brief This test checks the minimum invalid domain with default parameters.
 */
TEST(PortParametersDeathTest, Limit_Multicast_Domain_FAIL)
{
    PortParameters params;
    ASSERT_EXIT( { params.getMulticastPort(233); }, ::testing::ExitedWithCode(EXIT_FAILURE), "");
}

/*!
 * @fn TEST(PortParameters, Limit_Unicast_Domain_OK)
 * @brief This test checks the maximum valid domain with default parameters.
 */
TEST(PortParameters, Limit_DiscoveryServer_Domain_OK)
{
    PortParameters params;
    uint32_t port = params.get_discovery_server_port(232);

    ASSERT_TRUE(port > 0);
}

/*!
 * @fn TEST(PortParametersDeathTest, Limit_DiscoveryServer_Domain_FAIL)
 * @brief This test checks the minimum invalid domain with default parameters.
 */
TEST(PortParametersDeathTest, Limit_DiscoveryServer_Domain_FAIL)
{
    PortParameters params;
    ASSERT_EXIT( { params.get_discovery_server_port(233); }, ::testing::ExitedWithCode(EXIT_FAILURE), "");
}


int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
