/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PeriodicHeartbeat.h
 *
 */

#ifndef PERIODICHEARTBEAT_H_
#define PERIODICHEARTBEAT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "fastrtps/rtps/resources/TimedEvent.h"
#include "fastrtps/rtps/common/CDRMessage_t.h"

namespace eprosima {
namespace fastrtps{
namespace rtps{

class StatefulWriter;


/**
 * PeriodicHeartbeat class, controls the periodic send operation of HB.
 * @ingroup WRITERMODULE
 */
class PeriodicHeartbeat: public TimedEvent {
public:
	/**
	*
	* @param p_RP
	* @param interval
	*/
	PeriodicHeartbeat(StatefulWriter* p_RP,double interval);
	virtual ~PeriodicHeartbeat();
	
	/**
	* Method invoked when the event occurs
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);

	//!
	CDRMessage_t m_periodic_hb_msg;
	//!
	StatefulWriter* mp_SFW;
};




}
}
} /* namespace eprosima */
#endif
#endif /* PERIODICHEARTBEAT_H_ */
