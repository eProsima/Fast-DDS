/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PeriodicHeartbeat.h
 *
 *  Created on: Mar 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PERIODICHEARTBEAT_H_
#define PERIODICHEARTBEAT_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/timedevent/TimedEvent.h"


namespace eprosima {
namespace rtps{

class StatefulWriter;
class ReaderProxy;

class PeriodicHeartbeat:public TimedEvent {
public:
	PeriodicHeartbeat(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval);
	virtual ~PeriodicHeartbeat();

	void event(const boost::system::error_code& ec,ReaderProxy*RP);
	StatefulWriter* SW;
	CDRMessage_t m_periodic_hb_msg;
};





}
} /* namespace eprosima */

#endif /* PERIODICHEARTBEAT_H_ */
