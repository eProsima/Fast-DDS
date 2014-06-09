/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResourceEvent.h
 *
 */

#ifndef RESOURCEEVENT_H_
#define RESOURCEEVENT_H_


#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>




namespace eprosima {
namespace rtps {

class ParticipantImpl;

/**
 * Class ResourceEvent used to manage the temporal events.
 * @ingroup MANAGEMENTMODULE
 */
class ResourceEvent {
public:
	ResourceEvent(ParticipantImpl*p);
	virtual ~ResourceEvent();
	boost::thread* b_thread;
	boost::asio::io_service io_service;
	boost::asio::io_service::work work;

	/**
	 * Method to initialize the thread.
	 */
	void init_thread();
	/**
	 * Task to announce the correctness of the thread.
	 */
	void announce_thread();

	//!Method to run the tasks
	void run_io_service();

	ParticipantImpl* mp_participantImpl;
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* RESOURCEEVENT_H_ */
