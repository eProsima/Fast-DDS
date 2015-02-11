/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file History.h
 *
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include "fastrtps/config/fastrtps_dll.h"

#include "fastrtps/rtps/history/CacheChangePool.h"

#include "fastrtps/rtps/common/SequenceNumber.h"
#include "fastrtps/rtps/common/Guid.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"


namespace boost
{
	class recursive_mutex;
}

namespace eprosima {
namespace fastrtps{
namespace rtps {



/**
 * Class History, container of the different CacheChanges and the methods to access them.
 * @ingroup COMMON_MODULE
 */
class History
{
protected:
	History(const HistoryAttributes&  att);
	virtual ~History();
public:
	//!Attributes of the History
	HistoryAttributes m_att;
	/**
	 * Reserve a CacheChange_t from the CacheChange pool.
	 * @param[out] change Pointer to pointer to the CacheChange_t to reserve
	 * @return True is reserved
	 */
	RTPS_DllAPI inline bool reserve_Cache(CacheChange_t** change){ return m_changePool.reserve_Cache(change); }
	/**
	 * release a previously reserved CacheChange_t.
	 * @param ch Pointer to the CacheChange_t.
	 */
	RTPS_DllAPI inline void release_Cache(CacheChange_t* ch)	{ return m_changePool.release_Cache(ch); }

	/**
	* Check if the history is full
	* @return true if the History is full.
	*/
	RTPS_DllAPI bool isFull()	{ return m_isHistoryFull; }
	/**
	 * Get the History size.
	 * @return Size of the history.
	 */
	RTPS_DllAPI size_t getHistorySize(){ return m_changes.size(); }
	/**
	 * Remove all changes from the History
	 * @return True if everything was correctly removed.
	 */
	RTPS_DllAPI bool remove_all_changes();
	/**
	 * Update the maximum and minimum sequenceNumbers.
	 */
	virtual void updateMaxMinSeqNum()=0;
	/**
	 * Remove a specific change from the history.
	 * @param ch Pointer to the CacheChange_t.
	 * @return True if removed.
	 */
	virtual bool remove_change(CacheChange_t* ch)=0;
	/**
	 * Get the beginning of the changes history iterator.
	 * @return Iterator to the beginning of the vector.
	 */
	RTPS_DllAPI std::vector<CacheChange_t*>::iterator changesBegin(){ return m_changes.begin(); }
	/**
	 * Get the end of the changes history iterator.
	 * @return Iterator to the end of the vector.
	 */
	RTPS_DllAPI std::vector<CacheChange_t*>::iterator changesEnd(){ return m_changes.end(); }
	/**
	 * Get the minimum CacheChange_t.
	 * @param min_change Pointer to pointer to the minimum change.
	 * @return True if correct.
	 */
	RTPS_DllAPI bool get_min_change(CacheChange_t** min_change);
	/**
	 * Get the maximum CacheChange_t.
	 * @param max_change Pointer to pointer to the maximum change.
	 * @return True if correct.
	 */
	RTPS_DllAPI bool get_max_change(CacheChange_t** max_change);
	/**
	 * Get the maximum serialized payload size
	 * @return Maximum serialized payload size
	 */
	RTPS_DllAPI inline uint32_t getTypeMaxSerialized(){ return m_changePool.getPayloadSize(); }
	/**
	 * Get the mutex
	 * @return Mutex
	 */
	RTPS_DllAPI inline boost::recursive_mutex* getMutex() {return mp_mutex;};

	RTPS_DllAPI bool get_change(SequenceNumber_t& seq, GUID_t& guid,CacheChange_t** change);

protected:
	//!Vector of pointers to the CacheChange_t.
	std::vector<CacheChange_t*> m_changes;
	//!Variable to know if the history is full without needing to block the History mutex.
	bool m_isHistoryFull;
	//!Pointer to and invalid cacheChange used to return the maximum and minimum when no changes are stored in the history.
	CacheChange_t* mp_invalidCache;
	//!Pool of cache changes reserved when the History is created.
	CacheChangePool m_changePool;
	//!Pointer to the minimum sequeceNumber CacheChange.
	CacheChange_t* mp_minSeqCacheChange;
	//!Pointer to the maximum sequeceNumber CacheChange.
	CacheChange_t* mp_maxSeqCacheChange;
	//!Print the seqNum of the changes in the History (for debugging purposes).
	void print_changes_seqNum2();
	//!Mutex for the History.
	boost::recursive_mutex* mp_mutex;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* HISTORY_H_ */
