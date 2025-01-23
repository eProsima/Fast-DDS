// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rtps/attributes/ServerAttributes.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>

// Port used if the ros environment variable doesn't specify one
constexpr uint16_t DEFAULT_ROS2_SERVER_PORT = 11811;
// Port used by default for tcp transport
constexpr uint16_t DEFAULT_TCP_SERVER_PORT = 42100;

//! Tests the server-client setup using environment variable works fine
TEST(ServerAttributesTests, ServerClientEnvironmentSetUp)
{
    using namespace std;
    using namespace eprosima::fastdds::rtps;

    LocatorList_t output, standard;
    Locator_t loc, loc6(LOCATOR_KIND_UDPv6, 0);

    // We are going to use several test string and check they are properly parsed and turn into RemoteServerList_t
    // 1. Single server address without specific port provided
    string text = "192.168.36.34";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, text);
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 2. Single server IPv6 address without specific port provided
    text = "2a02:26f0:dd:499::356e";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, text);
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 3. Single server address specifying a custom listening port
    text = "192.168.36.34:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, text);
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 4. Single server IPv6 address specifying a custom listening port
    text = "[2001:470:142:5::116]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2001:470:142:5::116");
    IPLocator::setPhysicalPort(loc6, 14520);
    standard.push_back(loc6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 5. Check any locator is turned into localhost
    text = "0.0.0.0:14520;[::]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, "127.0.0.1");
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    IPLocator::setIPv6(loc6, "::1");
    IPLocator::setPhysicalPort(loc6, 14520);
    standard.push_back(loc6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 6. Check empty string scenario is handled
    text = "";
    output.clear();

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_TRUE(output.empty());

    // 7. Check at least one server be present scenario is handled
    text = ";;;;";
    output.clear();

    ASSERT_FALSE(load_environment_server_info(text, output));

    // 8. Check several server scenario
    text = "192.168.36.34:14520;172.29.55.77:8783;172.30.80.1:31090";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    IPLocator::setIPv4(loc, string("172.29.55.77"));
    IPLocator::setPhysicalPort(loc, 8783);
    standard.push_back(loc);

    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 9. Check several server scenario with IPv6 addresses too
    text = "192.168.36.34:14520;[2a02:ec80:600:ed1a::3]:8783;172.30.80.1:31090";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, "192.168.36.34");
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    IPLocator::setIPv6(loc6, "2a02:ec80:600:ed1a::3");
    IPLocator::setPhysicalPort(loc6, 8783);
    standard.push_back(loc6);

    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 10. Check multicast addresses are identified as such
    text = "239.255.0.1;ff1e::ffff:efff:1";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, "239.255.0.1");
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc);

    IPLocator::setIPv6(loc6, "ff1e::ffff:efff:1");
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 11. Check ignore some servers scenario
    text = ";192.168.36.34:14520;;172.29.55.77:8783;172.30.80.1:31090;";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    IPLocator::setIPv4(loc, string("172.29.55.77"));
    IPLocator::setPhysicalPort(loc, 8783);
    standard.push_back(loc);

    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 12. Check addresses as dns name (test domain urls are checked on a specific test)
    text = "localhost.test:12345";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, "127.0.0.1");
    IPLocator::setPhysicalPort(loc, 12345);
    standard.push_back(loc);
    IPLocator::setIPv6(loc6, "::1");
    IPLocator::setPhysicalPort(loc6, 12345);
    standard.push_back(loc6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 13. Check mixed scenario with addresses and dns
    text = "192.168.36.34:14520;localhost.test:12345;172.30.80.1:31090;";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    IPLocator::setIPv4(loc, string("127.0.0.1"));
    IPLocator::setPhysicalPort(loc, 12345);
    standard.push_back(loc);
    IPLocator::setIPv6(loc6, string("::1"));
    IPLocator::setPhysicalPort(loc6, 12345);
    standard.push_back(loc6);

    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCP transport

    Locator_t loc_tcp(LOCATOR_KIND_TCPv4, 0);
    Locator_t loc_tcp_6(LOCATOR_KIND_TCPv6, 0);

    // 14. Single TCPv4 address without specifying a custom listening port

    text = "TCPv4:[192.168.36.34]";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "192.168.36.34");
    IPLocator::setPhysicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    standard.push_back(loc_tcp);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 15. Single TCPv6 address without specifying a custom listening port

    text = "TCPv6:[2a02:26f0:dd:499::356e]";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a02:26f0:dd:499::356e");
    IPLocator::setPhysicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    standard.push_back(loc_tcp_6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 16. Single TCPv4 address specifying a custom listening port

    text = "TCPv4:[192.168.36.34]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "192.168.36.34");
    IPLocator::setPhysicalPort(loc_tcp, 14520);
    IPLocator::setLogicalPort(loc_tcp, 14520);
    standard.push_back(loc_tcp);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 17. Single TCPv6 address specifying a custom listening port

    text = "TCPv6:[2a02:26f0:dd:499::356e]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a02:26f0:dd:499::356e");
    IPLocator::setPhysicalPort(loc_tcp_6, 14520);
    IPLocator::setLogicalPort(loc_tcp_6, 14520);
    standard.push_back(loc_tcp_6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);
}

//! Tests the server-client setup using environment variable works fine using DNS
TEST(ServerAttributesTests, ServerClientEnvironmentSetUpDNS)
{
    using namespace std;
    using namespace eprosima::fastdds::rtps;

    LocatorList_t output, standard;
    Locator_t loc, loc6(LOCATOR_KIND_UDPv6, 0);

    Locator_t loc_tcp(LOCATOR_KIND_TCPv4, 0);
    Locator_t loc_tcp_6(LOCATOR_KIND_TCPv6, 0);

    // 1. single server DNS address resolution without specific port provided
    std::string text = "www.acme.com.test";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc6);
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 2. single server DNS address specifying a custom listening port
    text = "www.acme.com.test:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, 14520);
    standard.push_back(loc6);
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 3. single server DNS address specifying a custom locator type
    // UDPv4
    text = "UDPv4:[www.acme.com.test]";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // UDPv6
    text = "UDPv6:[www.acme.com.test]";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    standard.push_back(loc6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv4
    text = "TCPv4:[www.acme.com.test]";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "216.58.215.164");
    IPLocator::setPhysicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    standard.push_back(loc_tcp);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv6
    text = "TCPv6:[www.acme.com.test]";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    standard.push_back(loc_tcp_6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 4. single server DNS address specifying a custom locator type and listening port
    // UDPv4
    text = "UDPv4:[www.acme.com.test]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // UDPv6
    text = "UDPv6:[www.acme.com.test]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, 14520);
    standard.push_back(loc6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv4
    text = "TCPv4:[www.acme.com.test]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "216.58.215.164");
    IPLocator::setPhysicalPort(loc_tcp, 14520);
    IPLocator::setLogicalPort(loc_tcp, 14520);
    standard.push_back(loc_tcp);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv6
    text = "TCPv6:[www.acme.com.test]:14520";

    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc_tcp_6, 14520);
    IPLocator::setLogicalPort(loc_tcp_6, 14520);
    standard.push_back(loc_tcp_6);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // SHM Locator kind should fail
    text = "SHM:[www.acme.com.test]";

    output.clear();
    ASSERT_FALSE(load_environment_server_info(text, output));

    // 5. Check mixed scenario with addresses and dns
    text = "192.168.36.34:14520;UDPv6:[www.acme.com.test]:14520;172.30.80.1:31090;";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    standard.push_back(loc);

    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, 14520);
    standard.push_back(loc6);

    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 6. Check Domain ID is transformed into a Discovery Server port
    text = "192.168.36.34:0;172.30.80.1:7;";

    output.clear();
    standard.clear();

    IPLocator::setIPv4(loc, string("192.168.36.34"));
    PortParameters port_params;
    IPLocator::setPhysicalPort(loc, port_params.get_discovery_server_port(0));
    standard.push_back(loc);

    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, port_params.get_discovery_server_port(7));
    standard.push_back(loc);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
