/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEventImpl.h
 *
 */



#ifndef TIMEDEVENTIMPL_H_
#define TIMEDEVENTIMPL_H_

#include <boost/asio/io_service.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "fastrtps/rtps/common/Time_t.h"



namespace eprosima {
namespace fastrtps{
namespace rtps{

class TimedEvent;

/**
 * Timed Event class used to define any timed events.
 * All timedEvents must be a specification of this class, implementing the event method.
 * @ingroup MANAGEMENTMODULE
 */
class TimedEventImpl {
public:
	virtual ~TimedEventImpl();
	//! A io_service must be provided as well as the interval of the timedEvent.
	TimedEventImpl(TimedEvent* ev,boost::asio::io_service* serv,boost::posix_time::milliseconds interval);
	//! Pure abstract virtual method used to perform the event.
	void event(const boost::system::error_code& ec);


protected:
	//!Pointer to the timer.
	boost::asio::deadline_timer* timer;
	//!Interval to be used in the timed Event.
	boost::posix_time::milliseconds m_interval_msec;
	//!TimedEvent pointer
	TimedEvent* mp_event;
public:
	bool m_isWaiting;
	//!Method to restart the timer.
	void restart_timer();
	//! TO update the interval, the timer is not restarted and the new interval will onyl be used the next time you call restart_timer().
	bool update_interval(const Duration_t& time);
	bool update_interval_millisec(int64_t time_millisec);
	//!Stop the timer
	void stop_timer();

	double getIntervalMsec()
	{
		return m_interval_msec.total_nanoseconds()/1000;
	}

	double getRemainingTimeMilliSec()
	{
		return (double)timer->expires_from_now().total_milliseconds();
	}
	//Duration_t m_timeInfinite;
	//!Semaphore to wait for the listen thread creation.
	boost::interprocess::interprocess_semaphore* mp_stopSemaphore;
};



}
}
} /* namespace eprosima */

#endif /* PERIODICEVENT_H_ */
