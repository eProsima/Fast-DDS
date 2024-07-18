// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

#include "../types/HelloWorldTypeObjectSupport.hpp"
#include "../types/TestRegression3361PubSubTypes.hpp"
#include "../types/TestRegression3361TypeObjectSupport.hpp"
#include "../utils/filter_helpers.hpp"
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

struct ContentFilterInfoCounter
{
    std::atomic_size_t user_data_count;
    std::atomic_size_t content_filter_info_count;
    std::atomic_uint32_t max_filter_signature_number;
    std::shared_ptr<rtps::test_UDPv4TransportDescriptor> transport;

    ContentFilterInfoCounter()
        : user_data_count(0)
        , content_filter_info_count(0)
        , max_filter_signature_number(0)
        , transport(std::make_shared<rtps::test_UDPv4TransportDescriptor>())
    {
        transport->interfaceWhiteList.push_back("127.0.0.1");
        transport->drop_data_messages_filter_ = [this](fastdds::rtps::CDRMessage_t& msg) -> bool
                {
                    // Check if it has inline_qos
                    uint8_t flags = msg.buffer[msg.pos - 3];
                    auto old_pos = msg.pos;

                    // Skip extraFlags, read octetsToInlineQos, and calculate inline qos position.
                    msg.pos += 2;
                    uint16_t to_inline_qos = eprosima::fastdds::helpers::cdr_parse_u16(
                        (char*)&msg.buffer[msg.pos]);
                    msg.pos += 2;
                    uint32_t inline_qos_pos = msg.pos + to_inline_qos;

                    // Read writerId, and skip if built-in.
                    msg.pos += 4;
                    fastdds::rtps::GUID_t writer_guid;
                    writer_guid.entityId = eprosima::fastdds::helpers::cdr_parse_entity_id(
                        (char*)&msg.buffer[msg.pos]);
                    msg.pos = old_pos;

                    if (writer_guid.is_builtin())
                    {
                        return false;
                    }

                    ++user_data_count;
                    if (0x02 == (flags & 0x02))
                    {
                        // Process inline qos
                        msg.pos = inline_qos_pos;
                        while (msg.pos < msg.length)
                        {
                            uint16_t pid = eprosima::fastdds::helpers::cdr_parse_u16(
                                (char*)&msg.buffer[msg.pos]);
                            msg.pos += 2;
                            uint16_t plen = eprosima::fastdds::helpers::cdr_parse_u16(
                                (char*)&msg.buffer[msg.pos]);
                            msg.pos += 2;
                            uint32_t next_pos = msg.pos + plen;

                            if (pid == PID_CONTENT_FILTER_INFO)
                            {
                                ++content_filter_info_count;

                                // Should have at least numBitmaps, one bitmap, numSignatures, and one signature
                                if (plen >= 4 + 4 + 4 + 16)
                                {
                                    // Read numBitmaps and skip bitmaps
                                    uint32_t num_bitmaps = eprosima::fastdds::helpers::cdr_parse_u32(
                                        (char*)&msg.buffer[msg.pos]);
                                    msg.pos += 4;
                                    msg.pos += 4 * num_bitmaps;

                                    // Read numSignatures and keep maximum
                                    uint32_t num_signatures = eprosima::fastdds::helpers::cdr_parse_u32(
                                        (char*)&msg.buffer[msg.pos]);
                                    msg.pos += 4;
                                    if (max_filter_signature_number < num_signatures)
                                    {
                                        max_filter_signature_number = num_signatures;
                                    }
                                }
                            }
                            else if (pid == PID_SENTINEL)
                            {
                                break;
                            }

                            msg.pos = next_pos;
                        }

                        msg.pos = old_pos;
                    }

                    // Never drop packet
                    return false;
                };
    }

};

enum class communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DDSContentFilter : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        using namespace eprosima::fastdds;

        enable_datasharing = false;

        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case communication_type::INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case communication_type::DATASHARING:
                enable_datasharing = true;
                break;
            case communication_type::TRANSPORT:
            default:
                break;
        }

        using_transport_communication_ = (communication_type::TRANSPORT == GetParam());
    }

    void TearDown() override
    {
        using namespace eprosima::fastdds;

        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case communication_type::INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case communication_type::DATASHARING:
                break;
            case communication_type::TRANSPORT:
            default:
                break;
        }

        enable_datasharing = false;
        using_transport_communication_ = false;
    }

protected:

    class TestState
    {
    public:

        TestState()
            : writer(TEST_TOPIC_NAME)
            , direct_reader(TEST_TOPIC_NAME)
        {
        }

        ~TestState()
        {
            if (participant_ && subscriber_)
            {
                EXPECT_EQ(RETCODE_OK, subscriber_->delete_contained_entities());
                EXPECT_EQ(RETCODE_OK, participant_->delete_subscriber(subscriber_));
            }

            if (participant_ && filtered_topic_)
            {
                EXPECT_EQ(RETCODE_OK, participant_->delete_contentfilteredtopic(filtered_topic_));
            }
        }

        PubSubWriter<HelloWorldPubSubType> writer;
        PubSubReader<HelloWorldPubSubType> direct_reader;

        void init(
                bool writer_side_filtering,
                const std::shared_ptr<rtps::TransportDescriptorInterface>& transport,
                fastdds::ResourceLimitedContainerConfig filter_limits)
        {
            writer_side_filter_ = writer_side_filtering && filter_limits.maximum > 0;

            writer.qos().writer_resource_limits().reader_filters_allocation = filter_limits;
            writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
            writer.history_depth(10).init();
            ASSERT_TRUE(writer.isInitialized());

            // Ensure the direct reader always receives DATA messages using the test transport
            fastdds::rtps::GuidPrefix_t custom_prefix;
            memset(custom_prefix.value, 0xee, custom_prefix.size);
            direct_reader.datasharing_off().guid_prefix(custom_prefix);
            direct_reader.disable_builtin_transport().add_user_transport_to_pparams(transport);
            direct_reader.reliability(ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS);
            direct_reader.durability_kind(DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS);
            direct_reader.history_depth(10).init();
            ASSERT_TRUE(direct_reader.isInitialized());

            direct_reader.wait_discovery();

            participant_ = writer.getParticipant();
            ASSERT_NE(nullptr, participant_);
            auto topic = static_cast<Topic*>(participant_->lookup_topicdescription(writer.topic_name()));
            ASSERT_NE(nullptr, topic);
            filtered_topic_ = participant_->create_contentfilteredtopic("filtered_topic", topic, "", {});
            ASSERT_NE(nullptr, filtered_topic_);
            subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
            ASSERT_NE(nullptr, subscriber_);
        }

        DataReader* create_filtered_reader()
        {
            DataReaderQos reader_qos = subscriber_->get_default_datareader_qos();
            reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
            reader_qos.durability().kind = DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS;
            reader_qos.history().depth = 10;
            if (enable_datasharing)
            {
                reader_qos.data_sharing().automatic();
            }
            else
            {
                reader_qos.data_sharing().off();
            }
            auto reader = subscriber_->create_datareader(filtered_topic_, reader_qos);

            EXPECT_NE(reader, nullptr);
            if (nullptr != reader)
            {
                SubscriptionMatchedStatus status;
                do
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    reader->get_subscription_matched_status(status);
                } while (status.current_count < 1);
            }

            return reader;
        }

        void delete_reader(
                DataReader* reader)
        {
            EXPECT_EQ(RETCODE_OK, subscriber_->delete_datareader(reader));
        }

        void set_filter_expression(
                const std::string& filter_expression,
                const std::vector<std::string>& expression_parameters)
        {
            EXPECT_EQ(RETCODE_OK,
                    filtered_topic_->set_filter_expression(filter_expression, expression_parameters));
            // Avoid discovery race condition
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        void set_expression_parameters(
                const std::vector<std::string>& expression_parameters)
        {
            EXPECT_EQ(RETCODE_OK, filtered_topic_->set_expression_parameters(expression_parameters));
            // Avoid discovery race condition
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        void send_data(
                DataReader* reader,
                ContentFilterInfoCounter& filter_counter,
                uint64_t expected_samples,
                const std::vector<uint16_t>& index_values,
                bool expect_wr_filters,
                uint32_t num_writer_filters)
        {
            filter_counter.user_data_count = 0;
            filter_counter.content_filter_info_count = 0;
            filter_counter.max_filter_signature_number = 0;

            // Ensure writer is in clean state
            drop_data_on_all_readers();
            EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(5)));
            EXPECT_EQ(reader->get_unread_count(), 0);

            // Send 10 samples with index 1 to 10
            auto data = default_helloworld_data_generator();
            writer.send(data);
            EXPECT_TRUE(data.empty());

            // On data-sharing, reader acknowledges samples on return_loan.
            if (enable_datasharing)
            {
                while (reader->get_unread_count() < expected_samples)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                }
            }
            else
            {
                // Waiting for all samples to be acknowledged ensures the reader has processed all samples sent
                EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(5)));
            }

            // Only the expected samples should have made its way into the history
            EXPECT_EQ(reader->get_unread_count(), expected_samples);

            // Take and check the received samples
            FASTDDS_CONST_SEQUENCE(HelloWorldSeq, HelloWorld);
            HelloWorldSeq recv_data;
            SampleInfoSeq recv_info;

            ReturnCode_t expected_ret;
            expected_ret = expected_samples == 0 ? RETCODE_NO_DATA : RETCODE_OK;
            EXPECT_EQ(expected_ret, reader->take(recv_data, recv_info));
            EXPECT_EQ(recv_data.length(), expected_samples);
            for (HelloWorldSeq::size_type i = 0;
                    i < recv_data.length() && static_cast<uint32_t>(i) < expected_samples;
                    ++i)
            {
                EXPECT_EQ(index_values[i], recv_data[i].index());
            }
            if (expected_samples > 0)
            {
                EXPECT_EQ(RETCODE_OK, reader->return_loan(recv_data, recv_info));
            }

            // Ensure writer ends in clean state
            drop_data_on_all_readers();
            EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(5)));

            EXPECT_GE(filter_counter.user_data_count, 10u);
            if (writer_side_filter_ && expect_wr_filters)
            {
                EXPECT_EQ(filter_counter.content_filter_info_count, filter_counter.user_data_count);
                EXPECT_EQ(filter_counter.max_filter_signature_number, num_writer_filters);
            }
            else
            {
                EXPECT_EQ(filter_counter.content_filter_info_count, 0u);
                EXPECT_EQ(filter_counter.max_filter_signature_number, 0u);
            }
        }

    private:

        DomainParticipant* participant_ = nullptr;
        Subscriber* subscriber_ = nullptr;
        ContentFilteredTopic* filtered_topic_ = nullptr;
        bool writer_side_filter_ = false;

        void drop_data_on_all_readers()
        {
            drop_data_on_reader(direct_reader.get_native_reader());

            std::vector<DataReader*> readers;
            subscriber_->get_datareaders(readers);
            for (DataReader* reader : readers)
            {
                drop_data_on_reader(*reader);
            }
        }

        void drop_data_on_reader(
                DataReader& reader)
        {
            FASTDDS_CONST_SEQUENCE(HelloWorldSeq, HelloWorld);
            HelloWorldSeq recv_data;
            SampleInfoSeq recv_info;

            while (RETCODE_OK == reader.take(recv_data, recv_info))
            {
                reader.return_loan(recv_data, recv_info);
            }
        }

    };

    bool using_transport_communication_ = false;

    ContentFilterInfoCounter filter_counter;

    DataReader* prepare_test(
            TestState& state,
            fastdds::ResourceLimitedContainerConfig filter_limits,
            uint32_t nb_of_additional_filter_readers)
    {
        state.init(using_transport_communication_, filter_counter.transport, filter_limits);

        for (uint32_t i = 0; i < nb_of_additional_filter_readers; ++i)
        {
            state.create_filtered_reader();
        }

        auto reader = state.create_filtered_reader();

        state.writer.wait_discovery(2 + nb_of_additional_filter_readers);

        return reader;
    }

    void test_run(
            DataReader* reader,
            TestState& state,
            uint32_t num_writer_filters)
    {
        std::cout << std::endl << "Test with empty expression..." << std::endl;
        state.send_data(reader, filter_counter, 10u, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, false, num_writer_filters);

        std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"2\", \"4\"}..." << std::endl;
        state.set_filter_expression("index BETWEEN %0 AND %1", { "2", "4" });
        state.send_data(reader, filter_counter, 3u, {2, 3, 4}, true, num_writer_filters);

        std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"6\", \"9\"}..." << std::endl;
        state.set_expression_parameters({ "6", "9" });
        state.send_data(reader, filter_counter, 4u, {6, 7, 8, 9}, true, num_writer_filters);

        std::cout << std::endl << "Test 'message match %0', {\"'HelloWorld 1.*'\"}..." << std::endl;
        state.set_filter_expression("message match %0", { "'HelloWorld 1.*'" });
        state.send_data(reader, filter_counter, 2u, {1, 10}, true, num_writer_filters);

        std::cout << std::endl << "Test 'message match %0', {\"'WRONG MESSAGE .*'\"}..." << std::endl;
        state.set_expression_parameters({ "'WRONG MESSAGE .*'" });
        state.send_data(reader, filter_counter, 0u, {}, true, num_writer_filters);

        std::cout << std::endl << "Go back to empty expression..." << std::endl;
        state.set_filter_expression("", {});
        state.send_data(reader, filter_counter, 10u, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, false, num_writer_filters);
    }

};

TEST_P(DDSContentFilter, BasicTest)
{
    TestState state;

    auto reader = prepare_test(state, {}, 0);
    ASSERT_NE(nullptr, reader);

    test_run(reader, state, 1);
}

TEST_P(DDSContentFilter, WriterFiltersDisabled)
{
    TestState state;

    auto reader = prepare_test(state, {0, 0, 0}, 0);
    ASSERT_NE(nullptr, reader);

    test_run(reader, state, 0);
}

TEST_P(DDSContentFilter, NoLimitsSeveralReaders)
{
    // TODO(Miguel C): Remove when multiple filtering readers case is fixed for data-sharing
    if (enable_datasharing)
    {
        GTEST_SKIP() << "Several filtering readers not correctly working on data sharing";
    }

    TestState state;

    auto reader = prepare_test(state, {}, 3u);
    ASSERT_NE(nullptr, reader);

    test_run(reader, state, 4u);
}

TEST_P(DDSContentFilter, WithLimitsSeveralReaders)
{
    // TODO(Miguel C): Remove when multiple filtering readers case is fixed for data-sharing
    if (enable_datasharing)
    {
        GTEST_SKIP() << "Several filtering readers not correctly working on data sharing";
    }

    TestState state;

    auto reader = prepare_test(state, fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2u), 3u);
    ASSERT_NE(nullptr, reader);

    test_run(reader, state, 2u);
}

TEST_P(DDSContentFilter, WithLimitsDynamicReaders)
{
    // TODO(Miguel C): Remove when multiple filtering readers case is fixed for data-sharing
    if (enable_datasharing)
    {
        GTEST_SKIP() << "Several filtering readers not correctly working on data sharing";
    }

    TestState state;

    // Only one filtered reader created
    auto reader = prepare_test(state, fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2u), 0u);
    ASSERT_NE(nullptr, reader);

    // We want a single filter to be applied, and check only for reader discovery changes
    state.set_filter_expression("index BETWEEN %0 AND %1", { "2", "4" });

    std::cout << "========= First reader =========" << std::endl;
    state.send_data(reader, filter_counter, 3u, { 2, 3, 4 }, true, 1u);

    // Adding a second reader should increase the number of writer filters
    std::cout << "========= Create a second reader =========" << std::endl;
    auto reader_2 = state.create_filtered_reader();
    ASSERT_NE(nullptr, reader_2);

    // Wait for the writer to discover the new reader, and give time for old samples to be delivered.
    state.writer.wait_discovery(3);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    state.send_data(reader_2, filter_counter, 3u, { 2, 3, 4 }, true, 2u);

    // Adding a third reader should not increase the number of writer filters (as the limit is 2)
    std::cout << "========= Create a third reader =========" << std::endl;
    auto reader_3 = state.create_filtered_reader();
    ASSERT_NE(nullptr, reader_3);

    // Wait for the writer to discover the new reader, and give time for old samples to be delivered.
    state.writer.wait_discovery(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    state.send_data(reader_3, filter_counter, 3u, { 2, 3, 4 }, true, 2u);

    // Deleting the second reader will decrease the number of writer filters
    std::cout << "========= Delete the second reader =========" << std::endl;
    state.delete_reader(reader_2);
    state.writer.wait_reader_undiscovery(3);
    state.send_data(reader_3, filter_counter, 3u, { 2, 3, 4 }, true, 1u);

    // Adding a fourth will increase the number of writer filters again
    std::cout << "========= Create a fourth reader =========" << std::endl;
    auto reader_4 = state.create_filtered_reader();
    ASSERT_NE(nullptr, reader_4);

    // Wait for the writer to discover the new reader, and give time for old samples to be delivered.
    state.writer.wait_discovery(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    state.send_data(reader_4, filter_counter, 3u, { 2, 3, 4 }, true, 2u);
}

//! Regression test for https://github.com/eProsima/Fast-DDS/issues/3361
//! Correctly resolve an alias defined in another header
TEST(DDSContentFilter, CorrectlyHandleAliasOtherHeader)
{
    auto dpf = DomainParticipantFactory::get_instance();

    auto participant = dpf->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type( new TestRegression3361PubSubType());

    auto ret = type.register_type(participant);

    if (ret != RETCODE_OK)
    {
        throw std::runtime_error("Failed to register type");
    }

    auto sub = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (sub == nullptr)
    {
        throw std::runtime_error("Failed to create subscriber");
    }

    auto topic = participant->create_topic("TestTopic", type->get_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr)
    {
        throw std::runtime_error("Failed to create topic");
    }

    std::string expression = "uuid <> %0";
    std::vector<std::string> parameters = {"'1235'"};

    auto filtered_topic = participant->create_contentfilteredtopic(
        "FilteredTestTopic", topic, expression, parameters);

    EXPECT_NE(nullptr, filtered_topic);
}

/*
 * Regression test for https://eprosima.easyredmine.com/issues/20815
 * Check that the content filter is only applied to alive changes.
 * The test creates a reliable writer and a reader with a content filter that only accepts messages with a specific
 * string. After discovery, the writer sends 10 samples which pass the filer in 10 different instances, with the
 * particularity that after each write, the instance is unregistered.
 * The DATA(u) generated would not pass the filter if it was applied. To check that the filter is only applied to
 * ALIVE changes (not unregister or disposed), the test checks that the reader receives 10 valid samples (one per
 * sample sent) and 10 invalid samples (one per unregister). Furthermore, it also checks that no samples are lost.writer
 */
TEST(DDSContentFilter, OnlyFilterAliveChanges)
{
    /* PuBSubReader class to check reception of UNREGISTER samples */
    class CustomPubSubReader : public PubSubReader<KeyedHelloWorldPubSubType>
    {
    public:

        CustomPubSubReader(
                const std::string& topic_name,
                const std::string& filter_expression,
                const std::vector<std::string>& expression_parameters)
            : PubSubReader(topic_name, filter_expression, expression_parameters)
        {
        }

        std::atomic<uint16_t> valid_samples{0};
        std::atomic<uint16_t> invalid_samples{0};

    private:

        void postprocess_sample(
                const type& /* sample */,
                const SampleInfo& info) override final
        {
            if (info.valid_data)
            {
                ++valid_samples;
            }
            else
            {
                ++invalid_samples;
            }
        }

    };

    /* Create reader with CFT */
    std::string expression = "index = 1";
    CustomPubSubReader reader("TestTopic", expression, {});
    reader.reliability(RELIABLE_RELIABILITY_QOS).history_depth(2).init();
    ASSERT_TRUE(reader.isInitialized());

    /* Create writer */
    PubSubWriter<KeyedHelloWorldPubSubType> writer("TestTopic");
    writer.reliability(RELIABLE_RELIABILITY_QOS).history_depth(2).init();
    ASSERT_TRUE(writer.isInitialized());

    /* Wait for discovery */
    writer.wait_discovery();
    reader.wait_discovery();

    /* Send 10 samples, each on a different instance, unregistering instances after writing */
    const size_t num_samples = 10;
    reader.startReception(num_samples);

    for (size_t i = 0; i < num_samples; ++i)
    {
        KeyedHelloWorld data;
        data.key(static_cast<uint16_t>(i));
        data.index(1u);  // All samples pass the filter
        InstanceHandle_t handle = writer.register_instance(data);
        ASSERT_NE(HANDLE_NIL, handle);
        ASSERT_EQ(RETCODE_OK, writer.send_sample(data, handle));
        ASSERT_EQ(true, writer.unregister_instance(data, handle));
    }

    // Wait until all samples are acknowledged
    writer.waitForAllAcked(std::chrono::seconds(3));

    /* Check that both samples and unregisters are received */
    ASSERT_EQ(reader.valid_samples.load(), 10u);
    ASSERT_EQ(reader.invalid_samples.load(), 10u);
    ASSERT_EQ(reader.get_sample_lost_status().total_count, 0);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSContentFilter,
        DDSContentFilter,
        testing::Values(
            communication_type::TRANSPORT,
            communication_type::INTRAPROCESS,
            communication_type::DATASHARING),
        [](const testing::TestParamInfo<DDSContentFilter::ParamType>& info)
        {
            switch (info.param)
            {
                case communication_type::INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case communication_type::DATASHARING:
                    return "Datasharing";
                    break;
                case communication_type::TRANSPORT:
                default:
                    return "Transport";
            }

        });

} // namespace dds
} // namespace fastdds
} // namespace eprosima
