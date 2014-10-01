/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file UnsentChangesNotEmptyEvent.h
 *
 */

#ifndef UNSENTCHANGESNOTEMPTYEVENT_H_
#define UNSENTCHANGESNOTEMPTYEVENT_H_

#include "eprosimartps/utils/TimedEvent.h"

namespace eprosima {
namespace rtps {

class RTPSWriter;

class UnsentChangesNotEmptyEvent: public TimedEvent {
public:
	UnsentChangesNotEmptyEvent(RTPSWriter* writer,boost::posix_time::milliseconds interval);
	virtual ~UnsentChangesNotEmptyEvent();

	void event(const boost::system::error_code& ec);

	RTPSWriter* mp_writer;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* UNSENTCHANGESNOTEMPTYEVENT_H_ */
