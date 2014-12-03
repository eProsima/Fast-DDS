/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackSupressionDuration.h
 *
 */

#ifndef NACKSUPRESSIONDURATION_H_
#define NACKSUPRESSIONDURATION_H_

#include "fastrtps/rtps/resources/TimedEvent.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulWriter;
class ReaderProxy;

/**
 * NackSupressionDuration class, used to avoid too "recent" NACK messages.
 * @ingroup WRITERMODULE
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
	*
	* @param code
	* @param msg
	*/
	void event(EventCode code, const char* msg= nullptr);

	//!Reader proxy
	ReaderProxy* mp_RP;
};

}
}
} /* namespace eprosima */

#endif /* NACKSUPRESSIONDURATION_H_ */
