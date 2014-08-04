/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Subscriber.h
 */


#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_
#include <iostream>

#include "eprosimartps/common/types/Locator.h"
#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/dds/attributes/SubscriberAttributes.h"

namespace eprosima {

namespace rtps{
class RTPSReader;
class ParticipantImpl;
}

using namespace rtps;

namespace dds {

class DDSTopicDataType;
class SubscriberListener;
class SampleInfo_t;






/**
 * Class SubscriberImpl, contains the actual implementation of the behaviour of the Subscriber.
 */
class SubscriberImpl {
public:
	SubscriberImpl(ParticipantImpl* p,RTPSReader* Rin,DDSTopicDataType* ptype);
	virtual ~SubscriberImpl();

	/**
	 * Method to block the current thread until an unread message is available
	 */
	void waitForUnreadMessage();

	/**
	 * Assign a RTPSListener to perform actions when certain events happen.
	 * @param[in] p_listener Pointer to the RTPSListener.
	 */
	bool assignListener(SubscriberListener* p_listener);


	/**
	 * Function to determine if the history is full
	 */
	bool isHistoryFull();

	/**
	 * Get the number of elements currently stored in the HistoryCache.
	 */
	size_t getHistoryElementsNumber();


	/** @name Read or take data methods.
	 * Methods to read or take data from the History.
	 */

	///@{

	bool readNextData(void* data,SampleInfo_t* info);
	bool takeNextData(void* data,SampleInfo_t* info);

	///@}
	/**
	 * Update the Attributes of the subscriber;
	 * @param rqos Reference to a ReaderQos object to update the parameters;
	 * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
	 */
	bool updateAttributes(SubscriberAttributes& att);


	size_t getMatchedPublishers();

	const GUID_t& getGuid();

	RTPSReader* getReaderPtr() {
		return mp_Reader;
	}
	/**
	 * Get the Attributes of the Subscriber.
	 */
	SubscriberAttributes getAttributes(){return m_attributes;}

private:
	//!Pointer to associated RTPSReader
	RTPSReader* mp_Reader;
	//! Pointer to the DDSTopicDataType object.
	DDSTopicDataType* mp_type;
	//!Attributes of the Subscriber
	SubscriberAttributes m_attributes;
	//!Pointer to the participant
	ParticipantImpl* mp_participant;

};

/**
 * Class Subscriber, contains the public API that allows the user to control the reception of messages.
 * This class should not be instantiated directly. DomainParticipant class should be used to correctly create this element.
 * @ingroup DDSMODULE
 * @snippet dds_example.cpp ex_Subscriber
 */
class RTPS_DllAPI Subscriber
{
public:
	Subscriber(SubscriberImpl* pimpl):mp_impl(pimpl){};
	virtual ~Subscriber(){};
	const GUID_t& getGuid()
	{
		return mp_impl->getGuid();
	}

	/**
	 * Method to block the current thread until an unread message is available
	 */
	void waitForUnreadMessage()
	{
		return mp_impl->waitForUnreadMessage();
	}

	/**
	 * Assign a RTPSListener to perform actions when certain events happen.
	 * @param[in] p_listener Pointer to the RTPSListener.
	 */
	bool assignListener(SubscriberListener* p_listener)
	{
		return mp_impl->assignListener(p_listener);
	}


	/**
	 * Function to determine if the history is full
	 */
	bool isHistoryFull()
	{
		return mp_impl->isHistoryFull();
	}

	/**
	 * Get the number of elements currently stored in the HistoryCache.
	 */
	size_t getHistoryElementsNumber()
	{
		return mp_impl->getHistoryElementsNumber();
	}



	/** @name Read or take data methods.
	 * Methods to read or take data from the History.
	 */

	///@{

	bool readNextData(void* data,SampleInfo_t* info)
	{
		return mp_impl->readNextData(data,info);
	}
	bool takeNextData(void* data,SampleInfo_t* info)
	{
		return mp_impl->takeNextData(data,info);
	}
	///@}

	/**
	 * Update the Attributes of the subscriber;
	 * @param rqos Reference to a ReaderQos object to update the parameters;
	 * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
	 */
	bool updateAttributes(SubscriberAttributes& att)
	{
		return mp_impl->updateAttributes(att);
	}
	/**
	 * Get the Attributes of the Subscriber.
	 */
	SubscriberAttributes getAttributes(){return mp_impl->getAttributes();}


	size_t getMatchedPublishers(){return mp_impl->getMatchedPublishers();}
private:
	SubscriberImpl* mp_impl;
};



} /* namespace dds */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
