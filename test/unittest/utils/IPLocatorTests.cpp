// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/rtps/common/Locator.h>

#include <gtest/gtest.h>

#include <string>

using namespace eprosima::fastrtps::rtps;

static bool address_match(const Locator_t& loc1, const Locator_t& loc2)
{
    for (int i=0; i<16; i++)
    {
        if (loc1.address[i] != loc2.address[i])
        {
            return false;
        }
    }
    return true;
}

class IPLocatorTests: public ::testing::Test
{

public:
    const std::string ipv4_address = "74.65.86.73";
    const std::string ipv4_address_repeated = "74.65.86.073";
    const std::string ipv4_address_2 = "106.97.118.105";
    const std::string ipv4_lo_address = "127.0.0.1";
    const std::string ipv4_multicast_address = "224.0.0.0";
    const std::string ipv4_multicast_address_2 = "239.255.255.255";

    const std::string ipv6_address = "4a41:5649::50:4152:4953";
    const std::string ipv6_address_repeated = "4a41:5649:0000:0000:0000:0050:4152:4953";
    const std::string ipv6_address_2 = "4a41:5649::150:4152:4953";
    const std::string ipv6_address_full = "4a41:5649:85a3:a8d3:1319:0050:4152:4953";
    const std::string ipv6_lo_address = "::1";
    const std::string ipv6_multicast_address = "FF00::";
    const std::string ipv6_multicast_address_2 = "FFFF::";
    const std::string ipv6_multicast_address_3 = "FF0A::1:2:3";

    const std::string ipv4_any = "0.0.0.0";
    const std::string ipv4_invalid = "0.0.0.0";
    const std::string ipv6_any = "::";
    const std::string ipv6_invalid = "0:0:0:0:0:0:0:0";

    const uint32_t port1 = 6666;
    const uint32_t port2 = 7400;
};

// All the tests of ipv4 depends on this function
TEST_F(IPLocatorTests, setIPv4_from_string)
{
    Locator_t locator;

    {
        // empty string
        ASSERT_TRUE(IPLocator::setIPv4(locator, "0.0.0.0"));
        std::vector<int> vec{0, 0, 0, 0};
        for (int i=0; i<12; i++)
        {
            ASSERT_EQ(locator.address[i], 0);
        }
        for (int i=0; i<4; i++)
        {
            ASSERT_EQ(locator.address[i+12], vec[i]);
        }
    }

    {
        // std string
        ASSERT_TRUE(IPLocator::setIPv4(locator, "1.2.3.4"));
        std::vector<int> vec{1, 2, 3, 4};

        for (int i=0; i<12; i++)
        {
            ASSERT_EQ(locator.address[i], 0);
        }
        for (int i=0; i<4; i++)
        {
            ASSERT_EQ(locator.address[i+12], vec[i]);
        }
    }

    {
        // std string with 0
        ASSERT_TRUE(IPLocator::setIPv4(locator, "01.002.0003.000104"));
        std::vector<int> vec{1, 2, 3, 104};

        for (int i=0; i<12; i++)
        {
            ASSERT_EQ(locator.address[i], 0);
        }
        for (int i=0; i<4; i++)
        {
            ASSERT_EQ(locator.address[i+12], vec[i]);
        }
    }

    {
        // multicast
        ASSERT_TRUE(IPLocator::setIPv4(locator, "255.255.255.255"));
        std::vector<int> vec{255, 255, 255, 255};

        for (int i=0; i<12; i++)
        {
            ASSERT_EQ(locator.address[i], 0);
        }
        for (int i=0; i<4; i++)
        {
            ASSERT_EQ(locator.address[i+12], vec[i]);
        }
    }

    {
        // error
        ASSERT_FALSE(IPLocator::setIPv4(locator, "1.1.1.256"));
        ASSERT_FALSE(IPLocator::setIPv4(locator, "1.1.1"));
    }
}

TEST_F(IPLocatorTests, setIPv6_from_string_empty)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv6;

    // empty string
    std::vector<int> vec(16, 0);

    ASSERT_TRUE(IPLocator::setIPv6(locator, "::"));
    for (int i=0; i<16; i++)
    {
        ASSERT_EQ(locator.address[i], vec[i]);
    }

    ASSERT_TRUE(IPLocator::setIPv6(locator, "::0"));
    for (int i=0; i<16; i++)
    {
        ASSERT_EQ(locator.address[i], vec[i]);
    }

    ASSERT_TRUE(IPLocator::setIPv6(locator, "0000000::"));
    for (int i=0; i<16; i++)
    {
        ASSERT_EQ(locator.address[i], vec[i]);
    }

    ASSERT_TRUE(IPLocator::setIPv6(locator, "0::0"));
    for (int i=0; i<16; i++)
    {
        ASSERT_EQ(locator.address[i], vec[i]);
    }

    ASSERT_TRUE(IPLocator::setIPv6(locator, "0:0:0:0::0:0"));
    for (int i=0; i<16; i++)
    {
        ASSERT_EQ(locator.address[i], vec[i]);
    }

    ASSERT_TRUE(IPLocator::setIPv6(locator, "0000::0000"));
    for (int i=0; i<16; i++)
    {
        ASSERT_EQ(locator.address[i], vec[i]);
    }

    ASSERT_TRUE(IPLocator::setIPv6(locator, "0000:0000:0000:0000:0000:0000:0000:0000"));
    for (int i=0; i<16; i++)
    {
        ASSERT_EQ(locator.address[i], vec[i]);
    }
}

TEST_F(IPLocatorTests, setIPv6_from_string_std)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv6;

    // localhost IP
    {
        std::vector<int> vec(15, 0);
        vec.push_back(1);

        ASSERT_TRUE(IPLocator::setIPv6(locator, "::1"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }

        ASSERT_TRUE(IPLocator::setIPv6(locator, "0:0::0:0001"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }
    }

    // std IP
    {
        std::vector<int> vec(16, 0);
        vec[2] = 2;
        vec[3] = 3;
        vec[6] = 6;
        vec[7] = 7;
        vec[12] = 12;
        vec[13] = 13;

        ASSERT_TRUE(IPLocator::setIPv6(
            locator,
            "0000:0203:0000:0607:0000:0000:0c0d:0000"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }

        ASSERT_TRUE(IPLocator::setIPv6(
            locator,
            "0:203:0:607:0:0:c0d:0"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }

        ASSERT_TRUE(IPLocator::setIPv6(
            locator,
            "::203:0:607:0:0:c0d:0"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }

        ASSERT_TRUE(IPLocator::setIPv6(
            locator,
            "0:203:0:607:0:0:c0d::"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }

        ASSERT_TRUE(IPLocator::setIPv6(
            locator,
            "0:203::607:0:0:c0d:0"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }

        ASSERT_TRUE(IPLocator::setIPv6(
            locator,
            "0:203:0:607::c0d:0"));
        for (int i=0; i<16; i++)
        {
            ASSERT_EQ(locator.address[i], vec[i]);
        }
    }
}

TEST_F(IPLocatorTests, setIPv6_from_string_invalid)
{
    Locator_t locator;
    ASSERT_FALSE(IPLocator::setIPv6(locator, ":"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "::1:"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, ":1::"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "::1::"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "1::1::1"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "1:::1"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "1:1:1:1:1:1:1:1:1"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "1:1:1:1:1:1:1::1"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "10000::"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "::10000"));
    ASSERT_FALSE(IPLocator::setIPv6(locator, "1:10000::1"));
}

TEST_F(IPLocatorTests, createLocator)
{
    int32_t kind;
    Locator_t res_locator;
    Locator_t probe_locator;

    // create UDP IPv4
    kind = LOCATOR_KIND_UDPv4;
    IPLocator::createLocator(kind, ipv4_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    IPLocator::setIPv4(probe_locator, ipv4_address_repeated);
    ASSERT_TRUE(address_match(probe_locator, res_locator));

    // create TCP IPv4
    kind = LOCATOR_KIND_TCPv4;
    IPLocator::createLocator(kind, ipv4_lo_address, port2, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port2);
    IPLocator::setIPv4(probe_locator, ipv4_lo_address);
    ASSERT_TRUE(address_match(probe_locator, res_locator));

    // create UDP IPv6
    kind = LOCATOR_KIND_UDPv6;
    IPLocator::createLocator(kind, ipv6_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    IPLocator::setIPv6(probe_locator, ipv6_address_repeated);
    ASSERT_TRUE(address_match(probe_locator, res_locator));

    // create TCP IPv6
    kind = LOCATOR_KIND_TCPv6;
    IPLocator::createLocator(kind, ipv6_any, port2, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port2);
    IPLocator::setIPv6(probe_locator, ipv6_invalid);
    ASSERT_TRUE(address_match(probe_locator, res_locator));

    // create SHM
    kind = LOCATOR_KIND_SHM;
    IPLocator::createLocator(kind, ipv6_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    IPLocator::setIPv6(probe_locator, ipv6_invalid);
    ASSERT_TRUE(address_match(probe_locator, res_locator));
}

TEST_F(IPLocatorTests, setIPv4)
{
    Locator_t locator;
    Locator_t probe_locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_lo_address, port1, locator);

    // setIPv4 char*
    // TODO
    IPLocator::setIPv4(probe_locator, ipv4_lo_address);
    unsigned char arr[4]{127, 0, 0, 1};
    ASSERT_TRUE(IPLocator::setIPv4(locator, arr));
    ASSERT_TRUE(address_match(probe_locator, locator));

    // setIPv4 4xoctet
    IPLocator::setIPv4(probe_locator, ipv4_address_2);
    ASSERT_TRUE(IPLocator::setIPv4(locator, 106, 97, 118, 105));
    ASSERT_TRUE(address_match(probe_locator, locator));

    // setIPv4 locator
    Locator_t ipv4_locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address_2, port1, ipv4_locator);
    ASSERT_TRUE(IPLocator::setIPv4(locator, ipv4_locator));
    ASSERT_TRUE(address_match(ipv4_locator, locator));
}

TEST_F(IPLocatorTests, setIPv6)
{
    Locator_t locator;
    Locator_t probe_locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_lo_address, port1, locator);

    // setIPv6 char*
    IPLocator::setIPv6(probe_locator, ipv6_lo_address);
    unsigned char arr[16]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    ASSERT_TRUE(IPLocator::setIPv6(locator, arr));
    ASSERT_TRUE(address_match(probe_locator, locator));

    // setIPv6 4xoctet
    IPLocator::setIPv6(probe_locator, ipv6_address);
    ASSERT_TRUE(IPLocator::setIPv6(locator, 0x4a41, 0x5649, 0x0000, 0x0000, 0x0000, 0x0050, 0x4152, 0x4953));
    ASSERT_TRUE(address_match(probe_locator, locator));

    // setIPv6 locator
    Locator_t ipv6_locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_address, port1, ipv6_locator);
    ASSERT_TRUE(IPLocator::setIPv6(locator, ipv6_locator));
    ASSERT_TRUE(address_match(ipv6_locator, locator));
}

TEST_F(IPLocatorTests, getIPv4)
{
    Locator_t locator;
    IPLocator::setIPv4(locator, ipv4_lo_address);
    auto res = IPLocator::getIPv4(locator);
    ASSERT_EQ(res[0], 127);
    ASSERT_EQ(res[1], 0);
    ASSERT_EQ(res[2], 0);
    ASSERT_EQ(res[3], 1);
}

TEST_F(IPLocatorTests, getIPv6)
{
    Locator_t locator;
    IPLocator::setIPv6(locator, ipv6_lo_address);
    auto res = IPLocator::getIPv6(locator);
    for (int i=0; i<15; i++)
    {
        ASSERT_EQ(res[i], 0);
    }
    ASSERT_EQ(res[15], 1);
}

TEST_F(IPLocatorTests, hasIP)
{
    Locator_t locator;

    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator);
    ASSERT_TRUE(IPLocator::hasIPv4(locator));
    ASSERT_FALSE(IPLocator::hasIPv6(locator));

    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_lo_address, port1, locator);
    ASSERT_TRUE(IPLocator::hasIPv4(locator));
    ASSERT_FALSE(IPLocator::hasIPv6(locator));

    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address, port1, locator);
    ASSERT_TRUE(IPLocator::hasIPv6(locator));
    ASSERT_FALSE(IPLocator::hasIPv4(locator));

    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_lo_address, port1, locator);
    ASSERT_TRUE(IPLocator::hasIPv6(locator));
    ASSERT_FALSE(IPLocator::hasIPv4(locator));

    IPLocator::createLocator(LOCATOR_PORT_INVALID, ipv4_address, port1, locator);
    ASSERT_FALSE(IPLocator::hasIPv4(locator));
    ASSERT_FALSE(IPLocator::hasIPv6(locator));

    IPLocator::createLocator(LOCATOR_KIND_SHM, ipv6_address, port1, locator);
    ASSERT_FALSE(IPLocator::hasIPv4(locator));
    ASSERT_FALSE(IPLocator::hasIPv6(locator));
}

TEST_F(IPLocatorTests, toIPv4string)
{
    Locator_t locator;
    IPLocator::setIPv4(locator, 127, 0, 0, 1);
    ASSERT_EQ(IPLocator::toIPv4string(locator), ipv4_lo_address);
    IPLocator::setIPv4(locator, ipv4_address.c_str());
    ASSERT_EQ(IPLocator::toIPv4string(locator), ipv4_address);
}

TEST_F(IPLocatorTests, toIPv6string)
{
    Locator_t locator;
    IPLocator::setIPv6(locator, 0, 0, 0, 0, 0, 0, 0, 1);
    ASSERT_EQ(IPLocator::toIPv6string(locator), ipv6_lo_address);
    IPLocator::setIPv6(locator, ipv6_address_repeated.c_str());
    ASSERT_EQ(IPLocator::toIPv6string(locator), ipv6_address);

    IPLocator::setIPv6(locator, "11::");
    ASSERT_EQ(IPLocator::toIPv6string(locator), "11::");
    IPLocator::setIPv6(locator, "::11");
    ASSERT_EQ(IPLocator::toIPv6string(locator), "::11");
    IPLocator::setIPv6(locator, "11::01");
    ASSERT_EQ(IPLocator::toIPv6string(locator), "11::1");
    IPLocator::setIPv6(locator, "0::01");
    ASSERT_EQ(IPLocator::toIPv6string(locator), "::1");
    IPLocator::setIPv6(locator, "0::0:0");
    ASSERT_EQ(IPLocator::toIPv6string(locator), "::");
}

TEST_F(IPLocatorTests, copyIPv4)
{
    Locator_t locator;
    IPLocator::setIPv4(locator, ipv4_lo_address);
    unsigned char arr[4];
    ASSERT_TRUE(IPLocator::copyIPv4(locator, arr));
    ASSERT_EQ(arr[0], 127);
    ASSERT_EQ(arr[1], 0);
    ASSERT_EQ(arr[2], 0);
    ASSERT_EQ(arr[3], 1);
}

TEST_F(IPLocatorTests, copyIPv6)
{
    Locator_t locator;
    IPLocator::setIPv6(locator, ipv6_lo_address);
    unsigned char arr[16];
    ASSERT_TRUE(IPLocator::copyIPv6(locator, arr));
    for (int i=0; i<15; i++)
    {
        ASSERT_EQ(arr[i], 0);
    }
    ASSERT_EQ(arr[15], 1);
}

TEST_F(IPLocatorTests, ip)
{
    Locator_t locator;

    // UDPv4
    locator.kind = LOCATOR_KIND_UDPv4;
    ASSERT_TRUE(IPLocator::ip(locator, ipv4_address_repeated));
    ASSERT_EQ(IPLocator::toIPv4string(locator), ipv4_address);

    // TCPv4
    locator.kind = LOCATOR_KIND_TCPv4;
    ASSERT_TRUE(IPLocator::ip(locator, ipv4_lo_address));
    ASSERT_EQ(IPLocator::toIPv4string(locator), ipv4_lo_address);

    // UDPv6
    locator.kind = LOCATOR_KIND_UDPv6;
    ASSERT_TRUE(IPLocator::ip(locator, ipv6_address_repeated));
    ASSERT_EQ(IPLocator::toIPv6string(locator), ipv6_address);

    // TCPv6
    locator.kind = LOCATOR_KIND_TCPv6;
    ASSERT_TRUE(IPLocator::ip(locator, ipv6_lo_address));
    ASSERT_EQ(IPLocator::toIPv6string(locator), ipv6_lo_address);
}

TEST_F(IPLocatorTests, ip_to_string)
{
    Locator_t locator;

    // v4
    IPLocator::setIPv4(locator, 127, 0, 0, 1);
    locator.kind = LOCATOR_KIND_UDPv4;
    ASSERT_EQ(IPLocator::ip_to_string(locator), ipv4_lo_address);
    IPLocator::setIPv4(locator, ipv4_address_repeated);
    locator.kind = LOCATOR_KIND_TCPv4;
    ASSERT_EQ(IPLocator::ip_to_string(locator), ipv4_address);

    // v6
    IPLocator::setIPv6(locator, 0, 0, 0, 0, 0, 0, 0, 1);
    locator.kind = LOCATOR_KIND_UDPv6;
    ASSERT_EQ(IPLocator::ip_to_string(locator), ipv6_lo_address);
    IPLocator::setIPv6(locator, ipv6_address_repeated);
    locator.kind = LOCATOR_KIND_TCPv6;
    ASSERT_EQ(IPLocator::ip_to_string(locator), ipv6_address);
}

TEST_F(IPLocatorTests, logicalPort)
{
    Locator_t locator;
    ASSERT_TRUE(IPLocator::setLogicalPort(locator, port1));
    ASSERT_EQ(IPLocator::getLogicalPort(locator), port1);
    ASSERT_NE(IPLocator::getLogicalPort(locator), port2);
    ASSERT_TRUE(IPLocator::setLogicalPort(locator, port2));
    ASSERT_EQ(IPLocator::getLogicalPort(locator), port2);
}

TEST_F(IPLocatorTests, setPhysicalPort)
{
    Locator_t locator;
    ASSERT_TRUE(IPLocator::setPhysicalPort(locator, port1));
    ASSERT_EQ(IPLocator::getPhysicalPort(locator), port1);
    ASSERT_NE(IPLocator::getPhysicalPort(locator), port2);
    ASSERT_TRUE(IPLocator::setPhysicalPort(locator, port2));
    ASSERT_EQ(IPLocator::getPhysicalPort(locator), port2);
}

// TEST_F(IPLocatorTests, setWan)
// {
//     // TODO
//     ASSERT_TRUE(false);
// }

TEST_F(IPLocatorTests, getWan)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address, port1, locator);
    IPLocator::setWan(locator, "0.1.2.3");
    for (int i=0; i<4; i++)
    {
        ASSERT_EQ(IPLocator::getWan(locator)[i], i);
    }
}

// TEST_F(IPLocatorTests, hasWan)
// {
//     // TODO
//     ASSERT_TRUE(false);
// }

TEST_F(IPLocatorTests, toWanstring)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address, port1, locator);
    IPLocator::setWan(locator, "0.1.2.3");
    ASSERT_EQ(IPLocator::toWanstring(locator), "0.1.2.3");
}

// TEST_F(IPLocatorTests, setLanID)
// {
//     // TODO
//     ASSERT_TRUE(false);
// }

TEST_F(IPLocatorTests, getLanID)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address, port1, locator);
    IPLocator::setLanID(locator, "0.1.2.3.4.5.6.7");
    for (int i=0; i<8; i++)
    {
        ASSERT_EQ(IPLocator::getLanID(locator)[i], i);
    }
}

TEST_F(IPLocatorTests, toLanIDstring)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address, port1, locator);
    IPLocator::setLanID(locator, "0.1.2.3.4.5.6.7");
    ASSERT_EQ(IPLocator::toLanIDstring(locator), "0.1.2.3.4.5.6.7");

    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator);
    IPLocator::setWan(locator, ipv4_lo_address);
    ASSERT_EQ(IPLocator::toLanIDstring(locator), "");
}

TEST_F(IPLocatorTests, toPhysicalLocator)
{
    Locator_t locator;
    ASSERT_TRUE(IPLocator::setLogicalPort(locator, port1));
    ASSERT_EQ(IPLocator::getLogicalPort(locator), port1);
    locator = IPLocator::toPhysicalLocator(locator);
    ASSERT_NE(IPLocator::getLogicalPort(locator), port1);
}

TEST_F(IPLocatorTests, ip_equals_wan)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address, port1, locator);
    IPLocator::setWan(locator, ipv4_address);
    ASSERT_TRUE(IPLocator::ip_equals_wan(locator));

    IPLocator::setWan(locator, ipv4_lo_address);
    ASSERT_FALSE(IPLocator::ip_equals_wan(locator));
}

TEST_F(IPLocatorTests, setPortRTPS)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator);
    ASSERT_TRUE(IPLocator::setPortRTPS(locator, port2));
    ASSERT_EQ(IPLocator::getPortRTPS(locator), static_cast<uint16_t>(port2));

    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address, port2, locator);
    IPLocator::setPortRTPS(locator, port1);
    ASSERT_EQ(IPLocator::getPortRTPS(locator), static_cast<uint16_t>(port1));

    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address_2, port1, locator);
    ASSERT_TRUE(IPLocator::setPortRTPS(locator, port2));
    ASSERT_EQ(IPLocator::getPortRTPS(locator), static_cast<uint16_t>(port2));

    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_address_2, port2, locator);
    ASSERT_TRUE(IPLocator::setPortRTPS(locator, port1));
    ASSERT_EQ(IPLocator::getPortRTPS(locator), static_cast<uint16_t>(port1));

    IPLocator::createLocator(LOCATOR_KIND_SHM, ipv4_lo_address, port1, locator);
    ASSERT_FALSE(IPLocator::setPortRTPS(locator, port1));
    ASSERT_EQ(IPLocator::getPortRTPS(locator), 0);
}

TEST_F(IPLocatorTests, isLocal)
{
    // ipv4 UDP
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(locator, ipv4_address);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv4(locator, ipv4_any);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv4(locator, ipv4_lo_address);
    ASSERT_TRUE(IPLocator::isLocal(locator));

    // ipv4 TCP
    locator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(locator, ipv4_address);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv4(locator, ipv4_any);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv4(locator, ipv4_lo_address);
    ASSERT_TRUE(IPLocator::isLocal(locator));

    // ipv6 UDP
    locator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(locator, ipv6_address);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv6(locator, ipv6_any);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv6(locator, ipv6_lo_address);
    ASSERT_TRUE(IPLocator::isLocal(locator));

    // ipv6 TCP
    locator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv6(locator, ipv6_address);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv6(locator, ipv6_any);
    ASSERT_FALSE(IPLocator::isLocal(locator));
    IPLocator::setIPv6(locator, ipv6_lo_address);
    ASSERT_TRUE(IPLocator::isLocal(locator));
}

TEST_F(IPLocatorTests, isAny)
{
    // IPv4
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator);
    ASSERT_FALSE(IPLocator::isAny(locator));
    IPLocator::setIPv4(locator, ipv4_any);
    ASSERT_TRUE(IPLocator::isAny(locator));

    // IPv6
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address, port1, locator);
    ASSERT_FALSE(IPLocator::isAny(locator));
    IPLocator::setIPv4(locator, ipv6_any);
    ASSERT_TRUE(IPLocator::isAny(locator));
}

TEST_F(IPLocatorTests, compareAddress)
{
    Locator_t locator1, locator2;

    // UDP v4
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator1);
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address_repeated, port1, locator2);
    ASSERT_TRUE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address_2, port1, locator2);
    ASSERT_FALSE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));

    // TCP v4
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address, port1, locator1);
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address_repeated, port1, locator2);
    ASSERT_TRUE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address_2, port1, locator2);
    ASSERT_FALSE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));

    // UDP v6
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address, port1, locator1);
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address_repeated, port1, locator2);
    ASSERT_TRUE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address_2, port1, locator2);
    ASSERT_FALSE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));

    // TCP v6
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_address, port1, locator1);
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_address_repeated, port1, locator2);
    ASSERT_TRUE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_address_2, port1, locator2);
    ASSERT_FALSE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
}

TEST_F(IPLocatorTests, compareAddressAndPhysicalPort)
{
    Locator_t locator1, locator2;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator1);
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator2);
    ASSERT_TRUE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address, port1, locator2);
    ASSERT_FALSE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));

    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address, port1, locator1);
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address, port1, locator2);
    ASSERT_TRUE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_address, port1, locator2);
    ASSERT_FALSE(IPLocator::compareAddressAndPhysicalPort(locator1, locator2));
}

TEST_F(IPLocatorTests, to_string)
{
    Locator_t locator;

    // UDPv4
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "0.0.0.1", 1, locator);
    ASSERT_EQ(IPLocator::to_string(locator), "1:0.0.0.1:1");

    // TCPv4
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, "0.0.1.1", 2, locator);
    ASSERT_EQ(IPLocator::to_string(locator), "4:0.0.1.1:2");

    // UDPv6
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, "200::", 3, locator);
    ASSERT_EQ(IPLocator::to_string(locator), "2:2.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0:3");

    // TCPv6
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::2", 4, locator);
    ASSERT_EQ(IPLocator::to_string(locator), "8:0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.2:4");

    // SHM
    IPLocator::createLocator(LOCATOR_KIND_SHM, "", 5, locator);
    ASSERT_EQ(IPLocator::to_string(locator), "16:SHM::5");
}

TEST_F(IPLocatorTests, isMulticast)
{
    Locator_t locator;

    // UDP v4
    locator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(locator, ipv4_address);
    ASSERT_FALSE(IPLocator::isMulticast(locator));
    IPLocator::setIPv4(locator, ipv4_multicast_address);
    ASSERT_TRUE(IPLocator::isMulticast(locator));
    IPLocator::setIPv4(locator, ipv4_multicast_address_2);
    ASSERT_TRUE(IPLocator::isMulticast(locator));

    // UDP v6
    locator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(locator, ipv6_address);
    ASSERT_FALSE(IPLocator::isMulticast(locator));
    IPLocator::setIPv6(locator, ipv6_multicast_address);
    ASSERT_TRUE(IPLocator::isMulticast(locator));
    IPLocator::setIPv6(locator, ipv6_multicast_address_2);
    ASSERT_TRUE(IPLocator::isMulticast(locator));
    IPLocator::setIPv6(locator, ipv6_multicast_address_3);
    ASSERT_TRUE(IPLocator::isMulticast(locator));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
