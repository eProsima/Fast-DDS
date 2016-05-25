#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <boost/thread.hpp>
#include <boost/thread/lock_guard.hpp>

#include <algorithm>
#include <cassert>
#include <stdexcept>

using namespace eprosima::fastrtps::rtps;

AsyncWriterThread* AsyncWriterThread::instance_ = nullptr;

AsyncWriterThread::AsyncWriterThread() : thread_(nullptr), running_(false), run_scheduled_(false)
{
}

AsyncWriterThread::~AsyncWriterThread()
{
    if(thread_ != nullptr)
    {
        running_ = false;
        thread_->join();
        delete thread_;
    }
    instance_ = nullptr;
}

AsyncWriterThread* AsyncWriterThread::instance()
{
   if (!instance_)
      instance_ = new AsyncWriterThread();
   return instance_;
}

bool AsyncWriterThread::addWriter(RTPSWriter& writer)
{
    bool returnedValue = false;

     std::lock_guard<std::mutex> guard(mutex_);
     async_writers.push_back(&writer);
     returnedValue = true;

     // If thread not running, start it.
     if(thread_ == nullptr)
     {
         running_ = true;
         run_scheduled_ = true;
         thread_ = new std::thread(&AsyncWriterThread::run, this);
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

    std::unique_lock<std::mutex> guard(mutex_);
    auto it = std::find(async_writers.begin(), async_writers.end(), &writer);

    if(it != async_writers.end())
    {
        async_writers.erase(it);
        returnedValue = true;

        // If there is not more asynchronous writers, stop the thread.
        if(async_writers.empty())
        {
            running_ = false;
            run_scheduled_ = false;
            guard.unlock();
            cv_.notify_all();
            thread_->join();
            guard.lock();
            delete thread_;
            thread_ = nullptr;
        }
    }

    return returnedValue;
}

void AsyncWriterThread::wakeUp()
{
   run_scheduled_ = true;
   cv_.notify_all();
}

void AsyncWriterThread::run()
{
    while(running_)
    {
       std::unique_lock<std::mutex> guard(mutex_);
       while(run_scheduled_ && running_)
       {
          run_scheduled_ = false;
          for(auto it = async_writers.begin(); it != async_writers.end(); ++it)
          {
              (*it)->send_any_unsent_changes();
          }
       }

       cv_.wait(guard);
    }
}
