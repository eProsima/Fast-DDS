// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Excerpts from: STL/main/tests/std/tests/Dev11_1150223_shared_mutex/test.cpp

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <vector>

#include <fastrtps/utils/shared_mutex.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace eprosima;

template<class T>
class SharedMutexTest : public testing::Test
{

public:

    using Mutex = T;

    void join_and_clear(
            vector<thread>& threads)
    {
        for (auto& t : threads)
        {
            t.join();
        }

        threads.clear();
    }

};

using SharedMutexTypes = ::testing::Types<
    detail::shared_mutex<detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP>,
    detail::shared_mutex<detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_READER_NP>,
    detail::debug_wrapper<detail::shared_mutex<detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP>>,
    detail::debug_wrapper<detail::shared_mutex<detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_READER_NP>>>;

TYPED_TEST_SUITE(SharedMutexTest, SharedMutexTypes);

TYPED_TEST(SharedMutexTest, test_one_writer)
{
    // One simultaneous writer.
    atomic<int> atom(-1);
    Mutex mut;
    vector<thread> threads;

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&atom, &mut]
                {
                    while (atom == -1)
                    {
                    }
                    lock_guard<Mutex> ExclusiveLock(mut);
                    const int val = ++atom;
                    this_thread::sleep_for(25ms); // Not a timing assumption.
                    ASSERT_EQ(atom, val);
                });
    }

    ASSERT_EQ(atom.exchange(0), -1);
    join_and_clear(threads);
    ASSERT_EQ(atom, 4);
}

TYPED_TEST(SharedMutexTest, test_multiple_readers)
{
    // Many simultaneous readers.
    atomic<int> atom(-1);
    Mutex mut;
    vector<thread> threads;

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&atom, &mut]
                {
                    while (atom == -1)
                    {
                    }
                    shared_lock<Mutex> SharedLock(mut);
                    ++atom;
                    while (atom < 4)
                    {
                    }
                });
    }

    ASSERT_EQ(atom.exchange(0), -1);
    join_and_clear(threads);
    ASSERT_EQ(atom, 4);
}

TYPED_TEST(SharedMutexTest, test_writer_blocking_readers)
{
    // One writer blocking many readers.
    atomic<int> atom(-4);
    Mutex mut;
    vector<thread> threads;

    threads.emplace_back([&atom, &mut]
            {
                while (atom < 0)
                {
                }
                lock_guard<Mutex> ExclusiveLock(mut);
                ASSERT_EQ(atom.exchange(1000), 0);
                this_thread::sleep_for(50ms); // Not a timing assumption.
                ASSERT_EQ(atom.exchange(1729), 1000);
            });

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&atom, &mut]
                {
                    ++atom;
                    while (atom < 1000)
                    {
                    }
                    shared_lock<Mutex> SharedLock(mut);
                    ASSERT_EQ(atom, 1729);
                });
    }

    join_and_clear(threads);
    ASSERT_EQ(atom, 1729);
}

TYPED_TEST(SharedMutexTest, test_readers_blocking_writer)
{
    // Many readers blocking one writer.
    atomic<int> atom(-5);
    Mutex mut;
    vector<thread> threads;

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&atom, &mut]
                {
                    shared_lock<Mutex> SharedLock(mut);
                    ++atom;
                    while (atom < 0)
                    {
                    }
                    this_thread::sleep_for(50ms); // Not a timing assumption.
                    atom += 10;
                });
    }

    threads.emplace_back([&atom, &mut]
            {
                ++atom;
                while (atom < 0)
                {
                }
                lock_guard<Mutex> ExclusiveLock(mut);
                ASSERT_EQ(atom, 40);
            });

    join_and_clear(threads);
    ASSERT_EQ(atom, 40);
}

TYPED_TEST(SharedMutexTest, test_try_lock_and_try_lock_shared)
{
    // Test try_lock() and try_lock_shared().
    Mutex mut;

    {
        unique_lock<Mutex> MainExclusive(mut, try_to_lock);
        ASSERT_TRUE(MainExclusive.owns_lock());

        thread t([&mut]
                {
                    {
                        unique_lock<Mutex> ExclusiveLock(mut, try_to_lock);
                        ASSERT_FALSE(ExclusiveLock.owns_lock());
                    }

                    {
                        shared_lock<Mutex> SharedLock(mut, try_to_lock);
                        ASSERT_FALSE(SharedLock.owns_lock());
                    }
                });

        t.join();
    }

    {
        shared_lock<Mutex> MainShared(mut, try_to_lock);
        ASSERT_TRUE(MainShared.owns_lock());

        thread t([&mut]
                {
                    {
                        unique_lock<Mutex> ExclusiveLock(mut, try_to_lock);
                        ASSERT_FALSE(ExclusiveLock.owns_lock());
                    }

                    {
                        shared_lock<Mutex> SharedLock(mut, try_to_lock);
                        ASSERT_TRUE(SharedLock.owns_lock());
                    }
                });

        t.join();
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
