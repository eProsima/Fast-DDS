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
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

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

	virtual ~StatefulReader();
private:
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

	/**
	 * Check if the reader accepts messages from a writer with a specific GUID_t.
	 *
	 * @param entityGUID GUID to check
	 * @param wp Pointer to pointer of the WriterProxy. Since we already look for it wee return the pointer
	 * so the execution can run faster.
	 * @return true if the reader accepts messages from the writer with GUID_t entityGUID.
	 */
	bool acceptMsgFrom(GUID_t& entityGUID,WriterProxy** wp = nullptr);

	/**
	 * Method to indicate the reader that some change has been removed due to HistoryQos requirements.
	 * @param change Pointer to the CacheChange_t.
	 * @param prox Pointer to the WriterProxy.
	 * @return True if correctly removed.
	 */
	bool change_removed_by_history(CacheChange_t*,WriterProxy* prox = nullptr);

	/**
	 * This method is called when a new change is received. This method calls the received_change of the History
	 * and depending on the implementation performs different actions.
	 * @param a_change Pointer of the change to add.
	 * @param prox Pointer to the WriterProxy that adds the Change.
	 * @return True if added.
	 */
	bool change_received(CacheChange_t* a_change,WriterProxy* prox = nullptr);

	/**
	 * Get the RTPS participant
	 * @return Associated RTPS participant
	 */
	inline RTPSParticipantImpl* getRTPSParticipant() const {return mp_RTPSParticipant;}

	/**
	 * Read the next unread CacheChange_t from the history
	 * @param change POinter to pointer of CacheChange_t
	 * @return True if read.
	 */
	bool nextUnreadCache(CacheChange_t** change,WriterProxy** wpout=nullptr);

	/**
	 * Take the next CacheChange_t from the history;
	 * @param change Pointer to pointer of CacheChange_t
	 * @return True if read.
	 */
	bool nextUntakenCache(CacheChange_t** change,WriterProxy** wpout=nullptr);


	/**
	 * Update the times parameters of the Reader.
	 * @param times ReaderTimes reference.
	 * @return True if correctly updated.
	 */
	bool updateTimes(ReaderTimes& times);

	/**
	 *
	 * @return Reference to the ReaderTimes.
	 */
	inline ReaderTimes& getTimes(){return m_times;};

	/**
	 * Get the number of matched writers
	 * @return Number of matched writers
	 */
	inline size_t getMatchedWritersSize() const {return matched_writers.size();};

private:

	//!ReaderTimes of the StatefulReader.
	ReaderTimes m_times;
	//! Vector containing pointers to the matched writers.
	std::vector<WriterProxy*> matched_writers;
};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* STATEFULREADER_H_ */
