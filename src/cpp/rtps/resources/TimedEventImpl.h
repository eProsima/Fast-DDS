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

#include <asio.hpp>

#include <thread>
#include <memory>
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
            /**
             * Timed Event class used to define any timed events.
             * All timedEvents must be a specification of this class, implementing the event method.
             *@ingroup MANAGEMENT_MODULE
             */
            class TimedEventImpl
            {
                using Callback = std::function<bool(TimedEvent::EventCode)>;
                public:

                    typedef enum
                    {
                        INACTIVE = 0,
                        READY,
                        WAITING,
                    } StateCode;

                    ~TimedEventImpl();

                    /**
                     * @param serv IO service
                     * @param milliseconds Interval of the timedEvent.
                     */
                    TimedEventImpl(
                            asio::io_service& service,
                            Callback callback,
                            std::chrono::microseconds interval);

                    /**
                     * Method invoked when the event occurs. Abstract method.
                     *
                     * @param code Code representing the status of the event
                     * @param msg Message associated to the event
                     */
                    void event(
                            std::weak_ptr<Callback> callback_weak_ptr,
                            const std::error_code& ec);

                protected:

                    //!Interval to be used in the timed Event.
                    std::chrono::microseconds m_interval_microsec;

                public:

                    TimedEventImpl* next() const
                    {
                        return next_;
                    }

                    void next(TimedEventImpl* next)
                    {
                        next_ = next;
                    }

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

                    bool go_ready();

                    bool go_cancel();

                    void update();

                    void terminate();

                private:

                    //!Pointer to the timer.
                    asio::steady_timer timer_;

                    Callback callback_;

                    std::shared_ptr<Callback> callback_ptr_;

                    std::atomic<StateCode> state_;

                    std::atomic<bool> cancel_;

                    TimedEventImpl* next_;

                    //Duration_t m_timeInfinite;
                    std::mutex mutex_;
            };



        }
    }
} /* namespace eprosima */
#endif
#endif /* PERIODICEVENT_H_ */
