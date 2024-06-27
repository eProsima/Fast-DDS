// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <chrono>
#include <thread>

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"

using namespace eprosima::fastdds;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
};

class Volatile : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
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
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

// Test created to check a bug with writers that use BEST_EFFORT WITH VOLATILE that don't remove messages from history.
TEST_P(Volatile, AsyncPubSubAsNonReliableVolatileKeepAllHelloworld)
{
    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT).
            durability(eprosima::fastdds::rtps::DurabilityKind_t::VOLATILE).
            add_to_multicast_locator_list(ip, global_port).
            auto_remove_on_volatile().init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Wait for acks to be sent and check writer history is empty
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_TRUE(writer.is_history_empty());
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(Volatile,
        Volatile,
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<Volatile::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });
