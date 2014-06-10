/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackResponseDelay.h
 *
 */

#ifndef NACKRESPONSEDELAY_H_
#define NACKRESPONSEDELAY_H_

#include "eprosimartps/timedevent/TimedEvent.h"
#include "eprosimartps/writer/RTPSMessageGroup.h"


namespace eprosima {
namespace rtps {

class StatefulWriter;
class ReaderProxy;

/**
 * NackResponseDelay class use to delay the response to an NACK message.
 * @ingroup WRITERMODULE
 */
class NackResponseDelay:public TimedEvent {
public:
	NackResponseDelay(ReaderProxy* p_RP,boost::posix_time::milliseconds interval);
	virtual ~NackResponseDelay();

	void event(const boost::system::error_code& ec);

	ReaderProxy* mp_RP;
	RTPSMessageGroup_t m_cdrmessages;
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* NACKRESPONSEDELAY_H_ */
