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
#include <fastdds/utils/IPLocator.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/network/utils/external_locators.hpp>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::rtps::network;

// -------------------- Auxiliary methods to compare locator lists --------------------

static bool operator == (
        const eprosima::fastdds::ResourceLimitedVector<Locator>& lhs,
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
        const eprosima::fastdds::ResourceLimitedVector<Locator>& lhs,
        const eprosima::fastdds::ResourceLimitedVector<Locator>& rhs)
{
    LocatorList right_list;

    for (const Locator_t& loc : rhs)
    {
        right_list.push_back(loc);
    }

    return lhs == right_list;
}

static bool operator == (
        const eprosima::fastdds::rtps::RemoteLocatorList& lhs,
        const eprosima::fastdds::rtps::RemoteLocatorList& rhs)
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
    external_locators::add_external_locators(pdata, meta_ext_locators, def_ext_locators);
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

TEST(ExternalLocatorsTests, add_external_locators_participant)
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
    external_locators::add_external_locators(rdata, ext_locators);
    ASSERT_TRUE(rdata.remote_locators == check_locators);
}

template<typename ProxyData>
void test_add_external_locators_endpoint(
        ProxyData& working_data)
{
    ExternalLocators empty_locators;
    RemoteLocatorList empty_test_list(working_data.remote_locators);
    RemoteLocatorList test_list(working_data.remote_locators);
    LocatorWithMask test_locator;
    std::stringstream stream("UDPv4:[1.1.1.1]:9999");
    stream >> test_locator;
    test_list.add_unicast_locator(test_locator);

    ProxyData initial_data(working_data);

    ASSERT_TRUE(working_data.remote_locators == empty_test_list);

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

TEST(ExternalLocatorsTests, add_external_locators_reader)
{
    test_add_external_locators_endpoint<ReaderProxyData>();
}

TEST(ExternalLocatorsTests, add_external_locators_writer)
{
    test_add_external_locators_endpoint<WriterProxyData>();
}

// -------------------- Locator matching algorithm --------------------

/*******************************************************************************************************************/
/*  DEPLOYMENT EXAMPLE SCENARIO                                                         +---------+                */
/*                                                        +-------- [INTERNET] -------- | Node_ZZ |                */
/*  Router_NN and Host_N                                  |                             +---------+                */
/*  have direct port mappings                      +------------+                       100.10.10.1                */
/*  for UDP 7410-7417                              | WAN Router |                       7410 / 7411                */
/*                                                 +------------+                                                  */
/*                                                        |                                                        */
/*                     +-----------+ 192.168.10.10        |        192.168.10.11 +-----------+                     */
/*                     | Router_10 |---------------------------------------------| Router_11 |                     */
/*                     +-----------+                                             +-----------+                     */
/*                           |                                                         |                           */
/*             +---------------------------+                             +---------------------------+             */
/*  10.10.10.4 |                10.10.10.5 |                  10.10.10.4 |                10.10.10.5 |             */
/*        +---------+                 +---------+                   +---------+                 +---------+        */
/*        | Host_1  |                 | Host_2  |                   | Host_3  |                 | Host_4  |        */
/*        +---------+                 +---------+                   +---------+                 +---------+        */
/*             |                           |                             |                           |             */
/*      +-------------+             +-------------+               +-------------+             +-------------+      */
/*      |             |             |             |               |             |             |             |      */
/* +---------+   +---------+   +---------+   +---------+     +---------+   +---------+   +---------+   +---------+ */
/* | Node_11 |   | Node_12 |   | Node_21 |   | Node_22 |     | Node_31 |   | Node_32 |   | Node_41 |   | Node_42 | */
/* +---------+   +---------+   +---------+   +---------+     +---------+   +---------+   +---------+   +---------+ */
/* 172.17.0.10   172.17.0.11   172.17.0.10   172.17.0.11     172.17.0.10   172.17.0.11   172.17.0.10   172.17.0.11 */
/* 7410 / 7411   7412 / 7413   7414 / 7415   7416 / 7417     7410 / 7411   7412 / 7413   7414 / 7415   7416 / 7417 */
/*                                                                                                                 */
/*******************************************************************************************************************/

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
        announced_data.metatraffic_locators.add_unicast_locator(container_locators.metatraffic);
        announced_data.default_locators.add_unicast_locator(container_locators.user);
        announced_data.metatraffic_locators.add_unicast_locator(host_locators.metatraffic);
        announced_data.default_locators.add_unicast_locator(host_locators.user);
        announced_data.metatraffic_locators.add_unicast_locator(wan_locators.metatraffic);
        announced_data.default_locators.add_unicast_locator(wan_locators.user);
    }

    struct ExperimentConfig
    {
        uint8_t host_externality;
        uint8_t host_cost;
        uint8_t wan_externality;
        uint8_t wan_cost;
    };

    ExternalLocators get_metatraffic_external_locators(
            ExperimentConfig cfg = {1, 0, 2, 0}) const
    {
        ExternalLocators ret_value;

        ret_value[0][0].push_back(container_locators.metatraffic);
        ret_value[cfg.host_externality][cfg.host_cost].push_back(host_locators.metatraffic);
        ret_value[cfg.wan_externality][cfg.wan_cost].push_back(wan_locators.metatraffic);

        return ret_value;
    }

    ExternalLocators get_default_external_locators(
            ExperimentConfig cfg = {1, 0, 2, 0}) const
    {
        ExternalLocators ret_value;

        ret_value[0][0].push_back(container_locators.user);
        ret_value[cfg.host_externality][cfg.host_cost].push_back(host_locators.user);
        ret_value[cfg.wan_externality][cfg.wan_cost].push_back(wan_locators.user);

        return ret_value;
    }

    // User and metatraffic locators for externality 2
    ExternalAddress wan_locators;
    // User and metatraffic locators for externality 1
    ExternalAddress host_locators;
    // User and metatraffic locators for externality 0
    ExternalAddress container_locators;

    ParticipantProxyData announced_data;
};

/* All nodes except Node_ZZ, which has no external locators */
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

// The key of this map is a pair representing <local_node, discovered_node>
// The value is a ExternalAddress with the locators that should be selected by local_node to communicate with the
// discovered_node
const std::map<std::pair<size_t, size_t>, ExternalAddress> expected_communication_results
{
    // Node_11
    { {0, 0}, {"172.17.0.10",   7410, 7411} },
    { {0, 1}, {"172.17.0.11",   7412, 7413} },
    { {0, 2}, {"10.10.10.5",    7414, 7415} },
    { {0, 3}, {"10.10.10.5",    7416, 7417} },
    { {0, 4}, {"192.168.10.11", 7410, 7411} },
    { {0, 5}, {"192.168.10.11", 7412, 7413} },
    { {0, 6}, {"192.168.10.11", 7414, 7415} },
    { {0, 7}, {"192.168.10.11", 7416, 7417} },
    // Node_12
    { {1, 0}, {"172.17.0.10",   7410, 7411} },
    { {1, 1}, {"172.17.0.11",   7412, 7413} },
    { {1, 2}, {"10.10.10.5",    7414, 7415} },
    { {1, 3}, {"10.10.10.5",    7416, 7417} },
    { {1, 4}, {"192.168.10.11", 7410, 7411} },
    { {1, 5}, {"192.168.10.11", 7412, 7413} },
    { {1, 6}, {"192.168.10.11", 7414, 7415} },
    { {1, 7}, {"192.168.10.11", 7416, 7417} },
    // Node_21
    { {2, 0}, {"10.10.10.4",    7410, 7411} },
    { {2, 1}, {"10.10.10.4",    7412, 7413} },
    { {2, 2}, {"172.17.0.10",   7414, 7415} },
    { {2, 3}, {"172.17.0.11",   7416, 7417} },
    { {2, 4}, {"192.168.10.11", 7410, 7411} },
    { {2, 5}, {"192.168.10.11", 7412, 7413} },
    { {2, 6}, {"192.168.10.11", 7414, 7415} },
    { {2, 7}, {"192.168.10.11", 7416, 7417} },
    // Node_22
    { {3, 0}, {"10.10.10.4",    7410, 7411} },
    { {3, 1}, {"10.10.10.4",    7412, 7413} },
    { {3, 2}, {"172.17.0.10",   7414, 7415} },
    { {3, 3}, {"172.17.0.11",   7416, 7417} },
    { {3, 4}, {"192.168.10.11", 7410, 7411} },
    { {3, 5}, {"192.168.10.11", 7412, 7413} },
    { {3, 6}, {"192.168.10.11", 7414, 7415} },
    { {3, 7}, {"192.168.10.11", 7416, 7417} },
    // Node_31
    { {4, 0}, {"192.168.10.10", 7410, 7411} },
    { {4, 1}, {"192.168.10.10", 7412, 7413} },
    { {4, 2}, {"192.168.10.10", 7414, 7415} },
    { {4, 3}, {"192.168.10.10", 7416, 7417} },
    { {4, 4}, {"172.17.0.10",   7410, 7411} },
    { {4, 5}, {"172.17.0.11",   7412, 7413} },
    { {4, 6}, {"10.10.10.5",    7414, 7415} },
    { {4, 7}, {"10.10.10.5",    7416, 7417} },
    // Node_32
    { {5, 0}, {"192.168.10.10", 7410, 7411} },
    { {5, 1}, {"192.168.10.10", 7412, 7413} },
    { {5, 2}, {"192.168.10.10", 7414, 7415} },
    { {5, 3}, {"192.168.10.10", 7416, 7417} },
    { {5, 4}, {"172.17.0.10",   7410, 7411} },
    { {5, 5}, {"172.17.0.11",   7412, 7413} },
    { {5, 6}, {"10.10.10.5",    7414, 7415} },
    { {5, 7}, {"10.10.10.5",    7416, 7417} },
    // Node_41
    { {6, 0}, {"192.168.10.10", 7410, 7411} },
    { {6, 1}, {"192.168.10.10", 7412, 7413} },
    { {6, 2}, {"192.168.10.10", 7414, 7415} },
    { {6, 3}, {"192.168.10.10", 7416, 7417} },
    { {6, 4}, {"10.10.10.4",    7410, 7411} },
    { {6, 5}, {"10.10.10.4",    7412, 7413} },
    { {6, 6}, {"172.17.0.10",   7414, 7415} },
    { {6, 7}, {"172.17.0.11",   7416, 7417} },
    // Node_42
    { {7, 0}, {"192.168.10.10", 7410, 7411} },
    { {7, 1}, {"192.168.10.10", 7412, 7413} },
    { {7, 2}, {"192.168.10.10", 7414, 7415} },
    { {7, 3}, {"192.168.10.10", 7416, 7417} },
    { {7, 4}, {"10.10.10.4",    7410, 7411} },
    { {7, 5}, {"10.10.10.4",    7412, 7413} },
    { {7, 6}, {"172.17.0.10",   7414, 7415} },
    { {7, 7}, {"172.17.0.11",   7416, 7417} },
};

void test_matching_locators_scenario(
        bool ignore_non_matching)
{
    const std::vector<BasicNodeConfig::ExperimentConfig> configs =
    {
        {1, 0, 2, 0},
        {1, 1, 1, 0},
        {2, 100, 2, 50}
    };

    for (const auto& cfg : configs)
    {
        // Node to node test
        for (const auto& test_case : expected_communication_results)
        {
            const BasicNodeConfig& from_node = internal_nodes[test_case.first.first];
            const BasicNodeConfig& to_node = internal_nodes[test_case.first.second];
            const ExternalAddress& expected_result = test_case.second;

            ExternalLocators meta_ext_locators = from_node.get_metatraffic_external_locators(cfg);
            ExternalLocators user_ext_locators = from_node.get_default_external_locators(cfg);
            const ParticipantProxyData& discovered_data = to_node.announced_data;

            LocatorSelectorEntry entry(4u, 1u);
            entry.multicast = discovered_data.metatraffic_locators.multicast;
            entry.unicast = discovered_data.metatraffic_locators.unicast;

            external_locators::filter_remote_locators(entry, meta_ext_locators, ignore_non_matching);
            ASSERT_TRUE(entry.multicast == discovered_data.metatraffic_locators.multicast);
            ASSERT_EQ(entry.unicast.size(), 1u);
            ASSERT_EQ(entry.unicast[0], expected_result.metatraffic);

            ParticipantProxyData filtered_data = discovered_data;

            external_locators::filter_remote_locators(
                filtered_data, meta_ext_locators, user_ext_locators, ignore_non_matching);


            ASSERT_TRUE(filtered_data.metatraffic_locators.multicast == discovered_data.metatraffic_locators.multicast);
            ASSERT_EQ(filtered_data.metatraffic_locators.unicast.size(), 1u);
            ASSERT_EQ(filtered_data.metatraffic_locators.unicast[0], expected_result.metatraffic);

            ASSERT_TRUE(filtered_data.default_locators.multicast == discovered_data.default_locators.multicast);
            ASSERT_EQ(filtered_data.default_locators.unicast.size(), 1u);
            ASSERT_EQ(filtered_data.default_locators.unicast[0], expected_result.user);
        }

        // Test against Node_ZZ
        ExternalAddress zz_address("100.10.10.1", 7410, 7411);
        ParticipantProxyData zz_discovered_data;
        ExternalLocators zz_meta_locators;
        ExternalLocators zz_user_locators;
        zz_meta_locators[0][0].push_back(zz_address.metatraffic);
        zz_user_locators[0][0].push_back(zz_address.user);
        zz_discovered_data.metatraffic_locators.add_unicast_locator(zz_address.metatraffic);
        zz_discovered_data.default_locators.add_unicast_locator(zz_address.user);

        for (const BasicNodeConfig& node : internal_nodes)
        {
            LocatorSelectorEntry entry(4u, 1u);
            ExternalLocators meta_ext_locators = node.get_metatraffic_external_locators(cfg);
            ExternalLocators user_ext_locators = node.get_default_external_locators(cfg);
            ParticipantProxyData filtered_data = zz_discovered_data;

            // Node -> Node_ZZ
            entry.multicast = filtered_data.metatraffic_locators.multicast;
            entry.unicast = filtered_data.metatraffic_locators.unicast;

            external_locators::filter_remote_locators(entry, meta_ext_locators, ignore_non_matching);

            ASSERT_TRUE(entry.multicast == zz_discovered_data.metatraffic_locators.multicast);
            if (ignore_non_matching)
            {
                ASSERT_TRUE(entry.unicast.empty());
            }
            else
            {
                ASSERT_EQ(entry.unicast.size(), 1u);
                ASSERT_EQ(entry.unicast[0], zz_address.metatraffic);
            }

            external_locators::filter_remote_locators(
                filtered_data, meta_ext_locators, user_ext_locators, ignore_non_matching);

            ASSERT_TRUE(
                filtered_data.metatraffic_locators.multicast == zz_discovered_data.metatraffic_locators.multicast);
            ASSERT_TRUE(filtered_data.default_locators.multicast == zz_discovered_data.default_locators.multicast);

            if (ignore_non_matching)
            {
                ASSERT_TRUE(filtered_data.metatraffic_locators.unicast.empty());
                ASSERT_TRUE(filtered_data.default_locators.unicast.empty());
            }
            else
            {
                ASSERT_EQ(filtered_data.metatraffic_locators.unicast.size(), 1u);
                ASSERT_EQ(filtered_data.metatraffic_locators.unicast[0], zz_address.metatraffic);

                ASSERT_EQ(filtered_data.default_locators.unicast.size(), 1u);
                ASSERT_EQ(filtered_data.default_locators.unicast[0], zz_address.user);
            }

            // Node_ZZ -> Node
            filtered_data = node.announced_data;
            entry.multicast = filtered_data.metatraffic_locators.multicast;
            entry.unicast = filtered_data.metatraffic_locators.unicast;

            external_locators::filter_remote_locators(entry, zz_meta_locators, ignore_non_matching);

            ASSERT_TRUE(entry.multicast == node.announced_data.metatraffic_locators.multicast);
            if (ignore_non_matching)
            {
                ASSERT_TRUE(entry.unicast.empty());
            }
            else
            {
                ASSERT_TRUE(entry.unicast == node.announced_data.metatraffic_locators.unicast);
            }

            external_locators::filter_remote_locators(
                filtered_data, zz_meta_locators, zz_user_locators, ignore_non_matching);

            ASSERT_TRUE(
                filtered_data.metatraffic_locators.multicast == node.announced_data.metatraffic_locators.multicast);
            ASSERT_TRUE(filtered_data.default_locators.multicast == node.announced_data.default_locators.multicast);

            if (ignore_non_matching)
            {
                ASSERT_TRUE(filtered_data.metatraffic_locators.unicast.empty());
                ASSERT_TRUE(filtered_data.default_locators.unicast.empty());
            }
            else
            {
                ASSERT_TRUE(
                    filtered_data.metatraffic_locators.unicast == node.announced_data.metatraffic_locators.unicast);
                ASSERT_TRUE(filtered_data.default_locators.unicast == node.announced_data.default_locators.unicast);
            }
        }
    }
}

TEST(ExternalLocatorsTests, matching_locators_scenario)
{
    test_matching_locators_scenario(true);
    test_matching_locators_scenario(false);
}

TEST(ExternalLocatorsTests, matching_locators_mask_test)
{
    struct TestCase
    {
        std::string local_address;
        uint8_t mask;
        std::string remote_address;
        bool should_match;
    };

    const TestCase test_cases[] =
    {
        {"192.168.1.127", 24, "192.168.0.127", false},   // 01 vs 00
        {"192.168.1.127", 24, "192.168.1.1",   true},    // 01 vs 01
        {"192.168.1.127", 25, "192.168.1.128", false},   // 7F vs 80
        {"192.168.1.127", 25, "192.168.1.1",   true},    // 7F vs 01
        {"192.168.1.127", 26, "192.168.1.1",   false},   // 7F vs 01
        {"192.168.1.127", 26, "192.168.1.64",  true},    // 7F vs 40
        {"192.168.1.127", 27, "192.168.1.64",  false},   // 7F vs 40
        {"192.168.1.127", 27, "192.168.1.96",  true},    // 7F vs 60
        {"192.168.1.127", 28, "192.168.1.96",  false},   // 7F vs 60
        {"192.168.1.127", 28, "192.168.1.112", true},    // 7F vs 70
        {"192.168.1.127", 29, "192.168.1.112", false},   // 7F vs 70
        {"192.168.1.127", 29, "192.168.1.120", true},    // 7F vs 78
        {"192.168.1.127", 30, "192.168.1.120", false},   // 7F vs 78
        {"192.168.1.127", 30, "192.168.1.124", true},    // 7F vs 7C
        {"192.168.1.127", 31, "192.168.1.124", false},   // 7F vs 7C
        {"192.168.1.127", 31, "192.168.1.126", true},    // 7F vs 7E

        {"192.168.1.127", 23, "192.168.0.127",   true},  // 01 vs 00
        {"192.168.1.127", 23, "192.168.2.127",   false}, // 01 vs 02
        {"192.168.1.127", 22, "192.168.2.127",   true},  // 01 vs 02
        {"192.168.1.127", 22, "192.168.4.127",   false}, // 01 vs 04
        {"192.168.1.127", 21, "192.168.4.127",   true},  // 01 vs 04
        {"192.168.1.127", 21, "192.168.8.127",   false}, // 01 vs 08
        {"192.168.1.127", 20, "192.168.8.127",   true},  // 01 vs 08
        {"192.168.1.127", 20, "192.168.16.127",  false}, // 01 vs 10
        {"192.168.1.127", 19, "192.168.16.127",  true},  // 01 vs 10
        {"192.168.1.127", 19, "192.168.32.127",  false}, // 01 vs 20
        {"192.168.1.127", 18, "192.168.32.127",  true},  // 01 vs 20
        {"192.168.1.127", 18, "192.168.64.127",  false}, // 01 vs 40
        {"192.168.1.127", 17, "192.168.64.127",  true},  // 01 vs 40
        {"192.168.1.127", 17, "192.168.128.127", false}, // 01 vs 80
        {"192.168.1.127", 16, "192.168.128.127", true},  // 01 vs 80
        {"192.168.1.127", 16, "192.169.1.127",   false}, // A8 vs A9

        {"192.168.1.127", 15, "192.169.1.127", true},    // A8 vs A9
        {"192.168.1.127", 15, "192.170.1.127", false},   // A8 vs AA
        {"192.168.1.127", 14, "192.170.1.127", true},    // A8 vs AA
        {"192.168.1.127", 14, "192.172.1.127", false},   // A8 vs AC
        {"192.168.1.127", 13, "192.172.1.127", true},    // A8 vs AC
        {"192.168.1.127", 13, "192.167.1.127", false},   // A8 vs A7
        {"192.168.1.127", 12, "192.167.1.127", true},    // A8 vs A7
        {"192.168.1.127", 12, "192.184.1.127", false},   // A8 vs B8
        {"192.168.1.127", 11, "192.184.1.127", true},    // A8 vs B8
        {"192.168.1.127", 11, "192.136.1.127", false},   // A8 vs 88
        {"192.168.1.127", 10, "192.136.1.127", true},    // A8 vs 88
        {"192.168.1.127", 10, "192.232.1.127", false},   // A8 vs E8
        {"192.168.1.127",  9, "192.232.1.127", true},    // A8 vs E8
        {"192.168.1.127",  9, "192.40.1.127",  false},   // A8 vs 28
        {"192.168.1.127",  8, "192.40.1.127",  true},    // A8 vs 28
        {"192.168.1.127",  8, "193.168.1.127", false},   // C0 vs C1

        {"192.168.1.127",  7, "193.168.1.127", true},    // C0 vs C1
        {"192.168.1.127",  7, "194.168.1.127", false},   // C0 vs C2
        {"192.168.1.127",  6, "194.168.1.127", true},    // C0 vs C2
        {"192.168.1.127",  6, "196.168.1.127", false},   // C0 vs C4
        {"192.168.1.127",  5, "196.168.1.127", true},    // C0 vs C4
        {"192.168.1.127",  5, "200.168.1.127", false},   // C0 vs C8
        {"192.168.1.127",  4, "200.168.1.127", true},    // C0 vs C8
        {"192.168.1.127",  4, "208.168.1.127", false},   // C0 vs D0
        {"192.168.1.127",  3, "208.168.1.127", true},    // C0 vs D0
        {"192.168.1.127",  3, "224.168.1.127", false},   // C0 vs E0
        {"192.168.1.127",  2, "224.168.1.127", true},    // C0 vs E0
        {"192.168.1.127",  2, "128.168.1.127", false},   // C0 vs 80
        {"192.168.1.127",  1, "128.168.1.127", true},    // C0 vs 80
        {"192.168.1.127",  1, "64.168.1.127",  false},   // C0 vs 40
    };

    for (const TestCase& test : test_cases)
    {
        LocatorWithMask local_locator;
        local_locator.mask(test.mask);
        IPLocator::setIPv4(local_locator, test.local_address);

        ExternalLocators local;
        local[0][0].push_back(local_locator);

        Locator remote_locator;
        IPLocator::setIPv4(remote_locator, test.remote_address);

        LocatorSelectorEntry remote(4u, 1u);
        remote.unicast.push_back(remote_locator);

        external_locators::filter_remote_locators(remote, local, true);
        if (test.should_match)
        {
            ASSERT_EQ(remote.unicast.size(), 1u);
            ASSERT_EQ(remote.unicast[0], remote_locator);
        }
        else
        {
            ASSERT_TRUE(remote.unicast.empty());
        }
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
