/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>

#include <rtps/congestion-control/CongestionControlFactory.hpp>
#include <rtps/congestion-control/CongestionControlParameters.hpp>
#include <rtps/congestion-control/ICongestionControl.hpp>
#include <rtps/congestion-control/basic/CongestionControlBasic.hpp>

#include <rtps/flowcontrol/FlowControllerFactory.hpp>
#include <rtps/flowcontrol/GrainedFlowController.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>

using namespace eprosima::fastdds::rtps;

TEST(CongestionControlFactory, plugin_creation)
{
    ICongestionControl* cc = nullptr;

    {
        cc = CongestionControlFactory::create_congestion_control(CongestionControlFactory::BASIC_CONGESTION_CONTROL);
        ASSERT_NE(cc, nullptr);
        auto* cc_basic = dynamic_cast<CongestionControlBasic*>(cc);
        EXPECT_NE(cc_basic, nullptr);
        delete cc;
    }

    {
        cc = CongestionControlFactory::create_congestion_control("non_existing_cc");
        EXPECT_EQ(cc, nullptr);
    }
}

/**
 * @brief Helper struct to wait for the next period in tests.
 */
struct TimeWaiter
{
    /**
     * @brief Constructor.
     *
     * @param period_ms  Period duration in milliseconds.
     * @param delta_ms   Initial delay in milliseconds before the first wait.
     *                   This is to avoid waiting exactly on the period boundary.
     */
    TimeWaiter(
            uint32_t period_ms,
            uint32_t delta_ms)
        : wait_tp_(std::chrono::steady_clock::now() + std::chrono::milliseconds(delta_ms))
        , period_ms_(period_ms)
    {
    }

    /**
     * @brief Shows a message and waits until the next period.
     *
     * @param message  Message to display before waiting.
     */
    void wait_for_next_period(
            const char* message)
    {
        std::cout << "Waiting for next period to verify " << message << "..." << std::endl;
        wait_tp_ += std::chrono::milliseconds(period_ms_);
        std::this_thread::sleep_until(wait_tp_);
    }

private:

    /// Next time point to wait for.
    std::chrono::steady_clock::time_point wait_tp_;
    /// Period duration in milliseconds.
    uint32_t period_ms_;
};

TEST(CongestionControlBasic, initialization)
{
    using ParticipantMock = testing::StrictMock<RTPSParticipantImpl>;

    struct FlowControllerFactoryExposer : public FlowControllerFactory
    {
        FlowController* get_flow_controller(
                const std::string& name)
        {
            auto it = flow_controllers_.find(name);
            return it != flow_controllers_.end() ? it->second.get() : nullptr;
        }

    };

    // Verify that GrainedFlowController is registered with max payload taken from the participant
    for (uint32_t max_data_size : { 64000, 32000, 16000, 8000 })
    {
        ParticipantMock participant;
        FlowControllerFactoryExposer flow_controller_factory;
        EXPECT_CALL(participant, getMaxDataSize).WillOnce(testing::Return(max_data_size));
        CongestionControlParameters params{ 1000, max_data_size* 4, 1.5f, 0.5f };
        CongestionControlBasic uut;
        EXPECT_TRUE(uut.initialize(participant, flow_controller_factory, params));
        // Should have registered a GrainedFlowController in the factory
        FlowController* fc = flow_controller_factory.get_flow_controller("GrainedFlowController");
        EXPECT_NE(fc, nullptr);
        GrainedFlowController* grained_fc = dynamic_cast<GrainedFlowController*>(fc);
        ASSERT_NE(grained_fc, nullptr);
        EXPECT_EQ(grained_fc->get_max_payload(), max_data_size);
    }
}

TEST(CongestionControlBasic, one_reader)
{
    using namespace std::chrono_literals;

    constexpr uint32_t max_data_size = 64000;
    constexpr uint32_t initial_target_bps = max_data_size * 4;
    constexpr uint32_t period_duration_ms = 1200;

    using ParticipantMock = testing::StrictMock<RTPSParticipantImpl>;
    using FlowControllerMock = testing::StrictMock<GrainedFlowController>;

    ParticipantMock participant;

    // Create and register a GrainedFlowController strict mock in the factory
    FlowControllerFactory flow_controller_factory;
    FlowControllerMock* grained_fc = nullptr;
    grained_fc = flow_controller_factory.register_flow_controller_type<FlowControllerMock>(
        "GrainedFlowController",
        ThreadSettings{},
        max_data_size,
        1000);

    // Prepare writer and reader information
    GUID_t writer_guid;
    writer_guid.guidPrefix.value[0] = 0x01;
    writer_guid.entityId = 0x00000402;

    SubscriptionBuiltinTopicData reader_info;
    GUID_t reader_guid;
    reader_guid.guidPrefix.value[0] = 0x01;
    reader_guid.entityId = 0x00000401;
    reader_info.guid = reader_guid;

    // Test setup
    CongestionControlParameters params { period_duration_ms, initial_target_bps, 1.5f, 0.5f };
    CongestionControlBasic uut;

    // Set log verbosity to warning because some warnings are expected during the test
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Warning);

    // Several calls to getMaxDataSize are expected throughout the whole test
    EXPECT_CALL(participant, getMaxDataSize).WillRepeatedly(testing::Return(max_data_size));

    // Test that initialization call is forwarded to the flow controller
    {
        EXPECT_CALL(*grained_fc, init);

        bool init_result = uut.initialize(participant, flow_controller_factory, params);
        EXPECT_TRUE(init_result);
    }

    // Check that writer attributes are prepared correctly
    {
        WriterAttributes original_attributes;
        original_attributes.endpoint.reliabilityKind = RELIABLE;
        original_attributes.flow_controller_name = "";
        original_attributes.mode = SYNCHRONOUS_WRITER;

        // Positive case: reliable non-builtin writer
        WriterAttributes modified_attributes = uut.prepare_writer_attributes(
            original_attributes,
            writer_guid.entityId,
            false);
        EXPECT_EQ(modified_attributes.flow_controller_name, "GrainedFlowController");
        EXPECT_EQ(modified_attributes.mode, ASYNCHRONOUS_WRITER);

        // Negative case: non-reliable writer
        original_attributes.endpoint.reliabilityKind = BEST_EFFORT;
        modified_attributes = uut.prepare_writer_attributes(
            original_attributes,
            writer_guid.entityId,
            false);
        EXPECT_EQ(modified_attributes.flow_controller_name, "");
        EXPECT_EQ(modified_attributes.mode, SYNCHRONOUS_WRITER);

        // Negative case: builtin writer
        original_attributes.endpoint.reliabilityKind = RELIABLE;
        modified_attributes = uut.prepare_writer_attributes(
            original_attributes,
            writer_guid.entityId,
            true);
        EXPECT_EQ(modified_attributes.flow_controller_name, "");
        EXPECT_EQ(modified_attributes.mode, SYNCHRONOUS_WRITER);

        // Warning case: original attributes already specify a flow controller
        std::cout << std::endl << "--------- On purpose warning log expected ---------" << std::endl;
        original_attributes.flow_controller_name = "CustomFlowController";
        original_attributes.mode = ASYNCHRONOUS_WRITER;
        modified_attributes = uut.prepare_writer_attributes(
            original_attributes,
            writer_guid.entityId,
            false);
        std::cout << "----------------------------------------------------" << std::endl << std::endl;

        EXPECT_EQ(modified_attributes.flow_controller_name, "GrainedFlowController");
        EXPECT_EQ(modified_attributes.mode, ASYNCHRONOUS_WRITER);
    }

    // Since no readers have been registered, the periodic timer should not be scheduled
    {
        std::cout << "Waiting for two periods to verify no updates occur without readers..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(period_duration_ms * 2));
        std::cout << std::endl;
        // No calls to update_remote_reader_bytes_per_period should have occurred (verified by StrictMock)
    }

    // Register a reader and verify that it is forwarded to the flow controller
    TimeWaiter time_waiter(period_duration_ms, 100);
    {
        // Expect the registration to be forwarded to the flow controller
        EXPECT_CALL(*grained_fc, register_remote_reader(reader_guid, initial_target_bps));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::DISCOVERED_READER, reader_info);

        // Updating the information of an existing reader should not trigger a new registration
        uut.notify_reader_discovery(ReaderDiscoveryStatus::CHANGED_QOS_READER, reader_info);

        // Unregistering should also be forwarded to the flow controller
        EXPECT_CALL(*grained_fc, unregister_remote_reader(reader_guid));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::REMOVED_READER, reader_info);

        // Unregistering a non-existing reader should do nothing
        uut.notify_reader_discovery(ReaderDiscoveryStatus::REMOVED_READER, reader_info);

        // Re-register the reader for further tests
        EXPECT_CALL(*grained_fc, register_remote_reader(reader_guid, initial_target_bps));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::DISCOVERED_READER, reader_info);

        // Ignoring a reader behaves like removal
        EXPECT_CALL(*grained_fc, unregister_remote_reader(reader_guid));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::IGNORED_READER, reader_info);

        // Ignoring a non-existing reader should do nothing
        uut.notify_reader_discovery(ReaderDiscoveryStatus::IGNORED_READER, reader_info);

        // Re-register the reader for further tests
        EXPECT_CALL(*grained_fc, register_remote_reader(reader_guid, initial_target_bps));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::DISCOVERED_READER, reader_info);
    }

    // A reader is present but no activity has occurred, so no updates should happen
    {
        time_waiter.wait_for_next_period("no updates occur without activity");
        // No calls to update_remote_reader_bytes_per_period should have occurred (verified by StrictMock)
    }

    // Prepare auxiliary variables for further tests
    uint32_t current_limitation = initial_target_bps;
    SequenceNumber_t seq_num(1);
    LocatorList empty_locators;
    LocatorSelectorEntry locators = LocatorSelectorEntry::create_fully_selected_entry(empty_locators, empty_locators);

    // Expect an increase in limitation due to high acked bytes
    {
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, initial_target_bps * 2, 100ms, locators);
        current_limitation = static_cast<uint32_t>(current_limitation * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
        time_waiter.wait_for_next_period("increase in limitation due to high acked bytes");
    }

    // Expect a decrease in limitation due to resent bytes (only second time the same seq_num is resent)
    {
        seq_num++;
        uut.on_writer_resend_data(writer_guid, reader_guid, seq_num, 1, locators);
        current_limitation = static_cast<uint32_t>(current_limitation * 0.5f);
        time_waiter.wait_for_next_period("first resent bytes ignored");
        // No calls to update_remote_reader_bytes_per_period should have occurred (verified by StrictMock)

        uut.on_writer_resend_data(writer_guid, reader_guid, seq_num, 1, locators);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
        time_waiter.wait_for_next_period("decrease in limitation due to resent bytes");
    }

    // Further resends should keep causing decreases but not max_data_size limit
    {
        EXPECT_GT(current_limitation, max_data_size);
        for (int i = 0; i < 4; ++i)
        {
            uut.on_writer_resend_data(writer_guid, reader_guid, seq_num, 1, locators);
            current_limitation = static_cast<uint32_t>(current_limitation * 0.5f);
            if (current_limitation < max_data_size)
            {
                current_limitation = max_data_size;
            }
            EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
            time_waiter.wait_for_next_period("further decrease in limitation due to resent bytes");
        }
        EXPECT_EQ(current_limitation, max_data_size);
    }

    // Acknowledging less than 80% of the current limitation should not cause an increase
    {
        seq_num++;
        uint32_t acked_bytes = static_cast<uint32_t>(current_limitation * 0.8f) - 1;
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        time_waiter.wait_for_next_period("no increase when acked bytes < 80% of limitation");
        // No calls to update_remote_reader_bytes_per_period should have occurred (verified by StrictMock)
    }

    // Acknowledging more than 80% of the current limitation should cause an increase
    {
        seq_num++;
        uint32_t acked_bytes = static_cast<uint32_t>(current_limitation * 0.8f) + 1;
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        current_limitation = static_cast<uint32_t>(current_limitation * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
        time_waiter.wait_for_next_period("increase when acked bytes > 80% of limitation");
    }

    // Verify that several acknowledgements are accumulated within the same period
    {
        seq_num++;
        uint32_t acked_bytes = static_cast<uint32_t>(current_limitation * 0.5f);
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        seq_num++;
        acked_bytes = static_cast<uint32_t>(current_limitation * 0.4f);
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        current_limitation = static_cast<uint32_t>(current_limitation * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
        time_waiter.wait_for_next_period("accumulated acknowledgements causing increase");
    }

    // Verify that resends have priority over acknowledgements
    {
        seq_num++;
        uint32_t acked_bytes = static_cast<uint32_t>(current_limitation * 0.9f);
        uut.on_writer_resend_data(writer_guid, reader_guid, seq_num, 1, locators);
        uut.on_writer_resend_data(writer_guid, reader_guid, seq_num, 1, locators);
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        current_limitation = static_cast<uint32_t>(current_limitation * 0.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
        time_waiter.wait_for_next_period("resends have priority over acknowledgements");
    }
}

TEST(CongestionControlBasic, several_readers)
{
    using namespace std::chrono_literals;

    constexpr uint32_t max_data_size = 64000;
    constexpr uint32_t initial_target_bps = max_data_size * 4;
    constexpr uint32_t period_duration_ms = 1200;

    using ParticipantMock = testing::StrictMock<RTPSParticipantImpl>;
    using FlowControllerMock = testing::StrictMock<GrainedFlowController>;

    ParticipantMock participant;

    // Create and register a GrainedFlowController strict mock in the factory
    FlowControllerFactory flow_controller_factory;
    FlowControllerMock* grained_fc = nullptr;
    grained_fc = flow_controller_factory.register_flow_controller_type<FlowControllerMock>(
        "GrainedFlowController",
        ThreadSettings{},
        max_data_size,
        1000);

    // Prepare writer and reader information
    GUID_t writer_guid;
    writer_guid.guidPrefix.value[0] = 0x01;
    writer_guid.entityId = 0x00000402;

    uint32_t current_limitation_1 = initial_target_bps;
    SubscriptionBuiltinTopicData reader_info_1;
    GUID_t reader_guid_1;
    reader_guid_1.guidPrefix.value[0] = 0x01;
    reader_guid_1.entityId = 0x00000401;
    reader_info_1.guid = reader_guid_1;

    uint32_t current_limitation_2 = initial_target_bps;
    SubscriptionBuiltinTopicData reader_info_2;
    GUID_t reader_guid_2;
    reader_guid_2.guidPrefix.value[0] = 0x01;
    reader_guid_2.entityId = 0x00000403;
    reader_info_2.guid = reader_guid_2;

    // Test setup
    CongestionControlParameters params{ period_duration_ms, initial_target_bps, 1.5f, 0.5f };
    CongestionControlBasic uut;

    // Several calls to getMaxDataSize are expected throughout the whole test
    EXPECT_CALL(participant, getMaxDataSize).WillRepeatedly(testing::Return(max_data_size));

    // Test that initialization call is forwarded to the flow controller
    {
        EXPECT_CALL(*grained_fc, init);

        bool init_result = uut.initialize(participant, flow_controller_factory, params);
        EXPECT_TRUE(init_result);
    }

    // Register two readers
    {
        EXPECT_CALL(*grained_fc, register_remote_reader(reader_guid_1, initial_target_bps));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::DISCOVERED_READER, reader_info_1);
        EXPECT_CALL(*grained_fc, register_remote_reader(reader_guid_2, initial_target_bps));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::DISCOVERED_READER, reader_info_2);
    }

    TimeWaiter time_waiter(period_duration_ms, 100);
    SequenceNumber_t seq_num(1);
    LocatorList empty_locators;
    LocatorSelectorEntry locators = LocatorSelectorEntry::create_fully_selected_entry(empty_locators, empty_locators);

    // Each reader should be managed independently
    {
        // Reader 1 causes an increase
        uut.on_writer_data_acknowledged(writer_guid, reader_guid_1, seq_num, initial_target_bps, 100ms, locators);
        current_limitation_1 = static_cast<uint32_t>(current_limitation_1 * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_1, current_limitation_1));
        // Reader 2 causes a decrease
        uut.on_writer_resend_data(writer_guid, reader_guid_2, seq_num, 1, locators);
        uut.on_writer_resend_data(writer_guid, reader_guid_2, seq_num, 1, locators);
        current_limitation_2 = static_cast<uint32_t>(current_limitation_2 * 0.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_2, current_limitation_2));
        time_waiter.wait_for_next_period("reader 1 increase and reader 2 decrease");
    }
    {
        seq_num++;
        // Reader 1 causes a decrease
        uut.on_writer_resend_data(writer_guid, reader_guid_1, seq_num, 1, locators);
        uut.on_writer_resend_data(writer_guid, reader_guid_1, seq_num, 1, locators);
        current_limitation_1 = static_cast<uint32_t>(current_limitation_1 * 0.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_1, current_limitation_1));
        // Reader 2 causes an increase
        uut.on_writer_data_acknowledged(writer_guid, reader_guid_2, seq_num, initial_target_bps, 100ms, locators);
        current_limitation_2 = static_cast<uint32_t>(current_limitation_2 * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_2, current_limitation_2));
        time_waiter.wait_for_next_period("reader 1 decrease and reader 2 increase");
    }
    {
        seq_num++;
        // Both readers cause increases
        uut.on_writer_data_acknowledged(writer_guid, reader_guid_1, seq_num, initial_target_bps, 100ms, locators);
        current_limitation_1 = static_cast<uint32_t>(current_limitation_1 * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_1, current_limitation_1));
        uut.on_writer_data_acknowledged(writer_guid, reader_guid_2, seq_num, initial_target_bps, 100ms, locators);
        current_limitation_2 = static_cast<uint32_t>(current_limitation_2 * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_2, current_limitation_2));
        time_waiter.wait_for_next_period("both readers increase");
    }
    {
        seq_num++;
        // Both readers cause decreases
        uut.on_writer_resend_data(writer_guid, reader_guid_1, seq_num, 1, locators);
        uut.on_writer_resend_data(writer_guid, reader_guid_1, seq_num, 1, locators);
        current_limitation_1 = static_cast<uint32_t>(current_limitation_1 * 0.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_1, current_limitation_1));
        uut.on_writer_resend_data(writer_guid, reader_guid_2, seq_num, 1, locators);
        uut.on_writer_resend_data(writer_guid, reader_guid_2, seq_num, 1, locators);
        current_limitation_2 = static_cast<uint32_t>(current_limitation_2 * 0.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid_2, current_limitation_2));
        time_waiter.wait_for_next_period("both readers decrease");
    }
}

TEST(CongestionControlBasic, long_update_period)
{
    using namespace std::chrono_literals;

    constexpr uint32_t max_data_size = 64000;
    constexpr uint32_t initial_target_bps = max_data_size * 4;
    constexpr uint32_t period_duration_ms = 5000;

    using ParticipantMock = testing::StrictMock<RTPSParticipantImpl>;
    using FlowControllerMock = testing::StrictMock<GrainedFlowController>;

    ParticipantMock participant;

    // Create and register a GrainedFlowController strict mock in the factory
    FlowControllerFactory flow_controller_factory;
    FlowControllerMock* grained_fc = nullptr;
    grained_fc = flow_controller_factory.register_flow_controller_type<FlowControllerMock>(
        "GrainedFlowController",
        ThreadSettings{},
        max_data_size,
        1000);

    // Prepare writer and reader information
    GUID_t writer_guid;
    writer_guid.guidPrefix.value[0] = 0x01;
    writer_guid.entityId = 0x00000402;

    SubscriptionBuiltinTopicData reader_info;
    GUID_t reader_guid;
    reader_guid.guidPrefix.value[0] = 0x01;
    reader_guid.entityId = 0x00000401;
    reader_info.guid = reader_guid;

    // Test setup
    CongestionControlParameters params{ period_duration_ms, initial_target_bps, 1.5f, 0.5f };
    CongestionControlBasic uut;

    // Several calls to getMaxDataSize are expected throughout the whole test
    EXPECT_CALL(participant, getMaxDataSize).WillRepeatedly(testing::Return(max_data_size));

    // Initialize and register a reader
    {
        EXPECT_CALL(*grained_fc, init);
        bool init_result = uut.initialize(participant, flow_controller_factory, params);
        EXPECT_TRUE(init_result);

        EXPECT_CALL(*grained_fc, register_remote_reader(reader_guid, initial_target_bps));
        uut.notify_reader_discovery(ReaderDiscoveryStatus::DISCOVERED_READER, reader_info);
    }

    SequenceNumber_t seq_num(1);
    LocatorList empty_locators;
    LocatorSelectorEntry locators = LocatorSelectorEntry::create_fully_selected_entry(empty_locators, empty_locators);
    TimeWaiter time_waiter(period_duration_ms, 100);
    constexpr float num_periods = period_duration_ms / 1000.0f;
    uint32_t current_limitation = initial_target_bps;

    // Verify that bandwidth calculation takes into account the period duration
    {
        uint32_t acked_bytes = current_limitation;
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        time_waiter.wait_for_next_period("no increase occurs with low acked bytes over long period");
        // No calls to update_remote_reader_bytes_per_period should have occurred (verified by StrictMock)

        // Acknowledging enough bytes to cause an increase
        seq_num++;
        acked_bytes = static_cast<uint32_t>(current_limitation * num_periods);
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        current_limitation = static_cast<uint32_t>(current_limitation * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
        time_waiter.wait_for_next_period("increase occurs with sufficient acked bytes over long period");

        // Acknowledging enough bytes (80%) to cause another increase
        seq_num++;
        acked_bytes = static_cast<uint32_t>(current_limitation * num_periods * 0.8f) + 1;
        uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, acked_bytes, 100ms, locators);
        current_limitation = static_cast<uint32_t>(current_limitation * 1.5f);
        EXPECT_CALL(*grained_fc, update_remote_reader_bytes_per_period(reader_guid, current_limitation));
        time_waiter.wait_for_next_period("another increase occurs with sufficient acked bytes over long period");
    }
}

TEST(CongestionControlBasic, negative_tests)
{
    using namespace std::chrono_literals;

    constexpr uint32_t max_data_size = 64000;
    constexpr uint32_t initial_target_bps = max_data_size * 4;
    constexpr uint32_t period_duration_ms = 1200;

    using ParticipantMock = testing::StrictMock<RTPSParticipantImpl>;
    using FlowControllerMock = testing::StrictMock<GrainedFlowController>;

    {
        ParticipantMock participant;

        // Create and register a GrainedFlowController strict mock in the factory
        FlowControllerFactory flow_controller_factory;
        FlowControllerMock* grained_fc = nullptr;
        grained_fc = flow_controller_factory.register_flow_controller_type<FlowControllerMock>(
            "GrainedFlowController",
            ThreadSettings{},
            max_data_size,
            1000);

        CongestionControlParameters params{ period_duration_ms, initial_target_bps, 1.5f, 0.5f };
        CongestionControlBasic uut;

        SequenceNumber_t seq_num(1);
        LocatorList empty_locators;
        LocatorSelectorEntry locators = LocatorSelectorEntry::create_fully_selected_entry(empty_locators,
                        empty_locators);
        SubscriptionBuiltinTopicData reader_info;
        GUID_t reader_guid;
        reader_guid.guidPrefix.value[0] = 0x01;
        reader_guid.entityId = 0x00000401;
        reader_info.guid = reader_guid;
        GUID_t writer_guid;
        writer_guid.guidPrefix.value[0] = 0x01;
        writer_guid.entityId = 0x00000402;

        // Using public API without initialization should not crash
        EXPECT_NO_THROW(uut.on_writer_resend_data(writer_guid, reader_guid, seq_num, 1, locators));
        EXPECT_NO_THROW(
            uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, initial_target_bps, 100ms, locators));
        EXPECT_NO_THROW(uut.notify_reader_discovery(ReaderDiscoveryStatus::DISCOVERED_READER, reader_info));
        EXPECT_NO_THROW(uut.notify_reader_discovery(ReaderDiscoveryStatus::REMOVED_READER, reader_info));
        WriterAttributes original_attributes;
        EXPECT_NO_THROW(uut.prepare_writer_attributes(original_attributes, writer_guid.entityId, false));

        // Initialization should work properly
        EXPECT_CALL(participant, getMaxDataSize).WillRepeatedly(testing::Return(max_data_size));
        EXPECT_CALL(*grained_fc, init());
        EXPECT_TRUE(uut.initialize(participant, flow_controller_factory, params));

        // Using public API without registered readers should not crash
        EXPECT_NO_THROW(uut.on_writer_resend_data(writer_guid, reader_guid, seq_num, 1, locators));
        EXPECT_NO_THROW(
            uut.on_writer_data_acknowledged(writer_guid, reader_guid, seq_num, initial_target_bps, 100ms, locators));

        // Initializing twice should fail
        EXPECT_FALSE(uut.initialize(participant, flow_controller_factory, params));
    }

    {
        // If a GrainedFlowController with different type is registered, initialization should fail
        ParticipantMock participant;
        FlowControllerFactory flow_controller_factory;
        class CustomFlowController : public FlowController
        {
        public:

            CustomFlowController(
                    RTPSParticipantImpl* /* participant */,
                    uint32_t /* flow_controller_index */)
            {
            }

            void init() override
            {
            }

            void register_writer(
                    BaseWriter*) override
            {
            }

            void unregister_writer(
                    BaseWriter*) override
            {
            }

            bool add_new_sample(
                    BaseWriter*,
                    CacheChange_t*,
                    const std::chrono::time_point<std::chrono::steady_clock>&) override
            {
                return true;
            }

            bool add_old_sample(
                    BaseWriter*,
                    CacheChange_t*) override
            {
                return true;
            }

            bool remove_change(
                    CacheChange_t*,
                    const std::chrono::time_point<std::chrono::steady_clock>&) override
            {
                return true;
            }

            uint32_t get_max_payload() override
            {
                return 0;
            }

        };

        flow_controller_factory.register_flow_controller_type<CustomFlowController>("GrainedFlowController");
        CongestionControlParameters params{ period_duration_ms, initial_target_bps, 1.5f, 0.5f };
        CongestionControlBasic uut;
        EXPECT_CALL(participant, getMaxDataSize).WillRepeatedly(testing::Return(max_data_size));
        EXPECT_FALSE(uut.initialize(participant, flow_controller_factory, params));
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
