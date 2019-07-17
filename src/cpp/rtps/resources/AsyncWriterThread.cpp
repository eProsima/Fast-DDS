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

#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <mutex>
#include <algorithm>
#include <cassert>
#include <stdexcept>

using namespace eprosima::fastrtps::rtps;

/*!
 * @brief This function removes a writer.
 * @param writer Asynchronous writer to be removed.
 * @return Result of the operation.
 */
void AsyncWriterThread::unregister_writer(RTPSWriter* writer)
{
    if(interestTree_.UnregisterInterest(writer))
    {
        std::unique_lock<std::timed_mutex> lock(condition_variable_mutex_);
        running_ = false;
        run_scheduled_ = false;
        cv_.notify_all();
        if (thread_)
        {
            lock.unlock();
            thread_->join();
            lock.lock();
            delete thread_;
            thread_ = nullptr;
            }
    }
}

void AsyncWriterThread::wake_up(
        RTPSWriter* interestedWriter)
{
    interestTree_.RegisterInterest(interestedWriter);
    std::unique_lock<std::timed_mutex> lock(condition_variable_mutex_);
    run_scheduled_ = true;
    // If thread not running, start it.
    if(thread_ == nullptr)
    {
        running_ = true;
        run_scheduled_ = true;
        thread_ = new std::thread(&AsyncWriterThread::run, this);
    }
    else
    {
        cv_.notify_all();
    }
}

void AsyncWriterThread::wake_up(
        RTPSWriter* interestedWriter,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    if(interestTree_.RegisterInterest(interestedWriter, max_blocking_time))
    {
        std::unique_lock<std::timed_mutex> lock(condition_variable_mutex_, std::defer_lock);

        if(lock.try_lock_until(max_blocking_time))
        {
            run_scheduled_ = true;
            // If thread not running, start it.
            if(thread_ == nullptr)
            {
                running_ = true;
                run_scheduled_ = true;
                thread_ = new std::thread(&AsyncWriterThread::run, this);
            }
            else
            {
                cv_.notify_all();
            }
        }
    }
}

void AsyncWriterThread::run()
{
    std::unique_lock<std::timed_mutex> cond_guard(condition_variable_mutex_);
    while(running_)
    {
        if(run_scheduled_)
        {
            run_scheduled_ = false;
            cond_guard.unlock();
            interestTree_.Swap();
            RTPSWriter* curr = nullptr;

            while ((curr = interestTree_.next_active()))
            {
                curr->send_any_unsent_changes();
            }

            cond_guard.lock();
        }
        else
        {
            cv_.wait(cond_guard);
        }
    }
}
