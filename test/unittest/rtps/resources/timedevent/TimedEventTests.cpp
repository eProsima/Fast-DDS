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
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <thread>
#include <random>
#include <gtest/gtest.h>

class TimedEventEnvironment : public ::testing::Environment
{
public:

    void SetUp()
    {
        service_ = new eprosima::fastrtps::rtps::ResourceEvent();
        service_->init_thread();
    }

    void TearDown()
    {
        delete service_;
    }

    eprosima::fastrtps::rtps::ResourceEvent* service_;
};

TimedEventEnvironment* const env =
        dynamic_cast<TimedEventEnvironment*>(testing::AddGlobalTestEnvironment(new TimedEventEnvironment));

/*!
 * @fn TEST(TimedEvent, Event_SuccessEvents)
 * @brief This test checks the correct behavior of launching events.
 * This test launches an event several times.
 * For each one it waits its execution and then launches it again.
 */
TEST(TimedEvent, Event_SuccessEvents)
{
    MockEvent event(*env->service_, 100, false);

    for (int i = 0; i < 10; ++i)
    {
        event.event().restart_timer();
        event.wait();
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 10);
}

/*!
 * @fn TEST(TimedEvent, Event_CancelEvents)
 * @brief This test  checks the correct behavior of cancelling events.
 * This test launches an event several times and cancels it.
 * For each one it launchs the event and inmediatly it cancels the event.
 * Then it waits more than the timer period and checks it has not been executed.
 */
TEST(TimedEvent, Event_CancelEvents)
{
    MockEvent event(*env->service_, 100, false);

    for (int i = 0; i < 10; ++i)
    {
        event.event().restart_timer();
        event.event().cancel_timer();
        ASSERT_FALSE(event.wait(120));
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 0);
}

/*!
 * @fn TEST(TimedEvent, Event_RestartEvents)
 * @brief This test checks the correct behaviour of restarting events.
 * This test restart continuisly several events.
 */
TEST(TimedEvent, Event_RestartEvents)
{
    MockEvent event(*env->service_, 100, false);

    for (int i = 0; i < 10; ++i)
    {
        event.event().cancel_timer();
        event.event().restart_timer();
    }

    // Should only be awaken once due to cancellation
    event.wait();
    for (int i = 1; i < 10; ++i)
    {
        ASSERT_FALSE(event.wait(120));
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 1);
}

/*!
 * @fn TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickCancelEvents)
 * @brief This test checks the event is not destroyed when it is canceled.
 * This test launches an event, configured to destroy itself when the event is executed successfully,
 * several times but inmediatly cancelling it.
 */
TEST(TimedEvent, Event_QuickCancelEvents)
{
    MockEvent event(*env->service_, 1, false);

    // Cancel ten times.
    for (int i = 0; i < 10; ++i)
    {
        event.event().restart_timer();
        event.event().cancel_timer();
    }

    ASSERT_FALSE(event.wait(5));

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 0);

    // Last event will be successful.
    event.event().restart_timer();
    event.wait();

    successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 1);
}

/*!
 * @fn TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickRestartEvents)
 * @brief This test checks the event is not destroyed when it is canceled and restarted.
 * This test restarts several events configure to destroy itself when the event is executed successfully.
 */
TEST(TimedEvent, Event_QuickRestartEvents)
{
    MockEvent event(*env->service_, 1, false);

    for (int i = 0; i < 10; ++i)
    {
        event.event().cancel_timer();
        event.event().restart_timer();
    }

    event.wait_success();

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, 1);
}

/*!
 * @fn TEST(TimedEvent, Event_AutoRestart)
 * @brief This test checks an event is able to restart itself.
 * This test launches an event several times and this event also restarts itself.
 */
TEST(TimedEvent, Event_AutoRestart)
{
    MockEvent event(*env->service_, 10, true);

    for (unsigned int i = 0; i < 100; ++i)
    {
        event.event().restart_timer();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_GE(successed, 100);
}


/*!
 * @fn TEST(TimedEvent, Event_AutoRestartAndDeleteRandomly)
 * This test checks an event, configured to restart itself, can be deleted while
 * it is being scheduled.
 * This test launches an event that restarts itself, and then randomly deletes it.
 */
TEST(TimedEvent, Event_AutoRestartAndDeleteRandomly)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10, 100);

    MockEvent event(*env->service_, 2, true);

    event.event().restart_timer();
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
}

/*!
 * @brief Auxyliary function to be run in multithread tests.
 * It restarts an event in a loop.
 * @param Event to be restarted.
 * @param num_loop Number of loops.
 * @param sleep_time Sleeping time between each loop.
 */
void restart(
        MockEvent* event,
        unsigned int num_loop,
        unsigned int sleep_time)
{
    for (unsigned int i = 0; i < num_loop; ++i)
    {
        event->event().restart_timer();

        if (sleep_time > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
}

/*!
 * @brief Auxyliary function to be run in multithread tests.
 * It cancels an event in a loop.
 * @param Event to be restarted.
 * @param num_loop Number of loops.
 * @param sleep_time Sleeping time between each loop.
 */
void cancel(
        MockEvent* event,
        unsigned int num_loop,
        unsigned int sleep_time)
{
    for (unsigned int i = 0; i < num_loop; ++i)
    {
        event->event().cancel_timer();

        if (sleep_time > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
}

/*!
 * @fn TEST(TimedEventMultithread, EventNonAutoDestruc_TwoStartTwoCancel)
 * @brief This function checks multithreading usage of events.
 * This function launches four threads. Two threads restart the event, and the other two
 * cancel it.
 */
TEST(TimedEventMultithread, Event_TwoStartTwoCancel)
{
    std::thread* thr1 = nullptr, * thr2 = nullptr,
            * thr3 = nullptr, * thr4 = nullptr;

    MockEvent event(*env->service_, 3, false);

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
    while (event.wait(500))
    {
        ++count;
    }

    int successed = event.successed_.load(std::memory_order_relaxed);

    ASSERT_EQ(successed, count);
}

/*!
 * @fn TEST(TimedEventMultithread, EventNonAutoDestruc_FourAutoRestart)
 * @brief This function checks an event, configured to restart itself, is able to
 * be restarted in several threads.
 * This function launches four threads and each one restart the event.
 */
TEST(TimedEventMultithread, Event_FourAutoRestart)
{
    std::thread* thr1 = nullptr, * thr2 = nullptr,
            * thr3 = nullptr, * thr4 = nullptr;

    MockEvent event(*env->service_, 2, true);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> AutoRestart 100 times waiting 2ms between each one.
    // Thread 2 -> AutoRestart 100 times waiting 3ms between each one.
    // Thread 3 -> AutoRestart 80 times waiting 4ms between each one.
    // Thread 4 -> AutoRestart 80 times waiting 5ms between each one.

    thr1 = new std::thread(restart, &event, 100, 2);
    thr2 = new std::thread(restart, &event, 100, 3);
    thr3 = new std::thread(restart, &event, 80, 4);
    thr4 = new std::thread(restart, &event, 80, 5);

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
}

TEST(TimedEventMultithread, PendingRaceCheck)
{
    using TimedEvent = eprosima::fastrtps::rtps::TimedEvent;
    using TimeClock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<TimeClock>;

    constexpr auto periodic_ms = std::chrono::milliseconds(1000);
    constexpr auto newcomer_ms = std::chrono::milliseconds(10);

    bool stop_test = false;

    // Code to check the newcomer event
    auto newcomer_thread_code = [&]()
            {
                std::condition_variable cv;
                std::mutex mtx;
                bool triggered = false;

                // The event will just inform it has been triggered
                auto callback = [&]()
                        {
                            std::lock_guard<std::mutex> guard(mtx);
                            triggered = true;
                            cv.notify_one();

                            return false;
                        };

                // We create the timed event and then enter in a loop where we will start
                // it and wait for its completion. We will then check that it is triggered
                // in less time than half the periodic timed event
                TimedEvent newcomer_event(*env->service_, callback, 1.0 * newcomer_ms.count());
                while (!stop_test)
                {
                    TimePoint start_time;
                    TimePoint stop_time;

                    start_time = TimeClock::now();

                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        triggered = false;
                        newcomer_event.restart_timer();
                        cv.wait(lock, [&]()
                                {
                                    return triggered;
                                });
                        stop_time = TimeClock::now();
                    }

                    EXPECT_LT(stop_time - start_time, periodic_ms / 2);
                }
            };

    // Periodic event that triggers every second
    auto periodic_callback = []()
            {
                return true;
            };
    TimedEvent main_event(*env->service_, periodic_callback, 1.0 * periodic_ms.count());

    // Let the periodic event run for several periods
    main_event.restart_timer();
    std::this_thread::sleep_for(periodic_ms * 2);

    // Launch the checking thread and let it run for several periods
    std::thread* checking_thr = new std::thread(newcomer_thread_code);
    std::this_thread::sleep_for(periodic_ms * 10);

    stop_test = true;
    checking_thr->join();
    delete checking_thr;
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
