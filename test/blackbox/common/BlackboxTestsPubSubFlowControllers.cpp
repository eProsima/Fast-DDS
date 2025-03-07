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

#include <cstdio>
#include <thread>

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubWriterReader.hpp"
#include "PubSubParticipant.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

#define SIZE_PDP 625
#define SIZE_EDP 1675

class PubSubFlowControllers : public testing::TestWithParam<eprosima::fastdds::rtps::FlowControllerSchedulerPolicy>
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

TEST_P(PubSubFlowControllers, AsyncPubSubAsReliableData64kbWithParticipantFlowControl)
{
    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(3).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 68000;
    uint32_t periodInMs = 500;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs);

    writer.history_depth(3).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubFlowControllers, AsyncPubSubAsReliableData64kbWithParticipantFlowControlAndUserTransport)
{
    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(3).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 65000;
    uint32_t periodInMs = 500;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs);

    auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(3).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubFlowControllers, AsyncPubSubWithFlowController64kb)
{
    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> slowWriter(TEST_TOPIC_NAME);

    reader.history_depth(2).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    uint32_t sizeToClear = 68000; //68kb
    uint32_t periodInMs = 1000; //1sec

    slowWriter.history_depth(2).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(scheduler_policy_, sizeToClear, periodInMs).init();
    ASSERT_TRUE(slowWriter.isInitialized());

    slowWriter.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(2);

    reader.startReception(data);
    slowWriter.send(data);
    // In 1 second only one of the messages has time to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(reader.getReceivedCount(), 1u);
}

TEST_P(PubSubFlowControllers, FlowControllerIfNotAsync)
{
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    uint32_t size = 10000;
    uint32_t periodInMs = 1000;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, size, periodInMs).init();
    ASSERT_FALSE(writer.isInitialized());
}

TEST_P(PubSubFlowControllers, AsyncMultipleWritersFlowController64kb)
{
    PubSubWriterReader<Data64kbPubSubType> entities(TEST_TOPIC_NAME);

    // Readers configuration
    entities.sub_history_depth(3).
            sub_durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS).
            sub_reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

    // Writers configuration.
    uint32_t bytesPerPeriod = 68000;
    uint32_t periodInMs = 500;
    entities.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs).
            pub_history_depth(3).
            pub_durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS).
            pub_reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);

    // Creation.
    entities.init();

    ASSERT_TRUE(entities.isInitialized());

    // Create second writer.
    eprosima::fastdds::rtps::PropertySeq writers2_properties;
    eprosima::fastdds::rtps::Property priority;
    priority.name("fastdds.sfc.priority");
    priority.value("-1");
    writers2_properties.push_back(priority);
    eprosima::fastdds::rtps::Property bandwidth_limit;
    bandwidth_limit.name("fastdds.sfc.bandwidth_reservation");
    bandwidth_limit.value("10");
    writers2_properties.push_back(bandwidth_limit);
    ASSERT_TRUE(entities.create_additional_topics(1, "/", writers2_properties));

    eprosima::fastdds::rtps::PropertySeq writers3_properties;
    priority.name("fastdds.sfc.priority");
    priority.value("1");
    writers3_properties.push_back(priority);
    bandwidth_limit.name("fastdds.sfc.bandwidth_reservation");
    bandwidth_limit.value("15");
    writers3_properties.push_back(bandwidth_limit);
    ASSERT_TRUE(entities.create_additional_topics(1, "/", writers3_properties));

    eprosima::fastdds::rtps::PropertySeq writers4_properties;
    priority.name("fastdds.sfc.priority");
    priority.value("4");
    writers4_properties.push_back(priority);
    bandwidth_limit.name("fastdds.sfc.bandwidth_reservation");
    bandwidth_limit.value("20");
    writers4_properties.push_back(bandwidth_limit);
    ASSERT_TRUE(entities.create_additional_topics(1, "/", writers4_properties));

    // Because its volatile the durability
    // Wait for discovery.
    entities.wait_discovery();

    auto data = default_data64kb_data_generator(3);

    entities.startReception(data);

    // Send data
    entities.send(data);

    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    entities.block_for_all();
}

TEST_P(PubSubFlowControllers, AsyncPubSubAsReliableData64kbWithParticipantFlowControlAndPersistence)
{
    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    // Get info about current test
    auto info = ::testing::UnitTest::GetInstance()->current_test_info();

    // Create DB file name from test name and PID
    std::ostringstream ss;
    std::string test_case_name(info->test_case_name());
    std::string test_name(info->name());
    ss <<
        test_case_name.replace(test_case_name.find_first_of('/'), 1, "_") << "_" <<
        test_name.replace(test_name.find_first_of('/'), 1, "_")  << "_" << GET_PID() << ".db";
    std::string db_file_name = {ss.str()};

    reader.history_depth(3).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 65000;
    uint32_t periodInMs = 500;
    writer.add_flow_controller_descriptor_to_pparams(scheduler_policy_, bytesPerPeriod, periodInMs);

    writer.history_depth(3)
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE)
            .make_transient(db_file_name, "33.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64")
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    reader.destroy();
    writer.destroy();
    std::remove(db_file_name.c_str());
}

// This test checks that the PDP and EDP discovery are successful when proper parameters
//  for builtin flow controller are set
TEST_P(PubSubFlowControllers, BuiltinFlowControllerPubSub)
{
    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(3).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = SIZE_PDP + SIZE_EDP;
    uint32_t periodInMs = 50000;
    writer.add_builtin_flow_controller(scheduler_policy_, bytesPerPeriod, periodInMs);

    writer.history_depth(3).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// This test checks that the PDP discovery process is not successful when the builtin
//  flow controller is set with not enough size to send the PDP
TEST_P(PubSubFlowControllers, BuiltinFlowControllerFailDiscovery)
{
    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = SIZE_PDP - 20; // Not enough size to send Data P
    uint32_t periodInMs = 50000;
    writer.add_builtin_flow_controller(scheduler_policy_, bytesPerPeriod, periodInMs);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Reader discovery should fail
    ASSERT_FALSE(reader.wait_participant_discovery(1, std::chrono::seconds(1)));
}

// This test checks that the WLP service is not able to send non stop liveliness messages when the builtin
//  flow controller is set with not enough size to send all the liveliness messages
TEST_P(PubSubFlowControllers, BuiltinFlowControllerWLPLimited)
{
    eprosima::fastdds::LibrarySettings att;
    att.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(att);

    unsigned int lease_duration_ms = 501;
    unsigned int announcement_period_ms = 200;

    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = SIZE_PDP + SIZE_EDP + 5000;
    uint32_t periodInMs = 50000;
    writer.add_builtin_flow_controller(scheduler_policy_, bytesPerPeriod, periodInMs);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .liveliness_announcement_period(announcement_period_ms * 1e-3).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Once asserted that the writer is matched, it should eventually be declared as not alive because the
    // flow controller will not allow the wlp writer to send more liveliness messages
    std::atomic<bool> stop(false);
    auto assert_liveliness_func = [&writer, lease_duration_ms, &stop]()
            {
                while (!stop)
                {
                    writer.assert_liveliness();
                    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms / 10));
                }
            };
    std::thread liveliness_thread(assert_liveliness_func);

    reader.wait_liveliness_lost(1);
    stop.store(true);
    liveliness_thread.join();
}

TEST_P(PubSubFlowControllers, BuiltinFlowControllerNotRegistered)
{
    using namespace eprosima::fastdds::dds;

    // Create the main participant
    auto main_participant = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);
    WireProtocolConfigQos main_wire_protocol;
    main_wire_protocol.builtin.flow_controller_name = "NotRegisteredTestFlowController";

    // The main participant will use the test transport, specific announcements configuration and a flowcontroller
    main_participant->wire_protocol(main_wire_protocol);

    // Start the main participant
    ASSERT_FALSE(main_participant->init_participant());
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PubSubFlowControllers,
        PubSubFlowControllers,
        testing::Values(
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO,
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN,
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY,
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION
            ),
        [](const testing::TestParamInfo<PubSubFlowControllers::ParamType>& info)
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
