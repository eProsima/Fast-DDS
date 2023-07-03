// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "../api/dds-pim/PubSubReader.hpp"
#include "../api/dds-pim/PubSubWriter.hpp"
#include <rtps/transport/test_UDPv4Transport.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using test_UDPv4Transport = eprosima::fastdds::rtps::test_UDPv4Transport;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class RTPS : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                enable_datasharing = false;
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                enable_datasharing = false;
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

TEST_P(RTPS, Transport_SHM_UDP_test)
{
    static struct test_conditions{
        uint32_t pub_unicast_port = 7411;
        uint32_t pub_metatraffic_unicast_port = 7526;
        uint32_t sub_unicast_port = 7527;
        uint32_t sub_metatraffic_unicast_port = 7528;
    } conditions;

    // Set up
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto sub_shm_descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
    sub_shm_descriptor->segment_size(2 * 1024 * 1024);
    std::shared_ptr<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor> sub_udp_descriptor =
            std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(sub_shm_descriptor)
            .add_user_transport_to_pparams(sub_udp_descriptor)
            .data_sharing(enable_datasharing)
            .reliability(BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(VOLATILE_DURABILITY_QOS)
            .history_kind(KEEP_ALL_HISTORY_QOS)
            .add_to_unicast_locator_list("localhost", conditions.sub_unicast_port)
            .add_to_metatraffic_unicast_locator_list("localhost", conditions.sub_metatraffic_unicast_port)
            .init();
    ASSERT_TRUE(reader.isInitialized());

    //reader.unicastLocatorList()
    // get locator list and obtain port

    auto pub_shm_descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
    pub_shm_descriptor->segment_size(2 * 1024 * 1024);

    auto pub_udp_descriptor = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(pub_shm_descriptor)
            .add_user_transport_to_pparams(pub_udp_descriptor)
            .data_sharing(enable_datasharing)
            .reliability(BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(VOLATILE_DURABILITY_QOS)
            .history_kind(KEEP_ALL_HISTORY_QOS)
            .asynchronously(SYNCHRONOUS_PUBLISH_MODE)
            .add_to_unicast_locator_list("localhost", conditions.pub_unicast_port)
            .add_to_metatraffic_unicast_locator_list("localhost", conditions.pub_metatraffic_unicast_port)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability, wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    // Send some data.
    auto data = default_helloworld_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Check that reader receives the unmatched.
    reader.block_for_all();

    // check that no data has been received in the udp transport
    uint32_t n_packages_sent = sizeof(uint32_t);
    for (std::map<uint32_t,uint32_t>::iterator it = test_UDPv4Transport::messages_sent.begin(); it != test_UDPv4Transport::messages_sent.end(); ++it)
    {
        std::cout << "port: " << it->first << ", n_packages: " << it->second << std::endl;
        if (it->first == conditions.pub_unicast_port)
        {
            n_packages_sent = it->second;
        }
    }
    ASSERT_EQ(n_packages_sent, 0u);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(RTPS,
        RTPS,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<RTPS::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }
        });
