/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulWriter.h
 *
 */

#ifndef STATEFULWRITER_H_
#define STATEFULWRITER_H_


#include "fastrtps/rtps/writer/RTPSWriter.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class PeriodicHeartbeat;
class ReaderProxy;

/**
 * Class StatefulWriter, specialization of RTPSWriter that maintains information of each matched Reader.
 * @ingroup WRITERMODULE
 */
class StatefulWriter: public RTPSWriter
{
	friend class RTPSParticipantImpl;
private:
	//!Destructor
	virtual ~StatefulWriter();

	//!Constructor
	StatefulWriter(RTPSParticipantImpl*,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen=nullptr);

	Count_t m_heartbeatCount;
	//!Timed Event to manage the periodic HB to the Reader.
	PeriodicHeartbeat* mp_periodicHB;
	//!WriterTimes
	WriterTimes m_times;
	//! Vector containin all the associated ReaderProxies.
	std::vector<ReaderProxy*> matched_readers;
	//!EntityId used to send the HB.(only for builtin types performance)
	EntityId_t m_HBReaderEntityId;
public:
	/**
	 * Add a specific change to all ReaderLocators.
	 * @param p Pointer to the change.
	 */
	void unsent_change_added_to_history(CacheChange_t* p);
	/**
	 * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
	 * @param a_change Pointer to the change that is going to be removed.
	 * @return True if removed correctly.
	 */
	bool change_removed_by_history(CacheChange_t* a_change);
	/**
	 * Method to indicate that there are changes not sent in some of all ReaderProxy.
	 */
	void unsent_changes_not_empty();
	//!Increment the HB count.
	inline void incrementHBCount(){ ++m_heartbeatCount; };
	/**
	 * Add a matched reader.
	 * @param rdata Pointer to the ReaderProxyData object added.
	 * @return True if added.
	 */
	bool matched_reader_add(RemoteReaderAttributes& ratt);
	/**
	 * Remove a matched reader.
	 * @param rdata Pointer to the object to remove.
	 * @return True if removed.
	 */
	bool matched_reader_remove(RemoteReaderAttributes& ratt);
	/**
	 * Tells us if a specific Reader is matched against this writer
	 * @param rdata Pointer to the ReaderProxyData object
	 * @return True if it was matched.
	 */
	bool matched_reader_is_matched(RemoteReaderAttributes& ratt);
	/**
	 * Remove the change with the minimum SequenceNumber
	 * @return True if removed.
	 */
	bool is_acked_by_all(CacheChange_t* a_change);
	/**
	 * Update the Attributes of the Writer.
	 */
	void updateAttributes(WriterAttributes& att);

	/**
	 * Find a Reader Proxy in this writer.
	 * @param[in] readerGuid The GUID_t of the reader.
	 * @param[out] RP Pointer to pointer to return the ReaderProxy.
	 * @return True if correct.
	 */
	bool matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP);

	inline Count_t getHeartbeatCount() const {return this->m_heartbeatCount;};

	inline EntityId_t getHBReaderEntityId() {return this->m_HBReaderEntityId;};

	inline std::vector<ReaderProxy*>::iterator matchedReadersBegin(){return this->matched_readers.begin();};

	inline std::vector<ReaderProxy*>::iterator matchedReadersEnd(){return this->matched_readers.end();};

	inline RTPSParticipantImpl* getRTPSParticipant() const {return mp_RTPSParticipant;}


	void updateTimes(WriterTimes& times);
private:
	//
	//
	//	/**
	//	 * Add a matched reader.
	//	 * @param rdata Pointer to the ReaderProxyData object added.
	//	 * @return True if added.
	//	 */
	//	bool matched_reader_add(ReaderProxyData* rdata);
	//	/**
	//	 * Remove a matched reader.
	//	 * @param rdata Pointer to the object to remove.
	//	 * @return True if removed.
	//	 */
	//	bool matched_reader_remove(ReaderProxyData* rdata);
	//	/**
	//	 * Tells us if a specific Reader is matched against this writer
	//	 * @param rdata Pointer to the ReaderProxyData object
	//	 * @return True if it was matched.
	//	 */
	//	bool matched_reader_is_matched(ReaderProxyData* rdata);
	//	/**
	//	 * Find a Reader Proxy in this writer.
	//	 * @param[in] readerGuid The GUID_t of the reader.
	//	 * @param[out] RP Pointer to pointer to return the ReaderProxy.
	//	 * @return True if correct.
	//	 */
	//	bool matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP);
	//
	//	/**
	//	 * Find out if a change is acked by all ReaderProxy.
	//	 * @param change Pointer to the change.
	//	 * @return True if correct.
	//	 */
	//	bool is_acked_by_all(CacheChange_t* change);
	//
	//	/**
	//	 * Add the provided change to the unsent changes vectors of all matched Readers.
	//	 * @param[in] change Pointer to the change
	//	 */
	//	void unsent_change_add(CacheChange_t* change);
	//

	//	/**
	//	 * Remove the change with the minimum SequenceNumber
	//	 * @return True if removed.
	//	 */
	//	bool removeMinSeqCacheChange();
	//	/**
	//	 * Remove all changes from history
	//	 * @param n_removed Pointer to return the number of elements removed.
	//	 * @return True if correct.
	//	 */
	//	bool removeAllCacheChange(size_t* n_removed);

	//
	//
	//	auto matchedReadersBegin()
	//	{
	//		return matched_readers.begin();
	//	};
	//	auto matchedReadersEnd()
	//	{
	//		return matched_readers.end();
	//	}
	//	size_t matchedReadersSize()
	//	{
	//		return matched_readers.size();
	//	}
	//	Count_t getHeartbeatCount() const {
	//		return m_heartbeatCount;
	//	}
	//
	//	size_t getMatchedSubscribers(){return matched_readers.size();}
	//
	//	EntityId_t& getHBReaderEntityId()
	//	{
	//		return m_HBReaderEntityId;
	//	}
	//
	//	bool change_removed_by_history(CacheChange_t* a_change);
	//
	//	void updateTimes(WriterTimes& times);
	//
	//private:

	//
	//	WriterTimes m_PubTimes;
	//
	//
	//



};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATEFULWRITER_H_ */
