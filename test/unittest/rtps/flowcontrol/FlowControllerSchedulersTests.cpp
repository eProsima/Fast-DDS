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

#include <thread>

#include <gtest/gtest.h>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>

#include <rtps/flowcontrol/FlowControllerImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
std::ostream& operator <<(
        std::ostream& output,
        const BaseWriter& writer)
{
    return output << "Writer" << writer.getGuid().entityId.value[3];
}

std::ostream& operator <<(
        std::ostream& output,
        const CacheChange_t* change)
{
    return output << "change_writer" << uint16_t(change->writerGUID.entityId.value[3]) << "_" << change->sequenceNumber;
}

using namespace testing;

struct FlowControllerLimitedAsyncPublishModeMock : FlowControllerLimitedAsyncPublishMode
{
    FlowControllerLimitedAsyncPublishModeMock(
            RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor)
        : FlowControllerLimitedAsyncPublishMode(participant, descriptor)
    {
        group_mock = &group;
    }

    static RTPSMessageGroup* get_group()
    {
        return group_mock;
    }

    static RTPSMessageGroup* group_mock;
};
RTPSMessageGroup* FlowControllerLimitedAsyncPublishModeMock::group_mock = nullptr;

class FlowControllerSchedulers :  public testing::Test
{
protected:

    void TearDown() override
    {
        changes_delivered.clear();
        current_bytes_processed = 0;
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

    std::vector<CacheChange_t*> changes_delivered;

    std::mutex changes_delivered_mutex;

    std::condition_variable number_changes_delivered_cv;

    uint32_t current_bytes_processed = 0;

    bool allow_resetting = false;
};

#define INIT_CACHE_CHANGE(change, writer, seq) \
    change.writerGUID = writer.getGuid(); \
    change.writer_info.previous = nullptr; \
    change.writer_info.next = nullptr; \
    change.sequenceNumber.low = uint32_t(seq); \
    change.serializedPayload.length = 10000;

TEST_F(FlowControllerSchedulers, Fifo)
{
    FlowControllerDescriptor flow_controller_descr;
    flow_controller_descr.max_bytes_per_period = 10200;
    flow_controller_descr.period_ms = 10;
    FlowControllerImpl<FlowControllerLimitedAsyncPublishModeMock, FlowControllerFifoSchedule> async(nullptr,
            &flow_controller_descr, 0, ThreadSettings{});
    async.init();

    // Instantiate writers.
    BaseWriter writer1;
    BaseWriter writer2;
    BaseWriter writer3;
    BaseWriter writer4;
    BaseWriter writer5;
    BaseWriter writer6;
    BaseWriter writer7;
    BaseWriter writer8;
    BaseWriter writer9;
    BaseWriter writer10;

    // Initialize callback to get info.
    auto send_functor = [&](
        CacheChange_t* change,
        RTPSMessageGroup&,
        LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->current_bytes_processed += change->serializedPayload.length;
                {
                    std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                    this->changes_delivered.push_back(change);
                }
                this->number_changes_delivered_cv.notify_one();
            };

    // Register writers.
    async.register_writer(&writer1);
    async.register_writer(&writer2);
    async.register_writer(&writer3);
    async.register_writer(&writer4);
    async.register_writer(&writer5);
    async.register_writer(&writer6);
    async.register_writer(&writer7);
    async.register_writer(&writer8);
    async.register_writer(&writer9);
    async.register_writer(&writer10);

    CacheChange_t change_writer1_1;
    CacheChange_t change_writer1_2;
    CacheChange_t change_writer1_3;
    INIT_CACHE_CHANGE(change_writer1_1, writer1, 1);
    INIT_CACHE_CHANGE(change_writer1_2, writer1, 2);
    INIT_CACHE_CHANGE(change_writer1_3, writer1, 3);
    CacheChange_t change_writer2_1;
    CacheChange_t change_writer2_2;
    CacheChange_t change_writer2_3;
    INIT_CACHE_CHANGE(change_writer2_1, writer2, 1);
    INIT_CACHE_CHANGE(change_writer2_2, writer2, 2);
    INIT_CACHE_CHANGE(change_writer2_3, writer2, 3);
    CacheChange_t change_writer3_1;
    CacheChange_t change_writer3_2;
    CacheChange_t change_writer3_3;
    INIT_CACHE_CHANGE(change_writer3_1, writer3, 1);
    INIT_CACHE_CHANGE(change_writer3_2, writer3, 2);
    INIT_CACHE_CHANGE(change_writer3_3, writer3, 3);
    CacheChange_t change_writer4_1;
    CacheChange_t change_writer4_2;
    CacheChange_t change_writer4_3;
    INIT_CACHE_CHANGE(change_writer4_1, writer4, 1);
    INIT_CACHE_CHANGE(change_writer4_2, writer4, 2);
    INIT_CACHE_CHANGE(change_writer4_3, writer4, 3);
    CacheChange_t change_writer5_1;
    CacheChange_t change_writer5_2;
    CacheChange_t change_writer5_3;
    INIT_CACHE_CHANGE(change_writer5_1, writer5, 1);
    INIT_CACHE_CHANGE(change_writer5_2, writer5, 2);
    INIT_CACHE_CHANGE(change_writer5_3, writer5, 3);
    CacheChange_t change_writer6_1;
    CacheChange_t change_writer6_2;
    CacheChange_t change_writer6_3;
    INIT_CACHE_CHANGE(change_writer6_1, writer6, 1);
    INIT_CACHE_CHANGE(change_writer6_2, writer6, 2);
    INIT_CACHE_CHANGE(change_writer6_3, writer6, 3);
    CacheChange_t change_writer7_1;
    CacheChange_t change_writer7_2;
    CacheChange_t change_writer7_3;
    INIT_CACHE_CHANGE(change_writer7_1, writer7, 1);
    INIT_CACHE_CHANGE(change_writer7_2, writer7, 2);
    INIT_CACHE_CHANGE(change_writer7_3, writer7, 3);
    CacheChange_t change_writer8_1;
    CacheChange_t change_writer8_2;
    CacheChange_t change_writer8_3;
    INIT_CACHE_CHANGE(change_writer8_1, writer8, 1);
    INIT_CACHE_CHANGE(change_writer8_2, writer8, 2);
    INIT_CACHE_CHANGE(change_writer8_3, writer8, 3);
    CacheChange_t change_writer9_1;
    CacheChange_t change_writer9_2;
    CacheChange_t change_writer9_3;
    INIT_CACHE_CHANGE(change_writer9_1, writer9, 1);
    INIT_CACHE_CHANGE(change_writer9_2, writer9, 2);
    INIT_CACHE_CHANGE(change_writer9_3, writer9, 3);
    CacheChange_t change_writer10_1;
    CacheChange_t change_writer10_2;
    CacheChange_t change_writer10_3;
    INIT_CACHE_CHANGE(change_writer10_1, writer10, 1);
    INIT_CACHE_CHANGE(change_writer10_2, writer10, 2);
    INIT_CACHE_CHANGE(change_writer10_3, writer10, 3);


    {
        this->current_bytes_processed = 10100;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_3 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer10,
                deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                After(call_change_writer10_2).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    {
        this->current_bytes_processed = 10100;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_3 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer10,
                deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                After(call_change_writer9_3).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    // Register writers.
    async.unregister_writer(&writer1);
    async.unregister_writer(&writer2);
    async.unregister_writer(&writer3);
    async.unregister_writer(&writer4);
    async.unregister_writer(&writer5);
    async.unregister_writer(&writer6);
    async.unregister_writer(&writer7);
    async.unregister_writer(&writer8);
    async.unregister_writer(&writer9);
    async.unregister_writer(&writer10);
}

TEST_F(FlowControllerSchedulers, RoundRobin)
{
    FlowControllerDescriptor flow_controller_descr;
    flow_controller_descr.max_bytes_per_period = 10200;
    flow_controller_descr.period_ms = 10;
    FlowControllerImpl<FlowControllerLimitedAsyncPublishModeMock, FlowControllerRoundRobinSchedule> async(nullptr,
            &flow_controller_descr, 0, ThreadSettings{});
    async.init();

    // Instantiate writers.
    BaseWriter writer1;
    BaseWriter writer2;
    BaseWriter writer3;
    BaseWriter writer4;
    BaseWriter writer5;
    BaseWriter writer6;
    BaseWriter writer7;
    BaseWriter writer8;
    BaseWriter writer9;
    BaseWriter writer10;

    // Initialize callback to get info.
    auto send_functor = [&](
        CacheChange_t* change,
        RTPSMessageGroup&,
        LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->current_bytes_processed += change->serializedPayload.length;
                {
                    std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                    this->changes_delivered.push_back(change);
                }
                this->number_changes_delivered_cv.notify_one();
            };

    // Register writers.
    async.register_writer(&writer1);
    async.register_writer(&writer2);
    async.register_writer(&writer3);
    async.register_writer(&writer4);
    async.register_writer(&writer5);
    async.register_writer(&writer6);
    async.register_writer(&writer7);
    async.register_writer(&writer8);
    async.register_writer(&writer9);
    async.register_writer(&writer10);

    CacheChange_t change_writer1_1;
    CacheChange_t change_writer1_2;
    CacheChange_t change_writer1_3;
    INIT_CACHE_CHANGE(change_writer1_1, writer1, 1);
    INIT_CACHE_CHANGE(change_writer1_2, writer1, 2);
    INIT_CACHE_CHANGE(change_writer1_3, writer1, 3);
    CacheChange_t change_writer2_1;
    CacheChange_t change_writer2_2;
    CacheChange_t change_writer2_3;
    INIT_CACHE_CHANGE(change_writer2_1, writer2, 1);
    INIT_CACHE_CHANGE(change_writer2_2, writer2, 2);
    INIT_CACHE_CHANGE(change_writer2_3, writer2, 3);
    CacheChange_t change_writer3_1;
    CacheChange_t change_writer3_2;
    CacheChange_t change_writer3_3;
    INIT_CACHE_CHANGE(change_writer3_1, writer3, 1);
    INIT_CACHE_CHANGE(change_writer3_2, writer3, 2);
    INIT_CACHE_CHANGE(change_writer3_3, writer3, 3);
    CacheChange_t change_writer4_1;
    CacheChange_t change_writer4_2;
    CacheChange_t change_writer4_3;
    INIT_CACHE_CHANGE(change_writer4_1, writer4, 1);
    INIT_CACHE_CHANGE(change_writer4_2, writer4, 2);
    INIT_CACHE_CHANGE(change_writer4_3, writer4, 3);
    CacheChange_t change_writer5_1;
    CacheChange_t change_writer5_2;
    CacheChange_t change_writer5_3;
    INIT_CACHE_CHANGE(change_writer5_1, writer5, 1);
    INIT_CACHE_CHANGE(change_writer5_2, writer5, 2);
    INIT_CACHE_CHANGE(change_writer5_3, writer5, 3);
    CacheChange_t change_writer6_1;
    CacheChange_t change_writer6_2;
    CacheChange_t change_writer6_3;
    INIT_CACHE_CHANGE(change_writer6_1, writer6, 1);
    INIT_CACHE_CHANGE(change_writer6_2, writer6, 2);
    INIT_CACHE_CHANGE(change_writer6_3, writer6, 3);
    CacheChange_t change_writer7_1;
    CacheChange_t change_writer7_2;
    CacheChange_t change_writer7_3;
    INIT_CACHE_CHANGE(change_writer7_1, writer7, 1);
    INIT_CACHE_CHANGE(change_writer7_2, writer7, 2);
    INIT_CACHE_CHANGE(change_writer7_3, writer7, 3);
    CacheChange_t change_writer8_1;
    CacheChange_t change_writer8_2;
    CacheChange_t change_writer8_3;
    INIT_CACHE_CHANGE(change_writer8_1, writer8, 1);
    INIT_CACHE_CHANGE(change_writer8_2, writer8, 2);
    INIT_CACHE_CHANGE(change_writer8_3, writer8, 3);
    CacheChange_t change_writer9_1;
    CacheChange_t change_writer9_2;
    CacheChange_t change_writer9_3;
    INIT_CACHE_CHANGE(change_writer9_1, writer9, 1);
    INIT_CACHE_CHANGE(change_writer9_2, writer9, 2);
    INIT_CACHE_CHANGE(change_writer9_3, writer9, 3);
    CacheChange_t change_writer10_1;
    CacheChange_t change_writer10_2;
    CacheChange_t change_writer10_3;
    INIT_CACHE_CHANGE(change_writer10_1, writer10, 1);
    INIT_CACHE_CHANGE(change_writer10_2, writer10, 2);
    INIT_CACHE_CHANGE(change_writer10_3, writer10, 3);


    {
        this->current_bytes_processed = 10100;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_3 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer10,
                deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                After(call_change_writer9_3).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    {
        this->current_bytes_processed = 10100;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_3 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer10,
                deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                After(call_change_writer9_3).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    // Register writers.
    async.unregister_writer(&writer1);
    async.unregister_writer(&writer2);
    async.unregister_writer(&writer3);
    async.unregister_writer(&writer4);
    async.unregister_writer(&writer5);
    async.unregister_writer(&writer6);
    async.unregister_writer(&writer7);
    async.unregister_writer(&writer8);
    async.unregister_writer(&writer9);
    async.unregister_writer(&writer10);
}

TEST_F(FlowControllerSchedulers, HighPriority)
{
    FlowControllerDescriptor flow_controller_descr;
    flow_controller_descr.max_bytes_per_period = 10200;
    flow_controller_descr.period_ms = 10;
    FlowControllerImpl<FlowControllerLimitedAsyncPublishModeMock, FlowControllerHighPrioritySchedule> async(nullptr,
            &flow_controller_descr, 0, ThreadSettings{});
    async.init();

    // Instantiate writers.
    Property priority_property;
    BaseWriter writer1;
    priority_property.name("fastdds.sfc.priority");
    priority_property.value("1");
    writer1.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer2;
    priority_property.value("2");
    writer2.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer3;
    priority_property.value("3");
    writer3.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer4;
    priority_property.value("4");
    writer4.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer5;
    priority_property.value("5");
    writer5.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer6;
    priority_property.value("6");
    writer6.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer7;
    priority_property.value("7");
    writer7.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer8;
    priority_property.value("8");
    writer8.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer9;
    priority_property.value("9");
    writer9.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer10;
    priority_property.value("10");
    writer10.m_att.endpoint.properties.properties().push_back(priority_property);

    // Initialize callback to get info.
    auto send_functor = [&](
        CacheChange_t* change,
        RTPSMessageGroup&,
        LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->current_bytes_processed += change->serializedPayload.length;
                {
                    std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                    this->changes_delivered.push_back(change);
                }
                this->number_changes_delivered_cv.notify_one();
            };

    // Register writers.
    async.register_writer(&writer1);
    async.register_writer(&writer2);
    async.register_writer(&writer3);
    async.register_writer(&writer4);
    async.register_writer(&writer5);
    async.register_writer(&writer6);
    async.register_writer(&writer7);
    async.register_writer(&writer8);
    async.register_writer(&writer9);
    async.register_writer(&writer10);

    CacheChange_t change_writer1_1;
    CacheChange_t change_writer1_2;
    CacheChange_t change_writer1_3;
    INIT_CACHE_CHANGE(change_writer1_1, writer1, 1);
    INIT_CACHE_CHANGE(change_writer1_2, writer1, 2);
    INIT_CACHE_CHANGE(change_writer1_3, writer1, 3);
    CacheChange_t change_writer2_1;
    CacheChange_t change_writer2_2;
    CacheChange_t change_writer2_3;
    INIT_CACHE_CHANGE(change_writer2_1, writer2, 1);
    INIT_CACHE_CHANGE(change_writer2_2, writer2, 2);
    INIT_CACHE_CHANGE(change_writer2_3, writer2, 3);
    CacheChange_t change_writer3_1;
    CacheChange_t change_writer3_2;
    CacheChange_t change_writer3_3;
    INIT_CACHE_CHANGE(change_writer3_1, writer3, 1);
    INIT_CACHE_CHANGE(change_writer3_2, writer3, 2);
    INIT_CACHE_CHANGE(change_writer3_3, writer3, 3);
    CacheChange_t change_writer4_1;
    CacheChange_t change_writer4_2;
    CacheChange_t change_writer4_3;
    INIT_CACHE_CHANGE(change_writer4_1, writer4, 1);
    INIT_CACHE_CHANGE(change_writer4_2, writer4, 2);
    INIT_CACHE_CHANGE(change_writer4_3, writer4, 3);
    CacheChange_t change_writer5_1;
    CacheChange_t change_writer5_2;
    CacheChange_t change_writer5_3;
    INIT_CACHE_CHANGE(change_writer5_1, writer5, 1);
    INIT_CACHE_CHANGE(change_writer5_2, writer5, 2);
    INIT_CACHE_CHANGE(change_writer5_3, writer5, 3);
    CacheChange_t change_writer6_1;
    CacheChange_t change_writer6_2;
    CacheChange_t change_writer6_3;
    INIT_CACHE_CHANGE(change_writer6_1, writer6, 1);
    INIT_CACHE_CHANGE(change_writer6_2, writer6, 2);
    INIT_CACHE_CHANGE(change_writer6_3, writer6, 3);
    CacheChange_t change_writer7_1;
    CacheChange_t change_writer7_2;
    CacheChange_t change_writer7_3;
    INIT_CACHE_CHANGE(change_writer7_1, writer7, 1);
    INIT_CACHE_CHANGE(change_writer7_2, writer7, 2);
    INIT_CACHE_CHANGE(change_writer7_3, writer7, 3);
    CacheChange_t change_writer8_1;
    CacheChange_t change_writer8_2;
    CacheChange_t change_writer8_3;
    INIT_CACHE_CHANGE(change_writer8_1, writer8, 1);
    INIT_CACHE_CHANGE(change_writer8_2, writer8, 2);
    INIT_CACHE_CHANGE(change_writer8_3, writer8, 3);
    CacheChange_t change_writer9_1;
    CacheChange_t change_writer9_2;
    CacheChange_t change_writer9_3;
    INIT_CACHE_CHANGE(change_writer9_1, writer9, 1);
    INIT_CACHE_CHANGE(change_writer9_2, writer9, 2);
    INIT_CACHE_CHANGE(change_writer9_3, writer9, 3);
    CacheChange_t change_writer10_1;
    CacheChange_t change_writer10_2;
    CacheChange_t change_writer10_3;
    INIT_CACHE_CHANGE(change_writer10_1, writer10, 1);
    INIT_CACHE_CHANGE(change_writer10_2, writer10, 2);
    INIT_CACHE_CHANGE(change_writer10_3, writer10, 3);


    {
        this->current_bytes_processed = 10100;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_3 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer10,
                deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                After(call_change_writer10_2).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    {
        this->current_bytes_processed = 10100;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_3 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer7_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer10,
                deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                After(call_change_writer10_2).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    // Register writers.
    async.unregister_writer(&writer1);
    async.unregister_writer(&writer2);
    async.unregister_writer(&writer3);
    async.unregister_writer(&writer4);
    async.unregister_writer(&writer5);
    async.unregister_writer(&writer6);
    async.unregister_writer(&writer7);
    async.unregister_writer(&writer8);
    async.unregister_writer(&writer9);
    async.unregister_writer(&writer10);
}

TEST_F(FlowControllerSchedulers, PriorityWithReservation)
{
    FlowControllerDescriptor flow_controller_descr;
    flow_controller_descr.max_bytes_per_period = 102000;
    flow_controller_descr.period_ms = 10;
    FlowControllerImpl<FlowControllerLimitedAsyncPublishModeMock,
            FlowControllerPriorityWithReservationSchedule> async(nullptr,
            &flow_controller_descr, 0, ThreadSettings{});
    async.init();

    // Instantiate writers.
    Property priority_property;
    BaseWriter writer1;
    priority_property.name("fastdds.sfc.priority");
    priority_property.value("1");
    writer1.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer2;
    priority_property.value("2");
    writer2.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer3;
    priority_property.value("3");
    writer3.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer4;
    priority_property.value("4");
    writer4.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer5;
    priority_property.value("5");
    writer5.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer6;
    priority_property.value("6");
    writer6.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer7;
    priority_property.value("7");
    writer7.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer8;
    priority_property.value("8");
    writer8.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer9;
    priority_property.value("9");
    writer9.m_att.endpoint.properties.properties().push_back(priority_property);
    BaseWriter writer10;
    priority_property.value("10");
    writer10.m_att.endpoint.properties.properties().push_back(priority_property);
    Property reservation_property;
    reservation_property.name("fastdds.sfc.bandwidth_reservation");
    reservation_property.value("30");
    writer10.m_att.endpoint.properties.properties().push_back(reservation_property);
    reservation_property.value("20");
    writer9.m_att.endpoint.properties.properties().push_back(reservation_property);
    reservation_property.value("10");
    writer8.m_att.endpoint.properties.properties().push_back(reservation_property);

    // Initialize callback to get info.
    auto send_functor = [&](
        CacheChange_t* change,
        RTPSMessageGroup&,
        LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->current_bytes_processed += change->serializedPayload.length;
                {
                    std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                    this->changes_delivered.push_back(change);
                }
                this->number_changes_delivered_cv.notify_one();
            };

    // Register writers.
    async.register_writer(&writer1);
    async.register_writer(&writer2);
    async.register_writer(&writer3);
    async.register_writer(&writer4);
    async.register_writer(&writer5);
    async.register_writer(&writer6);
    async.register_writer(&writer7);
    async.register_writer(&writer8);
    async.register_writer(&writer9);
    async.register_writer(&writer10);

    CacheChange_t change_writer1_1;
    CacheChange_t change_writer1_2;
    CacheChange_t change_writer1_3;
    INIT_CACHE_CHANGE(change_writer1_1, writer1, 1);
    INIT_CACHE_CHANGE(change_writer1_2, writer1, 2);
    INIT_CACHE_CHANGE(change_writer1_3, writer1, 3);
    CacheChange_t change_writer2_1;
    CacheChange_t change_writer2_2;
    CacheChange_t change_writer2_3;
    INIT_CACHE_CHANGE(change_writer2_1, writer2, 1);
    INIT_CACHE_CHANGE(change_writer2_2, writer2, 2);
    INIT_CACHE_CHANGE(change_writer2_3, writer2, 3);
    CacheChange_t change_writer3_1;
    CacheChange_t change_writer3_2;
    CacheChange_t change_writer3_3;
    INIT_CACHE_CHANGE(change_writer3_1, writer3, 1);
    INIT_CACHE_CHANGE(change_writer3_2, writer3, 2);
    INIT_CACHE_CHANGE(change_writer3_3, writer3, 3);
    CacheChange_t change_writer4_1;
    CacheChange_t change_writer4_2;
    CacheChange_t change_writer4_3;
    INIT_CACHE_CHANGE(change_writer4_1, writer4, 1);
    INIT_CACHE_CHANGE(change_writer4_2, writer4, 2);
    INIT_CACHE_CHANGE(change_writer4_3, writer4, 3);
    CacheChange_t change_writer5_1;
    CacheChange_t change_writer5_2;
    CacheChange_t change_writer5_3;
    INIT_CACHE_CHANGE(change_writer5_1, writer5, 1);
    INIT_CACHE_CHANGE(change_writer5_2, writer5, 2);
    INIT_CACHE_CHANGE(change_writer5_3, writer5, 3);
    CacheChange_t change_writer6_1;
    CacheChange_t change_writer6_2;
    CacheChange_t change_writer6_3;
    INIT_CACHE_CHANGE(change_writer6_1, writer6, 1);
    INIT_CACHE_CHANGE(change_writer6_2, writer6, 2);
    INIT_CACHE_CHANGE(change_writer6_3, writer6, 3);
    CacheChange_t change_writer7_1;
    CacheChange_t change_writer7_2;
    CacheChange_t change_writer7_3;
    INIT_CACHE_CHANGE(change_writer7_1, writer7, 1);
    INIT_CACHE_CHANGE(change_writer7_2, writer7, 2);
    INIT_CACHE_CHANGE(change_writer7_3, writer7, 3);
    CacheChange_t change_writer8_1;
    CacheChange_t change_writer8_2;
    CacheChange_t change_writer8_3;
    INIT_CACHE_CHANGE(change_writer8_1, writer8, 1);
    INIT_CACHE_CHANGE(change_writer8_2, writer8, 2);
    INIT_CACHE_CHANGE(change_writer8_3, writer8, 3);
    CacheChange_t change_writer9_1;
    CacheChange_t change_writer9_2;
    CacheChange_t change_writer9_3;
    INIT_CACHE_CHANGE(change_writer9_1, writer9, 1);
    INIT_CACHE_CHANGE(change_writer9_2, writer9, 2);
    INIT_CACHE_CHANGE(change_writer9_3, writer9, 3);
    CacheChange_t change_writer10_1;
    CacheChange_t change_writer10_2;
    CacheChange_t change_writer10_3;
    INIT_CACHE_CHANGE(change_writer10_1, writer10, 1);
    INIT_CACHE_CHANGE(change_writer10_2, writer10, 2);
    INIT_CACHE_CHANGE(change_writer10_3, writer10, 3);


    {
        this->current_bytes_processed = 101000;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_3 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer10_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer9_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer7,
                deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                After(call_change_writer7_2).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Makes sure it start a new period.

    {
        this->current_bytes_processed = 101000;
        this->allow_resetting = false;
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                get_current_bytes_processed()).WillRepeatedly(
            ReturnPointee(&this->current_bytes_processed));
        EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                reset_current_bytes_processed()).WillRepeatedly([&]()
                {
                    if (this->allow_resetting)
                    {
                        this->current_bytes_processed = 0;
                    }
                });
        auto& call_change_writer8_1 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_1, _, Ref(writer8.async_locator_selector_), _)).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_1 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_1, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_2 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_2, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer9_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_1 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_1, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer9_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_2 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_2, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer10_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer10_3 = EXPECT_CALL(writer10,
                        deliver_sample_nts(&change_writer10_3, _, Ref(writer10.async_locator_selector_), _)).
                        After(call_change_writer10_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_1 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_1, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer10_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_2 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_2, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer1_3 = EXPECT_CALL(writer1,
                        deliver_sample_nts(&change_writer1_3, _, Ref(writer1.async_locator_selector_), _)).
                        After(call_change_writer1_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_1 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_1, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer1_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_2 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_2, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer2_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer9_3 = EXPECT_CALL(writer9,
                        deliver_sample_nts(&change_writer9_3, _, Ref(writer9.async_locator_selector_), _)).
                        After(call_change_writer8_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_2 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_2, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer9_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer2_3 = EXPECT_CALL(writer2,
                        deliver_sample_nts(&change_writer2_3, _, Ref(writer2.async_locator_selector_), _)).
                        After(call_change_writer2_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_1 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_1, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer2_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_2 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_2, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer3_3 = EXPECT_CALL(writer3,
                        deliver_sample_nts(&change_writer3_3, _, Ref(writer3.async_locator_selector_), _)).
                        After(call_change_writer3_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_1 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_1, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer3_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_2 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_2, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer4_3 = EXPECT_CALL(writer4,
                        deliver_sample_nts(&change_writer4_3, _, Ref(writer4.async_locator_selector_), _)).
                        After(call_change_writer4_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer8_3 = EXPECT_CALL(writer8,
                        deliver_sample_nts(&change_writer8_3, _, Ref(writer8.async_locator_selector_), _)).
                        After(call_change_writer4_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_1 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_1, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer8_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_2 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_2, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer5_3 = EXPECT_CALL(writer5,
                        deliver_sample_nts(&change_writer5_3, _, Ref(writer5.async_locator_selector_), _)).
                        After(call_change_writer5_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_1 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_1, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer5_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_2 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_2, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer6_3 = EXPECT_CALL(writer6,
                        deliver_sample_nts(&change_writer6_3, _, Ref(writer6.async_locator_selector_), _)).
                        After(call_change_writer6_2).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_1 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_1, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer6_3).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        auto& call_change_writer7_2 = EXPECT_CALL(writer7,
                        deliver_sample_nts(&change_writer7_2, _, Ref(writer7.async_locator_selector_), _)).
                        After(call_change_writer7_1).
                        WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        EXPECT_CALL(writer7,
                deliver_sample_nts(&change_writer7_3, _, Ref(writer7.async_locator_selector_), _)).
                After(call_change_writer7_2).
                WillOnce(DoAll(send_functor, Return(DeliveryRetCode::DELIVERED)));
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_1,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_2,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        writer1.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer1.getMutex().unlock();
        writer2.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer2, &change_writer2_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer2.getMutex().unlock();
        writer3.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer3, &change_writer3_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer3.getMutex().unlock();
        writer4.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer4, &change_writer4_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer4.getMutex().unlock();
        writer5.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer5, &change_writer5_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer5.getMutex().unlock();
        writer6.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer6, &change_writer6_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer6.getMutex().unlock();
        writer7.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer7, &change_writer7_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer7.getMutex().unlock();
        writer8.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer8, &change_writer8_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer8.getMutex().unlock();
        writer9.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer9, &change_writer9_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer9.getMutex().unlock();
        writer10.getMutex().lock();
        ASSERT_TRUE(async.add_new_sample(&writer10, &change_writer10_3,
                std::chrono::steady_clock::now() + std::chrono::hours(24)));
        writer10.getMutex().unlock();
        this->allow_resetting = true;
        this->wait_changes_was_delivered(30);
        this->changes_delivered.clear();
        this->current_bytes_processed = 0;
    }

    // Register writers.
    async.unregister_writer(&writer1);
    async.unregister_writer(&writer2);
    async.unregister_writer(&writer3);
    async.unregister_writer(&writer4);
    async.unregister_writer(&writer5);
    async.unregister_writer(&writer6);
    async.unregister_writer(&writer7);
    async.unregister_writer(&writer8);
    async.unregister_writer(&writer9);
    async.unregister_writer(&writer10);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
