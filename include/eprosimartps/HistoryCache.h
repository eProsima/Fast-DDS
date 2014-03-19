/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HistoryCache.h
 *	History Cache for both reader and writers.
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */



#include <boost/thread.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "rtps_all.h"

using namespace boost;

#ifndef HISTORYCACHE_H_
#define HISTORYCACHE_H_

namespace eprosima {
namespace rtps {


class RTPSWriter;
class RTPSReader;

/**
 * Class HistoryCache, container of the different CacheChanges and the methods to access them.
 * @ingroup COMMONMODULE
 */
class HistoryCache: public boost::basic_lockable_adapter<boost::recursive_mutex> {
public:
	HistoryCache();
	virtual ~HistoryCache();
	/**
	 * Get a pointer to a specific change based on its SequenceNumber_t.
	 * @param[in] seqNum SequenceNumber_t of the change.
	 * @param[in] writerGuid GUID_t of the writer that generated the change.
	 * @param[out] change_ptr_ptr Pointer to a pointer where to store the change.
	 * @return True if correct.
	 */
	bool get_change(SequenceNumber_t seqNum,GUID_t writerGuid,CacheChange_t** change_ptr_ptr);
	/**
	 * Add a change to the HistoryCache.
	 * @param[in] a_change Pointer to the change to add.
	 * @param[out] ch_ptr Pointer to pointer of cachechange, to return a pointer to the real change in the history.
	 * @return True if correct.
	 */
	bool add_change(CacheChange_t* a_change);

	/**
	 * Remove a change based on its SequenceNumber_t and writer GUID.
	 * @param[in] seqNum SequenceNumber_t of the change to eliminate.
	 * @param[in] writerGuid GUID of the writer of the change.
	 * @return True if correct
	 */
	bool remove_change(SequenceNumber_t seqNum, GUID_t writerGuid);

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

	//!Returns true if the History is full.
	bool isFull();
	//!Vector of pointers to the CacheChange_t.
	std::vector<CacheChange_t*> changes;
	//!Maximum history size.
	uint16_t historySize;
	///@name Pointer to the associated entity. Only one of them is initialized.
	//! @{
	RTPSWriter* rtpswriter;
	RTPSReader* rtpsreader;
	//!@}
	//!Type of History (WRITER or READER).
	HistoryKind_t historyKind;


private:
	//!Variable to know if the history is full without needing to block the History mutex.
	bool isHistoryFull;
	//!Minimum sequence number in the history
	SequenceNumber_t minSeqNum;
	//!Writer Guid of the minimum seqNum in the History.
	GUID_t minSeqNumGuid;
	//!Maximum sequence number in the history
	SequenceNumber_t maxSeqNum;
	//!Writer Guid of the maximum seqNum in the History.
	GUID_t maxSeqNumGuid;

	//!Update the max and min sequence number and Guid after a change in the cache changes.
	void updateMaxMinSeqNum();


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* HISTORYCACHE_H_ */
