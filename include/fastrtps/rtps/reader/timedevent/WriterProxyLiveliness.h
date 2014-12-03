/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxyLiveliness.h
 *
 */

#ifndef WRITERPROXYLIVELINESS_H_
#define WRITERPROXYLIVELINESS_H_

#include "fastrtps/rtps/resources/TimedEvent.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class WriterProxy;
/**
 * Class WriterProxyLiveliness, timed event to check the liveliness of a writer each leaseDuration.
 */
class WriterProxyLiveliness: public TimedEvent {
public:
	/**
	* @param wp
	* @param interval
	*/
	WriterProxyLiveliness(WriterProxy* wp,double interval);
	virtual ~WriterProxyLiveliness();
	/**
	* @param code
	* @param msg
	*/
	void event(EventCode code, const char* msg= nullptr);
	//!Pointer to the WriterProxy associated with this specific event.
	WriterProxy* mp_WP;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERPROXYLIVELINESS_H_ */
