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
*/

#ifndef HEARTBEATRESPONSEDELAY_H_
#define HEARTBEATRESPONSEDELAY_H_

#include "eprosimartps/timedevent/TimedEvent.h"
#include "eprosimartps/common/types/CDRMessage_t.h"
namespace eprosima {
namespace rtps {

class StatefulReader;
class WriterProxy;

/**
 * Class HeartbeatResponseDelay, TimedEvent used to delay the response to a specific HB.
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
