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

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/utils/IPFinder.h>

#include <gtest/gtest.h>

#include <memory>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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

//Verify that outLocatorList is used to select the desired output channel
TEST(BlackBox, PubSubOutLocatorSelection)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterOutLocators;
    Locator_t LocatorBuffer;

    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = 31337;

    WriterOutLocators.push_back(LocatorBuffer);


    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    resource_limits_allocated_samples(2).
    resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    std::shared_ptr<UDPv4TransportDescriptor> descriptor = std::make_shared<UDPv4TransportDescriptor>();
    descriptor->m_output_udp_socket = static_cast<uint16_t>(LocatorBuffer.port);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_kind(
        eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
    resource_limits_allocated_samples(20).
    disable_builtin_transport().
    add_user_transport_to_pparams(descriptor).
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

TEST(BlackBox, PubSubInterfaceWhitelistLocalhost)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    std::shared_ptr<UDPv4TransportDescriptor> descriptor = std::make_shared<UDPv4TransportDescriptor>();
    descriptor->interfaceWhiteList.push_back("127.0.0.1");

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
    disable_multicast(0).
    disable_builtin_transport().
    add_user_transport_to_pparams(descriptor).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
    disable_multicast(1).
    disable_builtin_transport().
    add_user_transport_to_pparams(descriptor).init();

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

TEST(BlackBox, PubSubInterfaceWhitelistUnicast)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    std::vector<IPFinder::info_IP> interfaces;
    GetIP4s(interfaces);

    std::shared_ptr<UDPv4TransportDescriptor> descriptor = std::make_shared<UDPv4TransportDescriptor>();
    for (const auto& interface : interfaces)
    {
        descriptor->interfaceWhiteList.push_back(interface.name);
    }

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
    disable_builtin_transport().
    add_user_transport_to_pparams(descriptor).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_depth(10).
    disable_builtin_transport().
    add_user_transport_to_pparams(descriptor).init();

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
