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




namespace boost
{
class thread;
namespace asio
{
class io_service;
//class io_service::work;
}
}


namespace eprosima {
namespace rtps {

class ParticipantImpl;

/**
 * Class ResourceEvent used to manage the temporal events.
 * @ingroup MANAGEMENTMODULE
 */
class ResourceEvent {
public:
	ResourceEvent();
	virtual ~ResourceEvent();

	boost::asio::io_service* getIOService(){return mp_io_service;};

	boost::thread* mp_b_thread;
	boost::asio::io_service* mp_io_service;
	boost::asio::io_service::work* mp_work;

	/**
	 * Method to initialize the thread.
	 */
	void init_thread(ParticipantImpl*p);
	/**
	 * Task to announce the correctness of the thread.
	 */
	void announce_thread();

	//!Method to run the tasks
	void run_io_service();

	ParticipantImpl* mp_participantImpl;
};

}
} /* namespace eprosima */

#endif /* RESOURCEEVENT_H_ */
