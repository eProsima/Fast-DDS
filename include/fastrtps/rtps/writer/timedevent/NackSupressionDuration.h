/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackSupressionDuration.h
 *
 */

#ifndef NACKSUPRESSIONDURATION_H_
#define NACKSUPRESSIONDURATION_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "fastrtps/rtps/resources/TimedEvent.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulWriter;
class ReaderProxy;

/**
 * NackSupressionDuration class, used to avoid too "recent" NACK messages.
 * @ingroup WRITER_MODULE
 */
class NackSupressionDuration : public TimedEvent
{
public:
	virtual ~NackSupressionDuration();
	/**
	*
	* @param p_RP
	* @param intervalmillisec
	*/
	NackSupressionDuration(ReaderProxy* p_RP,double intervalmillisec);

	/**
	* Method invoked when the event occurs
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);

	//!Reader proxy
	ReaderProxy* mp_RP;
};

}
}
} /* namespace eprosima */
#endif
#endif /* NACKSUPRESSIONDURATION_H_ */
