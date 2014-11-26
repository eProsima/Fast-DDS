/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Publisher.h 	
 */



#ifndef PUBLISHERIMPL_H_
#define PUBLISHERIMPL_H_


#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/rtps/common/Guid.h"

#include "fastrtps/attributes/PublisherAttributes.h"

#include "fastrtps/publisher/PublisherHistory.h"

#include "fastrtps/rtps/writer/WriterListener.h"

namespace eprosima {
namespace fastrtps{
namespace rtps
{
class RTPSWriter;
}

using namespace rtps;
/**
 * FASTRTPS namespace. Contains the public API to interact with the RTPS protocol.
 * @ingroup FASTRTPSMODULE
 */


class TopicDataType;
class PublisherListener;
class ParticipantImpl;


/**
 * Class PublisherImpl, contains the actual implementation of the behaviour of the Publisher.
 */
class PublisherImpl {
	friend class ParticipantImpl;
public:

	/**
	 * Create a publisher, assigning its pointer to the associated writer.
	 * Don't use directly, create Publisher using DomainRTPSParticipant static function.
	 */
	PublisherImpl(ParticipantImpl* p,TopicDataType* ptype,
			PublisherAttributes& att,PublisherListener* p_listen = nullptr);

	virtual ~PublisherImpl();


	bool create_new_change(ChangeKind_t kind, void* Data);


	/**
	 * Removes the cache change with the minimum sequence number
	 * @return True if correct.
	 */
	bool removeMinSeqChange();
	/**
	 * Removes all changes from the History.
	 * @param[out] removed Number of removed elements
	 * @return True if correct.
	 */
	bool removeAllChange(size_t* removed);

	/**
	 * Get the number of elements in the History.
	 * @return Number of elements in the History.
	 */
	size_t getHistoryElementsNumber();


	const GUID_t& getGuid();

	/**
	 * Update the Attributes of the publisher;
	 * @param att Reference to a PublisherAttributes object to update the parameters;
	 * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
	 */
	bool updateAttributes(PublisherAttributes& att);

	/**
	 * Get the Attributes of the Subscriber.
	 */
	inline const PublisherAttributes& getAttributes(){ return m_att; };

private:
	ParticipantImpl* mp_participant;
	//! Pointer to the associated Data Writer.
	RTPSWriter* mp_writer;
	//! Pointer to the TopicDataType object.
	TopicDataType* mp_type;
	//!Attributes of the Publisher
	PublisherAttributes m_att;
	//!Publisher History
	PublisherHistory m_history;
	//!PublisherListener
	PublisherListener* mp_listener;
	//!Listener to capture the events of the Writer
	class PublisherWriterListener: public WriterListener
	{
	public:
		PublisherWriterListener(PublisherImpl* p):mp_publisherImpl(p){};
		virtual ~PublisherWriterListener(){};
		void onWriterMatched(MatchingInfo info);
		PublisherImpl* mp_publisherImpl;
	}m_writerListener;

};

//
///**
// * Class Publisher, contains the public API to send new data. This class should not be instantiated directly.
// * DomainRTPSParticipant class should be used to correctly initialize this element.
// * @ingroup MODULE
// * @snippet _example.cpp ex_Publisher
// */
//class RTPS_DllAPI Publisher{
//public:
//	Publisher(PublisherImpl* pin):mp_impl(pin){};
//	virtual ~Publisher(){};
//
//
//	bool assignListener(PublisherListener* listen_in)
//	{
//		return mp_impl->assignListener(listen_in);
//	}
//	/**
//	 * Write data to the topic.
//	 * @param Data Pointer to the data
//	 * @return True if correct
//	 * @par Calling example:
//	 * @snippet pubsub_example.cpp ex_PublisherWrite
//	 */
//	bool write(void*Data)
//	{
//		return mp_impl->write(Data);
//	}
//
//	/**
//	 * Dispose of a previously written data.
//	 * @param Data Pointer to the data.
//	 * @return True if correct.
//	 */
//	bool dispose(void*Data)
//	{
//		return mp_impl->dispose(Data);
//	}
//	/**
//	 * Unregister a previously written data.
//	 * @param Data Pointer to the data.
//	 * @return True if correct.
//	 */
//	bool unregister(void*Data)
//	{
//		return mp_impl->unregister(Data);
//	}
//	/**
//	 * Dispose and unregister a previously written data.
//	 * @param Data Pointer to the data.
//	 * @return True if correct.
//	 */
//	bool dispose_and_unregister(void*Data)
//	{
//		return mp_impl->dispose_and_unregister(Data);
//	}
//
//
//	/**
//	 * Removes the cache change with the minimum sequence number
//	 * @return True if correct.
//	 */
//	bool removeMinSeqChange()
//	{
//		return mp_impl->removeMinSeqChange();
//	}
//	/**
//	 * Removes all changes from the History.
//	 * @param[out] removed Number of removed elements
//	 * @return True if correct.
//	 */
//	bool removeAllChange(size_t* removed)
//	{
//		return mp_impl->removeAllChange(removed);
//	}
//
//	/**
//	 * Get the number of elements in the History.
//	 * @return Number of elements in the History.
//	 */
//	size_t getHistoryElementsNumber()
//	{
//		return mp_impl->getHistoryElementsNumber();
//	}
//
//
//	const GUID_t& getGuid()
//	{
//		return mp_impl->getGuid();
//	}
//
//	size_t getMatchedSubscribers()
//	{
//		return mp_impl->getMatchedSubscribers();
//	}
//
//	/**
//	 * Update the Attributes of the Publisher;
//	 * @param att Reference to a PublisherAttributes object to update the parameters;
//	 * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
//	 */
//	bool updateAttributes(PublisherAttributes& att)
//	{
//		return mp_impl->updateAttributes(att);
//	}
//	/**
//	 * Get the Attributes of the Publisher.
//	 */
//	PublisherAttributes getAttributes(){return mp_impl->getAttributes();}
//
//private:
//	PublisherImpl* mp_impl;
//};


} /* namespace  */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
