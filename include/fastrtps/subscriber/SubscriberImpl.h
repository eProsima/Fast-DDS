/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberImpl.h
 *
 */

#ifndef SUBSCRIBERIMPL_H_
#define SUBSCRIBERIMPL_H_

#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/rtps/common/Guid.h"

#include "fastrtps/attributes/SubscriberAttributes.h"
#include "fastrtps/subscriber/SubscriberHistory.h"
#include "fastrtps/rtps/reader/ReaderListener.h"


namespace eprosima {
namespace fastrtps {
namespace rtps
{
class RTPSReader;
}

using namespace rtps;

class TopicDataType;
class SubscriberListener;
class ParticipantImpl;
class SampleInfo_t;

/**
 * Class SubscriberImpl, contains the actual implementation of the behaviour of the Subscriber.
 */
class SubscriberImpl {
	friend class ParticipantImpl;
public:
	SubscriberImpl(ParticipantImpl* p,TopicDataType* ptype,
			SubscriberAttributes& attr,SubscriberListener* listen = nullptr);
	virtual ~SubscriberImpl();

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




	const GUID_t& getGuid();

	/**
	 * Get the Attributes of the Subscriber.
	 */
	SubscriberAttributes getAttributes(){return m_att;}

private:
	//!Participant
	ParticipantImpl* mp_participant;
	//!Pointer to associated RTPSReader
	RTPSReader* mp_reader;
	//! Pointer to the TopicDataType object.
	TopicDataType* mp_type;
	//!Attributes of the Subscriber
	SubscriberAttributes m_att;
	//!History
	SubscriberHistory m_history;
	//!Listener
	SubscriberListener* mp_listener;
	class SubscriberReaderListener : public ReaderListener
	{
	public:
		SubscriberReaderListener(SubscriberImpl* s): mp_subscriberImpl(s){};
		virtual ~SubscriberReaderListener(){};
		void onReaderMatched(MatchingInfo info);
		void onNewCacheChangeAdded(RTPSReader * reader,CacheChange_t* change);
		SubscriberImpl* mp_subscriberImpl;
	}m_readerListener;

};


} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* SUBSCRIBERIMPL_H_ */
