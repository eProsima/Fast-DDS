/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
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
 * @ingroup FASTRTPS_MODULE
 * @snippet fastrtps_example.cpp ex_Subscriber
 */
class RTPS_DllAPI Subscriber
{
	friend class SubscriberImpl;
	virtual ~Subscriber(){};
public:
	/**
	 * @param pimpl Actual implementation of the subscriber
	 */
	Subscriber(SubscriberImpl* pimpl):mp_impl(pimpl){};


	/**
	 * Get the associated GUID
	 * @return Associated GUID
	 */
	const GUID_t& getGuid();

	/**
	 * Method to block the current thread until an unread message is available
	 */
	void waitForUnreadMessage();

	/**
	 * Read next unread Data from the Subscriber.
	 * @param data Pointer to the object where you want the data stored.
	 * @param info Pointer to a SampleInfo_t structure that informs you about your sample.
	 * @return True if a sample was read.
	 */
	bool readNextData(void* data,SampleInfo_t* info);
	/**
	 * Take next Data from the Subscriber. The data is removed from the subscriber.
	 * @param data Pointer to the object where you want the data stored.
	 * @param info Pointer to a SampleInfo_t structure that informs you about your sample.
	 * @return True if a sample was taken.
	 */
	bool takeNextData(void* data,SampleInfo_t* info);


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
