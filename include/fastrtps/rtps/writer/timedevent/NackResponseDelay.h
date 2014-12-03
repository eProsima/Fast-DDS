/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackResponseDelay.h
 *
 */

#ifndef NACKRESPONSEDELAY_H_
#define NACKRESPONSEDELAY_H_

#include "fastrtps/rtps/resources/TimedEvent.h"
#include "fastrtps/rtps/messages/RTPSMessageGroup.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulWriter;
class ReaderProxy;

/**
 * NackResponseDelay class use to delay the response to an NACK message.
 * @ingroup WRITERMODULE
 */
class NackResponseDelay:public TimedEvent {
public:
	/**
	*
	* @param p_RP
	* @param intervalmillisec	*/
	NackResponseDelay(ReaderProxy* p_RP,double intervalmillisec);
	virtual ~NackResponseDelay();

	/**
	*
	* @param code
	* @param msg
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

#endif /* NACKRESPONSEDELAY_H_ */
