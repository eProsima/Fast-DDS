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

#include "eprosimartps/utils/TimedEvent.h"
#include "eprosimartps/common/types/CDRMessage_t.h"

namespace eprosima {
namespace rtps{

class StatefulWriter;
class ReaderProxy;


/**
 * PeriodicHeartbeat class, controls the periodic send operation of HB.
 * @ingroup WRITERMODULE
 */
class PeriodicHeartbeat: public TimedEvent {
public:
	PeriodicHeartbeat(StatefulWriter* p_RP,boost::posix_time::milliseconds interval);
	virtual ~PeriodicHeartbeat();

	void event(const boost::system::error_code& ec);

	CDRMessage_t m_periodic_hb_msg;
	StatefulWriter* mp_SFW;
};





}
} /* namespace eprosima */

#endif /* PERIODICHEARTBEAT_H_ */
