/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberListener.h
 */

#ifndef RTPSLISTENER_H_
#define RTPSLISTENER_H_

#include "eprosimartps/common/types/common_types.h"
#include "eprosimartps/dds/MatchingInfo.h"
namespace eprosima {
namespace dds {

/**
 * Class SubscriberListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup DDSMODULE
 * @snippet dds_example.cpp ex_SubscriberListener
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
	 * Virtual method to be called when the subscriber is matched with a new Writer; i.e., when a writer publishing in the same topic is discovered.
	 */
	virtual void onSubscriptionMatched(MatchingInfo info);
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* DDSLISTENER_H_ */
