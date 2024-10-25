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

#include "BlackboxTests.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/LibrarySettings.hpp>

#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
};

class RTPSWriterTests : public testing::TestWithParam<communication_type>
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

/**
 * @test RTPS-WRITER-API-MRG-01
 *
 * matched_readers_guids() must return true with empty list when the entitiy is not matched.
 * matched_readers_guids() must return true with a correct list when the entitiy is matched.
 */
TEST_P(RTPSWriterTests, rtpswriter_matched_readers_guids)
{
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::rtps::BEST_EFFORT)
            .init();

    reader.reliability(eprosima::fastdds::rtps::RELIABLE)
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    // Expect not to discover
    writer.wait_discovery(std::chrono::seconds(1));
    ASSERT_FALSE(writer.get_matched());

    std::vector<GUID_t> matched_guids;
    auto& native_rtps_writer = writer.get_native_writer();
    ASSERT_TRUE(native_rtps_writer.matched_readers_guids(matched_guids));
    ASSERT_TRUE(matched_guids.empty());

    reader.destroy();
    writer.wait_undiscovery();

    const size_t num_matched_readers = 3;
    std::vector<std::unique_ptr<RTPSWithRegistrationReader<HelloWorldPubSubType>>> readers;
    std::vector<GUID_t> expected_matched_guids;

    readers.reserve(num_matched_readers);
    expected_matched_guids.reserve(num_matched_readers);

    for (size_t i = 0; i < num_matched_readers; ++i)
    {
        readers.emplace_back(new RTPSWithRegistrationReader<HelloWorldPubSubType>(TEST_TOPIC_NAME));
        readers.back()->init();
        expected_matched_guids.emplace_back(readers.back()->guid());
    }

    writer.wait_discovery(num_matched_readers, std::chrono::seconds::zero());
    ASSERT_EQ(num_matched_readers, writer.get_matched());
    native_rtps_writer.matched_readers_guids(matched_guids);
    ASSERT_EQ(expected_matched_guids.size(), matched_guids.size());
    ASSERT_TRUE(std::is_permutation(expected_matched_guids.begin(), expected_matched_guids.end(),
            matched_guids.begin()));
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(RTPSWriterTests,
        RTPSWriterTests,
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<RTPSWriterTests::ParamType>& info)
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
