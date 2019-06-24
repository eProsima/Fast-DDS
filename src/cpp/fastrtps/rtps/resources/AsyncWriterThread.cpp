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

std::thread* AsyncWriterThread::thread_;
std::mutex AsyncWriterThread::data_structure_mutex_;
std::mutex AsyncWriterThread::condition_variable_mutex_;
std::list<RTPSWriter*> AsyncWriterThread::async_writers;
bool AsyncWriterThread::running_;
bool AsyncWriterThread::run_scheduled_;
std::condition_variable AsyncWriterThread::cv_;
AsyncInterestTree AsyncWriterThread::interestTree;

bool AsyncWriterThread::addWriter(RTPSWriter& writer)
{
    bool returnedValue = false;

    data_structure_mutex_.lock();
    async_writers.push_back(&writer);
    returnedValue = true;

    std::unique_lock<std::mutex> cond_guard(condition_variable_mutex_);
    data_structure_mutex_.unlock();
    // If thread not running, start it.
    if(thread_ == nullptr)
    {
        running_ = true;
        run_scheduled_ = true;
        thread_ = new std::thread(AsyncWriterThread::run);
    }

    return returnedValue;
}

/*!
 * @brief This function removes a writer.
 * @param writer Asynchronous writer to be removed.
 * @return Result of the operation.
 */
bool AsyncWriterThread::removeWriter(RTPSWriter& writer)
{
    bool returnedValue = false;

    std::unique_lock<std::mutex> data_guard(data_structure_mutex_);
    auto it = std::find(async_writers.begin(), async_writers.end(), &writer);

    if(it != async_writers.end())
    {
        async_writers.erase(it);
        returnedValue = true;

        // If there is not more asynchronous writers, stop the thread.
        if(async_writers.empty())
        {
            std::unique_lock<std::mutex> cond_guard(condition_variable_mutex_);
            data_guard.unlock();
            running_ = false;
            run_scheduled_ = false;
            cv_.notify_all();
            cond_guard.unlock();
            thread_->join();
            cond_guard.lock();
            delete thread_;
            thread_ = nullptr;
        }
    }

    return returnedValue;
}

void AsyncWriterThread::wakeUp(const RTPSParticipantImpl* interestedParticipant)
{
   interestTree.RegisterInterest(interestedParticipant);
   { // Lock scope
      std::unique_lock<std::mutex> cond_guard(condition_variable_mutex_);
      run_scheduled_ = true;
   }
   cv_.notify_all();
}

void AsyncWriterThread::wakeUp(const RTPSWriter* interestedWriter)
{
   interestTree.RegisterInterest(interestedWriter);
   std::unique_lock<std::mutex> cond_guard(condition_variable_mutex_);
   run_scheduled_ = true;
   cv_.notify_all();
}

void AsyncWriterThread::run()
{
    std::unique_lock<std::mutex> cond_guard(condition_variable_mutex_);
    while(running_)
    {
       if(run_scheduled_)
       {
          run_scheduled_ = false;
          cond_guard.unlock();
          interestTree.Swap();
          auto interestedWriters = interestTree.GetInterestedWriters();

          std::unique_lock<std::mutex> data_guard(data_structure_mutex_);
          for(auto writer : async_writers)
             if (interestedWriters.count(writer))
               writer->send_any_unsent_changes();

          cond_guard.lock();
       }
       else
           cv_.wait(cond_guard);
    }
}
