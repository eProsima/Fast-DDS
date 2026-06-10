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

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include "../types/TestRegression3361PubSubTypes.hpp"
#include "../types/UnboundedHelloWorldPubSubTypes.hpp"
#include "../utils/filter_helpers.hpp"
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "UDPMessageSender.hpp"

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
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(
                    library_settings);
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
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(
                    library_settings);
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
        bool transient_local = false;

        void init(
                bool writer_side_filtering,
                const std::shared_ptr<rtps::TransportDescriptorInterface>& transport,
                fastdds::ResourceLimitedContainerConfig filter_limits,
                bool _transient_local)
        {
            filter_limits_ = filter_limits;
            transient_local = _transient_local;

            writer_side_filter_ = writer_side_filtering && filter_limits.maximum > 0;

            writer.qos().writer_resource_limits().reader_filters_allocation = filter_limits;
            if (transient_local)
            {
                writer.qos().properties().properties().emplace_back(
                    "fastdds.content_filtering_on_late_joiners",
                    "true");
            }
            writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
            writer.reliability(ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS);
            writer.durability_kind(transient_local ? DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS :
                    DurabilityQosPolicyKind_t::VOLATILE_DURABILITY_QOS);
            writer.history_depth(10).init();
            ASSERT_TRUE(writer.isInitialized());

            // Ensure the direct reader always receives DATA messages using the test transport
            fastdds::rtps::GuidPrefix_t custom_prefix;
            memset(custom_prefix.value, 0xee, custom_prefix.size);
            direct_reader.datasharing_off().guid_prefix(custom_prefix);
            direct_reader.disable_builtin_transport().add_user_transport_to_pparams(transport);
            direct_reader.reliability(ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS);
            direct_reader.durability_kind(transient_local ? DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS :
                    DurabilityQosPolicyKind_t::VOLATILE_DURABILITY_QOS);
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
            reader_qos.durability().kind =
                    transient_local ? DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS :
                    DurabilityQosPolicyKind_t::VOLATILE_DURABILITY_QOS;
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
                }
                while (status.current_count < 1);
            }

            return reader;
        }

        void delete_reader(
                DataReader* reader)
        {
            ASSERT_NE(subscriber_, nullptr);
            EXPECT_EQ(RETCODE_OK, subscriber_->delete_datareader(reader));
        }

        void delete_all_readers()
        {
            ASSERT_NE(subscriber_, nullptr);
            EXPECT_EQ(RETCODE_OK, subscriber_->delete_contained_entities());
        }

        void delete_all_filtered_readers()
        {
            ASSERT_NE(subscriber_, nullptr);
            std::vector<DataReader*> readers;
            subscriber_->get_datareaders(readers);
            for (DataReader* reader : readers)
            {
                if (reader != &direct_reader.get_native_reader())
                {
                    EXPECT_EQ(RETCODE_OK, subscriber_->delete_datareader(reader));
                }
            }
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

        void clean_state(
                ContentFilterInfoCounter& filter_counter)
        {
            filter_counter.user_data_count = 0;
            filter_counter.content_filter_info_count = 0;
            filter_counter.max_filter_signature_number = 0;

            // Ensure writer is in clean state
            drop_data_on_all_readers();
            EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(5)));
        }

        void send_data(
                size_t num_send_samples)
        {
            // Send num_send_samples samples with index 1 to num_send_samples
            auto data = default_helloworld_data_generator(num_send_samples);
            rtps::WriteParams write_params;
            rtps::SampleIdentity sample_id;
            sample_id.writer_guid(writer.datawriter_guid());
            write_params.related_sample_identity(sample_id);
            writer.send(data, 0, &write_params);
            EXPECT_TRUE(data.empty());
        }

        void check_received_data(
                DataReader* reader,
                ContentFilterInfoCounter& filter_counter,
                size_t num_sent_samples,
                const std::vector<uint16_t>& index_values,
                bool expect_wr_filters,
                size_t nb_of_filter_readers)
        {
            const size_t expected_samples = index_values.size();

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
            EXPECT_EQ(static_cast<uint64_t>(recv_data.length()), expected_samples);
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

            EXPECT_GE(filter_counter.user_data_count, num_sent_samples);
            if (writer_side_filter_ && expect_wr_filters)
            {
                size_t expected_wr_filters = nb_of_filter_readers;
                size_t nb_of_readers_out_of_limits = 0;
                if (filter_limits_.maximum < expected_wr_filters)
                {
                    nb_of_readers_out_of_limits = expected_wr_filters - filter_limits_.maximum;
                    expected_wr_filters = filter_limits_.maximum;
                }

                if (transient_local)
                {
                    // NOTE: messages sent to direct reader have no CFT because there are no matched readers with filter at send time
                    ASSERT_GE(filter_counter.user_data_count, num_sent_samples);
                    EXPECT_EQ(filter_counter.user_data_count - num_sent_samples,
                            filter_counter.content_filter_info_count);
                    if (expected_samples > 0 || nb_of_readers_out_of_limits > 0)
                    {
                        EXPECT_EQ(filter_counter.content_filter_info_count + num_sent_samples,
                                filter_counter.user_data_count);
                        EXPECT_EQ(filter_counter.content_filter_info_count,
                                expected_samples * expected_wr_filters + num_sent_samples *
                                nb_of_readers_out_of_limits);
                        EXPECT_EQ(filter_counter.max_filter_signature_number, expected_wr_filters);
                    }
                    else
                    {
                        // Since no messages passed the filter (and there are no readers with filter out of resource limits), no signature was sent
                        EXPECT_EQ(filter_counter.max_filter_signature_number, 0u);
                    }
                }
                else
                {
                    // NOTE: messages sent to direct reader also have CFT because there are matched readers with filter at send time
                    EXPECT_EQ(filter_counter.content_filter_info_count, filter_counter.user_data_count);
                    EXPECT_EQ(filter_counter.max_filter_signature_number, expected_wr_filters);
                }
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
        fastdds::ResourceLimitedContainerConfig filter_limits_;

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

    ContentFilterInfoCounter filter_counter_;

    void init_test(
            TestState& state,
            bool transient_local,
            fastdds::ResourceLimitedContainerConfig filter_limits = DATAWRITER_QOS_DEFAULT.writer_resource_limits().
                    reader_filters_allocation)
    {
        state.init(using_transport_communication_, filter_counter_.transport, filter_limits, transient_local);
    }

    DataReader* create_test_reader(
            TestState& state,
            size_t nb_of_filter_readers)
    {
        for (size_t i = 0; i < nb_of_filter_readers - 1; ++i)
        {
            state.create_filtered_reader();
            state.writer.wait_discovery(static_cast<unsigned int>(2 + i));
            state.writer.waitForAllAcked(std::chrono::seconds(3));
        }

        auto reader = state.create_filtered_reader();

        state.writer.wait_discovery(static_cast<unsigned int>(1 + nb_of_filter_readers));
        state.writer.waitForAllAcked(std::chrono::seconds(3));

        return reader;
    }

    DataReader* prepare_test(
            TestState& state,
            fastdds::ResourceLimitedContainerConfig filter_limits,
            size_t nb_of_additional_filter_readers,
            bool transient_local)
    {
        init_test(state, transient_local, filter_limits);

        return create_test_reader(state, 1 + nb_of_additional_filter_readers);
    }

    void send_and_check_data(
            TestState& state,
            DataReader* reader,
            size_t nb_of_filter_readers,
            ContentFilterInfoCounter& filter_counter,
            size_t num_send_samples,
            const std::vector<uint16_t>& index_values,
            bool expect_wr_filters)
    {
        ASSERT_EQ(state.transient_local, reader == nullptr);

        // Clean state before sending data
        state.clean_state(filter_counter);

        if (!state.transient_local)
        {
            // Ensure reader is in clean state
            EXPECT_EQ(reader->get_unread_count(), 0ull);
        }

        // Send data
        state.send_data(num_send_samples);

        if (state.transient_local)
        {
            // For transient local durability, readers are created after sending data
            reader = create_test_reader(state, nb_of_filter_readers);
        }

        // Check received data and transport counters
        state.check_received_data(reader, filter_counter, num_send_samples, index_values, expect_wr_filters,
                nb_of_filter_readers);

        if (state.transient_local)
        {
            // Destroy readers in the same scope they were created for transient local durability case
            state.delete_all_filtered_readers();
        }
    }

    void send_and_check_data(
            TestState& state,
            DataReader* reader,
            ContentFilterInfoCounter& filter_counter,
            size_t num_send_samples,
            const std::vector<uint16_t>& index_values,
            bool expect_wr_filters)
    {
        send_and_check_data(state, reader, 1, filter_counter, num_send_samples, index_values, expect_wr_filters);
    }

    void test_run(
            TestState& state,
            size_t nb_of_additional_filter_readers,
            bool transient_local,
            fastdds::ResourceLimitedContainerConfig filter_limits = DATAWRITER_QOS_DEFAULT.writer_resource_limits().
                    reader_filters_allocation)
    {
        // Test initialization
        init_test(state, transient_local, filter_limits);

        if (!state.transient_local)
        {
            std::cout << std::endl << "\n[EARLY-JOINER TEST CASE]\n" << std::endl;
        }
        else
        {
            std::cout << std::endl << "\n[LATE-JOINER TEST CASE]\n" << std::endl;
        }

        // Total number of readers with filter
        const size_t nb_of_filter_readers = 1 + nb_of_additional_filter_readers;

        DataReader* reader = nullptr;
        if (!state.transient_local)
        {
            // Create readers before sending data for volatile durability test case
            reader = create_test_reader(state, nb_of_filter_readers);
        }

        std::cout << std::endl << "Test with empty expression..." << std::endl;
        send_and_check_data(state, reader, nb_of_filter_readers, filter_counter_, 10u, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                false);

        std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"2\", \"4\"}..." << std::endl;
        state.set_filter_expression("index BETWEEN %0 AND %1", { "2", "4" });
        send_and_check_data(state, reader, nb_of_filter_readers, filter_counter_, 10u, {2, 3, 4}, true);

        std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"6\", \"9\"}..." << std::endl;
        state.set_expression_parameters({ "6", "9" });
        send_and_check_data(state, reader, nb_of_filter_readers, filter_counter_, 10u, {6, 7, 8, 9}, true);

        std::cout << std::endl << "Test 'message match %0', {\"'HelloWorld 1.*'\"}..." << std::endl;
        state.set_filter_expression("message match %0", { "'HelloWorld 1.*'" });
        send_and_check_data(state, reader, nb_of_filter_readers, filter_counter_, 10u, {1, 10}, true);

        std::cout << std::endl << "Test 'message match %0', {\"'WRONG MESSAGE .*'\"}..." << std::endl;
        state.set_expression_parameters({ "'WRONG MESSAGE .*'" });
        send_and_check_data(state, reader, nb_of_filter_readers, filter_counter_, 10u, {}, true);

        std::cout << std::endl << "Go back to empty expression..." << std::endl;
        state.set_filter_expression("", {});
        send_and_check_data(state, reader, nb_of_filter_readers, filter_counter_, 10u, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                false);

        if (!state.transient_local)
        {
            // Destroy readers in the same scope they were created for volatile durability case
            state.delete_all_filtered_readers();
        }
    }

};

TEST_P(DDSContentFilter, BasicTestDefaultLimits)
{
    // EARLY JOINER TEST CASE
    {
        TestState state;

        test_run(state, 0u, false);
    }

    // LATE JOINER TEST CASE
    {
        TestState state;

        test_run(state, 0u, true);
    }
}

TEST_P(DDSContentFilter, BasicTestNoLimits)
{
    // EARLY JOINER TEST CASE
    {
        TestState state;

        test_run(state, 0u, false, {});
    }

    // LATE JOINER TEST CASE
    // Not supported for no limits configuration, as that would imply reserving unlimited resources
}

TEST_P(DDSContentFilter, WriterFiltersDisabled)
{
    // EARLY JOINER TEST CASE
    {
        TestState state;

        test_run(state, 0u, false, {0, 0, 0});
    }

    // LATE JOINER TEST CASE
    {
        TestState state;

        test_run(state, 0u, true, {0, 0, 0});
    }
}

TEST_P(DDSContentFilter, SeveralReadersDefaultLimits)
{
    // TODO(Miguel C): Remove when multiple filtering readers case is fixed for data-sharing
    if (enable_datasharing)
    {
        GTEST_SKIP() << "Several filtering readers not correctly working on data sharing";
    }

    // EARLY JOINER TEST CASE
    {
        TestState state;

        test_run(state, 3u, false);
    }

    // LATE JOINER TEST CASE
    {
        TestState state;

        test_run(state, 3u, true);
    }
}

TEST_P(DDSContentFilter, SeveralReadersNoLimits)
{
    // TODO(Miguel C): Remove when multiple filtering readers case is fixed for data-sharing
    if (enable_datasharing)
    {
        GTEST_SKIP() << "Several filtering readers not correctly working on data sharing";
    }

    // EARLY JOINER TEST CASE
    {
        TestState state;

        test_run(state, 3u, false, {});
    }

    // LATE JOINER TEST CASE
    // Not supported for no limits configuration, as that would imply reserving unlimited resources
}

TEST_P(DDSContentFilter, SeveralReadersWithLimits)
{
    // TODO(Miguel C): Remove when multiple filtering readers case is fixed for data-sharing
    if (enable_datasharing)
    {
        GTEST_SKIP() << "Several filtering readers not correctly working on data sharing";
    }

    // EARLY JOINER TEST CASE
    {
        TestState state;

        test_run(state, 3u, false, fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2u));
    }

    // LATE JOINER TEST CASE
    {
        TestState state;

        test_run(state, 3u, true, fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2u));
    }
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
    auto reader = prepare_test(state, fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2u), 0u, false);
    ASSERT_NE(nullptr, reader);

    // We want a single filter to be applied, and check only for reader discovery changes
    state.set_filter_expression("index BETWEEN %0 AND %1", { "2", "4" });

    std::cout << "========= First reader =========" << std::endl;
    send_and_check_data(state, reader, 1u, filter_counter_, 10, { 2, 3, 4 }, true);

    // Adding a second reader should increase the number of writer filters
    std::cout << "========= Create a second reader =========" << std::endl;
    auto reader_2 = state.create_filtered_reader();
    ASSERT_NE(nullptr, reader_2);

    // Wait for the writer to discover the new reader, and give time for old samples to be delivered.
    state.writer.wait_discovery(3);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    send_and_check_data(state, reader_2, 2u, filter_counter_, 10, { 2, 3, 4 }, true);

    // Adding a third reader should not increase the number of writer filters (as the limit is 2)
    std::cout << "========= Create a third reader =========" << std::endl;
    auto reader_3 = state.create_filtered_reader();
    ASSERT_NE(nullptr, reader_3);

    // Wait for the writer to discover the new reader, and give time for old samples to be delivered.
    state.writer.wait_discovery(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    send_and_check_data(state, reader_3, 2u, filter_counter_, 10, { 2, 3, 4 }, true);

    // Deleting the second reader will decrease the number of writer filters
    std::cout << "========= Delete the second reader =========" << std::endl;
    state.delete_reader(reader_2);
    state.writer.wait_reader_undiscovery(3);
    send_and_check_data(state, reader_3, 1u, filter_counter_, 10, { 2, 3, 4 }, true);

    // Adding a fourth will increase the number of writer filters again
    std::cout << "========= Create a fourth reader =========" << std::endl;
    auto reader_4 = state.create_filtered_reader();
    ASSERT_NE(nullptr, reader_4);

    // Wait for the writer to discover the new reader, and give time for old samples to be delivered.
    state.writer.wait_discovery(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    send_and_check_data(state, reader_4, 2u, filter_counter_, 10, { 2, 3, 4 }, true);
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

/**
 * @test DataWriter Sample prefilter feature
 *
 * This test asserts that prefiltering with an active content filter works correctly.
 * It creates a ContentFilteredTopic an expression that only accepts samples with index <= 6.
 * On its side, the prefilter is set to only accept samples with 4 < index < 8
 */
TEST_P(DDSContentFilter, filter_with_prefilter)
{
    // TODO(Mario-DL): Remove when multiple filtering readers case is fixed for data-sharing
    if (enable_datasharing)
    {
        GTEST_SKIP() << "Several filtering readers not correctly working on data sharing";
    }

    struct CustomUserWriteData : public rtps::WriteParams::UserWriteData
    {
        CustomUserWriteData(
                const uint16_t& upper_bound_idx,
                const uint16_t& lower_bound_idx )
            : upper_bound_idx_(upper_bound_idx)
            , lower_bound_idx_(lower_bound_idx)
        {
        }

        uint16_t upper_bound_idx_;
        uint16_t lower_bound_idx_;
    };

    struct CustomPreFilter : public eprosima::fastdds::dds::IContentFilter
    {
        ~CustomPreFilter() override = default;

        //! Custom filter for the HelloWorld example
        bool evaluate(
                const SerializedPayload& payload,
                const FilterSampleInfo& filter_sample_info,
                const rtps::GUID_t&) const override
        {
            HelloWorldPubSubType hello_world_type_support;
            HelloWorld hello_world_sample;
            hello_world_type_support.deserialize(*const_cast<SerializedPayload*>(&payload), &hello_world_sample);

            bool sample_should_be_sent = true;

            auto custom_write_data =
                    std::static_pointer_cast<CustomUserWriteData>(filter_sample_info.user_write_data);

            // Filter out samples
            if (hello_world_sample.index() > custom_write_data->upper_bound_idx_ ||
                    hello_world_sample.index() < custom_write_data->lower_bound_idx_)
            {
                sample_should_be_sent = false;
            }
            return sample_should_be_sent;
        }

    };

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME, "index <= %0", {"6"}, true, false, false);

    // Initialize writer and the filtered reader
    TestState state;
    writer.init();
    ASSERT_TRUE(writer.isInitialized());
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // wait for discovery between writer and filtered reader
    writer.wait_discovery();
    reader.wait_discovery();

    // Set a prefilter on the filtered reader
    ASSERT_EQ(writer.set_sample_prefilter(
                std::make_shared<CustomPreFilter>()),
            eprosima::fastdds::dds::RETCODE_OK);

    // Set a user write data on the writer to filter out samples 4 < index < 8
    rtps::WriteParams write_params;
    write_params.user_write_data(std::make_shared<CustomUserWriteData>(
                (uint16_t)8u, (uint16_t)4u));

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data, 50, &write_params);

    // Reader should have received 3 samples
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(1)), 3u);
}

/*!
 * @test Regression test for https://eprosima.easyredmine.com/issues/23919
 * This test checks GAP messages are sent correctly when there is one reader with a content filter.
 * The idea is, in the middle of a GAP sequence, a heartbet period message is sent.
 */
TEST_P(DDSContentFilter, CorrectGAPSendingOneReader)
{
    int32_t total_count {0};
    // Set up the reader with a content filter for index 1, 2, and 6
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME, "index = 1 OR index = 2 OR index = 6", {}, true, false,
            false);
    reader
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sample_lost_status_functor([&total_count](const SampleLostStatus& status)
            {
                total_count = status.total_count;
            }).init();
    ASSERT_TRUE(reader.isInitialized());

    // Set up the writer
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer
            .heartbeat_period_seconds(0)
            .heartbeat_period_nanosec(100000000)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery
    reader.wait_discovery();
    writer.wait_discovery();

    // Send 10 samples
    auto data = default_helloworld_data_generator();

    decltype(data) expected_data;
    expected_data.push_back(*data.begin()); // index 1
    expected_data.push_back(*std::next(data.begin())); // index 2
    expected_data.push_back(*std::next(data.begin(), 5)); // index 6

    reader.startReception(expected_data);

    writer.send(data, 50);

    // Wait for reception and check
    reader.block_for_all();
    ASSERT_EQ(0, total_count);
}

/*!
 * @test Regression test for https://eprosima.easyredmine.com/issues/23919
 * This test checks GAP messages are sent correctly when there is two readers with a content filter.
 */
TEST_P(DDSContentFilter, CorrectGAPSendingTwoReader)
{
    int32_t total_count {0};
    int32_t total_count_2 {0};
    // Set up the reader with a content filter for index 1, 2, and 6
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME, "index = 1 OR index = 2 OR index = 6", {}, true, false,
            false);
    reader
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sample_lost_status_functor([&total_count](const SampleLostStatus& status)
            {
                total_count = status.total_count;
            }).init();
    ASSERT_TRUE(reader.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader_2(TEST_TOPIC_NAME, "index = 3 OR index = 10", {}, true, false,
            false);
    reader_2
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sample_lost_status_functor([&total_count_2](const SampleLostStatus& status)
            {
                total_count_2 = status.total_count;
            }).init();
    ASSERT_TRUE(reader_2.isInitialized());

    // Set up the writer
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer
            .heartbeat_period_seconds(0)
            .heartbeat_period_nanosec(100000000)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery
    reader.wait_discovery();
    reader_2.wait_discovery();
    writer.wait_discovery(2);

    // Send 10 samples
    auto data = default_helloworld_data_generator();

    decltype(data) expected_data;
    expected_data.push_back(*data.begin()); // index 1
    expected_data.push_back(*std::next(data.begin())); // index 2
    expected_data.push_back(*std::next(data.begin(), 5)); // index 6

    decltype(data) expected_data_2;
    expected_data_2.push_back(*std::next(data.begin(), 2)); // index 3
    expected_data_2.push_back(*std::next(data.begin(), 9)); // index 9

    reader.startReception(expected_data);
    reader_2.startReception(expected_data_2);

    writer.send(data, 50);

    // Wait for reception and check
    reader.block_for_all();
    reader_2.block_for_all();
    ASSERT_EQ(0, total_count);
    ASSERT_EQ(0, total_count_2);
}

/*
 * Regression test for https://eprosima.easyredmine.com/issues/23265
 *
 * This test checks that a DDSSQL content filter can be created with a type name that is different from the one
 * in the generated type support.
 */
TEST(DDSContentFilter, filter_other_type_name)
{
    using namespace eprosima::fastdds;

    // Create a DomainParticipant
    DomainParticipant* participant =
            dds::DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Create a ContentFilteredTopic with a different type name
    dds::TypeSupport type_support(new HelloWorldPubSubType());
    ASSERT_EQ(type_support.register_type(participant, "CustomType"), RETCODE_OK);
    dds::Topic* topic = participant->create_topic(
        "TestTopic", "CustomType", TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    dds::ContentFilteredTopic* filtered_topic = participant->create_contentfilteredtopic(
        "FilteredTopic", topic, "index <= %0", { "6" });
    ASSERT_NE(filtered_topic, nullptr);

    // Delete all entities
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
    ASSERT_EQ(dds::DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

/*
 * Regression test for https://eprosima.easyredmine.com/issues/24038
 *
 * This test checks that reusing a ReaderProxy object when a new DataReader is matched does not lead to incorrect
 * behaviour due to poorly initialised data.
 */
TEST(DDSContentFilter, reusing_reader_proxy)
{
    int32_t total_count {0};
    int32_t total_count_2 {0};

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME, "index = 1 OR index = 2 OR index = 6", {}, true, false,
            false);
    reader
            .reliability(RELIABLE_RELIABILITY_QOS)
            .durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS)
            .sample_lost_status_functor([&total_count](const SampleLostStatus& status)
            {
                total_count = status.total_count;
            }).init();
    ASSERT_TRUE(reader.isInitialized());

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    // Set up the writer
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer
            .add_user_transport_to_pparams(udp_transport)
            .disable_builtin_transport()
            .durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_depth(10)
    //.heartbeat_period_seconds(100)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery
    reader.wait_discovery();
    writer.wait_discovery();

    // Send 10 samples
    auto data = default_helloworld_data_generator();

    decltype(data) expected_data;
    expected_data.push_back(*data.begin()); // index 1
    expected_data.push_back(*std::next(data.begin())); // index 2
    expected_data.push_back(*std::next(data.begin(), 5)); // index 6
    decltype(data) expected_data_2;
    expected_data_2.push_back(*std::next(data.begin(), 8)); // index 9
    expected_data_2.push_back(*std::next(data.begin(), 2)); // index 3
    expected_data_2.push_back(*std::next(data.begin(), 3)); // index 4

    reader.startReception(expected_data);

    writer.send(data, 50);

    // Wait for reception and check
    reader.block_for_all();
    ASSERT_EQ(0, total_count);

    reader.destroy();

    data = default_helloworld_data_generator(4);

    writer.send(data, 50);

    PubSubReader<HelloWorldPubSubType> reader_2(TEST_TOPIC_NAME, "index = 3 OR index = 4 OR index = 9", {}, true, false,
            false);
    reader_2
            .reliability(RELIABLE_RELIABILITY_QOS)
            .durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS)
            .sample_lost_status_functor([&total_count_2](const SampleLostStatus& status)
            {
                total_count_2 += status.total_count;
            }).init();
    ASSERT_TRUE(reader_2.isInitialized());


    reader_2.wait_discovery();
    writer.wait_discovery();

    data = default_helloworld_data_generator(1);

    writer.send(data, 50);

    reader_2.startReception(expected_data_2);

    // Wait for reception and check
    reader_2.block_for_all();
}

/*
 * Regression test for CVE-2026-22591
 */
TEST(DDSContentFilter, ShouldNotFailWithTooManySubExpressionsDiscovered)
{
    using namespace eprosima::fastdds::rtps;

    const char* const topic_name = "DDSContentFilter_ShouldNotFailWithTooManySubExpressionsDiscovered";

    // Since we want to send a custom crafted EDP message, we will specify the metatraffic port so we can send the
    // crafted message to a known port.
    PubSubWriter<HelloWorldPubSubType> writer(topic_name);
    writer.setManualTopicName(topic_name);
    writer.add_to_metatraffic_unicast_locator_list("127.0.0.1", 6999);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader(topic_name);
    LocatorList initial_peers;
    Locator loc;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "127.0.0.1", 6999, loc);
    initial_peers.push_back(loc);
    reader.setManualTopicName(topic_name);
    reader.initial_peers(initial_peers);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery();

    // Craft a filter expression with too many sub-expressions (nested parentheses)
    constexpr size_t depth = 20000;
    constexpr size_t expression_length = depth + depth + 9;
    constexpr size_t expression_aligned_length = (expression_length + 1 /* null */ + 3) & ~3;
    std::string filter_expression = std::string(depth, '(') + "index = 1" + std::string(depth, ')');
    ASSERT_EQ(filter_expression.length(), expression_length);

    // Structure for the malicious DATA(r) message payload
    struct PayloadData
    {
        struct Encapsulation
        {
#if FASTDDS_IS_BIG_ENDIAN_TARGET
            std::array<uint8_t, 2> encapsulation{ 0x00, 0x02 };  // PL_CDR_BE
#else
            std::array<uint8_t, 2> encapsulation{ 0x00, 0x03 };  // PL_CDR_LE
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
            std::array<uint8_t, 2> encapsulation_opts{ 0x00, 0x00 };
        }
        encapsulation;

        struct ReaderGUIDParameter
        {
            uint16_t parameter_id{ 0x5a };
            uint16_t parameter_length{ 0x10 };
            GUID_t value;
        }
        entity_guid;

        struct TopicNameParameter
        {
            uint16_t parameter_id{ 0x05 };
            uint16_t parameter_length{ 68 + 4 };
            uint32_t string_length{ 66 }; // "DDSContentFilter_ShouldNotFailWithTooManySubExpressionsDiscovered" + null
            std::array<char, 68> value { "DDSContentFilter_ShouldNotFailWithTooManySubExpressionsDiscovered\0" };
        }
        topic_name;

        struct TypeNameParameter
        {
            uint16_t parameter_id{ 0x07 };
            uint16_t parameter_length{ 12 + 4 };
            uint32_t string_length{ 11 }; // "HelloWorld" + null
            std::array<char, 12> value { "HelloWorld\0" };
        }
        type_name;

        struct ContentFilterParameter
        {
            uint16_t parameter_id{ 0x35 };
            uint16_t parameter_length{ 24 + 4 + 68 + 4 + 8 + 4 + expression_aligned_length + 4 + 4 };
            struct ContentFilteredTopicName
            {
                uint32_t string_length{ 23 }; // "MaliciousFilteredTopic" + null
                std::array<char, 24> value {"MaliciousFilteredTopic\0"};
            }
            content_filtered_topic_name;
            struct RelatedTopicName
            {
                uint32_t string_length{ 66 }; // "DDSContentFilter_ShouldNotFailWithTooManySubExpressionsDiscovered" + null
                std::array<char, 68> value{ "DDSContentFilter_ShouldNotFailWithTooManySubExpressionsDiscovered\0" };
            }
            related_topic_name;
            struct FilterClassName
            {
                uint32_t string_length{ 7 }; // "DDSSQL" + null
                std::array<char, 8> value{ "DDSSQL\0" };
            }
            filter_class_name;
            struct FilterExpression
            {
                // Length will be set later
                uint32_t string_length{ expression_length + 1 };
                std::array<char, expression_aligned_length> value; // Padding to 4 bytes
            }
            filter_expression;
            // Sequence of expression parameters (empty)
            uint32_t expression_parameter_count{ 0 };
        }
        content_filter;

        struct SentinelParameter
        {
            uint16_t parameter_id{ 0x0001 };
            uint16_t parameter_length{ 0x0000 };
        }
        sentinel;
    };

    static_assert(
        sizeof(PayloadData) ==
        sizeof(PayloadData::Encapsulation) +
        sizeof(PayloadData::ReaderGUIDParameter) +
        sizeof(PayloadData::TopicNameParameter) +
        sizeof(PayloadData::TypeNameParameter) +
        sizeof(PayloadData::ContentFilterParameter) +
        sizeof(PayloadData::SentinelParameter),
        "Unexpected size for PayloadData");

    struct MaliciousDiscoveryPacket
    {
        struct RTPSHeader
        {
            std::array<char, 4> rtps_id{ {'R', 'T', 'P', 'S'} };
            std::array<uint8_t, 2> protocol_version{ {2, 3} };
            std::array<uint8_t, 2> vendor_id{ {0x01, 0x0F} };
            GuidPrefix_t sender_prefix{};
        }
        header;

        static_assert(sizeof(RTPSHeader) == RTPSMESSAGE_HEADER_SIZE, "Unexpected size for RTPS header");

        struct DataSubMsg
        {
            struct Header
            {
                uint8_t submessage_id = 0x15;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
                uint8_t flags = 0x04; // Serialized data
#else
                uint8_t flags = 0x05; // Serialized data, endianness
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
                uint16_t octets_to_next_header = 0;  // Last submessage, fills the rest of the packet
                uint16_t extra_flags = 0;
                uint16_t octets_to_inline_qos = 0x10;
                EntityId_t reader_id = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
                EntityId_t writer_id = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
                SequenceNumber_t sn{ 2 };
            };

            static_assert(sizeof(Header) == RTPSMESSAGE_DATA_MIN_LENGTH, "Unexpected size for DATA header");

            Header header;
            PayloadData payload;
        }
        data;

        static_assert(
            sizeof(DataSubMsg) ==
            sizeof(DataSubMsg::Header) + sizeof(PayloadData),
            "Unexpected size for DataSubMsg");
    };

    static_assert(
        sizeof(MaliciousDiscoveryPacket) ==
        sizeof(MaliciousDiscoveryPacket::RTPSHeader) + sizeof(MaliciousDiscoveryPacket::DataSubMsg),
        "Unexpected size for MaliciousDiscoveryPacket");

    UDPMessageSender fake_msg_sender;

    // Fill and send the malicious packet
    {
        GUID_t reader_guid = reader.datareader_guid();

        MaliciousDiscoveryPacket malicious_packet;
        malicious_packet.header.sender_prefix = reader_guid.guidPrefix;
        malicious_packet.data.payload.entity_guid.value = reader_guid;
        std::memcpy(
            malicious_packet.data.payload.content_filter.filter_expression.value.data(),
            filter_expression.data(),
            filter_expression.length());

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(malicious_packet));
        msg.init(reinterpret_cast<octet*>(&malicious_packet), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, loc);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

/*
 * Regression test for https://eprosima.easyredmine.com/issues/23681
 *
 * This test checks that content filtering and fragmentation work correctly together.
 */
TEST(DDSContentFilter, WithFragmentation)
{
    const size_t num_samples = 10;
    const size_t num_readers = 2;
    const size_t expected_samples = 3; // Choose different expressions, but all with this amount of expected samples
    ASSERT_GE(expected_samples, 1); // Needs to be a positive integer

    /* Create writer */
    PubSubWriter<UnboundedHelloWorldPubSubType> writer("TestTopic");
    std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_descriptor =
            std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    writer.disable_builtin_transport().add_user_transport_to_pparams(udp_descriptor);
    writer.datasharing_off();
    writer.reliability(ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS);
    writer.history_depth(num_samples).init();
    ASSERT_TRUE(writer.isInitialized());

    /* Create readers with filter */
    std::vector<std::shared_ptr<PubSubReader<UnboundedHelloWorldPubSubType>>> readers;
    for (size_t i = 0; i < num_readers; ++i)
    {
        const std::string expression = "index BETWEEN " + std::to_string(i) + " AND " +
                std::to_string(i + expected_samples - 1);
        auto reader = std::make_shared<PubSubReader<UnboundedHelloWorldPubSubType>>("TestTopic", expression,
                        std::vector<std::string>{});
        reader->reliability(ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS);
        reader->history_depth(num_samples).init();
        ASSERT_TRUE(reader->isInitialized());
        reader->wait_discovery();
        readers.push_back(reader);
    }

    /* Wait for discovery before start to publish */
    writer.wait_discovery(static_cast<unsigned int>(num_readers));

    /* Start samples reception */
    for (auto& reader : readers)
    {
        reader->startReception(expected_samples);
    }

    /* Create and fill sample to use fragmentation */
    UnboundedHelloWorld data;
    data.message(std::string(68000, 'A'));

    /* Send samples */
    for (size_t i = 0; i < num_samples; ++i)
    {
        data.index(static_cast<uint16_t>(i));
        ASSERT_TRUE(writer.send_sample(data));
    }

    /* Wait until all samples are acknowledged */
    writer.waitForAllAcked(std::chrono::seconds(3));

    /* Check that all samples are received */
    for (auto& reader : readers)
    {
        ASSERT_TRUE(reader->wait_for_all_received(std::chrono::seconds(3)));
    }
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
