/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberListener.h
 *
 *  Created on: Apr 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RTPSLISTENER_H_
#define RTPSLISTENER_H_

namespace eprosima {
namespace dds {

/**
 * Class SubscriberListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup DDSMODULE
 */
class SubscriberListener {
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
	virtual void onSubscriptionMatched();
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* DDSLISTENER_H_ */
