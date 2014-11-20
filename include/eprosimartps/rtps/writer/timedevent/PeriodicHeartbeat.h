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
 */

#ifndef PERIODICHEARTBEAT_H_
#define PERIODICHEARTBEAT_H_

#include "eprosimartps/rtps/resources/TimedEvent.h"
#include "eprosimartps/rtps/common/CDRMessage_t.h"

namespace eprosima {
namespace rtps{

class StatefulWriter;


/**
 * PeriodicHeartbeat class, controls the periodic send operation of HB.
 * @ingroup WRITERMODULE
 */
class PeriodicHeartbeat: public TimedEvent {
public:
	PeriodicHeartbeat(StatefulWriter* p_RP,double interval);
	virtual ~PeriodicHeartbeat();

	void event(EventCode code, const char* msg= nullptr);

	CDRMessage_t m_periodic_hb_msg;
	StatefulWriter* mp_SFW;
};





}
} /* namespace eprosima */

#endif /* PERIODICHEARTBEAT_H_ */
