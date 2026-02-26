/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <thread>

#include <gtest/gtest.h>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

#include <rtps/flowcontrol/GrainedFlowController.hpp>

using namespace eprosima::fastdds::rtps;
using namespace testing;

#define INIT_CACHE_CHANGE(change, writer, seq) \
    change.writerGUID = writer.getGuid(); \
    change.writer_info.previous = nullptr; \
    change.writer_info.next = nullptr; \
    change.sequenceNumber.low = uint32_t(seq); \
    change.serializedPayload.length = 10000;

class GrainedFlowControllerTests :  public testing::Test
{
protected:

    void TearDown() override
    {
        changes_delivered.clear();
    }

    void wait_changes_was_delivered(
            size_t number_of_changes)
    {
        std::unique_lock<std::mutex> lock(changes_delivered_mutex);
        number_changes_delivered_cv.wait(lock, [&]()
                {
                    return number_of_changes == changes_delivered.size();
                });
    }

    std::thread::id last_thread_delivering_sample;

    std::vector<eprosima::fastdds::rtps::CacheChange_t*> changes_delivered;

    std::vector<eprosima::fastdds::rtps::GUID_t> allowed_readers;

    std::mutex changes_delivered_mutex;

    std::condition_variable number_changes_delivered_cv;
};

TEST_F(GrainedFlowControllerTests, grained_async_publish_mode)
{
    GrainedFlowController flow_controller(nullptr, 0, ThreadSettings{}, 10001, 100);
    flow_controller.init();

    // Instantiate writers.
    BaseWriter writer1;
    BaseWriter writer2;

    // Initialize callback to get info.
    auto send_functor = [&](
        CacheChange_t* change,
        RTPSMessageGroup& group,
        LocatorSelectorSender& sender,
        const std::chrono::time_point<std::chrono::steady_clock>&) -> DeliveryRetCode
            {
                std::cout << "Sample delivered: " << change->sequenceNumber << " by thread "
                          << std::this_thread::get_id() << std::endl;
                this->last_thread_delivering_sample = std::this_thread::get_id();
                if (!group.limitation_->data_exceeds_limitation(*change,
                        change->serializedPayload.length, 0, sender))
                {
                    group.limitation_->add_sent_bytes_by_group(
                        change->serializedPayload.length, sender);
                    {
                        std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                        this->changes_delivered.push_back(change);
                        sender.locator_selector.for_every_entry([&](LocatorSelectorEntry* entry, size_t)
                                {
                                    if (entry->enabled && entry->allowed_to_send)
                                    {
                                        this->allowed_readers.push_back(entry->remote_guid);
                                    }
                                    entry->allowed_to_send = true;
                                    return true;
                                });
                        sender.locator_selector.initial_allow_to_send(true);
                    }
                    this->number_changes_delivered_cv.notify_one();
                    return DeliveryRetCode::DELIVERED;
                }

                return DeliveryRetCode::EXCEEDED_LIMIT;
            };

    // Register writers and remote readers
    writer1.async_locator_selector_.locator_selector.add_entry(new LocatorSelectorEntry(0, 0));
    writer1.async_locator_selector_.locator_selector.add_entry(new LocatorSelectorEntry(0, 0));
    writer1.async_locator_selector_.locator_selector.for_every_entry([&](LocatorSelectorEntry* entry, size_t)
            {
                static uint32_t index = 1;
                entry->remote_guid = GUID_t({}, index++);
                entry->enabled = true;
                return true;
            });
    flow_controller.register_writer(&writer1);
    flow_controller.register_remote_reader({{}, 1}, 10);
    flow_controller.register_remote_reader({{}, 2}, 10);
    writer2.async_locator_selector_.locator_selector.add_entry(new LocatorSelectorEntry(0, 0));
    writer2.async_locator_selector_.locator_selector.add_entry(new LocatorSelectorEntry(0, 0));
    writer2.async_locator_selector_.locator_selector.for_every_entry([&](LocatorSelectorEntry* entry, size_t)
            {
                static uint32_t index = 1;
                entry->remote_guid = GUID_t({}, index++);
                entry->enabled = true;
                return true;
            });
    flow_controller.register_writer(&writer2);
    flow_controller.register_remote_reader({{}, 1}, 10);
    flow_controller.register_remote_reader({{}, 2}, 10);


    CacheChange_t change_writer1;
    INIT_CACHE_CHANGE(change_writer1, writer1, 1);

    // Testing add_new_sample. Writer will not be able to deliver it because no reader has bytes enough.
    writer1.getMutex().lock();
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer1.getMutex().unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(0 == this->changes_delivered.size());

    EXPECT_CALL(writer1,
            deliver_sample_nts(_, _, Ref(writer1.async_locator_selector_), _)).
            WillRepeatedly(send_functor);

    // Testing update_remote_reader_bytes_per_period. Writer will be able to deliver it only to reader1.
    flow_controller.update_remote_reader_bytes_per_period({{}, 1}, 10001);

    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    ASSERT_EQ(1u, this->allowed_readers.size());
    EXPECT_EQ(eprosima::fastdds::rtps::GUID_t({}, 1), this->allowed_readers.at(0));
    this->changes_delivered.clear();
    this->allowed_readers.clear();

    // Testing add_old_sample. Writer will not be able to deliver it because no reader has bytes enough.
    flow_controller.update_remote_reader_bytes_per_period({{}, 1}, 10);
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer1));

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(0 == this->changes_delivered.size());

    // Testing update_remote_reader_bytes_per_period. Writer will be able to deliver it only to reader1.
    flow_controller.update_remote_reader_bytes_per_period({{}, 2}, 10001);

    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    ASSERT_EQ(1u, this->allowed_readers.size());
    EXPECT_EQ(eprosima::fastdds::rtps::GUID_t({}, 2), this->allowed_readers.at(0));
    this->changes_delivered.clear();
    this->allowed_readers.clear();

    // Send 10 samples using add_new_sample.
    flow_controller.update_remote_reader_bytes_per_period({{}, 2}, 10);

    CacheChange_t change_writer2;
    INIT_CACHE_CHANGE(change_writer2, writer1, 1);
    CacheChange_t change_writer3;
    INIT_CACHE_CHANGE(change_writer3, writer1, 1);
    CacheChange_t change_writer4;
    INIT_CACHE_CHANGE(change_writer4, writer1, 1);
    CacheChange_t change_writer5;
    INIT_CACHE_CHANGE(change_writer5, writer1, 1);
    CacheChange_t change_writer6;
    INIT_CACHE_CHANGE(change_writer6, writer1, 1);
    CacheChange_t change_writer7;
    INIT_CACHE_CHANGE(change_writer7, writer1, 1);
    CacheChange_t change_writer8;
    INIT_CACHE_CHANGE(change_writer8, writer1, 1);
    CacheChange_t change_writer9;
    INIT_CACHE_CHANGE(change_writer9, writer1, 1);
    CacheChange_t change_writer10;
    INIT_CACHE_CHANGE(change_writer10, writer1, 1);

    writer1.getMutex().lock();
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer3,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer4,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer5,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer6,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer7,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer8,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer9,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(flow_controller.add_new_sample(&writer1, &change_writer10,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer1.getMutex().unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(0 == this->changes_delivered.size());

    flow_controller.update_remote_reader_bytes_per_period({{}, 1}, 10001);
    flow_controller.update_remote_reader_bytes_per_period({{}, 2}, 10001);

    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    ASSERT_EQ(20u, this->allowed_readers.size());
    this->changes_delivered.clear();
    this->allowed_readers.clear();

    // Send 10 samples using add_old_sample.
    flow_controller.update_remote_reader_bytes_per_period({{}, 1}, 10);
    flow_controller.update_remote_reader_bytes_per_period({{}, 2}, 10);

    writer1.getMutex().lock();
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer1));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer2));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer3));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer4));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(flow_controller.add_old_sample(&writer1, &change_writer10));
    writer1.getMutex().unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(0 == this->changes_delivered.size());

    flow_controller.update_remote_reader_bytes_per_period({{}, 1}, 10001);
    flow_controller.update_remote_reader_bytes_per_period({{}, 2}, 10001);

    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    ASSERT_EQ(20u, this->allowed_readers.size());
    this->changes_delivered.clear();
    this->allowed_readers.clear();

    // Unregister remote readers and writers
    flow_controller.unregister_remote_reader({{}, 1});
    flow_controller.unregister_remote_reader({{}, 2});
    flow_controller.unregister_writer(&writer1);
    flow_controller.unregister_remote_reader({{}, 1});
    flow_controller.unregister_remote_reader({{}, 2});
    flow_controller.unregister_writer(&writer2);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
