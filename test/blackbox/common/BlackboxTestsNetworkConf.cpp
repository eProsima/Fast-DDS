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
#ifdef __APPLE__
            // TODO: fix IPv6 issues related with zone ID
            GTEST_SKIP() << "UDPv6 tests are disabled in Mac";
#endif // ifdef __APPLE__
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
        std::vector<IPFinder::info_IP>& interfaces,
        bool return_loopback = false)
{
    IPFinder::getIPs(&interfaces, return_loopback);
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
        std::vector<IPFinder::info_IP>& interfaces,
        bool return_loopback = false)
{
    IPFinder::getIPs(&interfaces, return_loopback);
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

void interface_whitelist_test(
        const std::vector<IPFinder::info_IP>& pub_interfaces,
        const std::vector<IPFinder::info_IP>& sub_interfaces,
        bool interface_name = false)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);


    std::shared_ptr<UDPTransportDescriptor> pub_udp_descriptor;

    if (use_udpv4)
    {
        pub_udp_descriptor = std::make_shared<UDPv4TransportDescriptor>();
    }
    else
    {
        pub_udp_descriptor = std::make_shared<UDPv6TransportDescriptor>();
    }

    // include the interfaces in the transport descriptor
    for (const auto& interface : pub_interfaces)
    {
        if (!interface_name)
        {
            pub_udp_descriptor->interfaceWhiteList.push_back(interface.name);
        }
        else
        {
            pub_udp_descriptor->interfaceWhiteList.push_back(interface.dev);
        }
    }

    // Set the transport descriptor WITH interfaces in the writer
    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
            disable_builtin_transport().
            add_user_transport_to_pparams(pub_udp_descriptor).init();

    ASSERT_TRUE(writer.isInitialized());

    std::shared_ptr<UDPTransportDescriptor> sub_udp_descriptor;

    if (use_udpv4)
    {
        sub_udp_descriptor = std::make_shared<UDPv4TransportDescriptor>();
    }
    else
    {
        sub_udp_descriptor = std::make_shared<UDPv6TransportDescriptor>();
    }

    // include the interfaces in the transport descriptor
    for (const auto& interface : sub_interfaces)
    {
        if (!interface_name)
        {
            sub_udp_descriptor->interfaceWhiteList.push_back(interface.name);
        }
        else
        {
            sub_udp_descriptor->interfaceWhiteList.push_back(interface.dev);
        }
    }

    // Set the transport descriptor WITH interfaces in the reader
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
            disable_builtin_transport().
            add_user_transport_to_pparams(sub_udp_descriptor).init();

    ASSERT_TRUE(reader.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(5));
    reader.wait_discovery(std::chrono::seconds(5));

    // Check that endpoints have discovered each other
    ASSERT_EQ(reader.get_matched(), 1u);
    ASSERT_EQ(writer.get_matched(), 1u);

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();
    reader.startReception(data);
    writer.send(data);

    // Check that the sample data has been sent and received successfully
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
}

// Setting the interface whitelist with all available interfaces case, checking UDP case
TEST_P(NetworkConfig, PubSubInterfaceWhitelistUnicast)
{
    std::vector<IPFinder::info_IP> all_interfaces;
    use_udpv4 ? GetIP4s(all_interfaces) : GetIP6s(all_interfaces);

    {
        // IP addresses
        {
            // Whitelist only in publisher
            interface_whitelist_test(all_interfaces, all_interfaces);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(all_interfaces, all_interfaces);
        }
    }

    {
        // Interface names
        {
            // Whitelist only in publisher
            interface_whitelist_test(all_interfaces, all_interfaces, true);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(all_interfaces, all_interfaces, true);
        }
    }
}


// Regression test for redmine issue #18854 to check in UDP that setting the interface whitelist in one
// of the endpoints, but not in the other, connection is established anyways.
// All available interfaces case.
TEST_P(NetworkConfig, PubSubAsymmetricInterfaceWhitelistAllInterfaces)
{
    std::vector<IPFinder::info_IP> no_interfaces;

    std::vector<IPFinder::info_IP> all_interfaces;
    use_udpv4 ? GetIP4s(all_interfaces, true) : GetIP6s(all_interfaces, true);

    {
        // IP addresses
        {
            // Whitelist only in publisher
            interface_whitelist_test(all_interfaces, no_interfaces);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(no_interfaces, all_interfaces);
        }
    }

    {
        // Interface names
        {
            // Whitelist only in publisher
            interface_whitelist_test(all_interfaces, no_interfaces, true);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(no_interfaces, all_interfaces, true);
        }
    }
}

// Regression test for redmine issue #18854 to check in UDP that setting the interface whitelist in one
// of the endpoints, but not in the other, connection is established anyways.
// Only loopback interface case.
TEST_P(NetworkConfig, PubSubAsymmetricInterfaceWhitelistLocalhostOnly)
{
#ifdef _WIN32
    GTEST_SKIP() << "This test is skipped on Windows";
#else
    std::vector<IPFinder::info_IP> no_interfaces;

    std::vector<IPFinder::info_IP> locahost_interface_only;
    if (use_udpv4)
    {
        GetIP4s(locahost_interface_only, true);
        auto new_end = remove_if(locahost_interface_only.begin(),
                        locahost_interface_only.end(),
                        [](IPFinder::info_IP info_ip_)
                        {
                            return info_ip_.type != IPFinder::IP4_LOCAL;
                        });
        locahost_interface_only.erase(new_end, locahost_interface_only.end());
    }
    else
    {
        GetIP6s(locahost_interface_only, true);
        auto new_end = remove_if(locahost_interface_only.begin(),
                        locahost_interface_only.end(),
                        [](IPFinder::info_IP info_ip_)
                        {
                            return info_ip_.type != IPFinder::IP6_LOCAL;
                        });
        locahost_interface_only.erase(new_end, locahost_interface_only.end());
    }

    {
        // IP Address
        {
            // Whitelist only in publisher
            interface_whitelist_test(locahost_interface_only, no_interfaces);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(no_interfaces, locahost_interface_only);
        }
    }

    {
        // Interface name
        {
            // Whitelist only in publisher
            interface_whitelist_test(locahost_interface_only, no_interfaces, true);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(no_interfaces, locahost_interface_only, true);
        }
    }
#endif // _WIN32
}

// Regression test for redmine issue #18854 to check in UDP that setting the interface whitelist in one
// of the endpoints, but not in the other, connection is established anyways.
// All available interfaces except loopback case.
TEST_P(NetworkConfig, PubSubAsymmetricInterfaceWhitelistAllExceptLocalhost)
{
    std::vector<IPFinder::info_IP> no_interfaces;

    std::vector<IPFinder::info_IP> all_interfaces_except_localhost;
    use_udpv4 ? GetIP4s(all_interfaces_except_localhost, false) : GetIP6s(all_interfaces_except_localhost, false);

    {
        // IP address
        {
            // Whitelist only in publisher
            interface_whitelist_test(all_interfaces_except_localhost, no_interfaces);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(no_interfaces, all_interfaces_except_localhost);
        }
    }

    {
        // Interface name
        {
            // Whitelist only in publisher
            interface_whitelist_test(all_interfaces_except_localhost, no_interfaces, true);
        }

        {
            // Whitelist only in subscriber
            interface_whitelist_test(no_interfaces, all_interfaces_except_localhost, true);
        }
    }
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

// Regression test for redmine issue #19587
TEST_P(NetworkConfig, double_binding_fails)
{
    PubSubReader<HelloWorldPubSubType> p1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p2(TEST_TOPIC_NAME);

    // Create a participant without whitelist
    p1.disable_builtin_transport().add_user_transport_to_pparams(descriptor_);
    p1.init();
    ASSERT_TRUE(p1.isInitialized());

    // Add the announced addresses to the interface whitelist
    LocatorList_t locators_p1;
    p1.get_native_reader().get_listening_locators(locators_p1);
    for (const auto& loc : locators_p1)
    {
        descriptor_->interfaceWhiteList.push_back(IPLocator::ip_to_string(loc));
    }

    // Try to listen on the same locators as the first participant
    p2.disable_builtin_transport().add_user_transport_to_pparams(descriptor_);
    p2.set_default_unicast_locators(locators_p1);
    p2.init();
    ASSERT_TRUE(p2.isInitialized());

    // Should be listening on different locators
    LocatorList_t locators_p2;
    p2.get_native_reader().get_listening_locators(locators_p2);
    EXPECT_FALSE(locators_p1 == locators_p2);
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
