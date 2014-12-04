/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Subscriber.h
 */


#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

#include "fastrtps/rtps/common/Guid.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps{

class SubscriberImpl;
class SampleInfo_t;
class SubscriberAttributes;



/**
 * Class Subscriber, contains the public API that allows the user to control the reception of messages.
 * This class should not be instantiated directly. DomainRTPSParticipant class should be used to correctly create this element.
 * @ingroup PUBSUBMODULE
 * @snippet pubsub_example.cpp ex_Subscriber
 */
class RTPS_DllAPI Subscriber
{
public:
	/**
	* @param pimpl Actual implementation of the subscriber
	*/
	Subscriber(SubscriberImpl* pimpl):mp_impl(pimpl){};
	virtual ~Subscriber(){};
	
	/**
	* Get the associated GUID
	* @return Associated GUID
	*/
	const GUID_t& getGuid();

	/**
	 * Method to block the current thread until an unread message is available
	 */
	void waitForUnreadMessage();

	/** @name Read or take data methods.
	 * Methods to read or take data from the History.
	 */

	///@{

	bool readNextData(void* data,SampleInfo_t* info);
	bool takeNextData(void* data,SampleInfo_t* info);
	///@}

	/**
	 * Update the Attributes of the subscriber;
	 * @param att Reference to a SubscriberAttributes object to update the parameters;
	 * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
	 */
	bool updateAttributes(SubscriberAttributes& att);
	
	/**
	 * Get the Attributes of the Subscriber.
	 * @return Attributes of the subscriber
	 */
	SubscriberAttributes getAttributes();


private:
	SubscriberImpl* mp_impl;
};



} /* namespace pubsub */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
