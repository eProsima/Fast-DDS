/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Semaphore.h
 *
*/

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

namespace eprosima {
namespace rtps {

/**
 * Class Semaphore implements an easy to use Semaphore.
 * Based on Boost Mutex.
 */
class Semaphore {
public:
	Semaphore():m_count(0),is_waiting(false){};
	virtual ~Semaphore(){};
	void wait()
	{
		boost::mutex::scoped_lock lock(m_mutex);
		while(m_count <= 0)
		{
			is_waiting = true;
			m_condition.wait(lock);
		}
		is_waiting = false;
		--m_count;
	}

	void post()
	{
		boost::mutex::scoped_lock lock(m_mutex);
		++m_count;
		if(is_waiting)
			m_condition.notify_one();
	}

	void reset()
	{
		boost::mutex::scoped_lock lock(m_mutex);
		m_count = 0;
	}


private:
	uint32_t m_count;
	bool is_waiting;
	boost::mutex m_mutex;
	boost::condition_variable m_condition;



};

} /* namespace dds */
} /* namespace eprosima */

#endif /* SEMAPHORE_H_ */
