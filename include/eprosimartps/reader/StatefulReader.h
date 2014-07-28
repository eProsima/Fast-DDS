/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulReader.h
 */

#ifndef STATEFULREADER_H_
#define STATEFULREADER_H_


#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/WriterProxy.h"


namespace eprosima {
namespace rtps {

/**
 * Class StatefulReader, specialization of RTPSReader than stores the state of the matched writers.
 * @ingroup READERMODULE
 */
class StatefulReader:public RTPSReader {
public:
	//StatefulReader();
	virtual ~StatefulReader();
	StatefulReader(const SubscriberAttributes& wParam,
			const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype);


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
	/**
	 * Look for a specific WriterProxy.
	 * @param writerGUID GUID_t of the writer we are looking for.
	 * @param WP Pointer to pointer to a WriterProxy.
	 * @return True if found.
	 */
	bool matched_writer_lookup(GUID_t& writerGUID,WriterProxy** WP);

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
	size_t getMatchedPublishers(){return matched_writers.size();}

	std::vector<WriterProxy*>::iterator MatchedWritersBegin(){return matched_writers.begin();}
	std::vector<WriterProxy*>::iterator MatchedWritersEnd(){return matched_writers.end();}
	//!Method to indicate the reader that some change has been removed due to HistoryQos requirements.
	bool change_removed_by_history(CacheChange_t*);
	//!Returns true if the reader accepts messages from the writer with GUID_t entityGUID.
	bool acceptMsgFrom(GUID_t& entityId);

	bool updateTimes(SubscriberTimes time);

private:
	SubscriberTimes m_SubTimes;
	//! Vector containing pointers to the matched writers.
	std::vector<WriterProxy*> matched_writers;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATEFULREADER_H_ */
