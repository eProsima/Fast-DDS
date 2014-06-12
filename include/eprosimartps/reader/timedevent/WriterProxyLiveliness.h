/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxyLiveliness.h
 *
 *  Created on: Jun 12, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef WRITERPROXYLIVELINESS_H_
#define WRITERPROXYLIVELINESS_H_

#include "eprosimartps/utils/TimedEvent.h"

namespace eprosima {
namespace rtps {

class WriterProxy;

class WriterProxyLiveliness: public TimedEvent {
public:
	WriterProxyLiveliness(WriterProxy* wp,boost::posix_time::milliseconds interval);
	virtual ~WriterProxyLiveliness();
	void event(const boost::system::error_code& ec);
	//!Pointer to the WriterProxy associated with this specific event.
	WriterProxy* mp_WP;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERPROXYLIVELINESS_H_ */
