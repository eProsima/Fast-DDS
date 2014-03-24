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
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef NACKRESPONSEDELAY_H_
#define NACKRESPONSEDELAY_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/timedevent/TimedEvent.h"


namespace eprosima {
namespace rtps {

class StatefulWriter;
class ReaderProxy;

class NackResponseDelay:public TimedEvent {
public:
	NackResponseDelay();
	NackResponseDelay(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval);
	virtual ~NackResponseDelay();

	void event(const boost::system::error_code& ec,ReaderProxy* rp);
	StatefulWriter* SW;
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* NACKRESPONSEDELAY_H_ */
