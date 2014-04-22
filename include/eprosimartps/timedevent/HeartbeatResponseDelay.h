/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HeartbeatResponseDelay.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef HEARTBEATRESPONSEDELAY_H_
#define HEARTBEATRESPONSEDELAY_H_
#include "eprosimartps/rtps_all.h"
#include "eprosimartps/timedevent/TimedEvent.h"

namespace eprosima {
namespace rtps {

class StatefulReader;
class WriterProxy;

/**
 * HeartbeatResponseDelay class used to response to a specific HB.
 * @ingroup READERMODULE
 */
class HeartbeatResponseDelay:public TimedEvent {
public:
	virtual ~HeartbeatResponseDelay();
	HeartbeatResponseDelay(WriterProxy* p_WP,boost::posix_time::milliseconds interval);

	void event(const boost::system::error_code& ec);
	//!Pointer to the WriterProxy associated with this specific event.
	WriterProxy* mp_WP;
	//!CDRMessage_t used in the response.
	CDRMessage_t m_heartbeat_response_msg;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* HEARTBEATRESPONSEDELAY_H_ */
