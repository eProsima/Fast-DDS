/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Publisher.h 	
 */



#ifndef PUBLISHER_H_
#define PUBLISHER_H_
#include <iostream>

#include "eprosimartps/common/types/Locator.h"
#include "eprosimartps/common/types/Guid.h"

namespace eprosima {

namespace rtps{

class RTPSWriter;

}

using namespace rtps;

/**
 * DDS namespace. Contains the public API to interact with the DDS-RTPS protocol.
 * @ingroup DDSMODULE
 */
namespace dds {

class DDSTopicDataType;
class PublisherListener;


/**
 * Class PublisherImpl, contains the actual implementation of the behaviour of the Publisher.
 */
class PublisherImpl {
public:

	/**
	 * Create a publisher, assigning its pointer to the associated writer.
	 * Don't use directly, create Publisher using DomainParticipant static function.
	 */
	PublisherImpl(RTPSWriter* Win,DDSTopicDataType* ptype);

	virtual ~PublisherImpl();



	/**
	 * Write data to the topic.
	 * @param Data Pointer to the data
	 * @return True if correct
	 * @par Calling example:
	 * @snippet dds_example.cpp ex_PublisherWrite
	 */
	bool write(void*Data);

	/**
	 * Dispose of a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool dispose(void*Data);
	/**
	 * Unregister a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool unregister(void*Data);
	/**
		 * Dispose and unregister a previously written data.
		 * @param Data Pointer to the data.
		 * @return True if correct.
		 */
	bool dispose_and_unregister(void*Data);


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


	//Since there is no discovery:
	/** @name Discovery substitution methods.
	 * Since no discovery is implemented in this version, these methods are needed.
	 */
	/// @{
//	/**
//	 * Add a Reader Locator to the publisher.
//	 * @param Loc Locator to add.
//	 * @param expectsInlineQos Parameter to indicate whether or not the locator expects inlineQos with its Data messages.
//	 * @return True if correct.
//	 */
//	bool addReaderLocator(Locator_t& Loc,bool expectsInlineQos);
//
//	/**
//	 * Add a reader proxy to the publisher.
//	 */
//	bool addReaderProxy(Locator_t& loc,GUID_t& guid,bool expectsInline);


	///@}


	bool assignListener(PublisherListener* listen);


	const GUID_t& getGuid();

	RTPSWriter* getWriterPtr() {
			return mp_Writer;
		}

	size_t getMatchedSubscribers();

private:
	//! Pointer to the associated Data Writer.
	RTPSWriter* mp_Writer;
	//! Pointer to the DDSTopicDataType object.
	DDSTopicDataType* mp_type;

};


/**
 * Class Publisher, contains the public API to send new data. This class should not be instantiated directly.
 * DomainParticipant class should be used to correctly initialize this element.
 * @ingroup DDSMODULE
 * @snippet dds_example.cpp ex_Publisher
 */
class RTPS_DllAPI Publisher{
public:
	Publisher(PublisherImpl* pin):mp_impl(pin){};
	virtual ~Publisher(){};


	bool assignListener(PublisherListener* listen_in)
	{
		return mp_impl->assignListener(listen_in);
	}
	/**
	 * Write data to the topic.
	 * @param Data Pointer to the data
	 * @return True if correct
	 * @par Calling example:
	 * @snippet dds_example.cpp ex_PublisherWrite
	 */
	bool write(void*Data)
	{
		return mp_impl->write(Data);
	}

	/**
	 * Dispose of a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool dispose(void*Data)
	{
		return mp_impl->dispose(Data);
	}
	/**
	 * Unregister a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool unregister(void*Data)
	{
		return mp_impl->unregister(Data);
	}
	/**
	 * Dispose and unregister a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool dispose_and_unregister(void*Data)
	{
		return mp_impl->dispose_and_unregister(Data);
	}


	/**
	 * Removes the cache change with the minimum sequence number
	 * @return True if correct.
	 */
	bool removeMinSeqChange()
	{
		return mp_impl->removeMinSeqChange();
	}
	/**
	 * Removes all changes from the History.
	 * @param[out] removed Number of removed elements
	 * @return True if correct.
	 */
	bool removeAllChange(size_t* removed)
	{
		return mp_impl->removeAllChange(removed);
	}

	/**
	 * Get the number of elements in the History.
	 * @return Number of elements in the History.
	 */
	size_t getHistoryElementsNumber()
	{
		return mp_impl->getHistoryElementsNumber();
	}


	//Since there is no discovery:
	/** @name Discovery substitution methods.
	 * Since no discovery is implemented in this version, these methods are needed.
	 */
	/// @{
//	/**
//	 * Add a Reader Locator to the publisher.
//	 * @param Loc Locator to add.
//	 * @param expectsInlineQos Parameter to indicate whether or not the locator expects inlineQos with its Data messages.
//	 * @return True if correct.
//	 */
//	bool addReaderLocator(Locator_t& Loc,bool expectsInlineQos)
//	{
//		return mp_impl->addReaderLocator(Loc, expectsInlineQos);
//	}
//
//	/**
//	 * Add a reader proxy to the publisher.
//	 */
//	bool addReaderProxy(Locator_t& loc,GUID_t& guid,bool expectsInline)
//	{
//		return mp_impl->addReaderProxy(loc,guid, expectsInline);
//	}

	const GUID_t& getGuid()
	{
		return mp_impl->getGuid();
	}

	size_t getMatchedSubscribers()
		{
			return mp_impl->getMatchedSubscribers();
		}

private:
	PublisherImpl* mp_impl;
};


} /* namespace dds */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
