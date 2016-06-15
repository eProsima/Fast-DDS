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
             * @brief This class manages the thread which manages the asynchronous writers.
             * This thread is created and running only when there are asynchrnous writers.
             * @remarks This class is thread-safe.
             */
            class AsyncWriterThread
            {
                public:

                    /*!
                     * @brief Default constructor.
                     */
                    AsyncWriterThread();

                    /*!
                     * @brief Destructor
                     * The destructor is not thread-safe.
                     */
                    ~AsyncWriterThread();

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

                private:

                    //! @brief This function run the thread.
                    void run();

                    //! Thread
                    boost::thread* thread_;

                    //! Mutex
                    boost::mutex mutex_;

                    //! List of asynchronous writers.
                   std::list<RTPSWriter*> async_writers;
            };
        } // namespace rtps
    } // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_RESOURCES_ASYNCWRITERTHREAD_H_
