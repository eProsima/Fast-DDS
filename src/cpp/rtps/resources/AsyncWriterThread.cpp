#include "AsyncWriterThread.h"
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <boost/thread.hpp>
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
            thread_ = nullptr;
        }
    }

    return returnedValue;
}

void AsyncWriterThread::run()
{
    do
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
        catch(boost::thread_interrupted e)
        {
            return;
        }
    } while(1);
}
