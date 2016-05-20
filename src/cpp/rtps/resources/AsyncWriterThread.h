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
#include <fastrtps/rtps/filters/FlowFilter.h>
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

                    /*!
                     * @brief Adds a flow filter to the back of the filter list.
                     * @param filter Filter to add.
                     */
                    void add_flow_filter(std::unique_ptr<FlowFilter> filter);

                private:

                    //! @brief This function run the thread.
                    void run();

                    //! Thread
                    boost::thread* thread_;

                    //! Mutex
                    boost::mutex mutex_;

                    //! List of asynchronous writers.
                   std::list<RTPSWriter*> async_writers;
                   std::vector<std::unique_ptr<FlowFilter> > m_filters;
            };
        } // namespace rtps
    } // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_RESOURCES_ASYNCWRITERTHREAD_H_
