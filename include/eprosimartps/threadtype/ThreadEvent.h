/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThreadEvent.h
 *
 *  Created on: Mar 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef THREADEVENT_H_
#define THREADEVENT_H_


#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "eprosimartps/rtps_all.h"


namespace eprosima {
namespace rtps {

class ThreadEvent {
public:
	ThreadEvent();
	virtual ~ThreadEvent();
	boost::thread* b_thread;
	boost::asio::io_service io_service;
	boost::asio::io_service::work work;

	/**
	 * Method to initialize the thread.
	 */
	void init_thread();
	void announce_thread();
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* THREADEVENT_H_ */
