/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
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

class Subscriber;

/**
 * Class SubscriberListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup FASTRTPS_MODULE
 * @snippet fastrtps_example.cpp ex_SubscriberListener
 */
class RTPS_DllAPI SubscriberListener {
public:
	SubscriberListener(){};
	virtual ~SubscriberListener(){};
	/**
	 * Virtual function to be implemented by the user containing the actions to be performed when a new  Data Message is received.
	 * @param sub Subscriber
	 */
	virtual void onNewDataMessage(Subscriber * sub){};
//	/**
//	 * Virtual method to be called when the History is Full.
//	 * @param sub Subscriber
//	 */
//	virtual void onHistoryFull(Subscriber* sub){};
	/**
	 * Virtual method to be called when the subscriber is matched with a new Writer (or unmatched); i.e., when a writer publishing in the same topic is discovered.
	 * @param sub Subscriber
	 * @param info Matching information
	 */
	virtual void onSubscriptionMatched(Subscriber* sub, MatchingInfo& info){};
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* LISTENER_H_ */
