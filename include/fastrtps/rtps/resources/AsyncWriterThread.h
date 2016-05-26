/*************************************************************************
 * Copyright (c) 2016 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file AsyncWriterThread.h
 *
 */
#ifndef _RTPS_RESOURCES_ASYNCWRITERTHREAD_H_
#define _RTPS_RESOURCES_ASYNCWRITERTHREAD_H_

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <list>

namespace boost
{
    class thread;
}

namespace eprosima
{
namespace fastrtps
{
namespace rtps
{
   class RTPSWriter;

   /*!
    * @brief This static class owns a thread that manages asynchronous writes.
    * Asynchronous writes happen directly (when using an async writer) and
    * indirectly (when responding to a NACK).
    */
   class AsyncWriterThread
   {
       public:
           /*!
            * @brief Adds a writer to be managed by this thread.
            * Only asynchronous writers are permitted.
            * @param writer Asynchronous writer to be added. 
            * @return Result of the operation.
            */
           static bool addWriter(RTPSWriter& writer);

           /*!
            * @brief Removes a writer.
            * @param writer Asynchronous writer to be removed.
            * @return Result of the operation.
            */
           static bool removeWriter(RTPSWriter& writer);

           /*
            * Wakes the thread up.
            */
           static void wakeUp();

           /*!
            * @brief Destructor
            * The destructor is not thread-safe.
            */

       private:
           AsyncWriterThread() = delete;
           ~AsyncWriterThread() = delete;
           AsyncWriterThread(const AsyncWriterThread&) = delete;
           const AsyncWriterThread& operator=(const AsyncWriterThread&) = delete;

           //! @brief runs main method
           static void run();

           static std::thread* thread_;
           static std::mutex mutex_;
           
           //! List of asynchronous writers.
           static std::list<RTPSWriter*> async_writers;

           static std::atomic<bool> running_;
           static std::atomic<bool> run_scheduled_;
           static std::condition_variable cv_;
   };
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_RESOURCES_ASYNCWRITERTHREAD_H_
