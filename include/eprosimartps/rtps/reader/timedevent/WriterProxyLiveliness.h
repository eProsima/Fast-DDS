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
 */

#ifndef WRITERPROXYLIVELINESS_H_
#define WRITERPROXYLIVELINESS_H_

#include "eprosimartps/utils/TimedEvent.h"

namespace eprosima {
namespace rtps {

class WriterProxy;
/**
 * Class WriterProxyLiveliness, timed event to check the liveliness of a writer each leaseDuration.
 */
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
