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

/*!
 * @file TMutexTests.cpp
 *
 */

#include "TMutex.hpp"
#include <gtest/gtest.h>
#include <future>
#include <chrono>
#include <mutex>

using namespace eprosima::fastdds;

TEST(TMutexTests, record_mutexes)
{
    std::mutex mutex_1;
    std::recursive_mutex mutex_2;
    std::timed_mutex mutex_3;
    std::recursive_timed_mutex mutex_4;

    tmutex_start_recording();

    mutex_1.lock();
    mutex_2.lock();
    mutex_3.lock();
    mutex_4.lock();

    mutex_4.unlock();
    mutex_3.unlock();
    mutex_2.unlock();
    mutex_1.unlock();

    tmutex_stop_recording();

    ASSERT_EQ(4, tmutex_get_num_mutexes());
    ASSERT_EQ(mutex_1.native_handle(), tmutex_get_mutex(0));
    ASSERT_EQ(mutex_2.native_handle(), tmutex_get_mutex(1));
    ASSERT_EQ(mutex_3.native_handle(), tmutex_get_mutex(2));
    ASSERT_EQ(mutex_4.native_handle(), tmutex_get_mutex(3));
}

TEST(TMutexTests, lock_mutexes)
{

    std::mutex mutex_1;
    std::recursive_mutex mutex_2;
    std::timed_mutex mutex_3;
    std::recursive_timed_mutex mutex_4;

    tmutex_start_recording();

    mutex_1.lock();
    mutex_2.lock();
    mutex_3.lock();
    mutex_4.lock();

    mutex_4.unlock();
    mutex_3.unlock();
    mutex_2.unlock();
    mutex_1.unlock();

    tmutex_stop_recording();

    ASSERT_EQ(4, tmutex_get_num_lock_type());
    ASSERT_EQ(0, tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < tmutex_get_num_mutexes(); ++count)
    {
        tmutex_lock_mutex(count);
    }

    std::promise<int> promise_1;
    std::promise<int> promise_2;
    std::promise<int> promise_3;
    std::promise<int> promise_4;
    std::future<int> future_1 = promise_1.get_future();
    std::future<int> future_2 = promise_2.get_future();
    std::future<int> future_3 = promise_3.get_future();
    std::future<int> future_4 = promise_4.get_future();
    std::thread([&]
            {
                mutex_1.lock(); promise_1.set_value_at_thread_exit(0);
            }).detach();
    std::thread([&]
            {
                mutex_2.lock(); promise_2.set_value_at_thread_exit(0);
            }).detach();
    std::thread([&]
            {
                mutex_3.lock(); promise_3.set_value_at_thread_exit(0);
            }).detach();
    std::thread([&]
            {
                mutex_4.lock(); promise_4.set_value_at_thread_exit(0);
            }).detach();

    std::cout << "Waiting..." << std::endl;
    ASSERT_TRUE(future_1.wait_for(std::chrono::milliseconds(200)) == std::future_status::timeout);
    ASSERT_TRUE(future_2.wait_for(std::chrono::milliseconds(200)) == std::future_status::timeout);
    ASSERT_TRUE(future_3.wait_for(std::chrono::milliseconds(200)) == std::future_status::timeout);
    ASSERT_TRUE(future_4.wait_for(std::chrono::milliseconds(200)) == std::future_status::timeout);

    for (size_t count = 0; count < tmutex_get_num_mutexes(); ++count)
    {
        tmutex_unlock_mutex(count);
    }

    std::cout << "Waiting after unlock..." << std::endl;
    future_1.wait();
    future_2.wait();
    future_3.wait();
    future_4.wait();
}

TEST(TMutexTests, lock_timed_mutexes)
{

    std::timed_mutex mutex_1;
    std::recursive_timed_mutex mutex_2;

    tmutex_start_recording();

    mutex_1.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
    mutex_2.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));

    mutex_2.unlock();
    mutex_1.unlock();

    tmutex_stop_recording();

    ASSERT_EQ(0, tmutex_get_num_lock_type());
    ASSERT_EQ(2, tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < tmutex_get_num_mutexes(); ++count)
    {
        tmutex_lock_mutex(count);
    }

    std::promise<int> promise_1;
    std::promise<int> promise_2;
    std::future<int> future_1 = promise_1.get_future();
    std::future<int> future_2 = promise_2.get_future();
    std::thread([&]
            {
                mutex_1.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
                promise_1.set_value_at_thread_exit(0);
            }).detach();
    std::thread([&]
            {
                mutex_2.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
                promise_2.set_value_at_thread_exit(0);
            }).detach();

    std::cout << "Waiting..." << std::endl;
    future_1.wait();
    future_2.wait();

    for (size_t count = 0; count < tmutex_get_num_mutexes(); ++count)
    {
        tmutex_unlock_mutex(count);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
