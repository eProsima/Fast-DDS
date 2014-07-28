/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulWriter.h
 *
 */

#ifndef STATEFULWRITER_H_
#define STATEFULWRITER_H_


#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/writer/ReaderProxy.h"

typedef std::vector<ReaderProxy*>::iterator p_ReaderProxyIterator;

namespace eprosima {
namespace rtps {

/**
 * Class StatefulWriter, specialization of RTPSWriter that maintains information of each matched Reader.
 * @ingroup WRITERMODULE
 */
class StatefulWriter: public RTPSWriter {
public:
	//StatefulWriter();
	virtual ~StatefulWriter();

	StatefulWriter(const PublisherAttributes& param,
			const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype);

	/**
	 * Add a matched reader.
	 * @param rdata Pointer to the ReaderProxyData object added.
	 * @return True if added.
	 */
	bool matched_reader_add(ReaderProxyData* rdata);
	/**
	 * Remove a matched reader.
	 * @param rdata Pointer to the object to remove.
	 * @return True if removed.
	 */
	bool matched_reader_remove(ReaderProxyData* rdata);
	/**
	 * Tells us if a specific Reader is matched against this writer
	 * @param rdata Pointer to the ReaderProxyData object
	 * @return True if it was matched.
	 */
	bool matched_reader_is_matched(ReaderProxyData* rdata);
	/**
	 * Find a Reader Proxy in this writer.
	 * @param[in] readerGuid The GUID_t of the reader.
	 * @param[out] RP Pointer to pointer to return the ReaderProxy.
	 * @return True if correct.
	 */
	bool matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP);

	/**
	 * Find out if a change is acked by all ReaderProxy.
	 * @param change Pointer to the change.
	 * @return True if correct.
	 */
	bool is_acked_by_all(CacheChange_t* change);

	/**
	 * Add the provided change to the unsent changes vectors of all matched Readers.
	 * @param[in] change Pointer to the change
	 */
	void unsent_change_add(CacheChange_t* change);

	/**
	 * Method to indicate that there are changes not sent in some of all ReaderProxy.
	 */
	void unsent_changes_not_empty();
	/**
	 * Remove the change with the minimum SequenceNumber
	 * @return True if removed.
	 */
	bool removeMinSeqCacheChange();
	/**
	 * Remove all changes from history
	 * @param n_removed Pointer to return the number of elements removed.
	 * @return True if correct.
	 */
	bool removeAllCacheChange(size_t* n_removed);
	//!Increment the HB count.
	void incrementHBCount(){++m_heartbeatCount;};


	p_ReaderProxyIterator matchedReadersBegin()
	{
		return matched_readers.begin();
	};
	p_ReaderProxyIterator matchedReadersEnd()
	{
		return matched_readers.end();
	}
	size_t matchedReadersSize()
	{
		return matched_readers.size();
	}
	Count_t getHeartbeatCount() const {
		return m_heartbeatCount;
	}

	size_t getMatchedSubscribers(){return matched_readers.size();}

	EntityId_t& getHBReaderEntityId()
	{
		return m_HBReaderEntityId;
	}

	bool change_removed_by_history(CacheChange_t* a_change);
private:
	//! Vector containin all the associated ReaderProxies.
	std::vector<ReaderProxy*> matched_readers;

	PublisherTimes m_PubTimes;

	Count_t m_heartbeatCount;
	//!Timed Event to manage the periodic HB to the Reader.
	PeriodicHeartbeat* mp_periodicHB;

	EntityId_t m_HBReaderEntityId;


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATEFULWRITER_H_ */
