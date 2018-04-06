// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "mock/MockEvent.h"
#include "mock/MockParentEvent.h"
#include <thread>
#include <random>
#include <gtest/gtest.h>

class TimedEventEnvironment : public ::testing::Environment
{
    public:

        TimedEventEnvironment() : work_(service_) {}

        void SetUp()
        {
            thread_ = new std::thread(&TimedEventEnvironment::run, this);
        }

        void TearDown()
        {
            service_.stop();
            thread_->join();
            delete thread_;
        }

        void run()
        {
            service_.run();
        }

        asio::io_service service_;

        asio::io_service::work work_;

        std::thread *thread_;
};

TimedEventEnvironment* const env = dynamic_cast<TimedEventEnvironment*>(testing::AddGlobalTestEnvironment(new TimedEventEnvironment));

/*!
 * @fn TEST(TimedEvent, EventNonAutoDestruc_SuccessEvents)
 * @brief This test checks the correct behavior of launching events.
 * This test launches an event several times.
 * For each one it waits its execution and then launches it again.
 */
TEST(TimedEvent, EventNonAutoDestruc_SuccessEvents)
{
    MockEvent event(env->service_, *env->thread_, 100, false);

    for(int i = 0; i < 10; ++i)
    {
        event.restart_timer();
        event.wait();
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 10);

    int cancelled = event.cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(cancelled, 0);
}

/*!
 * @fn TEST(TimedEvent, EventNonAutoDestruc_CancelEvents)
 * @brief This test  checks the correct behavior of cancelling events.
 * This test launches an event several times and cancels it.
 * For each one it launchs the event and inmediatly it cancels the event.
 * Then it waits until the cancelation is executed.
 */
TEST(TimedEvent, EventNonAutoDestruc_CancelEvents)
{
    MockEvent event(env->service_, *env->thread_, 100, false);

    for(int i = 0; i < 10; ++i)
    {
        event.restart_timer();
        event.cancel_timer();
        event.wait();
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 0);

    int cancelled = event.cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(cancelled, 10);
}

/*!
 * @fn TEST(TimedEvent, EventNonAutoDestruc_RestartEvents)
 * @brief This test checks the correct behaviour of restarting events.
 * This test restart continuisly several events.
 */
TEST(TimedEvent, EventNonAutoDestruc_RestartEvents)
{
    MockEvent event(env->service_, *env->thread_, 100, false);

    for(int i = 0; i < 10; ++i)
    {
        event.cancel_timer();
        event.restart_timer();
    }

    for(int i = 0; i < 10; ++i)
    {
        event.wait();
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 1);

    int cancelled = event.cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(cancelled, 9);
}

/*!
 * @fn TEST(TimedEvent, EventOnSuccessAutoDestruc_SuccessEvents)
 * @brief This test checks the correct behaviour of autodestruction on successful execution.
 * This test launches an event configured to destroy itself when the event is executed successfully.
 */
TEST(TimedEvent, EventOnSuccessAutoDestruc_SuccessEvents)
{
    // Restart destruction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    event->restart_timer();

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::seconds(1));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventOnSuccessAutoDestruc_CancelEvents)
 * @brief This test checks the event is not destroyed when it is canceled.
 * This test launches an event, configured to destroy itself when the event is executed successfully,
 * several times but immediately cancelling it.
 */
TEST(TimedEvent, EventOnSuccessAutoDestruc_CancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    // Cancel ten times.
    for(int i = 0; i < 10; ++i)
    {
        event->restart_timer();
        event->cancel_timer();

        event->wait();

        ASSERT_EQ(MockEvent::destructed_, 0);
    }

    int successed = event->successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 0);

    int cancelled = event->cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(cancelled, 10);

    // Last event will be successful.
    event->restart_timer();

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::seconds(1));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickCancelEvents)
 * @brief This test checks the event is not destroyed when it is canceled.
 * This test launches an event, configured to destroy itself when the event is executed successfully,
 * several times but inmediatly cancelling it.
 */
TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickCancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 1, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    // Cancel ten times.
    for(int i = 0; i < 10; ++i)
    {
        event->restart_timer();
        event->cancel_timer();

        event->wait();

        ASSERT_EQ(MockEvent::destructed_, 0);
    }

    int successed = event->successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 0);

    int cancelled = event->cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(cancelled, 10);

    // Last event will be successful.
    event->restart_timer();

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::seconds(1));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventOnSuccessAutoDestruc_RestartEvents)
 * @brief This test checks the event is not destroyed when it is canceled and restarted.
 * This test restarts several events configure to destroy itself when the event is executed successfully.
 *
 */
TEST(TimedEvent, EventOnSuccessAutoDestruc_RestartEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    for(int i = 0; i < 10; ++i)
    {
        event->cancel_timer();
        event->restart_timer();
    }

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::seconds(1));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickRestartEvents)
 * @brief This test checks the event is not destroyed when it is canceled and restarted.
 * This test restarts several events configure to destroy itself when the event is executed successfully.
 *
 */
TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickRestartEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 1, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    for(int i = 0; i < 10; ++i)
    {
        event->cancel_timer();
        event->restart_timer();
    }

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::seconds(1));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventAlwaysAutoDestruc_SuccessEvents)
 * @brief This test checks the event is destroyed after a succesful execution.
 * This test launches an event that is configured to destroy itself.
 */
TEST(TimedEvent, EventAlwaysAutoDestruc_SuccessEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

    event->restart_timer();

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::seconds(1));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventAlwaysAutoDestruc_CancelEvents)
 * @brief This test check the event is destroy after the cancelation is executed.
 * This test launches an event and imediatly cancels it.
 */
TEST(TimedEvent, EventAlwaysAutoDestruc_CancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

    event->restart_timer();
    event->cancel_timer();

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::milliseconds(100));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventAlwaysAutoDestruc_CancelEvents)
 * @brief This test checks the event is destroy after the cancelation is executed.
 * This test launches an event and imediatly cancels it.
 */
TEST(TimedEvent, EventAlwaysAutoDestruc_QuickCancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 1, false, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

    event->restart_timer();
    event->cancel_timer();

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::milliseconds(100));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, EventNonAutoDestruct_AutoRestart)
 * @brief This test checks an event is able to restart itself.
 * This test launches an event several times and this event also restarts itself.
 */
TEST(TimedEvent, EventNonAutoDestruct_AutoRestart)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(env->service_, *env->thread_, 10 , true);

    for(unsigned int i = 0; i < 100; ++i)
    {
        event->restart_timer();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    int successed = event->successed_.load(std::memory_order_relaxed);

    ASSERT_GE(successed , 100);

    delete event;

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::milliseconds(100));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}


/*!
 * @fn TEST(TimedEvent, EventNonAutoDestruc_AutoRestartAndDeleteRandomly)
 * This test checks an event, configured to restart itself, can be deleted while
 * it is being scheduled.
 * This test launches an event that restarts itself, and then randomly deletes it.
 */
TEST(TimedEvent, EventNonAutoDestruc_AutoRestartAndDeleteRandomly)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10, 100);

    MockEvent* event = new MockEvent(env->service_, *env->thread_, 2, true);

    event->restart_timer();
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));

    delete event;

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::milliseconds(100));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @fn TEST(TimedEvent, ParentEventNonAutoDestruc_InternallyDeleteEventNonAutoDestruct)
 * This test checks an event can delete other event while the later is waiting.
 * This test launches an event that internally will destroy other event.
 */
TEST(TimedEvent, ParentEventNonAutoDestruc_InternallyDeleteEventNonAutoDestruct)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;

    MockParentEvent event(env->service_, *env->thread_, 10, 2);

    event.restart_timer();

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::milliseconds(300));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

/*!
 * @brief Auxyliary function to be run in multithread tests.
 * It restarts an event in a loop.
 * @param Event to be restarted.
 * @param num_loop Number of loops.
 * @param sleep_time Sleeping time between each loop.
 */
void restart(MockEvent *event, unsigned int num_loop, unsigned int sleep_time)
{
    for(unsigned int i = 0; i < num_loop; ++i)
    {
        event->restart_timer();

        if(sleep_time > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }
}

/*!
 * @brief Auxyliary function to be run in multithread tests.
 * It cancels an event in a loop.
 * @param Event to be restarted.
 * @param num_loop Number of loops.
 * @param sleep_time Sleeping time between each loop.
 */
void cancel(MockEvent *event, unsigned int num_loop, unsigned int sleep_time)
{
    for(unsigned int i = 0; i < num_loop; ++i)
    {
        event->cancel_timer();

        if(sleep_time > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }
}

/*!
 * @fn TEST(TimedEventMultithread, EventNonAutoDestruc_TwoStartTwoCancel)
 * @brief This function checks multithreading usage of events.
 * This function launches four threads. Two threads restart the event, and the other two
 * cancel it.
 */
TEST(TimedEventMultithread, EventNonAutoDestruc_TwoStartTwoCancel)
{
    std::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(env->service_, *env->thread_, 3, false);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> Restart 100 times waiting 100ms between each one.
    // Thread 2 -> Cancel 100 times waiting 102ms between each one.
    // Thread 3 -> Restart 80 times waiting 110ms between each one.
    // Thread 4 -> Cancel 80 times waiting 112ms between each one.

    thr1 = new std::thread(restart, &event, 100, 100);
    thr2 = new std::thread(cancel, &event, 100, 102);
    thr3 = new std::thread(restart, &event, 80, 110);
    thr4 = new std::thread(cancel, &event, 80, 112);

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;

    // Wait until finish the execution of the event.
    int count = 0;
    while(event.wait(500))
    {
        ++count;
    }

    int successed = event.successed_.load(std::memory_order_relaxed);
    int cancelled = event.cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, count);
}

/*!
 * @fn TEST(TimedEventMultithread, EventNonAutoDestruc_QuickTwoStartTwoCancel)
 * @brief This function checks multithreading usage of events.
 * This function launches four threads. Two threads restart the event, and the other two
 * cancel it.
 */
TEST(TimedEventMultithread, EventNonAutoDestruc_QuickTwoStartTwoCancel)
{
    std::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(env->service_, *env->thread_, 3, false);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> Restart 100 times waiting 2ms between each one.
    // Thread 2 -> Cancel 100 times waiting 2ms between each one.
    // Thread 3 -> Restart 80 times waiting 3ms between each one.
    // Thread 4 -> Cancel 80 times waiting 3ms between each one.

    thr1 = new std::thread(restart, &event, 100, 2);
    thr2 = new std::thread(cancel, &event, 100, 2);
    thr3 = new std::thread(restart, &event, 80, 4);
    thr4 = new std::thread(cancel, &event, 80, 4);

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;

    // Wait until finish the execution of the event.
    int count = 0;
    while(event.wait(500))
    {
        ++count;
    }

    int successed = event.successed_.load(std::memory_order_relaxed);
    int cancelled = event.cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, count);
}

/*!
 * @fn TEST(TimedEventMultithread, EventNonAutoDestruc_QuickestTwoStartTwoCancel)
 * @brief This function checks multithreading usage of events.
 * This function launches four threads. Two threads restart the event, and the other two
 * cancel it.
 */
TEST(TimedEventMultithread, EventNonAutoDestruc_QuickestTwoStartTwoCancel)
{
    std::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(env->service_, *env->thread_, 2, false);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> Restart 100 times waiting 0ms between each one.
    // Thread 2 -> Cancel 100 times waiting 0ms between each one.
    // Thread 3 -> Restart 80 times waiting 0ms between each one.
    // Thread 4 -> Cancel 80 times waiting 0ms between each one.

    thr1 = new std::thread(restart, &event, 100, 0);
    thr2 = new std::thread(cancel, &event, 100, 0);
    thr3 = new std::thread(restart, &event, 80, 0);
    thr4 = new std::thread(cancel, &event, 80, 0);

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;

    // Wait until finish the execution of the event.
    int count = 0;
    while(event.wait(500))
    {
        ++count;
    }

    int successed = event.successed_.load(std::memory_order_relaxed);
    int cancelled = event.cancelled_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, count);
}

/*!
 * @fn TEST(TimedEventMultithread, EventNonAutoDestruc_FourAutoRestart)
 * @brief This function checks an event, configured to restart itself, is able to
 * be restarted in several threads.
 * This function launches four threads and each one restart the event.
 */
TEST(TimedEventMultithread, EventNonAutoDestruc_FourAutoRestart)
{
    MockEvent::destructed_ = 0;
    std::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent *event = new MockEvent(env->service_, *env->thread_, 2, true);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> AutoRestart 100 times waiting 2ms between each one.
    // Thread 2 -> AutoRestart 100 times waiting 3ms between each one.
    // Thread 3 -> AutoRestart 80 times waiting 4ms between each one.
    // Thread 4 -> AutoRestart 80 times waiting 5ms between each one.

    thr1 = new std::thread(restart, event, 100, 2);
    thr2 = new std::thread(restart, event, 100, 3);
    thr3 = new std::thread(restart, event, 80, 4);
    thr4 = new std::thread(restart, event, 80, 5);

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;

    // Wait a prudential time before delete the event.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    delete event;

    std::unique_lock<std::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
    {
        MockEvent::destruction_cond_.wait_for(lock, std::chrono::seconds(1));
    }

    ASSERT_EQ(MockEvent::destructed_, 1);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
