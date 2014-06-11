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


namespace eprosima {

namespace rtps{
class RTPSReader;
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
	SubscriberImpl(RTPSReader* Rin,DDSTopicDataType* ptype);
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
	int getHistoryElementsNumber();

//	bool updateAttributes(const SubscriberAttributes& param);


/////@}

	/** @name Read or take data methods.
	 * Methods to read or take data from the History.
	 */

	///@{

	bool readNextData(void* data,SampleInfo_t* info);
	bool takeNextData(void* data,SampleInfo_t* info);

	///@}


	/**
	 * Add a writer proxy. Only until Discovery is good.
	 */
	bool addWriterProxy(Locator_t& loc,GUID_t& guid);

	size_t getMatchedPublishers();

const GUID_t& getGuid();

	RTPSReader* getReaderPtr() {
		return mp_Reader;
	}

private:
	//!Pointer to associated RTPSReader
	RTPSReader* mp_Reader;
	//! Pointer to the DDSTopicDataType object.
	DDSTopicDataType* mp_type;

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
	int getHistoryElementsNumber()
	{
			return mp_impl->getHistoryElementsNumber();
		}


	//	bool updateAttributes(const SubscriberAttributes& param);


	/////@}

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
	 * Add a writer proxy. Only until Discovery is good.
	 */
	bool addWriterProxy(Locator_t& loc,GUID_t& guid)
	{
		return mp_impl->addWriterProxy(loc,guid);
	}

	size_t getMatchedPublishers(){return mp_impl->getMatchedPublishers();}
private:
	SubscriberImpl* mp_impl;
};



} /* namespace dds */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
