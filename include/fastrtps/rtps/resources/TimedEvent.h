/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEvent.h
 *
 */

#ifndef TIMEDEVENT_H_
#define TIMEDEVENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <cstdint>
#include "../common/Time_t.h"
namespace boost
{
    class thread;

    namespace asio
    {
        class io_service;
    }
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

class TimedEventImpl;

/**
 * Timed Event class used to define any timed events.
 * @ingroup MANAGEMENT_MODULE
 */
class TimedEvent {
public:

	/**
	* Enum representing event statuses
	*/
	enum EventCode
	{
		EVENT_SUCCESS,
		EVENT_ABORT,
		EVENT_MSG
	};

    enum AUTODESTRUCTION_MODE
    {
        NONE,
        ON_SUCCESS,
        ALLWAYS
    };
	
	/**
	* @param serv IO service
	* @param milliseconds Interval of the timedEvent.
	*/
    TimedEvent(boost::asio::io_service &service, const boost::thread& event_thread, double milliseconds, TimedEvent::AUTODESTRUCTION_MODE autodestruction = TimedEvent::NONE);
	virtual ~TimedEvent();
	
	/**
	* Method invoked when the event occurs. Abstract method.
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event. It can be nullptr.
	*/
	virtual void event(EventCode code, const char* msg) = 0;

    void cancel_timer();
	
	//!Method to restart the timer.
	void restart_timer();
	
	/**
	* Update event interval.
	* When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
	*
	* @param inter New interval for the timedEvent
	* @return true on success
	*/
	bool update_interval(const Duration_t& inter);
	
	/**
	* Update event interval.
	* When updating the interval, the timer is not restarted and the new interval will only be used the next time you call restart_timer().
	*
	* @param time_millisec New interval for the timedEvent
	* @return true on success
	*/
	bool update_interval_millisec(double time_millisec);
	
	/**
	* Get the milliseconds interval
	* @return Mulliseconds interval
	*/
    double getIntervalMilliSec();
	
	/**
	* Get the remaining milliseconds for the timer to expire
	* @return Remaining milliseconds for the timer to expire
	*/
    double getRemainingTimeMilliSec();

private:
	TimedEventImpl* mp_impl;
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif
