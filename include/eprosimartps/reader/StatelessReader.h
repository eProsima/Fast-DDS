/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatelessReader.h
 */


#ifndef STATELESSREADER_H_
#define STATELESSREADER_H_


#include "eprosimartps/reader/RTPSReader.h"

#include "eprosimartps/dds/attributes/SubscriberAttributes.h"



namespace eprosima {
namespace rtps {

/**
 * Class StatelessReader, specialization of the RTPSReader.
 * @ingroup READERMODULE
 */
class StatelessReader: public RTPSReader {
public:
	virtual ~StatelessReader();
	StatelessReader(const SubscriberAttributes& wParam,
			const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype);


	/**
	 * Read the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
	 * pointed by the StatusInfo_t structure.
	 * @param data Pointer to memory that can hold a sample.
	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
	 * @return True if correct.
	 */
	bool readNextCacheChange(void*data,SampleInfo_t* info);
	/**
	 * Take the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
	 * pointed by the StatusInfo_t structure.
	 * @param data Pointer to memory that can hold a sample.
	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
	 * @return True if correct.
	 */
	bool takeNextCacheChange(void*data,SampleInfo_t* info);
	//!Returns true if there are unread cacheChanges.
	bool isUnreadCacheChange();
	/**
	 * Get the number of matched publishers.
	 * @return True if correct.
	 */
	size_t getMatchedPublishers(){return 0;}
	/**
	 * Add a matched writer represented by a WriterProxyData object.
	 * @param wdata Pointer to the WPD object to add.
	 * @return True if correctly added.
	 */
	bool matched_writer_add(WriterProxyData* wdata);
	/**
	 * Remove a WriterProxyData from the matached writers.
	 * @param wdata Pointer to the WPD object.
	 * @return True if correct.
	 */
	bool matched_writer_remove(WriterProxyData* wdata);
	/**
	 * Tells us if a specific Writer is matched against this reader
	 * @param wdata Pointer to the WriterProxyData object
	 * @return True if it is matched.
	 */
	bool matched_writer_is_matched(WriterProxyData* wdata);

	//!Method to indicate the reader that some change has been removed due to HistoryQos requirements.
	bool change_removed_by_history(CacheChange_t*);
	//!Returns true if the reader accepts messages from the writer with GUID_t entityGUID.
	bool acceptMsgFrom(GUID_t& entityId);

private:
	//!List of GUID_t os matched writers.
	//!Is only used in the Discovery, to correctly notify the user using SubscriptionListener::onSubscriptionMatched();
	std::vector<WriterProxyData*> m_matched_writers;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATELESSREADER_H_ */
