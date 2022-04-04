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

#include <gtest/gtest.h>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.h>

#include <fastrtps/attributes/LibrarySettingsAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "BlackboxTests.hpp"

#include "PubSubWriter.hpp"

#include "../types/HelloWorldTypeObject.h"

namespace eprosima {
namespace fastdds {
namespace dds {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

struct ContentFilterInfoCounter
{
    std::atomic_size_t content_filter_info_count;
    std::atomic_uint32_t max_filter_signature_number;
    std::shared_ptr<rtps::test_UDPv4TransportDescriptor> transport;

    ContentFilterInfoCounter()
        : content_filter_info_count(0)
        , max_filter_signature_number(0)
        , transport(std::make_shared<rtps::test_UDPv4TransportDescriptor>())
    {
        transport->drop_data_messages_filter_ = [this](fastrtps::rtps::CDRMessage_t& msg) -> bool
                {
                    // Check if it has inline_qos
                    uint8_t flags = msg.buffer[msg.pos - 3];
                    if (0x02 == (flags & 0x02))
                    {
                        auto old_pos = msg.pos;

                        // Skip extraFlags, read octetsToInlineQos, and skip there.
                        msg.pos += 2;
                        uint16_t to_inline_qos = 0;
                        fastrtps::rtps::CDRMessage::readUInt16(&msg, &to_inline_qos);
                        msg.pos += to_inline_qos;

                        while (msg.pos < msg.length)
                        {
                            uint16_t pid = 0;
                            uint16_t plen = 0;

                            fastrtps::rtps::CDRMessage::readUInt16(&msg, &pid);
                            fastrtps::rtps::CDRMessage::readUInt16(&msg, &plen);
                            uint32_t next_pos = msg.pos + plen;

                            if (pid == PID_CONTENT_FILTER_INFO)
                            {
                                ++content_filter_info_count;

                                if (plen >= 4 + 4 + 4 + 16)
                                {
                                    uint32_t num_bitmaps = 0;
                                    fastrtps::rtps::CDRMessage::readUInt32(&msg, &num_bitmaps);
                                    msg.pos += 4 * num_bitmaps;

                                    uint32_t num_signatures = 0;
                                    fastrtps::rtps::CDRMessage::readUInt32(&msg, &num_signatures);
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
        using namespace eprosima::fastrtps;

        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case communication_type::INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case communication_type::DATASHARING:
                enable_datasharing = true;
                break;
            case communication_type::TRANSPORT:
            default:
                break;
        }

        writer_side_filter = (communication_type::TRANSPORT == GetParam());
    }

    void TearDown() override
    {
        using namespace eprosima::fastrtps;

        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case communication_type::INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case communication_type::DATASHARING:
                enable_datasharing = false;
                break;
            case communication_type::TRANSPORT:
            default:
                break;
        }
        writer_side_filter = false;
    }

protected:

    bool writer_side_filter = false;
};

TEST_P(DDSContentFilter, BasicTest)
{
    registerHelloWorldTypes();

    ContentFilterInfoCounter filter_counter;

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(filter_counter.transport);
    writer.history_depth(10).init();

    auto participant = writer.getParticipant();
    ASSERT_NE(nullptr, participant);
    auto topic = static_cast<Topic*>(participant->lookup_topicdescription(writer.topic_name()));
    ASSERT_NE(nullptr, topic);
    auto filtered_topic = participant->create_contentfilteredtopic("filtered_topic", topic, "", {});
    ASSERT_NE(nullptr, filtered_topic);
    auto subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    DataReaderQos reader_qos = subscriber->get_default_datareader_qos();
    reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().depth = 10;
    auto reader = subscriber->create_datareader(filtered_topic, reader_qos);
    ASSERT_NE(nullptr, reader);

    auto send_data = [&](uint64_t expected_samples, const std::vector<uint16_t>& index_values, bool expect_wr_filters)
            {
                filter_counter.content_filter_info_count = 0;
                filter_counter.max_filter_signature_number = 0;

                // Send 10 samples with index 1 to 10
                auto data = default_helloworld_data_generator();
                writer.send(data);
                EXPECT_TRUE(data.empty());

                // On data-sharing, reader acknowledges samples on return_loan.
                if (!enable_datasharing)
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
                expected_ret = expected_samples == 0 ? ReturnCode_t::RETCODE_NO_DATA : ReturnCode_t::RETCODE_OK;
                EXPECT_EQ(expected_ret, reader->take(recv_data, recv_info));
                EXPECT_EQ(recv_data.length(), expected_samples);
                for (HelloWorldSeq::size_type i = 0; i < recv_data.length(); ++i)
                {
                    EXPECT_EQ(index_values[i], recv_data[i].index());
                }
                if (expected_samples > 0)
                {
                    EXPECT_EQ(ReturnCode_t::RETCODE_OK, reader->return_loan(recv_data, recv_info));
                }

                if (writer_side_filter && expect_wr_filters)
                {
                    EXPECT_NE(filter_counter.content_filter_info_count, 0);
                }
                else
                {
                    EXPECT_EQ(filter_counter.content_filter_info_count, 0);
                }
            };

    std::cout << std::endl << "TEST empty expression..." << std::endl;
    send_data(10u, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, false);

    std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"2\", \"4\"}..." << std::endl;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
            filtered_topic->set_filter_expression("index BETWEEN %0 AND %1", { "2", "4" }));
    send_data(3u, {2, 3, 4}, true);

    std::cout << std::endl << "Test 'index BETWEEN %0 AND %1', {\"6\", \"9\"}..." << std::endl;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
            filtered_topic->set_expression_parameters({ "6", "9" }));
    send_data(4u, {6, 7, 8, 9}, true);

    std::cout << std::endl << "Test 'message match %0', {\"'HelloWorld 1.*'\"}..." << std::endl;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
            filtered_topic->set_filter_expression("message match %0", { "'HelloWorld 1.*'" }));
    send_data(2u, {1, 10}, true);

    std::cout << std::endl << "Test 'message match %0', {\"'WRONG MESSAGE .*'\"}..." << std::endl;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
            filtered_topic->set_filter_expression("message match %0", { "'WRONG MESSAGE .*'" }));
    send_data(0u, {}, false);

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, subscriber->delete_datareader(reader));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(subscriber));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_contentfilteredtopic(filtered_topic));
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
