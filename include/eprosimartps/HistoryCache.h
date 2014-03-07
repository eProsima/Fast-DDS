/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HistoryCache.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "rtps_all.h"

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
class HistoryCache {
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
	 * @param[in] a_change The change to add.
	 * @return True if correct.
	 */
	bool add_change(CacheChange_t a_change);

	/**
	 * Remove a change based on its SequenceNumber_t and writer GUID.
	 * @param[in] seqNum SequenceNumber_t of the change to eliminate.
	 * @param[in] writerGuid GUID of the writer of the change.
	 * @return True if correct
	 */
	bool remove_change(SequenceNumber_t seqNum, GUID_t writerGuid);

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
	//!Vector of pointers to the CacheChange_t.
	std::vector<CacheChange_t*> changes;
	//!Maximum history size.
	int16_t historySize;
	///@name Pointer to the associated entity. Only one of them is initialized.
	//! @{
	RTPSWriter* rtpswriter;
	RTPSReader* rtpsreader;
	//!@}
	//!TYpe of History (WRITER or READER).
	HistoryKind_t historyKind;
	//!Recursive mutex to protect the access to the HistoryCache.
	boost::recursive_mutex historyMutex;

private:
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
