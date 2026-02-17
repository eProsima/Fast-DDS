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

#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/LibrarySettings.hpp>

#include "BlackboxTests.hpp"
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

using test_params = std::tuple<communication_type, eprosima::fastdds::rtps::FlowControllerSchedulerPolicy>;

class PubSubFragments : public testing::TestWithParam<test_params>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (std::get<0>(GetParam()))
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

        scheduler_policy_ = std::get<1>(GetParam());
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (std::get<0>(GetParam()))
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

protected:

    void do_fragment_test(
            const std::string& topic_name,
            std::list<Data1mb>& data,
            bool asynchronous,
            bool reliable,
            bool volatile_reader,
            bool volatile_writer,
            bool small_fragments,
            uint32_t loss_rate = 0)
    {
        PubSubReader<Data1mbPubSubType> reader(topic_name);
        PubSubWriter<Data1mbPubSubType> writer(topic_name);
        uint32_t fragment_count = 0;

        reader
                .socket_buffer_size(1048576) // accomodate large and fast fragments
                .history_depth(static_cast<int32_t>(data.size()))
                .reliability(reliable ?
                eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS :
                eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
                .durability_kind(volatile_reader ?
                eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS :
                eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
                .init();

        ASSERT_TRUE(reader.isInitialized());

        if (small_fragments || 0 < loss_rate)
        {
            auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();

            testTransport->receiveBufferSize = 65536;
            if (small_fragments)
            {
                testTransport->sendBufferSize = 1024;
                testTransport->maxMessageSize = 1024;
            }
            if (0 < loss_rate)
            {
                testTransport->drop_data_frag_messages_filter_ =
                        [&fragment_count, loss_rate](eprosima::fastdds::rtps::CDRMessage_t& msg)->bool
                        {
                            static_cast<void>(msg);

                            ++fragment_count;
                            if (fragment_count >= loss_rate)
                            {
                                fragment_count = 0;
                            }

                            return 1ul == fragment_count;
                        };
            }

            writer.disable_builtin_transport();
            writer.add_user_transport_to_pparams(testTransport);
        }

        if (asynchronous)
        {
            writer.asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
                    add_flow_controller_descriptor_to_pparams(scheduler_policy_, 0, 0);
        }

        writer
                .history_depth(static_cast<int32_t>(data.size()))
                .reliability(reliable ?
                eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS :
                eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
                .durability_kind(volatile_writer ?
                eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS :
                eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
                .init();

        ASSERT_TRUE(writer.isInitialized());

        // Because its volatile the durability
        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        reader.startReception(data);
        // Send data
        writer.send(data, reliable ? 0u : 10u);
        ASSERT_TRUE(data.empty());

        // Block reader until reception finished or timeout.
        if (reliable)
        {
            reader.block_for_all();
        }
        else
        {
            reader.block_for_at_least(2);
        }
    }

    eprosima::fastdds::rtps::FlowControllerSchedulerPolicy scheduler_policy_;
};

TEST_P(PubSubFragments, PubSubAsNonReliableData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, false, true, false, false);
}

TEST_P(PubSubFragments, PubSubAsNonReliableVolatileData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, false, true, true, false);
}

TEST_P(PubSubFragments, PubSubAsNonReliableTransientLocalData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, false, false, false, false);
}

TEST_P(PubSubFragments, PubSubAsNonReliableData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, false, true, false, true);
}

TEST_P(PubSubFragments, PubSubAsNonReliableVolatileData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, false, true, true, true);
}

TEST_P(PubSubFragments, PubSubAsNonReliableTransientLocalData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, false, false, false, true);
}

TEST_P(PubSubFragments, PubSubAsReliableData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, true, true, false, false);
}

TEST_P(PubSubFragments, PubSubAsReliableVolatileData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, true, true, true, false);
}

TEST_P(PubSubFragments, PubSubAsReliableTransientLocalData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, true, false, false, false);
}

TEST_P(PubSubFragments, PubSubAsReliableData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, true, true, false, true);
}

TEST_P(PubSubFragments, PubSubAsReliableVolatileData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, true, true, true, true);
}

TEST_P(PubSubFragments, PubSubAsReliableTransientLocalData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, true, false, false, true);
}

TEST_P(PubSubFragments, PubSubAsReliableTransientLocalData300kbSmallFragmentsLossy)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, false, true, false, false, true, 260);
}

TEST_P(PubSubFragments, AsyncPubSubAsNonReliableData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, false, true, false, false);
}

TEST_P(PubSubFragments, AsyncPubSubAsNonReliableVolatileData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, false, true, true, false);
}

TEST_P(PubSubFragments, AsyncPubSubAsNonReliableTransientLocalData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, false, false, false, false);
}

TEST_P(PubSubFragments, AsyncPubSubAsNonReliableData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, false, true, false, true);
}

TEST_P(PubSubFragments, AsyncPubSubAsNonReliableVolatileData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, false, true, true, true);
}

TEST_P(PubSubFragments, AsyncPubSubAsNonReliableTransientLocalData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, false, false, false, true);
}

TEST_P(PubSubFragments, AsyncPubSubAsReliableData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, true, true, false, false);
}

TEST_P(PubSubFragments, AsyncPubSubAsReliableVolatileData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, true, true, true, false);
}

TEST_P(PubSubFragments, AsyncPubSubAsReliableTransientLocalData300kb)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, true, false, false, false);
}

TEST_P(PubSubFragments, AsyncPubSubAsReliableData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, true, true, false, true);
}

TEST_P(PubSubFragments, AsyncPubSubAsReliableVolatileData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, true, true, true, true);
}

TEST_P(PubSubFragments, AsyncPubSubAsReliableTransientLocalData300kbSmallFragments)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, true, false, false, true);
}

TEST_P(PubSubFragments, AsyncPubSubAsReliableTransientLocalData300kbSmallFragmentsLossy)
{
    auto data = default_data300kb_data_generator();
    do_fragment_test(TEST_TOPIC_NAME, data, true, true, false, false, true, 260);
}

class PubSubFragmentsLimited : public testing::TestWithParam<eprosima::fastdds::rtps::FlowControllerSchedulerPolicy>
{
public:

    void SetUp() override
    {
        scheduler_policy_ = GetParam();
    }

    void TearDown() override
    {
    }

protected:

    eprosima::fastdds::rtps::FlowControllerSchedulerPolicy scheduler_policy_;
};

TEST_P(PubSubFragmentsLimited, AsyncPubSubAsNonReliableData300kbWithFlowControl)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(PubSubFragmentsLimited, AsyncPubSubAsReliableData300kbWithFlowControl)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubFragmentsLimited, AsyncPubSubAsReliableData300kbInLossyConditions)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        testTransport->test_transport_options->test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST_P(PubSubFragmentsLimited, AsyncPubSubAsReliableVolatileData300kbInLossyConditions)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
            durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        testTransport->test_transport_options->test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST_P(PubSubFragmentsLimited, AsyncPubSubAsReliableData300kbInLossyConditionsSmallFragments)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 1024;
    testTransport->maxMessageSize = 1024;
    testTransport->receiveBufferSize = 65536;
    // We are sending around 300 fragments per sample.
    // We drop 1% of all data frags
    testTransport->dropDataFragMessagesPercentage = 1;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        testTransport->test_transport_options->test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST_P(PubSubFragmentsLimited, AsyncPubSubAsReliableKeyedData300kbKeepLast1InLossyConditionsSmallFragments)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(2)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // To simulate lossy conditions, we are going to remove the default
    // builtin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->maxMessageSize = 1024;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 153601;
    uint32_t periodInMs = 100;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs)
            .heartbeat_period_seconds(0)
            .heartbeat_period_nanosec(1000000)
            .history_depth(1)
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyeddata300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data, 100);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_seq({ 0, 5 });

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        testTransport->test_transport_options->test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

// Regression test for 20257
// When a non existing change is removed, the change is also removed from the data instance changes sequence
TEST(PubSubFragmentsLimited,
        AsyncPubSubAsReliableKeyedData300kbKeepLast1LoosyConditionsSmallFragmentsCorrectlyBehavesWhenInlineQoSAreForced)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(2)
            .expect_inline_qos(true)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // To simulate lossy conditions, we are going to remove the default
    // builtin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->maxMessageSize = 1024;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 153601;
    uint32_t periodInMs = 100;
    writer.add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY, bytesPerPeriod, periodInMs)
            .heartbeat_period_seconds(0)
            .heartbeat_period_nanosec(1000000)
            .history_depth(1)
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyeddata300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data, 100);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_seq({ 0, 5 });

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        testTransport->test_transport_options->test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST_P(PubSubFragmentsLimited, AsyncPubSubAsReliableVolatileData300kbInLossyConditionsSmallFragments)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 1024;
    testTransport->maxMessageSize = 1024;
    testTransport->receiveBufferSize = 65536;
    // We are sending around 300 fragments per sample.
    // We drop 1% of all data frags
    testTransport->dropDataFragMessagesPercentage = 1;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
            durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        testTransport->test_transport_options->test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST_P(PubSubFragmentsLimited, AsyncFragmentSizeTest)
{
    // ThroghputController size large than maxMessageSize.
    {
        PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
        PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

        reader.history_depth(10).
                reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        // When doing fragmentation, it is necessary to have some degree of
        // flow control not to overrun the receive buffer.
        uint32_t size = 32536;
        uint32_t periodInMs = 500;
        writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, size, periodInMs);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 32000;
        testTransport->sendBufferSize = 65536;
        testTransport->receiveBufferSize = 65536;
        writer.disable_builtin_transport();
        writer.add_user_transport_to_pparams(testTransport);
        writer.history_depth(10).asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

        ASSERT_TRUE(writer.isInitialized());

        // Because its volatile the durability
        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_data64kb_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        std::this_thread::sleep_for(std::chrono::seconds(3));
        size_t current_received = reader.getReceivedCount();
        ASSERT_GE(current_received, static_cast<size_t>(1));
        ASSERT_LE(current_received, static_cast<size_t>(3));

    }
    // ThroghputController size smaller than maxMessageSize.
    {
        PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
        PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

        reader.history_depth(10).
                reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        // When doing fragmentation, it is necessary to have some degree of
        // flow control not to overrun the receive buffer.
        uint32_t size = 32000;
        uint32_t periodInMs = 500;
        writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, size, periodInMs);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 32536;
        testTransport->sendBufferSize = 65536;
        testTransport->receiveBufferSize = 65536;
        writer.disable_builtin_transport();
        writer.add_user_transport_to_pparams(testTransport);
        writer.history_depth(10).
                asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

        ASSERT_TRUE(writer.isInitialized());

        // Because its volatile the durability
        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_data64kb_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        std::this_thread::sleep_for(std::chrono::seconds(3));
        size_t current_received = reader.getReceivedCount();
        ASSERT_GE(current_received, static_cast<size_t>(1));
        ASSERT_LE(current_received, static_cast<size_t>(3));
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PubSubFragments,
        PubSubFragments,
        testing::Combine(
            testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
            testing::Values(
                eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO,
                eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN,
                eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY,
                eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION
                )),
        [](const testing::TestParamInfo<PubSubFragments::ParamType>& info)
        {
            std::string suffix;
            switch (std::get<1>(info.param))
            {
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION:
                    suffix = "_SCHED_RESERV";
                    break;
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY:
                    suffix = "_SCHED_HIGH";
                    break;
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN:
                    suffix = "_SCHED_ROBIN";
                    break;
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO:
                default:
                    suffix = "_SCHED_FIFO";
            }

            switch (std::get<0>(info.param))
            {
                case INTRAPROCESS:
                    return "Intraprocess" + suffix;
                case DATASHARING:
                    return "Datasharing" + suffix;
                case TRANSPORT:
                default:
                    return "Transport" + suffix;
            }
        });

GTEST_INSTANTIATE_TEST_MACRO(PubSubFragmentsLimited,
        PubSubFragmentsLimited,
        testing::Values(
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO,
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN,
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY,
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION
            ),
        [](const testing::TestParamInfo<PubSubFragmentsLimited::ParamType>& info)
        {
            std::string suffix;
            switch (info.param)
            {
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION:
                    suffix = "_SCHED_RESERV";
                    break;
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY:
                    suffix = "_SCHED_HIGH";
                    break;
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN:
                    suffix = "_SCHED_ROBIN";
                    break;
                case eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO:
                default:
                    suffix = "_SCHED_FIFO";
            }

            return "Transport" + suffix;
        });
