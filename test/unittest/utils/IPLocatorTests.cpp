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

// Check if an address given by string is equal to the locator addresss
// This function assumes setIPv4 and setIPv6 are well tested
static bool address_match(const std::string str_addr, const Locator_t& locator)
{
    Locator_t aux_locator;
    bool res = true;

    switch (locator.kind)
    {
        case LOCATOR_KIND_TCPv4:
        case LOCATOR_KIND_UDPv4:
        {
            IPLocator::setIPv4(aux_locator, str_addr);
            break;
        }
        case LOCATOR_KIND_TCPv6:
        case LOCATOR_KIND_UDPv6:
        {
            IPLocator::setIPv6(aux_locator, str_addr);
            break;
        }
    }
    for (int i=0; i<16; i++)
    {
        res &= locator.address[i] == aux_locator.address[i];
    }
    return res;
}

class IPLocatorTests: public ::testing::Test
{

public:
    const std::string ipv4_address = "74.65.86.73";
    const std::string ipv4_address_2 = "106.97.118.105";
    const std::string ipv4_lo_address = "127.0.0.1";

    const std::string ipv6_address = "4a41:5649::50:4152:4953";
    const std::string ipv6_address_repeated = "4a41:5649:0000:0000:0000:0050:4152:4953";
    const std::string ipv6_address_2 = "4a41:5649::150:4152:4953";
    const std::string ipv6_address_full = "4a41:5649:85a3:a8d3:1319:0050:4152:4953";
    const std::string ipv6_lo_address = "::1";

    const std::string ip_invalid = "0:0:0:0:0:0:0:0";
    const uint32_t port1 = 6666;
    const uint32_t port2 = 7400;
};

TEST_F(IPLocatorTests, address_match)
{
    Locator_t locator;
    ASSERT_TRUE(address_match(ip_invalid, locator));

    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_address, port1, locator);
    ASSERT_TRUE(address_match(ipv4_address, locator));
    ASSERT_FALSE(address_match(ip_invalid, locator));
    ASSERT_FALSE(address_match(ipv4_lo_address, locator));
    ASSERT_FALSE(address_match(ipv6_lo_address, locator));

    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_address, port1, locator);
    ASSERT_TRUE(address_match(ipv6_address, locator));
    ASSERT_TRUE(address_match(ipv6_address_repeated, locator));
    ASSERT_FALSE(address_match(ip_invalid, locator));
    ASSERT_FALSE(address_match(ipv4_address, locator));
    ASSERT_FALSE(address_match(ipv6_address_full, locator));
}

TEST_F(IPLocatorTests, createLocator)
{
    int32_t kind;
    Locator_t res_locator;

    // create UDP IPv4
    kind = LOCATOR_KIND_UDPv4;
    eprosima::fastrtps::rtps::IPLocator::createLocator(kind, ipv4_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    ASSERT_TRUE(address_match(ipv4_address, res_locator));

    // create TCP IPv4
    kind = LOCATOR_KIND_TCPv4;
    IPLocator::createLocator(kind, ipv4_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    ASSERT_TRUE(address_match(ipv4_address, res_locator));

    // create UDP IPv6
    kind = LOCATOR_KIND_UDPv6;
    IPLocator::createLocator(kind, ipv6_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    ASSERT_TRUE(address_match(ipv6_address, res_locator));

    // create TCP IPv6
    kind = LOCATOR_KIND_TCPv6;
    IPLocator::createLocator(kind, ipv6_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    ASSERT_TRUE(address_match(ipv6_address, res_locator));

    // create SHM
    kind = LOCATOR_KIND_SHM;
    IPLocator::createLocator(kind, ipv6_address, port1, res_locator);
    ASSERT_EQ(res_locator.kind, kind);
    ASSERT_EQ(res_locator.port, port1);
    ASSERT_TRUE(address_match(ip_invalid, res_locator));
}

TEST_F(IPLocatorTests, setIPv4)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, ipv4_lo_address, port1, locator);

    // setIPv4 char*
    IPLocator::setIPv4(locator, ipv4_address.c_str());
    ASSERT_TRUE(address_match(ipv4_address, locator));

    // setIPv4 4xoctet
    IPLocator::setIPv4(locator, 106, 97, 118, 105);
    ASSERT_TRUE(address_match(ipv4_address_2, locator));

    // setIPv4 string
    IPLocator::setIPv4(locator, ipv4_address);
    ASSERT_TRUE(address_match(ipv4_address, locator));

    // setIPv4 locator
    Locator_t ipv4_locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv4, ipv4_address_2, port1, ipv4_locator);
    IPLocator::setIPv4(locator, ipv4_locator);
    ASSERT_TRUE(address_match(ipv4_address_2, locator));
}

TEST_F(IPLocatorTests, setIPv6)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, ipv6_lo_address, port1, locator);

    // setIPv6 char*
    IPLocator::setIPv6(locator, ipv6_lo_address.c_str());
    ASSERT_TRUE(address_match(ipv6_lo_address, locator));

    // setIPv6 4xoctet
    IPLocator::setIPv6(locator, 0x4a41, 0x5649, 0x0000, 0x0000, 0x0000, 0x0050, 0x4152, 0x4953);
    ASSERT_TRUE(address_match(ipv6_address, locator));

    // setIPv6 string
    IPLocator::setIPv6(locator, ipv6_address_2);
    ASSERT_TRUE(address_match(ipv6_address_2, locator));

    // setIPv6 locator
    Locator_t ipv4_locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, ipv6_address, port1, ipv4_locator);
    IPLocator::setIPv6(locator, ipv6_address_repeated);
    ASSERT_TRUE(address_match(ipv6_address, locator));
}

TEST_F(IPLocatorTests, getIPv4)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, getIPv6)
{
    // TODO
    ASSERT_TRUE(false);
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
}

TEST_F(IPLocatorTests, copyIPv4)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, copyIPv6)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, ip)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, ip_to_string)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, setLogicalPort)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, getLogicalPort)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, setPhysicalPort)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, getPhysicalPort)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, setWan)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, getWan)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, hasWan)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, toWanstring)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, setLanID)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, getLanID)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, toLanIDstring)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, toPhysicalLocator)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, ip_equals_wan)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, setPortRTPS)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, getPortRTPS)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, isLocal)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, isAny)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, compareAddress)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, compareAddressAndPhysicalPort)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, to_string)
{
    // TODO
    ASSERT_TRUE(false);
}

TEST_F(IPLocatorTests, isMulticast)
{
    // TODO
    ASSERT_TRUE(false);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
