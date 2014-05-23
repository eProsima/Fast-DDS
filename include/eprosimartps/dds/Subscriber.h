/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Subscriber.h
 *  Subscriber public API.
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */


#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_
#include <iostream>

#include "eprosimartps/common/types/Locator.h"
#include "eprosimartps/common/types/Guid.h"

#include "eprosimartps/qos/ParameterList.h"


#include <boost/signals2.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/dds/SubscriberListener.h"

#include "eprosimartps/dds/SampleInfo.h"

namespace eprosima {

namespace rtps{
class RTPSReader;
}

using namespace rtps;

namespace dds {





/**
 * Class Subscriber, contains the public API to perform actions when messages are received. This class should not be instantiated directly.
 * DomainParticipant class should be used to correctly initialize this element.
 * @ingroup DDSMODULE
 * @snippet dds_example.cpp ex_Subscriber
 */
class RTPS_DllAPI Subscriber {
	friend class DomainParticipant;
	friend class RTPSReader;
public:
	Subscriber(RTPSReader* Rin);
	virtual ~Subscriber();

	/**
	 * Get the topic data type.
	 */
	const std::string& getTopicDataType() const {
		return topicDataType;
	}
	/**
	 * Get the topic Name.
	 */
	const std::string& getTopicName() const {
		return topicName;
	}

	/**
	 * Method to block the current thread until an unread message is available
	 */
	void waitForUnreadMessage();

	/**
	 * Assign a RTPSListener to perform actions when certain events happen.
	 * @param[in] p_listener Pointer to the RTPSListener.
	 */
	void assignListener(SubscriberListener* p_listener);


	/**
	 * Function to determine if the history is full
	 */
	bool isHistoryFull();

	/**
	 * Get the number of elements currently stored in the HistoryCache.
	 */
	int getHistory_n();

//	bool updateAttributes(const SubscriberAttributes& param);


/////@}

	/** @name Read or take data methods.
	 * Methods to read or take data from the History.
	 */

	///@{

	bool readNextData(void* data,SampleInfo_t* info);
	bool takeNextData(void* data,SampleInfo_t* info);

	///@}

	ParameterList_t ParamList;

	/**
	 * Add a writer proxy. Only until Discovery is good.
	 */
	bool addWriterProxy(Locator_t& loc,GUID_t& guid);


private:

	RTPSReader* mp_Reader;
	std::string topicName;
	std::string topicDataType;


};

} /* namespace dds */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
