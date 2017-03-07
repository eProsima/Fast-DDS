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
 * @file TimedEventImpl.h
 *
 */



#ifndef TIMEDEVENTIMPL_H_
#define TIMEDEVENTIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/rtps/common/Time_t.h>
#include <fastrtps/rtps/resources/TimedEvent.h>

#include <memory>

#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include <asio/deadline_timer.hpp>
#include <asio/placeholders.hpp>
#include <asio/io_service.hpp>

#include <fastrtps/utils/Semaphore.h>

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <system_error>



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
                    TimedEventImpl(TimedEvent* ev, asio::io_service &service, const std::thread& event_thread, std::chrono::microseconds interval, TimedEvent::AUTODESTRUCTION_MODE autodestruction);

                    /**
                     * Method invoked when the event occurs. Abstract method.
                     *
                     * @param code Code representing the status of the event
                     * @param msg Message associated to the event
                     */
                    void event(const std::error_code& ec, const std::shared_ptr<TimerState>& state);


                protected:
                    //!Pointer to the timer.
                    asio::steady_timer timer_;
                    //!Interval to be used in the timed Event.
                    std::chrono::microseconds m_interval_microsec;
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
                        auto total_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(m_interval_microsec);
                        return static_cast<double>(total_milliseconds.count());
                    }

                    /**
                     * Get the remaining milliseconds for the timer to expire
                     * @return Remaining milliseconds for the timer to expire
                     */
                    double getRemainingTimeMilliSec()
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(timer_.expires_from_now()).count());
                    }

                private:

                    TimedEvent::AUTODESTRUCTION_MODE autodestruction_;
                    //Duration_t m_timeInfinite;
                    std::mutex mutex_;
                    std::condition_variable cond_;

                    std::shared_ptr<TimerState> state_;

                    std::thread::id event_thread_id_;
            };



        }
    }
} /* namespace eprosima */
#endif
#endif /* PERIODICEVENT_H_ */
