// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file check_shared_mutex_priority.cpp
 *
 */

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>

using namespace std;

int main()
{
    shared_mutex sm;
    atomic_bool mark = false;

    // take first shared lock
    sm.lock_shared();

    // signal is taken
    thread exclusive([&]()
            {
                mark = true;
                lock_guard<shared_mutex> guard(sm);
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
        cout << "PTHREAD_RWLOCK_PREFER_READER_NP" << endl;
    }
    else
    {
        cout << "PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP" << endl;
    }

    // release first lock
    sm.unlock_shared();
    // wait for the main thread
    exclusive.join();

    return 0;
}
