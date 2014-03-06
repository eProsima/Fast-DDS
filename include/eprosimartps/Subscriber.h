/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.h
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <iostream>

#include "rtps_all.h"
#include "ParameterList.h"
#include "common/rtps_messages.h"

#include <boost/signals2.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>


#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

namespace eprosima {

namespace rtps{
class RTPSReader;
}

using namespace rtps;

namespace dds {

typedef struct RTPS_DllAPI ReadElement_t{
	SequenceNumber_t seqNum;
	GUID_t writerGuid;
}ReadElement_t;


/**
 * Class Subscriber, contains the public API to perform actions when messages are received. This class should not be instantiated directly.
 * DomainParticipant class should be used to correctly initialize this element.
 * @ingroup DDSMODULE
 */
class RTPS_DllAPI Subscriber {
	friend class DomainParticipant;
	friend class RTPSReader;
public:
	Subscriber();
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
	 * Method to block the current thread until a new message is received.
	 */
	void blockUntilNewMessage();

	/**
	 * Assign a function to be executed each time a message is received.
	 * @param fun Pointer to the function
	 */
	void assignNewMessageCallback(void(*fun)());


	/**
	 * Read the older unread element: the one with the minimum sequence number for all possible writers that publish in the topic.
	 * @param data_ptr Pointer to an already allocated memory to enough space to hold an instance of the type associated with the topic.
	 * @return True if suceeded.
	 */
	bool readMinSeqUnread(void* data_ptr);
	bool readElement(ReadElement_t r_elem,void* data_ptr);
	/**
	 * Read all unread elements in the associated RTPSReader HistoryCache.
	 * @param data_vec
	 * @return
	 */
	bool readAllUnread(std::vector<void*> data_vec);
	bool takeMinSeqRead(void* data_ptr);

	/**
	 * Function to determine if the history is full
	 */
	bool isHistoryFull();
	/**
	 * Get number of read elements.
	 */
	int getReadElements_n();
	/**
	 * Get the number of elements currently stored in the HistoryCache.
	 */
	int getHistory_n();

	ParameterList ParamList;



private:
	/**
	 *
	 */
	bool isCacheRead(SequenceNumber_t sn,GUID_t guid);
	std::vector<ReadElement_t> readElements;
	std::vector<SequenceNumber_t> readCacheChanges;
	RTPSReader* R;
	//bool initialized;
	std::string topicName;
	std::string topicDataType;
	TypeReg_t type;

	//boost::signals2::signal<void()> msg_signal;

};

} /* namespace dds */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
