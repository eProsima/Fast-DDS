#include <rtps/flowcontrol/FlowControllerImpl.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>

using namespace eprosima::fastdds::rtps;
using namespace testing;

template<typename T>
class FlowControllerPublishModes :  public testing::Test
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

    std::vector<eprosima::fastrtps::rtps::CacheChange_t*> changes_delivered;

    std::mutex changes_delivered_mutex;

    std::condition_variable number_changes_delivered_cv;
};

using Schedulers = Types<FlowControllerFifoSchedule,
                FlowControllerRoundRobinSchedule,
                FlowControllerHighPrioritySchedule,
                FlowControllerPriorityWithReservationSchedule>;

TYPED_TEST_SUITE(FlowControllerPublishModes, Schedulers);

#define INIT_CACHE_CHANGE(change, writer) \
    change.writerGUID = writer.getGuid(); \
    change.writer_info.previous = nullptr; \
    change.writer_info.next = nullptr; \
    change.serializedPayload.length = 10000;

TYPED_TEST(FlowControllerPublishModes, pure_sync_publish_mode)
{
    FlowControllerDescriptor flow_controller_descr;
    FlowControllerImpl<FlowControllerPureSyncPublishMode, TypeParam> pure_sync(nullptr,
            &flow_controller_descr);
    pure_sync.init();

    // Initialize callback to get info.
    auto send_functor = [&](
        eprosima::fastrtps::rtps::CacheChange_t*,
        eprosima::fastrtps::rtps::RTPSMessageGroup&,
        eprosima::fastrtps::rtps::LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->last_thread_delivering_sample = std::this_thread::get_id();
            };


    // Instantiate writers.
    eprosima::fastrtps::rtps::RTPSWriter writer1;
    eprosima::fastrtps::rtps::RTPSWriter writer2;

    // Register writers.
    pure_sync.register_writer(&writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer1;
    INIT_CACHE_CHANGE(change_writer1, writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer2;
    INIT_CACHE_CHANGE(change_writer2, writer2);

    // Testing add_new_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(pure_sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    EXPECT_CALL(writer2,
            deliver_sample_nts(&change_writer2, _, Ref(writer2.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(pure_sync.add_new_sample(&writer2, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    // Testing add_new_sample. Writer will not be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED)));
    ASSERT_FALSE(pure_sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    EXPECT_CALL(writer2,
            deliver_sample_nts(&change_writer2, _, Ref(writer2.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED)));
    ASSERT_FALSE(pure_sync.add_new_sample(&writer2, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    // Testing add_old_sample. Writers never be called.
    ASSERT_FALSE(pure_sync.add_old_sample(&writer1, &change_writer1));
    ASSERT_FALSE(pure_sync.add_old_sample(&writer2, &change_writer2));

    pure_sync.unregister_writer(&writer1);
}

TYPED_TEST(FlowControllerPublishModes, sync_publish_mode)
{
    FlowControllerDescriptor flow_controller_descr;
    FlowControllerImpl<FlowControllerSyncPublishMode, TypeParam> sync(nullptr,
            &flow_controller_descr);
    sync.init();

    // Instantiate writers.
    eprosima::fastrtps::rtps::RTPSWriter writer1;
    eprosima::fastrtps::rtps::RTPSWriter writer2;

    // Initialize callback to get info.
    auto send_functor_adding = [&](
        eprosima::fastrtps::rtps::CacheChange_t* change,
        eprosima::fastrtps::rtps::RTPSMessageGroup&,
        eprosima::fastrtps::rtps::LocatorSelectorSender&,
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
        eprosima::fastrtps::rtps::CacheChange_t*,
        eprosima::fastrtps::rtps::RTPSMessageGroup&,
        eprosima::fastrtps::rtps::LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->last_thread_delivering_sample = std::this_thread::get_id();
            };

    // Register writers.
    sync.register_writer(&writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer1;
    INIT_CACHE_CHANGE(change_writer1, writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer2;
    INIT_CACHE_CHANGE(change_writer2, writer2);

    // Testing add_new_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor_not_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    EXPECT_CALL(writer2,
            deliver_sample_nts(&change_writer2, _, Ref(writer2.general_locator_selector_), _)).
            WillOnce(DoAll(send_functor_not_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(sync.add_new_sample(&writer2, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    EXPECT_EQ(std::this_thread::get_id(), this->last_thread_delivering_sample);

    // Testing add_new_sample. Writer will not be able to deliver it.
    auto& fail_call = EXPECT_CALL(writer1,
                    deliver_sample_nts(&change_writer1, _, Ref(writer1.general_locator_selector_), _)).
                    WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).After(fail_call).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(sync.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer1));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will not be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(sync.add_old_sample(&writer1, &change_writer1));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample with a change already enqueued.
    change_writer1.writer_info.previous = (eprosima::fastrtps::rtps::CacheChange_t*)1;
    change_writer1.writer_info.next = (eprosima::fastrtps::rtps::CacheChange_t*)1;

    // Send 10 samples using add_old_sample.
    INIT_CACHE_CHANGE(change_writer1, writer1);
    INIT_CACHE_CHANGE(change_writer2, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer3;
    INIT_CACHE_CHANGE(change_writer3, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer4;
    INIT_CACHE_CHANGE(change_writer4, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer5;
    INIT_CACHE_CHANGE(change_writer5, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer6;
    INIT_CACHE_CHANGE(change_writer6, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer7;
    INIT_CACHE_CHANGE(change_writer7, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer8;
    INIT_CACHE_CHANGE(change_writer8, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer9;
    INIT_CACHE_CHANGE(change_writer9, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer10;
    INIT_CACHE_CHANGE(change_writer10, writer1);

    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
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
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Remove changes
    EXPECT_CALL(writer1,
            deliver_sample_nts(_, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor_adding, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED))).
            WillRepeatedly(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
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
    sync.remove_change(&change_writer10);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer9);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer8);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer7);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer6);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer5);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer4);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer3);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    sync.remove_change(&change_writer2);
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

TYPED_TEST(FlowControllerPublishModes, async_publish_mode)
{
    FlowControllerDescriptor flow_controller_descr;
    FlowControllerImpl<FlowControllerAsyncPublishMode, TypeParam> async(nullptr,
            &flow_controller_descr);
    async.init();

    // Instantiate writers.
    eprosima::fastrtps::rtps::RTPSWriter writer1;
    eprosima::fastrtps::rtps::RTPSWriter writer2;

    // Initialize callback to get info.
    auto send_functor = [&](
        eprosima::fastrtps::rtps::CacheChange_t* change,
        eprosima::fastrtps::rtps::RTPSMessageGroup&,
        eprosima::fastrtps::rtps::LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->last_thread_delivering_sample = std::this_thread::get_id();
                {
                    std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                    this->changes_delivered.push_back(change);
                }
                this->number_changes_delivered_cv.notify_one();
            };

    // Register writers.
    async.register_writer(&writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer1;
    INIT_CACHE_CHANGE(change_writer1, writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer2;
    INIT_CACHE_CHANGE(change_writer2, writer2);

    // Testing add_new_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_new_sample. Writer will not be able to deliver it.
    auto& fail_call = EXPECT_CALL(writer1,
                    deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
                    WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).After(fail_call).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will not be able to deliver it.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample with a change already enqueued.
    change_writer1.writer_info.previous = (eprosima::fastrtps::rtps::CacheChange_t*)1;
    change_writer1.writer_info.next = (eprosima::fastrtps::rtps::CacheChange_t*)1;

    // Send 10 samples using add_new_sample.
    INIT_CACHE_CHANGE(change_writer1, writer1);
    INIT_CACHE_CHANGE(change_writer2, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer3;
    INIT_CACHE_CHANGE(change_writer3, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer4;
    INIT_CACHE_CHANGE(change_writer4, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer5;
    INIT_CACHE_CHANGE(change_writer5, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer6;
    INIT_CACHE_CHANGE(change_writer6, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer7;
    INIT_CACHE_CHANGE(change_writer7, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer8;
    INIT_CACHE_CHANGE(change_writer8, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer9;
    INIT_CACHE_CHANGE(change_writer9, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer10;
    INIT_CACHE_CHANGE(change_writer10, writer1);

    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer3,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer4,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer5,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer6,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer7,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer8,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer9,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer10,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Send 10 samples using add_old_sample.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer2));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer3));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer4));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer10));
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Remove changes after add_new_sample.
    EXPECT_CALL(writer1,
            deliver_sample_nts(_, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED))).
            WillRepeatedly(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer3,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer4,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer5,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer6,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer7,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer8,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer9,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer10,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
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
    async.remove_change(&change_writer10);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer9);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer8);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer7);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer6);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer5);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer4);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer3);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer2);
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

    // Remove changes after add_old_sample.
    EXPECT_CALL(writer1,
            deliver_sample_nts(_, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED))).
            WillRepeatedly(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer2));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer3));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer4));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer10));
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
    async.remove_change(&change_writer10);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer9);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer8);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer7);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer6);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer5);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer4);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer3);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer2);
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

    async.unregister_writer(&writer1);
}

struct FlowControllerLimitedAsyncPublishModeMock : FlowControllerLimitedAsyncPublishMode
{
    FlowControllerLimitedAsyncPublishModeMock(
            eprosima::fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor)
        : FlowControllerLimitedAsyncPublishMode(participant, descriptor)
    {
        group_mock = &group;
    }

    static eprosima::fastrtps::rtps::RTPSMessageGroup* get_group()
    {
        return group_mock;
    }

    static eprosima::fastrtps::rtps::RTPSMessageGroup* group_mock;
};
eprosima::fastrtps::rtps::RTPSMessageGroup* FlowControllerLimitedAsyncPublishModeMock::group_mock = nullptr;

TYPED_TEST(FlowControllerPublishModes, limited_async_publish_mode)
{
    FlowControllerDescriptor flow_controller_descr;
    flow_controller_descr.max_bytes_per_period = 10200;
    flow_controller_descr.period_ms = 10;
    FlowControllerImpl<FlowControllerLimitedAsyncPublishModeMock, TypeParam> async(nullptr,
            &flow_controller_descr);
    async.init();

    // Instantiate writers.
    eprosima::fastrtps::rtps::RTPSWriter writer1;
    eprosima::fastrtps::rtps::RTPSWriter writer2;

    // Initialize callback to get info.
    auto send_functor = [&](
        eprosima::fastrtps::rtps::CacheChange_t* change,
        eprosima::fastrtps::rtps::RTPSMessageGroup&,
        eprosima::fastrtps::rtps::LocatorSelectorSender&,
        const std::chrono::time_point<std::chrono::steady_clock>&)
            {
                this->last_thread_delivering_sample = std::this_thread::get_id();
                {
                    std::unique_lock<std::mutex> lock(this->changes_delivered_mutex);
                    this->changes_delivered.push_back(change);
                }
                this->number_changes_delivered_cv.notify_one();
            };

    // Register writers.
    async.register_writer(&writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer1;
    INIT_CACHE_CHANGE(change_writer1, writer1);

    eprosima::fastrtps::rtps::CacheChange_t change_writer2;
    INIT_CACHE_CHANGE(change_writer2, writer2);

    // Testing add_new_sample. Writer will be able to deliver it.
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(), get_current_bytes_processed()).WillOnce(Return(
                0));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_new_sample. Writer will not be able to deliver it.
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).Times(2).WillRepeatedly(Return(
                0));
    auto& fail_call = EXPECT_CALL(writer1,
                    deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
                    WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).After(fail_call).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will be able to deliver it.
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(), get_current_bytes_processed()).WillOnce(Return(
                0));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample. Writer will not be able to deliver it.
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).Times(2).WillRepeatedly(Return(
                0));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Testing add_old_sample with a change already enqueued.
    change_writer1.writer_info.previous = (eprosima::fastrtps::rtps::CacheChange_t*)1;
    change_writer1.writer_info.next = (eprosima::fastrtps::rtps::CacheChange_t*)1;

    // Send 10 samples using add_new_sample.
    INIT_CACHE_CHANGE(change_writer1, writer1);
    INIT_CACHE_CHANGE(change_writer2, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer3;
    INIT_CACHE_CHANGE(change_writer3, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer4;
    INIT_CACHE_CHANGE(change_writer4, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer5;
    INIT_CACHE_CHANGE(change_writer5, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer6;
    INIT_CACHE_CHANGE(change_writer6, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer7;
    INIT_CACHE_CHANGE(change_writer7, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer8;
    INIT_CACHE_CHANGE(change_writer8, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer9;
    INIT_CACHE_CHANGE(change_writer9, writer1);
    eprosima::fastrtps::rtps::CacheChange_t change_writer10;
    INIT_CACHE_CHANGE(change_writer10, writer1);

    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).Times(10).WillRepeatedly(Return(
                0));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer3,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer4,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer5,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer6,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer7,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer8,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer9,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer10,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Send 10 samples using add_old_sample.
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).Times(10).WillRepeatedly(Return(
                0));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer2));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer3));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer4));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer10));
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Remove changes after add_new_sample.
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).WillRepeatedly(Return(
                0));
    EXPECT_CALL(writer1,
            deliver_sample_nts(_, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED))).
            WillRepeatedly(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer3,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer4,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer5,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer6,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer7,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer8,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer9,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer10,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
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
    async.remove_change(&change_writer10);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer9);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer8);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer7);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer6);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer5);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer4);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer3);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer2);
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

    // Remove changes after add_old_sample.
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).WillRepeatedly(Return(
                0));
    EXPECT_CALL(writer1,
            deliver_sample_nts(_, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED))).
            WillRepeatedly(Return(eprosima::fastrtps::rtps::DeliveryRetCode::NOT_DELIVERED));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer2));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer3));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer4));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer10));
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
    async.remove_change(&change_writer10);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer9);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer8);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer7);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer6);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer5);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer4);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer3);
    writer1.getMutex().unlock();
    writer1.getMutex().lock();
    async.remove_change(&change_writer2);
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

    // Sending 10 samples applying limitations with add_new_sample.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    auto& prev = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(prev).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer1,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    auto& prev2 = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(prev2).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer2,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(2);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    auto& prev3 = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(prev3).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer3,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(3);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    auto& prev4 = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(prev4).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer4,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(4);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer5,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer6,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer7,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer8,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer9,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    ASSERT_TRUE(async.add_new_sample(&writer1, &change_writer10,
            std::chrono::steady_clock::now() + std::chrono::hours(24)));
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    // Sending 10 samples applying limitations with add_old_sample.
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer1, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer2, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer3, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer4, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer5, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer6, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer7, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer8, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer9, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    EXPECT_CALL(writer1,
            deliver_sample_nts(&change_writer10, _, Ref(writer1.async_locator_selector_), _)).
            WillOnce(Return(eprosima::fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT)).
            WillOnce(DoAll(send_functor, Return(eprosima::fastrtps::rtps::DeliveryRetCode::DELIVERED)));
    auto& old_prev = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(old_prev).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer1));
    this->wait_changes_was_delivered(1);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    auto& old_prev2 = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(old_prev2).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer2));
    this->wait_changes_was_delivered(2);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    auto& old_prev3 = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(old_prev3).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer3));
    this->wait_changes_was_delivered(3);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    auto& old_prev4 = EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
                    get_current_bytes_processed()).
                    Times(10).WillRepeatedly(Return(10100));
    EXPECT_CALL(*FlowControllerLimitedAsyncPublishModeMock::get_group(),
            get_current_bytes_processed()).After(old_prev4).
            WillRepeatedly(Return(0));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer4));
    this->wait_changes_was_delivered(4);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer5));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer6));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer7));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer8));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer9));
    ASSERT_TRUE(async.add_old_sample(&writer1, &change_writer10));
    this->wait_changes_was_delivered(10);
    EXPECT_NE(std::this_thread::get_id(), this->last_thread_delivering_sample);
    this->changes_delivered.clear();

    async.unregister_writer(&writer1);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
