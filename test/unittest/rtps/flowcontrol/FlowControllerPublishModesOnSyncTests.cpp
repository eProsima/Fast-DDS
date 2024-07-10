// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "FlowControllerPublishModesTests.hpp"

#include <thread>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

using namespace eprosima::fastdds::rtps;
using namespace testing;

TYPED_TEST(FlowControllerPublishModes, sync_publish_mode)
{
    FlowControllerDescriptor flow_controller_descr;
    FlowControllerImpl<FlowControllerSyncPublishMode, TypeParam> sync(nullptr,
            &flow_controller_descr, 0, ThreadSettings{});
    sync.init();

    // Instantiate writers.
    BaseWriter writer1;
    BaseWriter writer2;

    // Initialize callback to get info.
    auto send_functor_adding = [&](
        CacheChange_t* change,
        RTPSMessageGroup&,
        LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->last_thread_delivering_sample = std::this_thread::get_id();
                {
                    std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                    this->changes_delivered.push_back(change);
                }
                this->number_changes_delivered_cv.notify_one();
            };
    auto send_functor_not_adding = [&](
        CacheChange_t*,
        RTPSMessageGroup&,
        LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->last_thread_delivering_sample = std::this_thread::get_id();
            };

    // Register writers.
    sync.register_writer(&writer1);

    CacheChange_t change_writer1;
    INIT_CACHE_CHANGE(change_writer1, writer1, 1);

    CacheChange_t change_writer2;
    INIT_CACHE_CHANGE(change_writer2, writer2, 1);

    // Testing add_new_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor_not_adding, Return(DeliveryRetCode::DELIVERED)));
    writer1.getMutex().lock();
    ASSERT_TRUE(sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer1.getMutex().unlock();
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    EXPECT_CALL(writer2,
            deliver_sample_nts(&change_writer2, _, Ref(writer2.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor_not_adding, Return(DeliveryRetCode::DELIVERED)));
    writer2.getMutex().lock();
    ASSERT_TRUE(sync.add_new_sample(&writer2, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer2.getMutex().unlock();
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    // Testing add_new_sample. Writer will not be able to deliver it.
    auto& fail_call = EXPECT_CALL(writer1,
                    deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
                    WillOnce(Return(DeliveryRetCode::NOT_DELIVERED));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).After(fail_call).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    writer1.getMutex().lock();
    ASSERT_TRUE(sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer1.getMutex().unlock();
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    writer1.getMutex().lock();
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer1));
    writer1.getMutex().unlock();
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will not be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(DeliveryRetCode::NOT_DELIVERED)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    writer1.getMutex().lock();
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer1));
    writer1.getMutex().unlock();
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample with a change already enqueued.
    change_writer1.writer_info.previous = (CacheChange_t*)1;
    change_writer1.writer_info.next = (CacheChange_t*)1;

    // Send 10 samples using add_old_sample.
    INIT_CACHE_CHANGE(change_writer1, writer1, 1);
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

    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED)));
    writer1.getMutex().lock();
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer1));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer2));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer3));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer4));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer10));
    writer1.getMutex().unlock();
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Remove changes
    EXPECT_CALL(writer1,
            deliver_sample_nts(_, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(DeliveryRetCode::DELIVERED))).
            WillRepeatedly(Return(DeliveryRetCode::NOT_DELIVERED));
    writer1.getMutex().lock();
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer1));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer2));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer3));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer4));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer10));
    writer1.getMutex().unlock();
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();
    writer1.getMutex().lock();
    ASSERT_TRUE(nullptr == change_writer1.writer_info.next &&
            nullptr == change_writer1.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer2.writer_info.next &&
            nullptr != change_writer2.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer3.writer_info.next &&
            nullptr != change_writer3.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer4.writer_info.next &&
            nullptr != change_writer4.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer5.writer_info.next &&
            nullptr != change_writer5.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer6.writer_info.next &&
            nullptr != change_writer6.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer7.writer_info.next &&
            nullptr != change_writer7.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer8.writer_info.next &&
            nullptr != change_writer8.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer9.writer_info.next &&
            nullptr != change_writer9.writer_info.previous);
    ASSERT_TRUE(nullptr != change_writer10.writer_info.next &&
            nullptr != change_writer10.writer_info.previous);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer10, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer9, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer8, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer7, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer6, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer5, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer4, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer3, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer2, std::chrono::steady_clock::now() + std::chrono::hours(24));
    writer1.getMutex().unlock();
    ASSERT_TRUE(nullptr == change_writer2.writer_info.next &&
            nullptr == change_writer2.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer3.writer_info.next &&
            nullptr == change_writer3.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer4.writer_info.next &&
            nullptr == change_writer4.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer5.writer_info.next &&
            nullptr == change_writer5.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer6.writer_info.next &&
            nullptr == change_writer6.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer7.writer_info.next &&
            nullptr == change_writer7.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer8.writer_info.next &&
            nullptr == change_writer8.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer9.writer_info.next &&
            nullptr == change_writer9.writer_info.previous);
    ASSERT_TRUE(nullptr == change_writer10.writer_info.next &&
            nullptr == change_writer10.writer_info.previous);

    sync.unregister_writer(&writer1);
}
