/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file UnsentChangesNotEmptyEvent.h
 *
 */

#ifndef UNSENTCHANGESNOTEMPTYEVENT_H_
#define UNSENTCHANGESNOTEMPTYEVENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "fastrtps/rtps/resources/TimedEvent.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class RTPSWriter;

/**
 * @ingroup WRITER_MODULE
 */
class UnsentChangesNotEmptyEvent: public TimedEvent {
public:
	/**
	*
	* @param writer
	* @param interval
	*/
	UnsentChangesNotEmptyEvent(RTPSWriter* writer,double interval);
	virtual ~UnsentChangesNotEmptyEvent();
	
	/**
	* Method invoked when the event occurs
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);
	
	//!
	RTPSWriter* mp_writer;
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* UNSENTCHANGESNOTEMPTYEVENT_H_ */
