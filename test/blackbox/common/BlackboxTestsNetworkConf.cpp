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

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubParticipant.hpp"

#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/utils/IPFinder.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum communication_type
{
    TRANSPORT
};

class NetworkConfig : public testing::TestWithParam<std::tuple<communication_type, bool>>
{
public:

    void SetUp() override
    {
        descriptor_.reset();
        use_udpv4 = std::get<1>(GetParam());
        if (use_udpv4)
        {
            descriptor_ = std::make_shared<UDPv4TransportDescriptor>();
        }
        else
        {
            descriptor_ = std::make_shared<UDPv6TransportDescriptor>();
        }
    }

    void TearDown() override
    {
        use_udpv4 = false;
    }

    std::shared_ptr<UDPTransportDescriptor> descriptor_;
    std::string ip;
};

static void GetIP4s(
        std::vector<IPFinder::info_IP>& interfaces)
{
    IPFinder::getIPs(&interfaces, false);
    auto new_end = remove_if(interfaces.begin(),
                    interfaces.end(),
                    [](IPFinder::info_IP ip)
                    {
                        return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;
                    });
    interfaces.erase(new_end, interfaces.end());
    std::for_each(interfaces.begin(), interfaces.end(), [](IPFinder::info_IP& loc)
            {
                loc.locator.kind = LOCATOR_KIND_UDPv4;
            });
}

static void GetIP6s(
        std::vector<IPFinder::info_IP>& interfaces)
{
    IPFinder::getIPs(&interfaces, false);
    auto new_end = remove_if(interfaces.begin(),
                    interfaces.end(),
                    [](IPFinder::info_IP ip)
                    {
                        return ip.type != IPFinder::IP6 && ip.type != IPFinder::IP6_LOCAL;
                    });
    interfaces.erase(new_end, interfaces.end());
    std::for_each(interfaces.begin(), interfaces.end(), [](IPFinder::info_IP& loc)
            {
                loc.locator.kind = LOCATOR_KIND_UDPv6;
            });
}

TEST_P(NetworkConfig, pub_unique_network_flows)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy properties;
    properties.properties().emplace_back("fastdds.unique_network_flows", "");

    if (!use_udpv4)
    {
        writer.disable_builtin_transport().add_user_transport_to_pparams(descriptor_);
    }

    writer.entity_property_policy(properties).init();

    // Creation should fail as feature is not implemented for writers
    EXPECT_FALSE(writer.isInitialized());
}

TEST_P(NetworkConfig, sub_unique_network_flows)
{
    // Two readers on the same participant requesting unique flows should give different listening locators
    {
        PubSubParticipant<HelloWorldPubSubType> participant(0, 2, 0, 0);

        PropertyPolicy properties;
        properties.properties().emplace_back("fastdds.unique_network_flows", "");

        participant.sub_topic_name(TEST_TOPIC_NAME).sub_property_policy(properties);

        if (!use_udpv4)
        {
            participant.disable_builtin_transport().add_user_transport_to_pparams(descriptor_);
        }

        ASSERT_TRUE(participant.init_participant());
        ASSERT_TRUE(participant.init_subscriber(0));
        ASSERT_TRUE(participant.init_subscriber(1));

        LocatorList_t locators;
        LocatorList_t locators2;

        participant.get_native_reader(0).get_listening_locators(locators);
        participant.get_native_reader(1).get_listening_locators(locators2);

        EXPECT_FALSE(locators == locators2);
    }

    // Two readers on the same participant not requesting unique flows should give the same listening locators
    {
        PubSubParticipant<HelloWorldPubSubType> participant(0, 2, 0, 0);

        participant.sub_topic_name(TEST_TOPIC_NAME);

        if (!use_udpv4)
        {
            participant.disable_builtin_transport().add_user_transport_to_pparams(descriptor_);
        }

        ASSERT_TRUE(participant.init_participant());
        ASSERT_TRUE(participant.init_subscriber(0));
        ASSERT_TRUE(participant.init_subscriber(1));

        LocatorList_t locators;
        LocatorList_t locators2;

        participant.get_native_reader(0).get_listening_locators(locators);
        participant.get_native_reader(1).get_listening_locators(locators2);

        EXPECT_TRUE(locators == locators2);
    }
}

//Verify that outLocatorList is used to select the desired output channel
TEST_P(NetworkConfig, PubSubOutLocatorSelection)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    Locator_t locator;
    locator.kind = use_udpv4 ? LOCATOR_KIND_UDPv4 : LOCATOR_KIND_UDPv6;
    locator.port = 31337;

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(descriptor_);
    }

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
            resource_limits_allocated_samples(2).
            resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    descriptor_->m_output_udp_socket = static_cast<uint16_t>(locator.port);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_kind(
        eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
            resource_limits_allocated_samples(20).
            disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).
            resource_limits_max_samples(20).init();


    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
}

TEST_P(NetworkConfig, PubSubInterfaceWhitelistLocalhost)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    ip = use_udpv4 ? "127.0.0.1" : "::1";

    descriptor_->interfaceWhiteList.push_back(ip);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
            disable_multicast(0).
            disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
            disable_multicast(1).
            disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
}

TEST_P(NetworkConfig, PubSubInterfaceWhitelistUnicast)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::vector<IPFinder::info_IP> interfaces;
    use_udpv4 ? GetIP4s(interfaces) : GetIP6s(interfaces);

    for (const auto& interface : interfaces)
    {
        descriptor_->interfaceWhiteList.push_back(interface.name);
    }

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
            disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
            disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
}

TEST_P(NetworkConfig, SubGetListeningLocators)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(descriptor_);
    }

    reader.init();

    LocatorList_t locators;
    reader.get_native_reader().get_listening_locators(locators);

    std::vector<IPFinder::info_IP> interfaces;
    use_udpv4 ? GetIP4s(interfaces) : GetIP6s(interfaces);

    auto check_interface = [](const IPFinder::info_IP& address, const Locator_t& locator) -> bool
            {
                return IPLocator::compareAddress(address.locator, locator);
            };

    int32_t kind = use_udpv4 ? LOCATOR_KIND_UDPv4 : LOCATOR_KIND_UDPv6;
    for (const Locator_t& locator : locators)
    {
        if (locator.kind == kind)
        {
            auto checker = std::bind(check_interface, std::placeholders::_1, locator);
            EXPECT_NE(interfaces.cend(), std::find_if(interfaces.cbegin(), interfaces.cend(), checker));
        }
    }
}

TEST_P(NetworkConfig, PubGetSendingLocators)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    constexpr uint32_t port = 31337u;

    descriptor_->m_output_udp_socket = static_cast<uint16_t>(port);

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).
            init();

    ASSERT_TRUE(writer.isInitialized());

    LocatorList_t locators;
    writer.get_native_writer().get_sending_locators(locators);

    EXPECT_FALSE(locators.empty());
    Locator_t locator = *(locators.begin());
    EXPECT_EQ(locator.port, port);
    int32_t kind = use_udpv4 ? LOCATOR_KIND_UDPv4 : LOCATOR_KIND_UDPv6;
    EXPECT_EQ(locator.kind, kind);
}

TEST_P(NetworkConfig, PubGetSendingLocatorsWhitelist)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::vector<IPFinder::info_IP> interfaces;
    use_udpv4 ? GetIP4s(interfaces) : GetIP6s(interfaces);

    constexpr uint32_t port = 31337u;

    descriptor_->m_output_udp_socket = static_cast<uint16_t>(port);
    for (const auto& interface : interfaces)
    {
        std::cout << "Adding interface '" << interface.name << "' (" << interface.name.size() << ")" << std::endl;
        descriptor_->interfaceWhiteList.push_back(interface.name);
    }

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).
            init();

    ASSERT_TRUE(writer.isInitialized());

    LocatorList_t locators;
    writer.get_native_writer().get_sending_locators(locators);

    std::vector<bool> interfaces_found(interfaces.size(), false);

    EXPECT_EQ(interfaces.size(), locators.size());
    int32_t kind = use_udpv4 ? LOCATOR_KIND_UDPv4 : LOCATOR_KIND_UDPv6;
    for (const Locator_t& locator : locators)
    {
        std::cout << "Checking locator " << locator << std::endl;

        EXPECT_EQ(locator.port, port);
        EXPECT_EQ(locator.kind, kind);

        auto locator_ip = IPLocator::ip_to_string(locator);
        std::cout << "Checking '" << locator_ip << "' (" << locator_ip.size() << ")" << std::endl;
        for (size_t idx = 0; idx < interfaces.size(); ++idx)
        {
            // Remove ipv6 scope (it does not affect to ipv4 addresses)
            std::string substr = interfaces[idx].name.substr(0, interfaces[idx].name.find('%'));
            if (substr == locator_ip)
            {
                std::cout << "Found " << locator_ip << std::endl;
                interfaces_found[idx] = true;
                break;
            }
        }
    }
    EXPECT_TRUE(std::all_of(interfaces_found.cbegin(), interfaces_found.cend(),
            [](const bool& v)
            {
                return v;
            }));

    for (size_t i = 0; i < interfaces.size(); ++i)
    {
        if (!interfaces_found[i])
        {
            std::cout << "Interface '" << interfaces[i].name << "' not found" << std::endl;
        }
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(NetworkConfig,
        NetworkConfig,
        testing::Combine(testing::Values(TRANSPORT), testing::Values(false, true)),
        [](const testing::TestParamInfo<NetworkConfig::ParamType>& info)
        {
            bool udpv4 = std::get<1>(info.param);
            std::string suffix = udpv4 ? "UDPv4" : "UDPv6";
            switch (std::get<0>(info.param))
            {
                case TRANSPORT:
                default:
                    return "Transport" + suffix;
            }

        });
