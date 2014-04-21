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

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/ParameterList.h"
#include "eprosimartps/common/rtps_messages.h"

#include <boost/signals2.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "eprosimartps/dds/DDSTopicDataType.h"
namespace eprosima {

namespace rtps{
class RTPSReader;
}

using namespace rtps;

namespace dds {

struct RTPS_DllAPI ReadElement_t{
	SequenceNumber_t seqNum;
	GUID_t writerGuid;
	ReadElement_t& operator=(const ReadElement_t& rElem)
	{
		seqNum = rElem.seqNum;
		writerGuid = rElem.writerGuid;
		return *this;
	}
};


/**
 * Class Subscriber, contains the public API to perform actions when messages are received. This class should not be instantiated directly.
 * DomainParticipant class should be used to correctly initialize this element.
 * @ingroup DDSMODULE
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
	 * Method to block the current thread until a new message is received.
	 */
	void blockUntilNewMessage();

	/**
	 * Assign a function to be executed each time a message is received.
	 * @param[in] fun_ptr Pointer to the function that must be called.
	 */
	void assignNewMessageCallback(void(*fun_ptr)());


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

	/** @name Read or take data methods.
	 * Methods to read or take data from the History.
	 */

	///@{


	/**
	 * Read the unread element with the minimum sequence number (for all possible writers).
	 * @param[out] data_ptr Pointer to an already allocated memory to enough space to hold an instance of the type associated with the topic.
	 * @return True if correct.
	 * @par Calling example:
	 * @snippet dds_example.cpp ex_readMinSeqUnread
	 */
	bool readMinSeqUnreadCache(void* data_ptr);
	/**
	 * Read a specific cache from the History. The sequence number and the
	 * GUID_t of the writer should be provided, as well as a pointer to enough allocated space
	 * to contain an instance of the type transmitted through the topic.
	 * @param[in] sn SequenceNumber_t of the cache to read
	 * @param[in] guid GUID_t of the writer that introduced that element.
	 * @param[out] data_ptr Pointer to a memory space big enough to fit an instance of the data transmitted in the topic.
	 * @return True if correct.
	 */
	bool readCache(SequenceNumber_t& sn, GUID_t& guid,void* data_ptr);
	/**
	 * Read all unread elements in the associated RTPSReader HistoryCache.
     * @param[out] data_vec Pointer to a vector of pointers to elements.
	 * @return True if correct.
	 * @par Calling example (NOT YET FULLY TESTED, possible memory allocation problems):
	 * @snippet dds_example.cpp ex_readAllUnreadCache
	 */
	bool readAllUnreadCache(std::vector<void*>* data_vec);

	/**
	 * Read the cache change with the minimum sequence number: the one with the minimum sequence number for all possible writers that publish in the topic.
	 * It dowsn't matter if it was already read previously.
	 * @param[out] data_ptr Pointer to where teh data should be stored.
	 * @param[out] minSeqNum Pointer to save the sequence number
	 * @param[out] minSeqNumGuid Pointer to save the GUID_t
	 * @return True if correct.
	 */
	bool readMinSeqCache(void* data_ptr,SequenceNumber_t* minSeqNum, GUID_t* minSeqNumGuid);
	/**
	 * Read all Caches in the History. Look Subscriber::readAllUnreadCache for calling example.
	 * @param[out] data_vec Pointer to a vector of pointers to elements.
	 * @return True if correct.
	 */
	bool readAllCache(std::vector<void*>* data_vec);
	/**
	 * Take the element with the minimum sequence number.
	 * @param[out] data_ptr Pointer to allocated space to contain an instance of the topic data.
	 * @return True if correct.
	 */
	bool takeMinSeqCache(void* data_ptr);
	/**
	 * Take all elements in the cache. Look Subscriber::readAllUnreadCache for calling example.
	 * @param[out] data_vec Pointer to a vector of pointers to elements.
	 * @return True if correct.
	 */
	bool takeAllCache(std::vector<void*>* data_vec);

	/**
	 * Read the last element added to the history.
	 */
	bool readLastAdded(void* data_ptr);

///@}

	ParameterList_t ParamList;

	bool addWriterProxy(Locator_t& loc,GUID_t& guid);


private:
	bool isCacheRead(SequenceNumber_t& sn,GUID_t& guid);
	bool minSeqRead(SequenceNumber_t* sn,GUID_t* guid,std::vector<ReadElement_t>::iterator* min_it);
	bool removeSeqFromRead(SequenceNumber_t& sn, GUID_t& guid);
	std::vector<ReadElement_t> readElements;
	RTPSReader* mp_Reader;
	std::string topicName;
	std::string topicDataType;
	DDSTopicDataType* mp_type;


};

} /* namespace dds */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
