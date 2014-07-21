/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file History.h
 *
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include <boost/thread.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

using namespace boost;

#include "eprosimartps/utils/CacheChangePool.h"
#include "eprosimartps/common/types/SequenceNumber.h"
#include "eprosimartps/common/types/Guid.h"

#include "eprosimartps/qos/DDSQosPolicies.h"
using namespace eprosima::dds;

namespace eprosima {
namespace rtps {


class RTPSWriter;
class RTPSReader;
class Endpoint;

/**
 * Class HistoryCache, container of the different CacheChanges and the methods to access them.
 * @ingroup COMMONMODULE
 */
class History: public boost::basic_lockable_adapter<boost::recursive_mutex>
{
public:
	History(Endpoint* endp,
			HistoryQosPolicy historypolicy=HistoryQosPolicy(),
			ResourceLimitsQosPolicy resourcelimits=ResourceLimitsQosPolicy(),
			uint32_t payload_max_size=5000);

	virtual ~History();
	/**
	 * Reserve a CacheChange_t from the CacheChange pool.
	 * @return Pointer to the CacheChange_t.
	 */
	CacheChange_t* reserve_Cache()
	{
		return m_changePool.reserve_Cache();
	}
	/**
	 * release a previously reserved CacheChange_t.
	 * @param ch Pointer to the CacheChange_t.
	 */
	void release_Cache(CacheChange_t* ch)
	{
		return m_changePool.release_Cache(ch);
	}

	//!Returns true if the History is full.
	bool isFull()
	{
		return m_isHistoryFull;
	}
	/**
	 * Get the History size.
	 * @return Size of the history.
	 */
	size_t getHistorySize(){
		return m_changes.size();
	}
	/**
	 * Remove all changes from the History
	 * @return True if everything was correctly removed.
	 */
	bool remove_all_changes();
	/**
	 * Update the maximum and minimum sequenceNumbers.
	 */
	virtual void updateMaxMinSeqNum()=0;
	/**
	 * Add a change to the History.
	 * @param a_change Pointer to the CacheChange_t to add to the History.
	 * @return True if correct.
	 */
	virtual bool add_change(CacheChange_t* a_change)=0;
	/**
	 * Remove a specific change from the history.
	 * @param ch Pointer to the CacheChange_t.
	 * @return True if removed.
	 */
	bool remove_change(CacheChange_t* ch);
	/**
	 * Find the key to the vector storing the changes of a specific key.
	 * @param a_change Pointer to the change.
	 * @param vecPairIterrator Pointer to the iterator.
	 * @return True if found correctly.
	 */
	bool find_Key(CacheChange_t* a_change,
			std::vector<std::pair<InstanceHandle_t,std::vector<CacheChange_t*>>>::iterator* vecPairIterrator);
	/**
	 * Get the beginning of the changes history iterator.
	 * @return Iterator to the beginning of the vector.
	 */
	std::vector<CacheChange_t*>::iterator changesBegin(){return m_changes.begin();}
	/**
	 * Get the end of the changes history iterator.
	 * @return Iterator to the end of the vector.
	 */
	std::vector<CacheChange_t*>::iterator changesEnd(){return m_changes.end();}
	/**
	 * Get the minimum CacheChange_t.
	 * @param min_change Pointer to pointer to the minimum change.
	 * @return True if correct.
	 */
	bool get_min_change(CacheChange_t** min_change);
	/**
	 * Get the maximum CacheChange_t.
	 * @param min_change Pointer to pointer to the maximum change.
	 * @return True if correct.
	 */
	bool get_max_change(CacheChange_t** max_change);

protected:
	//!Vector of pointers to the CacheChange_t.
	std::vector<CacheChange_t*> m_changes;
	//!Vector of pointer to the CacheChange_t divided by key.
	std::vector<std::pair<InstanceHandle_t,std::vector<CacheChange_t*>>> m_keyedChanges;
	//!Const pointer to the endpoint.
	const Endpoint* mp_Endpoint;
	//!HistoryQosPolicy values.
	HistoryQosPolicy m_historyQos;
	//!ResourceLimitsQosPolicy values.
	ResourceLimitsQosPolicy m_resourceLimitsQos;
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
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* HISTORY_H_ */
