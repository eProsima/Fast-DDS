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

TYPED_TEST(FlowControllerPublishModes, pure_sync_publish_mode)
{
    FlowControllerDescriptor flow_controller_descr;
    FlowControllerImpl<FlowControllerPureSyncPublishMode, TypeParam> pure_sync(nullptr,
            &flow_controller_descr, 0, ThreadSettings{});
    pure_sync.init();

    // Initialize callback to get info.
    auto send_functor = [&](
        CacheChange_t*,
        RTPSMessageGroup&,
        LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->last_thread_delivering_sample = std::this_thread::get_id();
            };


    // Instantiate writers.
    BaseWriter writer1;
    BaseWriter writer2;

    // Register writers.
    pure_sync.register_writer(&writer1);

    CacheChange_t change_writer1;
    INIT_CACHE_CHANGE(change_writer1, writer1, 1);

    CacheChange_t change_writer2;
    INIT_CACHE_CHANGE(change_writer2, writer2, 1);

    // Testing add_new_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
    writer1.getMutex().lock();
    ASSERT_TRUE(pure_sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer1.getMutex().unlock();
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    EXPECT_CALL(writer2,
            deliver_sample_nts(&change_writer2, _, Ref(writer2.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
    writer2.getMutex().lock();
    ASSERT_TRUE(pure_sync.add_new_sample(&writer2, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer2.getMutex().unlock();
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    // Testing add_new_sample. Writer will not be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(DeliveryRetCode::NOT_DELIVERED)));
    writer1.getMutex().lock();
    ASSERT_FALSE(pure_sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer1.getMutex().unlock();
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    EXPECT_CALL(writer2,
            deliver_sample_nts(&change_writer2, _, Ref(writer2.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(DeliveryRetCode::NOT_DELIVERED)));
    writer2.getMutex().lock();
    ASSERT_FALSE(pure_sync.add_new_sample(&writer2, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    writer2.getMutex().unlock();
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    // Testing add_old_sample. Writers never be called.
    writer1.getMutex().lock();
    ASSERT_FALSE(pure_sync.add_old_sample(&writer1, &change_writer1));
    writer1.getMutex().unlock();
    writer2.getMutex().lock();
    ASSERT_FALSE(pure_sync.add_old_sample(&writer2, &change_writer2));
    writer2.getMutex().unlock();

    pure_sync.unregister_writer(&writer1);
}
