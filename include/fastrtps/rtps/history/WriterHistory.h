/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterHistory.h
 *
 */

#ifndef WRITERHISTORY_H_
#define WRITERHISTORY_H_

#include "fastrtps/rtps/history/History.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class RTPSWriter;

/**
 * Class WriterHistory, container of the different CacheChanges of a writer
 * @ingroup WRITER_MODULE
 */
class WriterHistory : public History
{
	friend class RTPSWriter;
public:
	/**
	* Constructor of the WriterHistory.
	*/
	RTPS_DllAPI WriterHistory(const HistoryAttributes&  att);
	RTPS_DllAPI virtual ~WriterHistory();

	/**
	 * Update the maximum and minimum sequenceNumber cacheChanges.
	 */
	RTPS_DllAPI void updateMaxMinSeqNum();
	/**
	 * Add a CacheChange_t to the ReaderHistory.
	 * @param a_change Pointer to the CacheChange to add.
	 * @return True if added.
	 */
	RTPS_DllAPI bool add_change(CacheChange_t* a_change);
	/**
	 * Remove a specific change from the history.
	 * @param a_change Pointer to the CacheChange_t.
	 * @return True if removed.
	 */
	RTPS_DllAPI bool remove_change(CacheChange_t* a_change);
	/**
	 * Remove the CacheChange_t with the minimum sequenceNumber.
	 * @return True if correctly removed.
	 */
	RTPS_DllAPI bool remove_min_change();
protected:
	//!Last CacheChange Sequence Number added to the History.
	SequenceNumber_t m_lastCacheChangeSeqNum;
	//!Pointer to the associated RTPSWriter;
	RTPSWriter* mp_writer;
};
}
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* WRITERHISTORY_H_ */
