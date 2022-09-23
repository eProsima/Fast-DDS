// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/common/LocatorWithMask.hpp>
#include <fastrtps/utils/IPLocator.h>

#include <rtps/network/ExternalLocatorsProcessor.hpp>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastrtps::rtps;

// -------------------- Auxiliary methods to compare locator lists --------------------

static bool operator == (
        const eprosima::fastrtps::ResourceLimitedVector<Locator>& lhs,
        const eprosima::fastdds::rtps::LocatorList& rhs)
{
    LocatorList left_list;

    for (const Locator_t& loc : lhs)
    {
        left_list.push_back(loc);
    }

    return left_list == rhs;
}

static bool operator == (
        const eprosima::fastrtps::ResourceLimitedVector<Locator>& lhs,
        const eprosima::fastrtps::ResourceLimitedVector<Locator>& rhs)
{
    LocatorList right_list;

    for (const Locator_t& loc : rhs)
    {
        right_list.push_back(loc);
    }

    return lhs == right_list;
}

static bool operator == (
        const eprosima::fastrtps::rtps::RemoteLocatorList& lhs,
        const eprosima::fastrtps::rtps::RemoteLocatorList& rhs)
{
    return lhs.multicast == rhs.multicast && lhs.unicast == rhs.unicast;
}

// -------------------- Adding external locators to ParticipantProxyData --------------------

void single_participant_check(
        ParticipantProxyData& pdata,
        const ExternalLocators& def_ext_locators,
        const ExternalLocators& meta_ext_locators,
        const RemoteLocatorList& def_check_locators,
        const RemoteLocatorList& meta_check_locators)
{
    ExternalLocatorsProcessor::add_external_locators(pdata, meta_ext_locators, def_ext_locators);
    ASSERT_TRUE(pdata.default_locators == def_check_locators);
    ASSERT_TRUE(pdata.metatraffic_locators == meta_check_locators);
}

void test_add_external_locators_participant(
        ParticipantProxyData& working_data)
{
    ExternalLocators empty_locators;
    RemoteLocatorList def_empty_test_list(working_data.default_locators);
    RemoteLocatorList meta_empty_test_list(working_data.metatraffic_locators);

    RemoteLocatorList def_test_list(working_data.default_locators);
    RemoteLocatorList meta_test_list(working_data.metatraffic_locators);

    LocatorWithMask test_locator;
    std::stringstream stream("UDPv4:[1.1.1.1]:9999");
    stream >> test_locator;
    def_test_list.add_unicast_locator(test_locator);
    meta_test_list.add_unicast_locator(test_locator);

    ParticipantProxyData initial_data(working_data);

    // Adding empty external locators should leave working_data untouched
    single_participant_check(working_data, empty_locators, empty_locators, def_empty_test_list, meta_empty_test_list);

    // Adding empty external locators should leave working_data untouched
    {
        ExternalLocators accum_locators;

        for (uint8_t externality = std::numeric_limits<uint8_t>::max(); externality > 0; externality >>= 1)
        {
            for (uint8_t cost = 0; cost < std::numeric_limits<uint8_t>::max(); cost = (cost * 2) + 1)
            {
                for (uint8_t mask = 0; mask < 32; ++mask)
                {
                    ExternalLocators single_locator;
                    test_locator.mask(mask);
                    single_locator[externality][cost].emplace_back(test_locator);
                    accum_locators[externality][cost].emplace_back(test_locator);

                    working_data = initial_data;
                    single_participant_check(
                        working_data,
                        accum_locators, empty_locators,
                        def_test_list, meta_empty_test_list);
                    single_participant_check(
                        working_data,
                        single_locator, empty_locators,
                        def_test_list, meta_empty_test_list);

                    working_data = initial_data;
                    single_participant_check(
                        working_data,
                        single_locator, empty_locators,
                        def_test_list, meta_empty_test_list);
                    single_participant_check(
                        working_data,
                        accum_locators, empty_locators,
                        def_test_list, meta_empty_test_list);

                    working_data = initial_data;
                    single_participant_check(
                        working_data,
                        empty_locators, accum_locators,
                        def_empty_test_list, meta_test_list);
                    single_participant_check(
                        working_data,
                        empty_locators, single_locator,
                        def_empty_test_list, meta_test_list);

                    working_data = initial_data;
                    single_participant_check(
                        working_data,
                        empty_locators, single_locator,
                        def_empty_test_list, meta_test_list);
                    single_participant_check(
                        working_data,
                        empty_locators, accum_locators,
                        def_empty_test_list, meta_test_list);

                    working_data = initial_data;
                    single_participant_check(
                        working_data,
                        accum_locators, single_locator,
                        def_test_list, meta_test_list);
                    single_participant_check(
                        working_data,
                        single_locator, accum_locators,
                        def_test_list, meta_test_list);

                    working_data = initial_data;
                    single_participant_check(
                        working_data,
                        single_locator, accum_locators,
                        def_test_list, meta_test_list);
                    single_participant_check(
                        working_data,
                        accum_locators, single_locator,
                        def_test_list, meta_test_list);
                }
            }
        }
    }
}

TEST(ExternalLocatorsProcessorTests, add_external_locators_participant)
{
    Locator multicast_loc;
    {
        std::stringstream stream("UDPv4:[239.255.0.1]:12345");
        stream >> multicast_loc;
    }

    Locator unicast_loc;
    {
        std::stringstream stream("UDPv4:[10.10.10.10]:9999");
        stream >> unicast_loc;
    }

    {
        ParticipantProxyData data;
        test_add_external_locators_participant(data);
    }

    {
        ParticipantProxyData data;
        data.default_locators.add_multicast_locator(multicast_loc);
        test_add_external_locators_participant(data);
    }

    {
        ParticipantProxyData data;
        data.default_locators.add_unicast_locator(unicast_loc);
        test_add_external_locators_participant(data);
    }

    {
        ParticipantProxyData data;
        data.default_locators.add_unicast_locator(unicast_loc);
        data.default_locators.add_multicast_locator(multicast_loc);
        test_add_external_locators_participant(data);
    }

    {
        ParticipantProxyData data;
        data.metatraffic_locators.add_multicast_locator(multicast_loc);
        test_add_external_locators_participant(data);
    }

    {
        ParticipantProxyData data;
        data.metatraffic_locators.add_unicast_locator(unicast_loc);
        test_add_external_locators_participant(data);
    }

    {
        ParticipantProxyData data;
        data.metatraffic_locators.add_unicast_locator(unicast_loc);
        data.metatraffic_locators.add_multicast_locator(multicast_loc);
        test_add_external_locators_participant(data);
    }
}

// -------------------- Adding external locators to ReaderProxyData / WriterProxyData --------------------

template<typename ProxyData>
void single_endpoint_check(
        ProxyData& rdata,
        const ExternalLocators& ext_locators,
        const RemoteLocatorList& check_locators)
{
    ExternalLocatorsProcessor::add_external_locators(rdata, ext_locators);
    ASSERT_TRUE(rdata.remote_locators() == check_locators);
}

template<typename ProxyData>
void test_add_external_locators_endpoint(
        ProxyData& working_data)
{
    ExternalLocators empty_locators;
    RemoteLocatorList empty_test_list(working_data.remote_locators());
    RemoteLocatorList test_list(working_data.remote_locators());
    LocatorWithMask test_locator;
    std::stringstream stream("UDPv4:[1.1.1.1]:9999");
    stream >> test_locator;
    test_list.add_unicast_locator(test_locator);

    ProxyData initial_data(working_data);

    ASSERT_TRUE(working_data.remote_locators() == empty_test_list);

    // Adding empty external locators should leave working_data untouched
    single_endpoint_check(working_data, empty_locators, empty_test_list);

    // Adding empty external locators should leave working_data untouched
    {
        ExternalLocators accum_locators;

        for (uint8_t externality = std::numeric_limits<uint8_t>::max(); externality > 0; externality >>= 1)
        {
            for (uint8_t cost = 0; cost < std::numeric_limits<uint8_t>::max(); cost = (cost * 2) + 1)
            {
                for (uint8_t mask = 0; mask < 32; ++mask)
                {
                    ExternalLocators single_locator;
                    test_locator.mask(mask);
                    single_locator[externality][cost].emplace_back(test_locator);
                    accum_locators[externality][cost].emplace_back(test_locator);

                    working_data = initial_data;
                    single_endpoint_check(working_data, accum_locators, test_list);
                    single_endpoint_check(working_data, single_locator, test_list);

                    working_data = initial_data;
                    single_endpoint_check(working_data, single_locator, test_list);
                    single_endpoint_check(working_data, accum_locators, test_list);
                }
            }
        }
    }
}

template<typename ProxyData>
void test_add_external_locators_endpoint()
{

    Locator multicast_loc;
    {
        std::stringstream stream("UDPv4:[239.255.0.1]:12345");
        stream >> multicast_loc;
    }

    Locator unicast_loc;
    {
        std::stringstream stream("UDPv4:[10.10.10.10]:9999");
        stream >> unicast_loc;
    }

    {
        ProxyData data(4u, 1u);
        test_add_external_locators_endpoint(data);
    }

    {
        ProxyData data(4u, 1u);
        data.add_multicast_locator(multicast_loc);
        test_add_external_locators_endpoint(data);
    }

    {
        ProxyData data(4u, 1u);
        data.add_unicast_locator(unicast_loc);
        test_add_external_locators_endpoint(data);
    }

    {
        ProxyData data(4u, 1u);
        data.add_unicast_locator(unicast_loc);
        data.add_multicast_locator(multicast_loc);
        test_add_external_locators_endpoint(data);
    }
}

TEST(ExternalLocatorsProcessorTests, add_external_locators_reader)
{
    test_add_external_locators_endpoint<ReaderProxyData>();
}

TEST(ExternalLocatorsProcessorTests, add_external_locators_writer)
{
    test_add_external_locators_endpoint<WriterProxyData>();
}

// -------------------- Locator matching algorithm --------------------

/*******************************************************************************************************************
*                                                                                                                 *
*  DEPLOYMENT EXAMPLE SCENARIO                                                         +---------+                *
*                                                        +-------- [INTERNET] -------- | Node ZZ |                *
*  Router_NN and Host_N                                  |                             +---------+                *
*  have direct port mappings                      +------------+                       100.10.10.1                *
*  for UDP 7410-7417                              | WAN Router |                       7410 / 7411                *
*                                                 +------------+                                                  *
*                                                        |                                                        *
*                     +-----------+ 192.168.10.10        |        192.168.10.11 +-----------+                     *
*                     | Router_10 |---------------------------------------------| Router_11 |                     *
*                     +-----------+                                             +-----------+                     *
*                           |                                                         |                           *
*             +---------------------------+                             +---------------------------+             *
*  10.10.10.4 |                10.10.10.5 |                  10.10.10.4 |                10.10.10.5 |             *
*        +---------+                 +---------+                   +---------+                 +---------+        *
*        | Host_1  |                 | Host_2  |                   | Host_3  |                 | Host_4  |        *
*        +---------+                 +---------+                   +---------+                 +---------+        *
*             |                           |                             |                           |             *
*      +-------------+             +-------------+               +-------------+             +-------------+      *
*      |             |             |             |               |             |             |             |      *
* +---------+   +---------+   +---------+   +---------+     +---------+   +---------+   +---------+   +---------+ *
* | Node_11 |   | Node_12 |   | Node_21 |   | Node_22 |     | Node_31 |   | Node_32 |   | Node_41 |   | Node_42 | *
* +---------+   +---------+   +---------+   +---------+     +---------+   +---------+   +---------+   +---------+ *
* 172.17.0.10   172.17.0.11   172.17.0.10   172.17.0.11     172.17.0.10   172.17.0.11   172.17.0.10   172.17.0.11 *
* 7410 / 7411   7412 / 7413   7414 / 7415   7416 / 7417     7410 / 7411   7412 / 7413   7414 / 7415   7416 / 7417 *
*                                                                                                                 *
*******************************************************************************************************************/

struct ExternalAddress
{
    ExternalAddress(
            const std::string& address,
            uint32_t metatraffic_port,
            uint32_t user_port)
        : ExternalAddress(address, 24, metatraffic_port, user_port)
    {
    }

    ExternalAddress(
            const std::string& address,
            uint8_t mask,
            uint32_t metatraffic_port,
            uint32_t user_port)
    {
        user.kind = LOCATOR_KIND_UDPv4;
        user.port = user_port;
        user.mask(mask);
        IPLocator::setIPv4(user, address);

        metatraffic = user;
        metatraffic.port = metatraffic_port;
    }

    LocatorWithMask user;
    LocatorWithMask metatraffic;
};

struct BasicNodeConfig
{
    BasicNodeConfig(
            const std::string& wan_address,
            const std::string& host_address,
            const std::string& node_address,
            uint32_t metatraffic_port,
            uint32_t user_port)
        : wan_locators(wan_address, metatraffic_port, user_port)
        , host_locators(host_address, metatraffic_port, user_port)
        , container_locators(node_address, metatraffic_port, user_port)
    {
    }

    // User and metatraffic locators for externality 2
    ExternalAddress wan_locators;
    // User and metatraffic locators for externality 1
    ExternalAddress host_locators;
    // User and metatraffic locators for externality 0
    ExternalAddress container_locators;
};

/* All nodes except Node_Z, which has no external locators */
static const std::array<BasicNodeConfig, 8> internal_nodes =
{
    /* Node_11 */ BasicNodeConfig{"192.168.10.10", "10.10.10.4", "172.17.0.10", 7410, 7411},
    /* Node_12 */ BasicNodeConfig{"192.168.10.10", "10.10.10.4", "172.17.0.11", 7412, 7413},
    /* Node_21 */ BasicNodeConfig{"192.168.10.10", "10.10.10.5", "172.17.0.10", 7414, 7415},
    /* Node_22 */ BasicNodeConfig{"192.168.10.10", "10.10.10.5", "172.17.0.11", 7416, 7417},
    /* Node_31 */ BasicNodeConfig{"192.168.10.11", "10.10.10.4", "172.17.0.10", 7410, 7411},
    /* Node_32 */ BasicNodeConfig{"192.168.10.11", "10.10.10.4", "172.17.0.11", 7412, 7413},
    /* Node_41 */ BasicNodeConfig{"192.168.10.11", "10.10.10.5", "172.17.0.10", 7414, 7415},
    /* Node_42 */ BasicNodeConfig{"192.168.10.11", "10.10.10.5", "172.17.0.11", 7416, 7417}
};

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
