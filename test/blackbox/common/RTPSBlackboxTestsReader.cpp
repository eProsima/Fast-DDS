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

namespace {
enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
};
}  // namespace

class RTPSReaderTests : public testing::TestWithParam<communication_type>
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
 * @test RTPS-READER-API-MWG-01
 *
 * matched_writers_guids() must return true with empty list when the entitiy is not matched.
 * matched_writers_guids() must return true with a correct list when the entitiy is matched.
 */
TEST_P(RTPSReaderTests, rtpsreader_matched_writers_guids)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastdds::rtps::RELIABLE)
            .init();

    writer.reliability(eprosima::fastdds::rtps::BEST_EFFORT)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Expect not to discover
    reader.wait_discovery(std::chrono::seconds(1));
    ASSERT_FALSE(reader.get_matched());

    std::vector<GUID_t> matched_guids;
    auto& native_rtps_reader = reader.get_native_reader();
    ASSERT_TRUE(native_rtps_reader.matched_writers_guids(matched_guids));
    ASSERT_TRUE(matched_guids.empty());

    writer.destroy();
    reader.wait_undiscovery();

    const size_t num_matched_writers = 3;
    std::vector<std::unique_ptr<RTPSWithRegistrationWriter<HelloWorldPubSubType>>> writers;
    std::vector<GUID_t> expected_matched_guids;

    writers.reserve(num_matched_writers);
    expected_matched_guids.reserve(num_matched_writers);

    for (size_t i = 0; i < num_matched_writers; ++i)
    {
        writers.emplace_back(new RTPSWithRegistrationWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME));
        writers.back()->init();
        expected_matched_guids.emplace_back(writers.back()->guid());
    }

    reader.wait_discovery(num_matched_writers, std::chrono::seconds::zero());
    ASSERT_EQ(num_matched_writers, reader.get_matched());
    native_rtps_reader.matched_writers_guids(matched_guids);
    ASSERT_EQ(expected_matched_guids.size(), matched_guids.size());
    ASSERT_TRUE(std::is_permutation(expected_matched_guids.begin(), expected_matched_guids.end(),
            matched_guids.begin()));
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(RTPSReaderTests,
        RTPSReaderTests,
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<RTPSReaderTests::ParamType>& info)
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

/**
 * Checks that a reliable RTPS reader whose history is filled with changes received out of order can still receive
 * the change those are waiting for.
 *
 * The first DATA message sent by the writer is dropped, so every following change is received out of order and
 * cannot be delivered until the first one is repaired. The reader history only has room for 10 changes, and the
 * writer sends 20.
 */
TEST(RTPSReaderTests, reliable_reader_history_full_of_future_changes)
{
    const uint32_t reader_history_depth = 10;
    const size_t samples_to_send = 20;

    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            history_depth(reader_history_depth).
            init();
    ASSERT_TRUE(reader.isInitialized());

    std::atomic_bool drop_next_data {false};
    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->drop_data_messages_filter_ = [&drop_next_data](eprosima::fastdds::rtps::CDRMessage_t&)
            -> bool
            {
                // drop_data_filter never receives builtin data
                return drop_next_data.exchange(false);
            };

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            history_depth(static_cast<int32_t>(samples_to_send)).
            disable_builtin_transport().
            add_user_transport_to_pparams(test_transport).
            init();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // The first change is lost, so every following change is received out of order.
    drop_next_data = true;

    auto data = default_helloworld_data_generator(samples_to_send);
    reader.expected_data(data);
    reader.startReception();
    writer.send(data);
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(10));
    EXPECT_EQ(reader.getReceivedCount(), samples_to_send);
}
