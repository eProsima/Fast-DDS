/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackResponseDelay.h
 *
 */

#ifndef NACKRESPONSEDELAY_H_
#define NACKRESPONSEDELAY_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../../resources/TimedEvent.h"
#include "../../messages/RTPSMessageGroup.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulWriter;
class ReaderProxy;

/**
 * NackResponseDelay class use to delay the response to an NACK message.
 * @ingroup WRITER_MODULE
 */
class NackResponseDelay:public TimedEvent {
public:
	/**
	*
	* @param p_RP
	* @param intervalmillisec
	*/
	NackResponseDelay(ReaderProxy* p_RP,double intervalmillisec);
	virtual ~NackResponseDelay();

	/**
	* Method invoked when the event occurs
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);

	//!Associated reader proxy
	ReaderProxy* mp_RP;
	//!Messages
	RTPSMessageGroup_t m_cdrmessages;
};
}
}
} /* namespace eprosima */
#endif
#endif /* NACKRESPONSEDELAY_H_ */
