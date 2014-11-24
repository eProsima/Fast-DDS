/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEvent.h
 *
 */

#ifndef TIMEDEVENT_H_
#define TIMEDEVENT_H_

#include <cstdint>
#include "fastrtps/rtps/common/Time_t.h"
namespace boost
{
namespace asio
{
class io_service;
}
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

class TimedEventImpl;


class TimedEvent {
public:
	enum EventCode
	{
		EVENT_SUCCESS,
		EVENT_ABORT,
		EVENT_MSG
	};
	TimedEvent(boost::asio::io_service* serv,double milliseconds);
	virtual ~TimedEvent();
	virtual void event(EventCode code,const char* msg=nullptr)=0;

	void restart_timer();

	void stop_timer();

	bool update_interval(const Duration_t& inter);

	bool update_interval_millisec(int64_t time_millisec);

    double getIntervalMilliSec();

    void stopSemaphorePost();

private:
	TimedEventImpl* mp_impl;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif
