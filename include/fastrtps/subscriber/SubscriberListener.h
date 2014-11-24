/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberListener.h
 */

#ifndef SUBLISTENER_H_
#define SUBLISTENER_H_

#include "fastrtps/rtps/common/MatchingInfo.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

/**
 * Class SubscriberListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup PUBSUBMODULE
 * @snippet pubsub_example.cpp ex_SubscriberListener
 */
class RTPS_DllAPI SubscriberListener {
public:
	SubscriberListener() ;
	virtual ~SubscriberListener();
	/**
	 * Virtual function to be implemented by the user containing the actions to be performed when a new  Data Message is received.
	 */
	virtual void onNewDataMessage();
	/**
	 * Virtual method to be called when the History is Full.
	 */
	virtual void onHistoryFull();
	/**
	 * Virtual method to be called when the subscriber is matched with a new Writer (or unmatched); i.e., when a writer publishing in the same topic is discovered.
	 */
	virtual void onSubscriptionMatched(MatchingInfo info);
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* LISTENER_H_ */
