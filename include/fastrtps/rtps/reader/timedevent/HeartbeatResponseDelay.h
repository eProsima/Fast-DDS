/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HeartbeatResponseDelay.h
 *
*/

#ifndef HEARTBEATRESPONSEDELAY_H_
#define HEARTBEATRESPONSEDELAY_H_

#include "fastrtps/rtps/resources/TimedEvent.h"
#include "fastrtps/rtps/common/CDRMessage_t.h"

namespace eprosima {
namespace fastrtps{
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
	
	/**
	* @param p_WP
	* @param interval
	*/
	HeartbeatResponseDelay(WriterProxy* p_WP,double interval);
	
	/**
	* Method invoked when the event occurs
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);
	
	//!Pointer to the WriterProxy associated with this specific event.
	WriterProxy* mp_WP;
	//!CDRMessage_t used in the response.
	CDRMessage_t m_heartbeat_response_msg;

};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* HEARTBEATRESPONSEDELAY_H_ */
