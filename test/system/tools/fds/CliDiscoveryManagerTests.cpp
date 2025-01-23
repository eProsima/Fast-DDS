// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "MockCliDiscoveryManager.hpp"
#include "CliDiscoveryParser.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <thread>
#include <chrono>
#if __APPLE__
#include <signal.h>
#endif // if __APPLE__

#include <rtps/attributes/ServerAttributes.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>


using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::option;

// TestCase uses the following format:
// {argv, udp_ips, udp_ports, tcp_ips, tcp_ports}
using TestCase = std::tuple<std::vector<const char*>, std::vector<std::string>, std::vector<uint16_t>,
                std::vector<std::string>, std::vector<uint16_t>>;

class CliDiscoveryManagerTest : public ::testing::Test
{
public:

    void createOptionsAndParser(
            int argc,
            const char* argv[])
    {
        Stats stats(usage, argc, argv);
        options = std::vector<Option>(stats.options_max);
        std::vector<Option> buffer(stats.buffer_max);
        parse = Parser(usage, argc, argv, &options[0], &buffer[0]);
    }

    Option* getOption(
            optionIndex idx)
    {
        return options[idx];
    }

    void setTestEnv(
            std::string env_var_name_,
            std::string env_var_value)
    {
#ifdef _WIN32
        _putenv_s(env_var_name_.c_str(), env_var_value.c_str());
#else
        setenv(env_var_name_.c_str(), env_var_value.c_str(), 1);
#endif // _WIN32
    }

    void compareLocator(
            const Locator_t& loc,
            const std::string& ip,
            uint16_t port,
            bool is_tcp,
            bool is_ipv6)
    {
        // Build reference locator
        Locator_t locator_ref(port);
        if (is_tcp)
        {
            IPLocator::setLogicalPort(locator_ref, port);
            locator_ref.kind = LOCATOR_KIND_TCPv4;
        }
        else
        {
            locator_ref.kind = LOCATOR_KIND_UDPv4;
        }
        if (is_ipv6)
        {
            locator_ref.kind *= 2;
            IPLocator::setIPv6(locator_ref, ip);
        }
        else
        {
            IPLocator::setIPv4(locator_ref, ip);
        }
        // Compare locators
        EXPECT_EQ(loc, locator_ref);
    }

    void testServerLocatorsSetup(
            TestCase test_case,
            bool is_tcp,
            bool should_fail = false)
    {
        const auto& argv = std::get<0>(test_case);
        const auto& ips = is_tcp ? std::get<3>(test_case) : std::get<1>(test_case);
        const auto& ports = is_tcp ? std::get<4>(test_case) : std::get<2>(test_case);

        // This is a setup verification in order to have vectors with the same size
        ASSERT_EQ(ips.size(), ports.size());
        size_t num_servers = ips.size();

        // CreateOptionsAndParser does not work in MacOS when multiple ports are used
        option::Stats stats(usage, static_cast<int>(argv.size()), const_cast<const char**>(argv.data()));
        options = std::vector<Option>(stats.options_max);
        std::vector<Option> buffer(stats.buffer_max);
        parse = option::Parser(usage, static_cast<int>(argv.size()),
                        const_cast<const char**>(argv.data()), &options[0], &buffer[0]);

        manager.reset();
        manager.get_cli_ports_and_ips(
            getOption(UDP_PORT),
            getOption(UDPADDRESS),
            getOption(TCP_PORT),
            getOption(TCPADDRESS)
            );
        // Only fails if there is an error setting address and kind
        if (is_tcp)
        {
            EXPECT_EQ(manager.add_tcp_servers(), !should_fail);
        }
        else
        {
            EXPECT_EQ(manager.add_udp_servers(), !should_fail);
        }
        if (!should_fail)
        {
            DomainParticipantQos qos = manager.getServerQos();
            ASSERT_EQ(qos.wire_protocol().builtin.metatrafficUnicastLocatorList.size(), num_servers);
            size_t i = 0;
            for (const Locator_t& locator : qos.wire_protocol().builtin.metatrafficUnicastLocatorList)
            {
                compareLocator(
                    locator,
                    ips[i],
                    ports[i],
                    is_tcp,
                    false
                    );
                ++i;
            }
        }
    }

    void addServers(
            TestCase test_case)
    {
        const auto& argv = std::get<0>(test_case);
        // CreateOptionsAndParser does not work in MacOS when multiple ports are used
        option::Stats stats(usage, static_cast<int>(argv.size()), const_cast<const char**>(argv.data()));
        options = std::vector<Option>(stats.options_max);
        std::vector<Option> buffer(stats.buffer_max);
        parse = option::Parser(usage, static_cast<int>(argv.size()),
                        const_cast<const char**>(argv.data()), &options[0], &buffer[0]);
        manager.reset();
        manager.get_cli_ports_and_ips(
            getOption(UDP_PORT),
            getOption(UDPADDRESS),
            getOption(TCP_PORT),
            getOption(TCPADDRESS)
            );
        EXPECT_TRUE(manager.add_tcp_servers());
        EXPECT_TRUE(manager.add_udp_servers());
    }

    template<typename TransportDescriptorType>
    void transport_check(
            std::vector<std::shared_ptr<TransportDescriptorInterface>>& transports,
            uint8_t expected_count)
    {
        uint8_t count = 0;
        for (const auto& transportDescriptor : transports)
        {
            if (nullptr != dynamic_cast<TransportDescriptorType*>(transportDescriptor.get()))
            {
                ++count;
            }
        }
        EXPECT_EQ(count, expected_count);
    }

    void testTransportSetup(
            uint8_t shm,
            uint8_t udp4,
            uint8_t udp6,
            uint8_t tcp4,
            uint8_t tcp6)
    {
        manager.configure_transports();
        transport_check<SharedMemTransportDescriptor>(manager.getServerQos().transport().user_transports, shm);
        transport_check<UDPv4TransportDescriptor>(manager.getServerQos().transport().user_transports, udp4);
        transport_check<UDPv6TransportDescriptor>(manager.getServerQos().transport().user_transports, udp6);
        transport_check<TCPv4TransportDescriptor>(manager.getServerQos().transport().user_transports, tcp4);
        transport_check<TCPv6TransportDescriptor>(manager.getServerQos().transport().user_transports, tcp6);
    }

protected:

    void SetUp() override
    {
    }

    void TearDown() override
    {
        // Clean up code here
    }

    const std::map<std::string, TestCase> test_case_map =
    {
        {
            "udp_1_ip_1_port",
            {
                {"-l", "127.0.0.1", "-p", "7402"},
                {"127.0.0.1"},
                {7402},
                {},
                {},
            }
        },
        {
            "udp_0_ip_1_port",
            {
                {"-p", "8500"},
                {default_ip},  // default_ip
                {8500},
                {},
                {},
            }
        },
        {
            "udp_1_ip_0_port",
            {
                {"-l", "127.0.0.1"},
                {"127.0.0.1"},
                {DEFAULT_ROS2_SERVER_PORT},  // DEFAULT_ROS2_SERVER_PORT
                {},
                {},
            }
        },
        {
            "udp_2_ip_2_port",
            {
                {"-l", "127.0.0.1", "-p", "7402", "-l", "192.168.1.42", "-p", "7652"},
                {"127.0.0.1", "192.168.1.42"},
                {7402, 7652},
                {},
                {},
            }
        },
        {
            "udp_1_ip_2_port",
            {
                {"-l", "127.0.0.1", "-p", "7402", "-p", "7652"},
                {"127.0.0.1", default_ip},
                {7402, 7652},
                {},
                {},
            }
        },
        {
            "udp_2_ip_1_port",
            {
                {"-l", "127.0.0.1", "-p", "7402", "-l", "192.168.1.42"},
                {"127.0.0.1", "192.168.1.42"},
                {7402, DEFAULT_ROS2_SERVER_PORT},
                {},
                {},
            }
        },
        {
            "tcp_1_ip_1_port",
            {
                {"-t", "127.0.0.1", "-q", "7402"},
                {},
                {},
                {"127.0.0.1"},
                {7402},
            }
        },
        {
            "tcp_0_ip_1_port",
            {
                {"-q", "7402"},
                {},
                {},
                {default_ip},
                {7402},
            }
        },
        {
            "tcp_1_ip_0_port",
            {
                {"-t", "127.0.0.1"},
                {},
                {},
                {"127.0.0.1"},
                {DEFAULT_TCP_SERVER_PORT},
            }
        },
        {
            "tcp_2_ip_2_port",
            {
                {"-t", "127.0.0.1", "-q", "7402", "-t", "192.168.1.42", "-q", "7652"},
                {},
                {},
                {"127.0.0.1", "192.168.1.42"},
                {7402, 7652},
            }
        },
        {
            "tcp_1_ip_2_port",
            {
                {"-t", "127.0.0.1", "-q", "7402", "-q", "7652"},
                {},
                {},
                {"127.0.0.1", default_ip},
                {7402, 7652},
            }
        },
        {
            "tcp_2_ip_1_port",
            {
                // This must fail in TCP because the number of ports is lower than the number of IPs and
                // TCP transports cannot share listening port
                {"-t", "127.0.0.1", "-q", "7402", "-t", "192.168.1.42"},
                {},
                {},
                {"127.0.0.1", "192.168.1.42"},
                {7402, DEFAULT_TCP_SERVER_PORT},
            }
        },
        {
            "udp_1_ip_1_port_tcp_1_ip_1_port",
            {
                {"-l", "127.0.0.1", "-p", "7402", "-t", "192.168.1.42", "-q", "7652"},
                {"127.0.0.1"},
                {7402},
                {"192.168.1.42"},
                {7652},
            }
        },
        {
            // Special case to check number of opts read, not final locator list
            "OPT:udp_1_ip_0_port_tcp_0_ip_1_port",
            {
                {"-l", "127.0.0.1", "-q", "7652"},
                {"127.0.0.1"},
                {},
                {},
                {7652},
            }
        },
        {
            "OPT:udp_0_ip_1_port_tcp_1_ip_0_port",
            {
                {"-t", "127.0.0.1", "-p", "7652"},
                {},
                {7652},
                {"127.0.0.1"},
                {},
            }
        },
        {
            "udp_v6",
            {
                {"-l", "::1"},
                {"::1"},
                {DEFAULT_ROS2_SERVER_PORT},
                {},
                {},
            }
        },
        {
            "tcp_v6",
            {
                {"-t", "::1"},
                {},
                {},
                {"::1"},
                {DEFAULT_TCP_SERVER_PORT},
            }
        },
        {
            "tcp_v6_2_ports",
            {
                {"-t", "::1", "-q", "7402", "-t", "2a00:1450:400e:803::2004", "-q", "7652"},
                {},
                {},
                {"::1", "2a00:1450:400e:803::2004"},
                {7402, 7652},
            }
        },
    };

public:

    MockCliDiscoveryManager manager;
    std::vector<Option> options;
    Parser parse;
};

TEST_F(CliDiscoveryManagerTest, InitialOptionsFail)
{
    // Domain with Locator list should fail if check_nonOpts is true
    const char* argv[] = {"-d", "4", "127.0.0.1:0;192.168.1.42:1"};
    createOptionsAndParser(3, argv);
    bool result = manager.initial_options_fail(options, parse, true);
    EXPECT_TRUE(result);
    // Domain with Locator list should fail if check_nonOpts is false
    result = manager.initial_options_fail(options, parse, false);
    EXPECT_FALSE(result);

    // Locator list with domain should fail if check_nonOpts is true
    const char* argv2[] = {"127.0.0.1:0;192.168.1.42:1", "-d", "4"};
    createOptionsAndParser(3, argv2);
    result = manager.initial_options_fail(options, parse, true);
    EXPECT_TRUE(result);

    // Correct options
    const char* argv3[] = {"-d", "4"};
    createOptionsAndParser(2, argv3);
    result = manager.initial_options_fail(options, parse, true);
    EXPECT_FALSE(result);

    // Unknown option
    const char* argv4[] = {"-z", "4"};
    createOptionsAndParser(2, argv4);
    result = manager.initial_options_fail(options, parse, true);
    EXPECT_TRUE(result);
}

TEST_F(CliDiscoveryManagerTest, GetDiscoveryServerPortFromCLI)
{
    // Port directly received from CLI
    const char* argv_udp[] = {"-p", "11811"};
    createOptionsAndParser(2, argv_udp);
    Option* udp_port_opt = getOption(UDP_PORT);
    uint16_t udp_port = manager.get_discovery_server_port(udp_port_opt);
    EXPECT_EQ(udp_port, 11811);
    const char* argv_tcp[] = {"-q", "42100"};
    createOptionsAndParser(2, argv_tcp);
    Option* tcp_port_opt = getOption(TCP_PORT);
    uint16_t tcp_port = manager.get_discovery_server_port(tcp_port_opt);
    EXPECT_EQ(tcp_port, 42100);
}

#ifndef _WIN32
TEST_F(CliDiscoveryManagerTest, GetDiscoveryServerPortFromDomainId)
{
    PortParameters port_params;
    uint16_t port_1 = manager.get_discovery_server_port(1);
    EXPECT_EQ(port_1, port_params.get_discovery_server_port(1));
    uint16_t port_232 = manager.get_discovery_server_port(232);
    EXPECT_EQ(port_232, port_params.get_discovery_server_port(232));

    uint16_t port_fail = manager.get_discovery_server_port(233);
    EXPECT_EQ(port_fail, 0);
}

TEST_F(CliDiscoveryManagerTest, GetDomainIdFromCLI)
{
    const char* argv[] = {"-d", "4"};
    createOptionsAndParser(2, argv);
    Option* domain_option = getOption(DOMAIN_OPT);
    DomainId_t domain_id = manager.get_domain_id(domain_option);
    EXPECT_EQ(domain_id, static_cast<DomainId_t>(4));
    const char* argv2[] = {"-d", "2"};
    createOptionsAndParser(2, argv2);
    Option* domain_option2 = getOption(DOMAIN_OPT);
    DomainId_t domain_id2 = manager.get_domain_id(domain_option2);
    EXPECT_EQ(domain_id2, static_cast<DomainId_t>(2));
}

TEST_F(CliDiscoveryManagerTest, GetDomainIdFromDefaultValue)
{
    const char* argv[] = {"-q", "12345"};
    createOptionsAndParser(2, argv);
    DomainId_t domain_id = manager.get_domain_id(nullptr);
    EXPECT_EQ(domain_id, static_cast<DomainId_t>(0));
}

TEST_F(CliDiscoveryManagerTest, GetRemoteServersWithDomainParam)
{
    const char* argv[] = {"-d", "4", "127.0.0.1:0;192.168.1.42:1"};
    createOptionsAndParser(3, argv);
    std::string servers = manager.get_remote_servers(parse, parse.nonOptionsCount());

    // Use regex to check the string correctness
    LocatorList_t serverList;
    load_environment_server_info(servers, serverList);

    EXPECT_EQ(serverList.size(), 2);
    PortParameters port_params;
    std::vector<uint16_t> expected_ports({7402, port_params.get_discovery_server_port(1)});
    for (Locator_t& locator : serverList)
    {
        // Do NOT check kind, as it is not set in this case. We are using a string
        auto it = std::find(expected_ports.begin(), expected_ports.end(), locator.port);
        EXPECT_NE(it, expected_ports.end());
        if (it != expected_ports.end())
        {
            expected_ports.erase(it);
        }
    }
    EXPECT_TRUE(expected_ports.empty());
}

TEST_F(CliDiscoveryManagerTest, ExecCommand)
{
    std::string result = manager.exec_command("echo \"Hello CLI Tool\"");
    EXPECT_EQ(result, "Hello CLI Tool\n");
}
#endif // _WIN32

TEST_F(CliDiscoveryManagerTest, SetServerQos)
{
    manager.set_server_qos(7402);
    DomainParticipantQos qos = manager.getServerQos();
    EXPECT_EQ(qos.wire_protocol().builtin.metatrafficUnicastLocatorList.size(), 1);
    for (const Locator_t& locator : qos.wire_protocol().builtin.metatrafficUnicastLocatorList)
    {
        compareLocator(locator, "0.0.0.0", 7402, false, false);
    }
    EXPECT_FALSE(qos.transport().use_builtin_transports);
    EXPECT_EQ(qos.wire_protocol().builtin.discovery_config.discoveryProtocol, DiscoveryProtocol::SERVER);
    ASSERT_EQ(qos.transport().user_transports.size(), 1);
    EXPECT_TRUE(nullptr != dynamic_cast<UDPv4TransportDescriptor*>(qos.transport().user_transports[0].get()));
}

TEST_F(CliDiscoveryManagerTest, GetCliPortsAndIps)
{
    std::vector<std::string> test_cases =
    {
        {"udp_2_ip_2_port"},
        {"tcp_2_ip_2_port"},
        {"udp_1_ip_1_port_tcp_1_ip_1_port"},
        {"OPT:udp_1_ip_0_port_tcp_0_ip_1_port"},
        {"OPT:udp_0_ip_1_port_tcp_1_ip_0_port"},
    };

    for (const auto& key_case : test_cases)
    {
        const auto& test_case = test_case_map.at(key_case);
        // CreateOptionsAndParser does not work in MacOS when multiple ports are used
        option::Stats stats(usage, static_cast<int>(std::get<0>(test_case).size()),
                const_cast<const char**>(std::get<0>(test_case).data()));
        options = std::vector<Option>(stats.options_max);
        std::vector<Option> buffer(stats.buffer_max);
        parse = option::Parser(usage, static_cast<int>(std::get<0>(test_case).size()),
                        const_cast<const char**>(std::get<0>(test_case).data()), &options[0], &buffer[0]);
        manager.reset();
        manager.get_cli_ports_and_ips(
            getOption(UDP_PORT),
            getOption(UDPADDRESS),
            getOption(TCP_PORT),
            getOption(TCPADDRESS)
            );

        EXPECT_EQ(manager.getUdpIps().size(), std::get<1>(test_case).size());
        EXPECT_EQ(manager.getUdpPorts().size(), std::get<2>(test_case).size());
        EXPECT_EQ(manager.getTcpIps().size(), std::get<3>(test_case).size());
        EXPECT_EQ(manager.getTcpPorts().size(), std::get<4>(test_case).size());
    }
}

TEST_F(CliDiscoveryManagerTest, SetAddressAndKind)
{
    // This method does not set ports, only IP addresses. Need to set them manually to use compareLocator()
    std::string ipv4_address = "216.58.215.164"; // IPv4 address of www.acme.com.test
    std::string ipv6_address = "2a00:1450:400e:803::2004"; // IPv6 address of www.acme.com.test
    std::string dns_address = "www.acme.com.test";

    {
        // UDPv4 address
        Locator_t locator(7402);
        EXPECT_TRUE(manager.set_address_and_kind(ipv4_address, locator, false));
        compareLocator(locator, ipv4_address, 7402, false, false);
    }
    {
        // UDPv6 address
        Locator_t locator(7402);
        EXPECT_TRUE(manager.set_address_and_kind(ipv6_address, locator, false));
        compareLocator(locator, ipv6_address, 7402, false, true);
    }
    {
        // TCPv4 address
        Locator_t locator(7402);
        IPLocator::setLogicalPort(locator, 7402);
        EXPECT_TRUE(manager.set_address_and_kind(ipv4_address, locator, true));
        compareLocator(locator, ipv4_address, 7402, true, false);
    }
    {
        // TCPv6 address
        Locator_t locator(7402);
        IPLocator::setLogicalPort(locator, 7402);
        EXPECT_TRUE(manager.set_address_and_kind(ipv6_address, locator, true));
        compareLocator(locator, ipv6_address, 7402, true, true);
    }
    {
        // UDPv4 DNS address
        Locator_t locator(7402);
        EXPECT_TRUE(manager.set_address_and_kind(dns_address, locator, false));
        compareLocator(locator, ipv4_address, 7402, false, false);
    }
    {
        // TCPv4 DNS address
        Locator_t locator(7402);
        IPLocator::setLogicalPort(locator, 7402);
        EXPECT_TRUE(manager.set_address_and_kind(dns_address, locator, true));
        compareLocator(locator, ipv4_address, 7402, true, false);
    }
    {
        // UDPv4 empty address
        Locator_t locator(7402);
        EXPECT_TRUE(manager.set_address_and_kind("", locator, false));
        compareLocator(locator, default_ip, 7402, false, false);
    }
}

TEST_F(CliDiscoveryManagerTest, AddUdpServers)
{
    // Test cases: {argv, IPs_expected, ports_expected}
    std::vector<std::string> test_cases =
    {
        {"udp_1_ip_1_port"},
        {"udp_0_ip_1_port"},
        {"udp_1_ip_0_port"},
        {"udp_2_ip_2_port"},
        {"udp_1_ip_2_port"},
        {"udp_2_ip_1_port"},
        {"tcp_1_ip_0_port"},
    };
    for (const auto& key_case : test_cases)
    {
        const auto& test_case = test_case_map.at(key_case);
        testServerLocatorsSetup(test_case, false);
    }
}

TEST_F(CliDiscoveryManagerTest, AddTcpServers)
{
    std::vector<std::string> test_cases =
    {
        {"tcp_1_ip_1_port"},
        {"tcp_0_ip_1_port"},
        {"tcp_1_ip_0_port"},
        {"tcp_2_ip_2_port"},
        {"tcp_1_ip_2_port"},
        {"udp_1_ip_0_port"},
    };
    std::vector<std::string> test_cases_fail =
    {
        {"tcp_2_ip_1_port"},
    };
    for (const auto& key_case : test_cases)
    {
        const auto& test_case = test_case_map.at(key_case);
        testServerLocatorsSetup(test_case, true);
    }
    for (const auto& key_case : test_cases_fail)
    {
        const auto& test_case = test_case_map.at(key_case);
        testServerLocatorsSetup(test_case, true, true);
    }
}

// Note that a SHM transport is added when builtin transports are not used
TEST_F(CliDiscoveryManagerTest, ConfigureTransports)
{
    // UDPv4 Server use builtin transports
    addServers(test_case_map.at("udp_1_ip_1_port"));
    testTransportSetup(0, 0, 0, 0, 0);
    EXPECT_TRUE(manager.getServerQos().transport().use_builtin_transports);

    // UDPv6 Server adds one UDPv6 transport
    addServers(test_case_map.at("udp_v6"));
    testTransportSetup(1, 0, 1, 0, 0);
    EXPECT_FALSE(manager.getServerQos().transport().use_builtin_transports);

    // TCPv4 Server adds one TCPv4 transport per listening port
    addServers(test_case_map.at("tcp_1_ip_1_port"));
    testTransportSetup(1, 0, 0, 1, 0);
    EXPECT_FALSE(manager.getServerQos().transport().use_builtin_transports);

    addServers(test_case_map.at("tcp_2_ip_2_port"));
    testTransportSetup(1, 0, 0, 2, 0);
    EXPECT_FALSE(manager.getServerQos().transport().use_builtin_transports);

    // TCPv6 Server adds one TCPv6 transport per listening port
    addServers(test_case_map.at("tcp_v6"));
    testTransportSetup(1, 0, 0, 0, 1);
    EXPECT_FALSE(manager.getServerQos().transport().use_builtin_transports);

    // UDPv4 and TCPv4 Server uses builtin transports with an additional transport for TCPv4
    addServers(test_case_map.at("udp_1_ip_1_port_tcp_1_ip_1_port"));
    testTransportSetup(0, 0, 0, 1, 0);
    EXPECT_TRUE(manager.getServerQos().transport().use_builtin_transports);
}

#ifndef _WIN32
// This test verifies that the get_listening_ports method returns *at least* the ports we know that
// are being used by the servers. We cannot check the exact number of ports because the number of
// ports is not fixed and depends on the number of TCP connection of the host.
TEST_F(CliDiscoveryManagerTest, GetListeningPorts)
{
    // Check that the ports are not being used
    std::vector<uint16_t> ports = manager.real_get_listening_ports();
    bool port_7402_7652 = false;
    for (const uint16_t port : ports)
    {
        port_7402_7652 |= (port == 7402 || port == 7652);
    }
    ASSERT_FALSE(port_7402_7652);

    addServers(test_case_map.at("udp_2_ip_2_port"));
    manager.configure_transports();
    DomainParticipant* server = DomainParticipantFactory::get_instance()->create_participant(0,
                    manager.getServerQos());

    ports.clear();
    ports = manager.real_get_listening_ports();
    bool port_7402 = false;
    bool port_7652 = false;
    for (const uint16_t port : ports)
    {
        port_7402 |= (port == 7402);
        port_7652 |= (port == 7652);
    }
    EXPECT_TRUE(port_7402);
    EXPECT_TRUE(port_7652);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(server), RETCODE_OK);
}

// This test also checks:
// - is_server_running method, which checks if a server is running in a specific domain.
// - get_pid_of_server method, which returns the pid of the running server in the specified port.
TEST_F(CliDiscoveryManagerTest, GetLocalServers)
{
    // Lambda to verify servers
    auto verify_servers = [&](const std::vector<MetaInfo_DS>& servers,
                    std::vector<uint16_t> expected_ports,
                    std::vector<uint32_t> expected_domains)
            {
                for (const MetaInfo_DS& server : servers)
                {
                    auto it = std::find(expected_ports.begin(), expected_ports.end(), server.port);
                    EXPECT_NE(it, expected_ports.end()) << "Expected port not found: " << server.port;
                    if (it != expected_ports.end())
                    {
                        expected_ports.erase(it);
                    }
                    auto it_d = std::find(expected_domains.begin(), expected_domains.end(), server.domain_id);
                    EXPECT_NE(it_d, expected_domains.end()) << "Expected domain not found: " << server.domain_id;
                    if (it_d != expected_domains.end())
                    {
                        expected_domains.erase(it_d);
                    }
                    EXPECT_TRUE(manager.is_server_running(server.domain_id));
                }
                EXPECT_EQ(expected_ports.size(), 0);
                EXPECT_EQ(expected_domains.size(), 0);
            };

    // Use MockCliDiscoveryManager overwritten method to get the listening ports
    std::vector<MetaInfo_DS> servers = manager.get_local_servers();
    EXPECT_TRUE(servers.empty());

    {
        DomainId_t domain = 0;
        PortParameters port_params;
        uint16_t port = port_params.get_discovery_server_port(domain);
        ASSERT_FALSE(manager.is_server_running(domain));
        EXPECT_EQ(manager.get_pid_of_server(port), 0);
        // Simulate the creation of the DS with the mocked listening port
        manager.mocked_ports.push_back(7402);
        servers = manager.get_local_servers();
        EXPECT_EQ(servers.size(), 1);
        verify_servers(servers, {port}, {domain});
    }

    // Multiple servers
    manager.mocked_ports.clear();
    {
        DomainId_t d0 = 0;
        DomainId_t d1 = 1;
        PortParameters port_params;
        uint16_t p0 = port_params.get_discovery_server_port(d0);
        uint16_t p1 = port_params.get_discovery_server_port(d1);
        ASSERT_FALSE(manager.is_server_running(d0));
        ASSERT_FALSE(manager.is_server_running(d1));
        EXPECT_EQ(manager.get_pid_of_server(p0), 0);
        EXPECT_EQ(manager.get_pid_of_server(p1), 0);
        // Simulate the creation of the DS with the mocked listening port
        manager.mocked_ports.push_back(7402);
        manager.mocked_ports.push_back(7652);
        servers = manager.get_local_servers();
        EXPECT_EQ(servers.size(), 2);
        verify_servers(servers, {p0, p1}, {d0, d1});
    }
}
#endif // _WIN32

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
