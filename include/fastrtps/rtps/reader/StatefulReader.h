/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulReader.h
 */

#ifndef STATEFULREADER_H_
#define STATEFULREADER_H_


#include "fastrtps/rtps/reader/RTPSReader.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

class WriterProxy;

/**
 * Class StatefulReader, specialization of RTPSReader than stores the state of the matched writers.
 * @ingroup READERMODULE
 */
class StatefulReader:public RTPSReader {
public:
	friend class RTPSParticipantImpl;
private:
	virtual ~StatefulReader();
	StatefulReader(RTPSParticipantImpl*,GUID_t& guid,
			ReaderAttributes& att,ReaderHistory* hist,ReaderListener* listen=nullptr);
public:
	/**
	 * Add a matched writer represented by a WriterProxyData object.
	 * @param wdata Pointer to the WPD object to add.
	 * @return True if correctly added.
	 */
	bool matched_writer_add(RemoteWriterAttributes& wdata);
	/**
	 * Remove a WriterProxyData from the matached writers.
	 * @param wdata Pointer to the WPD object.
	 * @return True if correct.
	 */
	bool matched_writer_remove(RemoteWriterAttributes& wdata);
	/**
	 * Tells us if a specific Writer is matched against this reader
	 * @param wdata Pointer to the WriterProxyData object
	 * @return True if it is matched.
	 */
	bool matched_writer_is_matched(RemoteWriterAttributes& wdata);
	/**
	 * Look for a specific WriterProxy.
	 * @param writerGUID GUID_t of the writer we are looking for.
	 * @param WP Pointer to pointer to a WriterProxy.
	 * @return True if found.
	 */
	bool matched_writer_lookup(GUID_t& writerGUID,WriterProxy** WP);

	//!Returns true if the reader accepts messages from the writer with GUID_t entityGUID.
	bool acceptMsgFrom(GUID_t& entityGUID,WriterProxy** wp = nullptr);
	//!Method to indicate the reader that some change has been removed due to HistoryQos requirements.
	bool change_removed_by_history(CacheChange_t*,WriterProxy* prox = nullptr);
	//!
	bool change_received(CacheChange_t* a_change,WriterProxy* prox = nullptr);


	inline RTPSParticipantImpl* getRTPSParticipant() const {return mp_RTPSParticipant;}

//	/**
//	 * Read the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
//	 * pointed by the StatusInfo_t structure.
//	 * @param data Pointer to memory that can hold a sample.
//	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
//	 * @return True if correct.
//	 */
//	bool readNextCacheChange(void*data,SampleInfo_t* info);
//	/**
//	 * Take the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
//	 * pointed by the StatusInfo_t structure.
//	 * @param data Pointer to memory that can hold a sample.
//	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
//	 * @return True if correct.
//	 */
//	bool takeNextCacheChange(void*data,SampleInfo_t* info);
//
//	bool readNextCacheChange(CacheChange_t** change);
//	//!Returns true if there are unread cacheChanges.
//	bool isUnreadCacheChange();
//	/**
//	 * Get the number of matched publishers.
//	 * @return True if correct.
//	 */
//	size_t getMatchedPublishers(){return matched_writers.size();}
//
//	std::vector<WriterProxy*>::iterator MatchedWritersBegin(){return matched_writers.begin();}
//	std::vector<WriterProxy*>::iterator MatchedWritersEnd(){return matched_writers.end();}
//	//!Method to indicate the reader that some change has been removed due to HistoryQos requirements.
//	bool change_removed_by_history(CacheChange_t*,WriterProxy*prox = NULL);
//	//!Returns true if the reader accepts messages from the writer with GUID_t entityGUID.
//	bool acceptMsgFrom(GUID_t& entityId,WriterProxy**wp=NULL);
//
//	bool updateTimes(SubscriberTimes time);
//
//	bool add_change(CacheChange_t* a_change,WriterProxy* prox = NULL);

	inline ReaderTimes& getTimes(){return m_times;};

private:
	ReaderTimes m_times;
	//! Vector containing pointers to the matched writers.
	std::vector<WriterProxy*> matched_writers;
};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATEFULREADER_H_ */
