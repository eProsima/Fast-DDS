/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherListener.h	
 */

#ifndef PUBLISHERLISTENER_H_
#define PUBLISHERLISTENER_H_

#include "eprosimartps/rtps/common/Types.h"
#include "eprosimartps/rtps/common/MatchingInfo.h"

namespace eprosima {
namespace pubsub {

/**
 * Class PublisherListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup PUBSUBMODULE
 * @snippet pubsub_example.cpp ex_PublisherListener
 */
class RTPS_DllAPI PublisherListener
{
public:
	PublisherListener(){};
	virtual ~PublisherListener(){};
	/**
	 * This method is called when the Publisher is matched (or unatched) against an endpoint.
	 */
	virtual void onPublicationMatched(MatchingInfo info){};
	/**
	 * This method is called when the History is full.
	 */
	virtual void onHistoryFull(){};
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PUBLISHERLISTENER_H_ */
