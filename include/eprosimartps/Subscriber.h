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

class RTPS_DllAPI Subscriber {
	friend class DomainParticipant;
	friend class RTPSReader;
public:
	Subscriber();
	Subscriber(RTPSReader* Rin);
	virtual ~Subscriber();

	const std::string& getTopicDataType() const {
		return topicDataType;
	}

	const std::string& getTopicName() const {
		return topicName;
	}

	void blockUntilNewMessage();

	void assignNewMessageCallback(void(*fun)());

	///**
	// * Read all unread data.
	// * @param changes
	// * @return
	// */
	//bool readAllUnread(std::vector<void*>* data);
	///**
	// * Read a specific data, based on its Sequence Number.
	// * @param sn
	// * @param ch
	// * @return
	// */
	//bool readSeqNum(SequenceNumber_t sn,void* data);
	///**
	// * Read the older unread change (lowest SequenceNumber).
	// * @param ch
	// * @return
	// */
	//bool readOlderUnread(void* data);
	///**
	// * Take the older read change.
	// * @return
	// */
	//bool takeOlderRead();
	///**
	// * Take all read changes.
	// * @return
	// */
	//bool takeAllRead();

	bool isCacheRead(SequenceNumber_t,GUID_t guid);

	bool readMinSeqUnread(void* data_ptr);
	bool readElement(ReadElement_t r_elem,void* data_ptr);
	bool readAllUnread(std::vector<void*> data_vec);
	bool takeMinSeqRead(void* data_ptr);


	ParameterList ParamList;
	bool isHistoryFull();
	int getReadElements_n();
	int getHistory_n();


private:
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
