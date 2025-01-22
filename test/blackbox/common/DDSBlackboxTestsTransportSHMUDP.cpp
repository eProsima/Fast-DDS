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

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "mock/BlackboxMockConsumer.h"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class SHMUDP : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
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

void run_parametrized_test(
        bool reliable_writer,
        bool reliable_reader)
{
    // Set test parameters
    eprosima::fastdds::dds::ReliabilityQosPolicyKind writer_reliability =
            reliable_writer ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::
                    BEST_EFFORT_RELIABILITY_QOS;
    eprosima::fastdds::dds::ReliabilityQosPolicyKind reader_reliability =
            reliable_reader ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::
                    BEST_EFFORT_RELIABILITY_QOS;

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
            .reliability(reader_reliability)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .init();
    ASSERT_TRUE(reader.isInitialized());

    auto pub_shm_descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
    pub_shm_descriptor->segment_size(2 * 1024 * 1024);

    auto pub_udp_descriptor = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    std::atomic<uint32_t> messages_on_odd_port{ 0 };  // Messages corresponding to user data
    pub_udp_descriptor->locator_filter_ = [&messages_on_odd_port](
        const eprosima::fastdds::rtps::Locator& destination)
            {
                if (0 != (destination.port % 2))
                {
                    ++messages_on_odd_port;
                }
                return false;
            };

    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(pub_shm_descriptor)
            .add_user_transport_to_pparams(pub_udp_descriptor)
            .reliability(writer_reliability)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE)
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

    // check that no (user) data has been sent via UDP transport
    // TODO: check no data is sent for a specific port (set with add_to_default_unicast_locator_list or
    // add_to_unicast_locator_list). Currently this cannot be achieved, as adding a non-default UDP locator makes it
    // necessary to also add a non-default SHM one (if SHM communication is desired, as it is the case), but this cannot
    // be done until the creation of SHM locators is exposed (currently available in internal SHMLocator::create_locator).
    // As a workaround, it is checked that no user data is sent at any port, knowing that metatraffic ports are always
    // even and user ones odd.
    ASSERT_EQ(messages_on_odd_port, 0u);
}

TEST_P(SHMUDP, Transport_BestEffort_BestEffort_test)
{
    // Test BEST_EFFORT writer and reader
    run_parametrized_test(false, false);
}

TEST_P(SHMUDP, Transport_Reliable_BestEffort_test)
{
    // Test RELIABLE writer and BEST_EFFORT reader
    run_parametrized_test(true, false);
}

TEST_P(SHMUDP, Transport_Reliable_Reliable_test)
{
    // Test RELIABLE writer and reader
    run_parametrized_test(true, true);
}

static bool has_shm_locators(
        const ResourceLimitedVector<Locator_t>& locators)
{
    auto loc_is_shm = [](const Locator_t& loc)
            {
                return LOCATOR_KIND_SHM == loc.kind;
            };
    return std::any_of(locators.cbegin(), locators.cend(), loc_is_shm);
}

static void check_shm_locators(
        const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& info,
        bool unicast,
        bool multicast)
{
    EXPECT_EQ(multicast, has_shm_locators(info.metatraffic_locators.multicast));
    EXPECT_EQ(unicast, has_shm_locators(info.metatraffic_locators.unicast));
}

static void shm_metatraffic_test(
        const std::string& topic_name,
        const char* const value,
        bool unicast,
        bool multicast)
{
    PubSubWriter<HelloWorldPubSubType> writer(topic_name + "/" + value);
    PubSubReader<HelloWorldPubSubType> reader(topic_name + "/" + value);

    auto discovery_checker =
            [unicast, multicast](const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& info,
                    eprosima::fastdds::rtps::ParticipantDiscoveryStatus /*status*/)
            {
                check_shm_locators(info, unicast, multicast);
                return true;
            };
    reader.set_on_discovery_function(discovery_checker);
    reader.max_multicast_locators_number(2);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    PropertyPolicy properties;
    Property p;
    p.name("fastdds.shm.enforce_metatraffic");
    p.value(value);
    properties.properties().push_back(p);
    writer.property_policy(properties).avoid_builtin_multicast(false).max_multicast_locators_number(2);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    reader.wait_discovery();
    writer.wait_discovery();
}

TEST(SHMUDP, SHM_metatraffic_config)
{
    shm_metatraffic_test(TEST_TOPIC_NAME, "none", false, false);
    shm_metatraffic_test(TEST_TOPIC_NAME, "unicast", true, false);
    shm_metatraffic_test(TEST_TOPIC_NAME, "all", true, true);
}

TEST(SHMUDP, SHM_metatraffic_wrong_config)
{
    using eprosima::fastdds::dds::BlackboxMockConsumer;

    /* Set up log */
    BlackboxMockConsumer* helper_consumer = new BlackboxMockConsumer();
    eprosima::fastdds::dds::Log::ClearConsumers();  // Remove default consumers
    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(helper_consumer)); // Registering a consumer transfer ownership
    // Filter specific message
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Warning);
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("RTPS_NETWORK"));
    eprosima::fastdds::dds::Log::SetErrorStringFilter(std::regex(".*__WRONG_VALUE__.*"));

    // Perform test
    shm_metatraffic_test(TEST_TOPIC_NAME, "__WRONG_VALUE__", false, false);

    /* Check logs */
    eprosima::fastdds::dds::Log::Flush();
    EXPECT_EQ(helper_consumer->ConsumedEntries().size(), 1u);

    /* Clean-up */
    eprosima::fastdds::dds::Log::Reset();  // This calls to ClearConsumers, which deletes the registered consumer
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(SHMUDP,
        SHMUDP,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<SHMUDP::ParamType>& info)
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
