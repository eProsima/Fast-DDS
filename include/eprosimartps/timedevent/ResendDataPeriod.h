/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResendDataPeriod.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RESENDDATAPERIOD_H_
#define RESENDDATAPERIOD_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/timedevent/TimedEvent.h"

namespace eprosima {
namespace rtps {

class StatelessWriter;

class ResendDataPeriod: public TimedEvent {
public:
	ResendDataPeriod(StatelessWriter* SLW,boost::posix_time::milliseconds interval);
	virtual ~ResendDataPeriod();

	void event(const boost::system::error_code& ec);

	CDRMessage_t m_data_msg;
	StatelessWriter* mp_SLW;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESENDDATAPERIOD_H_ */
