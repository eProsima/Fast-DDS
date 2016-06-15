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

#include "AsyncWriterThread.h"
#include <fastrtps/rtps/writer/RTPSWriter.h>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif  // _MSC_VER
#include <boost/thread.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include <boost/thread/lock_guard.hpp>

#include <algorithm>
#include <cassert>

using namespace eprosima::fastrtps::rtps;

AsyncWriterThread::AsyncWriterThread() : thread_(nullptr)
{
}

AsyncWriterThread::~AsyncWriterThread()
{
    if(thread_ != nullptr)
    {
        thread_->interrupt();
        thread_->join();
        delete thread_;
    }
}

bool AsyncWriterThread::addWriter(RTPSWriter* writer)
{
    bool returnedValue = false;

    assert(writer != nullptr);

    if(writer->isAsync())
    {
        boost::lock_guard<boost::mutex> guard(mutex_);
        async_writers.push_back(writer);
        returnedValue = true;

        // If thread not running, start it.
        if(thread_ == nullptr)
        {
            thread_ = new boost::thread(&AsyncWriterThread::run, this);
        }
    }

    return returnedValue;
}

/*!
 * @brief This function removes a writer.
 * @param writer Asynchronous writer to be removed.
 * @return Result of the operation.
 */
bool AsyncWriterThread::removeWriter(RTPSWriter* writer)
{
    bool returnedValue = false;

    assert(writer != nullptr);

    boost::lock_guard<boost::mutex> guard(mutex_);
    auto it = std::find(async_writers.begin(), async_writers.end(), writer);

    if(it != async_writers.end())
    {
        async_writers.erase(it);
        returnedValue = true;

        // If there is not more asynchronous writers, stop the thread.
        if(async_writers.empty())
        {
            thread_->interrupt();
            thread_->join();
            delete thread_;
            thread_ = nullptr;
        }
    }

    return returnedValue;
}

void AsyncWriterThread::run()
{
    for(; ; )
    {
        try
        {
            // While the thread is in execution, it cannot be interrupted.
            {
                boost::this_thread::disable_interruption di;

                boost::lock_guard<boost::mutex> guard(mutex_);

                for(auto it = async_writers.begin(); it != async_writers.end(); ++it)
                {
                    (*it)->unsent_changes_not_empty();
                }

            }

            //TODO Make configurable the time.
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
        }
        catch(boost::thread_interrupted /*e*/)
        {
            return;
        }
    }
}
