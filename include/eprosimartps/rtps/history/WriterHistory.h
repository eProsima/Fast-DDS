/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterHistory.h
 *
 */

#ifndef WRITERHISTORY_H_
#define WRITERHISTORY_H_

#include "eprosimartps/rtps/history/History.h"

namespace eprosima {
namespace rtps {

class RTPSWriter;

class WriterHistory : public History
{
	friend class RTPSWriter;
public:
	WriterHistory(const HistoryAttributes&  att);
	virtual ~WriterHistory();

	/**
	 * Update the maximum and minimum sequenceNumber cacheChanges.
	 */
	void updateMaxMinSeqNum();
	/**
	 * Add a CacheChange_t to the ReaderHistory.
	 * @param a_change Pointer to the CacheChange to add.
	 * @param wp Pointer to the writerProxy associated with the change that is going to be added (NOT USED IN WriterHistory).
	 * @return True if added.
	 */
	bool add_change(CacheChange_t* a_change);
	/**
	 * Remove a specific change from the history.
	 * @param a_change Pointer to the CacheChange_t.
	 * @return True if removed.
	 */
	bool remove_change(CacheChange_t* a_change);
	/**
	 * Remove the CacheChange_t with the minimum sequenceNumber.
	 * @return True if correctly removed.
	 */
	bool remove_min_change();



protected:
	/**
	 * Assign the Writer Associated with this History.
	 * @param writer Pointer to the writer;
	 */
	void assignWriter(RTPSWriter* writer) {mp_writer = writer;};
	//!Last CacheChange Sequence Number added to the History.
	SequenceNumber_t m_lastCacheChangeSeqNum;
	//!Pointer to the associated RTPSWriter;
	RTPSWriter* mp_writer;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERHISTORY_H_ */
