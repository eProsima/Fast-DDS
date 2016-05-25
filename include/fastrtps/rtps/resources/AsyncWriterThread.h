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

#include <boost/thread/mutex.hpp>
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
    * @brief This singleton manages the thread which manages the asynchronous writers.
    * This thread is created and running only when there are asynchrnous writers.
    * @remarks This class is thread-safe.
    */
   class AsyncWriterThread
   {
       public:
           /*!
            * @brief This function adds a writer to be managed by this thread.
            * Only asynchronous writers are permitted.
            * @param writer Asynchronous writer to be added. Pointer cannot be null.
            * @return Result of the operation.
            */
           bool addWriter(RTPSWriter* writer);

           /*!
            * @brief This function removes a writer.
            * @param writer Asynchronous writer to be removed. Pointer cannot be null.
            * @return Result of the operation.
            */
           bool removeWriter(RTPSWriter* writer);

           /*!
            * @brief Returns the only singleton instance of AsyncWriterThread.
            */
           static AsyncWriterThread* instance();

           /*!
            * @brief Destructor
            * The destructor is not thread-safe.
            */
           ~AsyncWriterThread();

       private:

           /*!
            * @brief Default constructor.
            */
           AsyncWriterThread();

            //! Singleton, hence no copy.
           AsyncWriterThread(const AsyncWriterThread&) = delete;
           
            //! Singleton, hence no assignment.
           const AsyncWriterThread& operator=(const AsyncWriterThread&) = delete;

           //! @brief runs main method
           void run();

           //! Thread
           boost::thread* thread_;

           //! Mutex
           boost::mutex mutex_;
           
           static AsyncWriterThread* instance_;

           //! List of asynchronous writers.
          std::list<RTPSWriter*> async_writers;
   };
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_RESOURCES_ASYNCWRITERTHREAD_H_
