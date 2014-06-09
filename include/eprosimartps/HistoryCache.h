/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HistoryCache.h
 */







#ifndef HISTORYCACHE_H_
#define HISTORYCACHE_H_

#include <boost/thread.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
using namespace boost;
#include "eprosimartps/utils/CacheChangePool.h"
#include "eprosimartps/common/types/SequenceNumber.h"
#include "eprosimartps/common/types/Guid.h"


namespace eprosima {
namespace rtps {


class RTPSWriter;
class RTPSReader;
class Endpoint;

/**
 * Class HistoryCache, container of the different CacheChanges and the methods to access them.
 * @ingroup COMMONMODULE
 */
class HistoryCache: public boost::basic_lockable_adapter<boost::recursive_mutex>
{
public:
	HistoryCache(Endpoint* endp, uint16_t historymaxsize = 50, uint32_t payload_size = 500);

	virtual ~HistoryCache();
	/**
	 * Get a pointer to a specific change based on its SequenceNumber_t.
	 * @param[in] seqNum SequenceNumber_t of the change.
	 * @param[in] writerGuid GUID_t of the writer that generated the change.
	 * @param[out] ch_ptr_ptr Pointer to pointer to a cachechange.
	 * @return True if correct.
	 */
	bool get_change(SequenceNumber_t& seqNum,GUID_t& writerGuid,CacheChange_t** ch_ptr_ptr);
	/**
	 * Get a pointer to a specific change based on its SequenceNumber_t.
	 * @param[in] seqNum SequenceNumber_t of the change.
	 * @param[in] writerGuid GUID_t of the writer that generated the change.
	 * @param[out] ch_ptr_ptr Pointer to pointer to a cachechange.
	 * @param[out] ch_number Number of the change.
	 * @return True if correct.
	 */
	bool get_change(SequenceNumber_t& seqNum,GUID_t& writerGuid,CacheChange_t** ch_ptr_ptr,uint16_t *ch_number);


	/**
	 * Get the last element added to the history.
	 * @param ch_ptr
	 * @return True if correct.
	 */
	bool get_last_added_cache(CacheChange_t** ch_ptr);
	/**
	 * Add a change to the HistoryCache.
	 * @param[in] a_change Pointer to the change to add.
	 * @return True if correct.
	 */
	bool add_change(CacheChange_t* a_change);

	/**
	 * Remove a change based on its SequenceNumber_t and writer GUID.
	 * @param[in] seqNum SequenceNumber_t of the change to eliminate.
	 * @param[in] writerGuid GUID of the writer of the change.
	 * @return True if correct
	 */
	bool remove_change(SequenceNumber_t& seqNum, GUID_t& writerGuid);

	/**
	 * Remove a change based on a vector iterator.
	 * @param[in] it Iterator to the elements
	 * @return True if correct
	 */
	bool remove_change(std::vector<CacheChange_t*>::iterator it);

	/**
	 * Remove all elements of the History.
	 * @return True if correct.
	 */
	bool remove_all_changes();

	/**
	 * Get the minimum sequence number in the HistoryCache.
	 * @param[out] seqNum Pointer to store the sequence number
	 * @param[out] writerGuid Pointer to store the writerGuid.
	 * @return True if correct.
	 */
	bool get_seq_num_min(SequenceNumber_t* seqNum,GUID_t* writerGuid);
	/**
	 * Get the maximum sequence number in the HistoryCache.
	 * @param[out] seqNum Pointer to store the sequence number
	 * @param[out] writerGuid Pointer to store the writerGuid.
	 * @return True if correct.
	 */
	bool get_seq_num_max(SequenceNumber_t* seqNum,GUID_t* writerGuid);


	CacheChange_t* reserve_Cache()
	{
		return changePool.reserve_Cache();
	}

	void release_Cache(CacheChange_t* ch)
	{
		return changePool.release_Cache(ch);
	}

	//!Returns true if the History is full.
	bool isFull();

	size_t getHistorySize(){
		return m_changes.size();
	}

	//!Vector of pointers to the CacheChange_t.
	std::vector<CacheChange_t*> m_changes;

	void sortCacheChangesBySeqNum();

protected:

	const Endpoint* mp_Endpoint;

	//!Maximum history size.
	uint16_t m_history_max_size;

	//!Variable to know if the history is full without needing to block the History mutex.
	bool isHistoryFull;
	//!Minimum sequence number in the history
	SequenceNumber_t m_minSeqNum;
	//!Writer Guid of the minimum seqNum in the History.
	GUID_t m_minSeqNumGuid;
	//!Maximum sequence number in the history
	SequenceNumber_t m_maxSeqNum;
	//!Writer Guid of the maximum seqNum in the History.
	GUID_t m_maxSeqNumGuid;
	//! Pool of changes created in the beginning
	CacheChangePool changePool;
//!lastchangeseqNum
	SequenceNumber_t m_lastChangeSequenceNumber;
	//!Update the max and min sequence number and Guid after a change in the cache changes.
	void updateMaxMinSeqNum();
	bool m_isMaxMinUpdated;




};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* HISTORYCACHE_H_ */
