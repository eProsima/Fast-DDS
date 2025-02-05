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
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/utils/IPFinder.hpp>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubParticipant.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

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

// Regression test for redmine issue #22519 to check that readers using unique network flows cannot share locators
// with other readers. The mentioned issue referred to the case where TCP + builtin transports are present.
// In that concrete scenario, the problem was that while the TCP (and UDP) transports rightly were able
// to create a receiver in the dedicated "unique flow" port, shared memory failed for that same port as the other
// process (or participant) is already listening on it. However this was not being handled properly, so once matched,
// the publisher attempts to send data to the wrongfully announced shared memory locator.
// Note that the underlying problem is that, when creating unique network flows, all transports are requested to
// create a receiver for a specific port all together. This is, the creation of unique flow receivers is only
// considered to fail when it fails for all transports, instead of decoupling them and keep trying for alternative
// ports when the creation of a specific transport receiver fails.
// In this test a similar scenario is presented, but using instead UDP and shared memory transports. In the first
// participant, only shared memory is used (which should create a SHM receiver in the first "unique" port attempted).
// In the second participant both UDP and shared memory are used (which should create a UDP receiver in the first
// "unique" port attempted, and a shared memory receiver in the second "unique" port attempted, as the first one is
// already being used by the first participant). As a result, the listening shared memory locators of each data
// reader should be different. Finally, a third data reader is created in the second participant, and it is verified
// that its listening locators are different from those of the other reader created in the same participant, as well as
// from the (SHM) one of the reader created in the first participant.
TEST_P(NetworkConfig, sub_unique_network_flows_multiple_locators)
{
    // Enable unique network flows feature
    PropertyPolicy properties;
    properties.properties().emplace_back("fastdds.unique_network_flows", "");

    // First participant
    PubSubParticipant<HelloWorldPubSubType> participant(0, 1, 0, 0);

    participant.sub_topic_name(TEST_TOPIC_NAME).sub_property_policy(properties);

    std::shared_ptr<SharedMemTransportDescriptor> shm_descriptor = std::make_shared<SharedMemTransportDescriptor>();
    // Use only SHM transport in the first participant
    participant.disable_builtin_transport().add_user_transport_to_pparams(shm_descriptor);

    ASSERT_TRUE(participant.init_participant());
    ASSERT_TRUE(participant.init_subscriber(0));

    LocatorList_t locators;

    participant.get_native_reader(0).get_listening_locators(locators);
    ASSERT_EQ(locators.size(), 1u);
    ASSERT_EQ((*locators.begin()).kind, LOCATOR_KIND_SHM);

    // Second participant
    PubSubParticipant<HelloWorldPubSubType> participant2(0, 2, 0, 0);

    participant2.sub_topic_name(TEST_TOPIC_NAME).sub_property_policy(properties);

    // Use both UDP and SHM in the second participant
    if (!use_udpv4)
    {
        participant2.disable_builtin_transport().add_user_transport_to_pparams(descriptor_).
                add_user_transport_to_pparams(shm_descriptor);
    }

    ASSERT_TRUE(participant2.init_participant());
    ASSERT_TRUE(participant2.init_subscriber(0));

    LocatorList_t locators2_1;

    participant2.get_native_reader(0).get_listening_locators(locators2_1);
    ASSERT_TRUE(locators2_1.size() >= 2u); // There should be at least two locators, one for SHM and N(#interfaces) for UDP

    // Check SHM locator is different from the one in the first participant
    for (const Locator_t& loc : locators2_1)
    {
        if (LOCATOR_KIND_SHM == loc.kind)
        {
            // Ports should be different (expected second and first values of the unique network flows port range)
            ASSERT_FALSE(loc == *locators.begin());
        }
    }

    // Now create a second reader in the second participant
    ASSERT_TRUE(participant2.init_subscriber(1));

    LocatorList_t locators2_2;

    participant2.get_native_reader(1).get_listening_locators(locators2_2);
    ASSERT_TRUE(locators2_2.size() >= 2u); // There should be at least two locators, one for SHM and N(#interfaces) for UDP

    // Check SHM locator is different from the one in the first participant
    for (const Locator_t& loc : locators2_2)
    {
        if (LOCATOR_KIND_SHM == loc.kind)
        {
            // Ports should be different (expected third and first values of the unique network flows port range)
            ASSERT_FALSE(loc == *locators.begin());
        }
    }

    // Now check no locators are shared between the two readers in the second participant
    for (const Locator_t& loc_1 : locators2_1)
    {
        for (const Locator_t& loc_2 : locators2_2)
        {
            ASSERT_FALSE(loc_1 == loc_2);
        }
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

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS).
            resource_limits_allocated_samples(2).
            resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    descriptor_->m_output_udp_socket = static_cast<uint16_t>(locator.port);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).history_kind(
        eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS).
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

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).history_depth(10).
            disable_multicast(0).
            disable_builtin_transport().
            add_user_transport_to_pparams(descriptor_).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).history_depth(10).
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
    for (const auto& network_interface : pub_interfaces)
    {
        if (!interface_name)
        {
            pub_udp_descriptor->interfaceWhiteList.push_back(network_interface.name);
        }
        else
        {
            pub_udp_descriptor->interfaceWhiteList.push_back(network_interface.dev);
        }
    }

    // Set the transport descriptor WITH interfaces in the writer
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).history_depth(10).
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
    for (const auto& network_interface : sub_interfaces)
    {
        if (!interface_name)
        {
            sub_udp_descriptor->interfaceWhiteList.push_back(network_interface.name);
        }
        else
        {
            sub_udp_descriptor->interfaceWhiteList.push_back(network_interface.dev);
        }
    }

    // Set the transport descriptor WITH interfaces in the reader
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).history_depth(10).
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
    for (const auto& network_interface : interfaces)
    {
        std::cout << "Adding interface '" << network_interface.name << "' (" << network_interface.name.size() << ")" <<
            std::endl;
        descriptor_->interfaceWhiteList.push_back(network_interface.name);
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
