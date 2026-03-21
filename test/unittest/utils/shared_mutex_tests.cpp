// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Excerpts from: STL/main/tests/std/tests/Dev11_1150223_shared_mutex/test.cpp

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <utils/shared_mutex.hpp>

using namespace std;
using namespace std::chrono;
using namespace eprosima;

template<class Mutex>
detail::shared_mutex_type get_mutex_priority(
        const Mutex& m);

template<detail::shared_mutex_type mt>
detail::shared_mutex_type get_mutex_priority(
        const detail::shared_mutex<mt>&)
{
    return mt;
}

template<detail::shared_mutex_type mt>
detail::shared_mutex_type get_mutex_priority(
        const detail::debug_wrapper<detail::shared_mutex<mt>>&)
{
    return mt;
}

template<typename T>
class SharedMutexTest : public testing::Test
{
public:

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

TYPED_TEST_SUITE(SharedMutexTest, SharedMutexTypes, );

TYPED_TEST(SharedMutexTest, test_one_writer)
{
    // One simultaneous writer.
    atomic<int> atom(-1);
    TypeParam mut;
    vector<thread> threads;

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&atom, &mut]
                {
                    while (atom == -1)
                    {
                    }
                    lock_guard<TypeParam> ExclusiveLock(mut);
                    const int val = ++atom;
                    this_thread::sleep_for(milliseconds(25)); // Not a timing assumption.
                    ASSERT_EQ(atom, val);
                });
    }

    ASSERT_EQ(atom.exchange(0), -1);
    this->join_and_clear(threads);
    ASSERT_EQ(atom, 4);
}

TYPED_TEST(SharedMutexTest, test_multiple_readers)
{
    // Many simultaneous readers.
    atomic<int> atom(-1);
    TypeParam mut;
    vector<thread> threads;

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&atom, &mut]
                {
                    while (atom == -1)
                    {
                    }
                    shared_lock<TypeParam> SharedLock(mut);
                    ++atom;
                    while (atom < 4)
                    {
                    }
                });
    }

    ASSERT_EQ(atom.exchange(0), -1);
    this->join_and_clear(threads);
    ASSERT_EQ(atom, 4);
}

TYPED_TEST(SharedMutexTest, test_writer_blocking_readers)
{
    // One writer blocking many readers.
    atomic<int> atom(-4);
    TypeParam mut;
    vector<thread> threads;

    threads.emplace_back([&atom, &mut]
            {
                while (atom < 0)
                {
                }
                lock_guard<TypeParam> ExclusiveLock(mut);
                ASSERT_EQ(atom.exchange(1000), 0);
                this_thread::sleep_for(milliseconds(50)); // Not a timing assumption.
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
                    shared_lock<TypeParam> SharedLock(mut);
                    ASSERT_EQ(atom, 1729);
                });
    }

    this->join_and_clear(threads);
    ASSERT_EQ(atom, 1729);
}

TYPED_TEST(SharedMutexTest, test_readers_blocking_writer)
{
    // Many readers blocking one writer.
    atomic<int> atom(-5);
    TypeParam mut;
    vector<thread> threads;

    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&atom, &mut]
                {
                    shared_lock<TypeParam> SharedLock(mut);
                    ++atom;
                    while (atom < 0)
                    {
                    }
                    this_thread::sleep_for(milliseconds(50)); // Not a timing assumption.
                    atom += 10;
                });
    }

    threads.emplace_back([&atom, &mut]
            {
                ++atom;
                while (atom < 0)
                {
                }
                lock_guard<TypeParam> ExclusiveLock(mut);
                ASSERT_EQ(atom, 40);
            });

    this->join_and_clear(threads);
    ASSERT_EQ(atom, 40);
}

TYPED_TEST(SharedMutexTest, test_try_lock_and_try_lock_shared)
{
    // Test try_lock() and try_lock_shared().
    TypeParam mut;

    {
        unique_lock<TypeParam> MainExclusive(mut, try_to_lock);
        ASSERT_TRUE(MainExclusive.owns_lock());

        thread t([&mut]
                {
                    {
                        unique_lock<TypeParam> ExclusiveLock(mut, try_to_lock);
                        ASSERT_FALSE(ExclusiveLock.owns_lock());
                    }

                    {
                        shared_lock<TypeParam> SharedLock(mut, try_to_lock);
                        ASSERT_FALSE(SharedLock.owns_lock());
                    }
                });

        t.join();
    }

    {
        shared_lock<TypeParam> MainShared(mut, try_to_lock);
        ASSERT_TRUE(MainShared.owns_lock());

        thread t([&mut]
                {
                    {
                        unique_lock<TypeParam> ExclusiveLock(mut, try_to_lock);
                        ASSERT_FALSE(ExclusiveLock.owns_lock());
                    }

                    {
                        shared_lock<TypeParam> SharedLock(mut, try_to_lock);
                        ASSERT_TRUE(SharedLock.owns_lock());
                    }
                });

        t.join();
    }
}

TYPED_TEST(SharedMutexTest, test_mutex_priority)
{
    TypeParam sm;
    atomic_bool mark{false};

    // take first shared lock
    sm.lock_shared();

    // signal is taken
    thread exclusive([&]
            {
                mark.store(true);
                lock_guard<TypeParam> guard(sm);
            });

    // Wait till the thread takes the lock
    do
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    while (!mark);

    // try take the second shared lock
    bool success = sm.try_lock_shared();
    if (success)
    {
        sm.unlock_shared();
        ASSERT_EQ(get_mutex_priority(sm),
                detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_READER_NP);
    }
    else
    {
        ASSERT_EQ(get_mutex_priority(sm),
                detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    }

    // release first lock
    sm.unlock_shared();
    // wait for the main thread
    exclusive.join();
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
