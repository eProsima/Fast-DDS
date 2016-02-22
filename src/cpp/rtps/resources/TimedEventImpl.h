/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEventImpl.h
 *
 */



#ifndef TIMEDEVENTIMPL_H_
#define TIMEDEVENTIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/rtps/common/Time_t.h>
#include <fastrtps/rtps/resources/TimedEvent.h>

#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>



namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            class TimerState;

            /**
             * Timed Event class used to define any timed events.
             * All timedEvents must be a specification of this class, implementing the event method.
             *@ingroup MANAGEMENT_MODULE
             */
            class TimedEventImpl 
            {
                public:

                    ~TimedEventImpl();

                    /**
                     * @param serv IO service
                     * @param milliseconds Interval of the timedEvent.
                     */
                    TimedEventImpl(TimedEvent* ev, boost::asio::io_service &service, const boost::thread& event_thread, boost::posix_time::microseconds interval, TimedEvent::AUTODESTRUCTION_MODE autodestruction);

                    /**
                     * Method invoked when the event occurs. Abstract method.
                     *
                     * @param code Code representing the status of the event
                     * @param msg Message associated to the event
                     */
                    void event(const boost::system::error_code& ec, const std::shared_ptr<TimerState>& state);


                protected:
                    //!Pointer to the timer.
                    boost::asio::deadline_timer timer_;
                    //!Interval to be used in the timed Event.
                    boost::posix_time::microseconds m_interval_microsec;
                    //!TimedEvent pointer
                    TimedEvent* mp_event;

                public:
                    //!Method to restart the timer.
                    void restart_timer();

                    /**
                     * Update event interval.
                     * When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
                     *
                     * @param inter New interval for the timedEvent
                     * @return true on success
                     */
                    bool update_interval(const Duration_t& time);

                    /**
                     * Update event interval.
                     * When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
                     *
                     * @param time_millisec New interval for the timedEvent
                     * @return true on success
                     */
                    bool update_interval_millisec(double time_millisec);

                    void cancel_timer();

                    void destroy();

                    /**
                     * Get interval in milliseconds
                     * @return Event interval in milliseconds
                     */
                    double getIntervalMsec()
                    {
                        return (double)(m_interval_microsec.total_microseconds()/1000);
                    }

                    /**
                     * Get the remaining milliseconds for the timer to expire
                     * @return Remaining milliseconds for the timer to expire
                     */
                    double getRemainingTimeMilliSec()
                    {
                        boost::unique_lock<boost::mutex> lock(mutex_);
                        return (double)timer_.expires_from_now().total_milliseconds();
                    }

                private:

                    TimedEvent::AUTODESTRUCTION_MODE autodestruction_;
                    //Duration_t m_timeInfinite;
                    boost::mutex mutex_;
                    boost::condition_variable cond_;

                    std::shared_ptr<TimerState> state_;

                    boost::thread::id event_thread_id_;
            };



        }
    }
} /* namespace eprosima */
#endif
#endif /* PERIODICEVENT_H_ */
