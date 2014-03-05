/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * HistoryCache.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include <boost/thread/mutex.hpp>


#include "rtps_all.h"

#ifndef HISTORYCACHE_H_
#define HISTORYCACHE_H_

namespace eprosima {
namespace rtps {


class RTPSWriter;
class RTPSReader;

/**
 * Class HistoryCache, container of the different CacheChanges and the methods to access them.
 */
class HistoryCache {
public:
	HistoryCache();
	virtual ~HistoryCache();
	/**
	 * Get a pointer to a specific change based on its SequenceNumber_t.
	 * @param seqnum SequenceNumber_t of the change.
	 * @param change Pointer to the change.
	 * @return True if succeedeed.
	 */
	bool get_change(SequenceNumber_t seqnum,GUID_t writerGuid,CacheChange_t** change);
	/**
	 * Add a change to the HistoryCache.
	 * @param a_change The change to add.
	 * @return True if suceedeed.
	 */
	bool add_change(CacheChange_t a_change);
	/**
	 * Remove a change from the list.
	 * @param a_change
	 * @return
	 */
	bool remove_change(CacheChange_t a_change);
	/**
	 * Remove a change based on its SequenceNumber_t.
	 * @param seqNum
	 * @return
	 */
	bool remove_change(SequenceNumber_t seqNum);
	/**
	 * Get the minimum sequence number in the HistoryCache.
	 * @return
	 */
	bool get_seq_num_min(SequenceNumber_t* seqnum,GUID_t* guid);
	/**
	 * Get the maximum sequence number in the HistoryCache.
	 * @return
	 */
	bool get_seq_num_max(SequenceNumber_t* seqnum,GUID_t* guid);
	std::vector<CacheChange_t*> changes;
	int16_t historySize;
	RTPSWriter* rtpswriter;
	RTPSReader* rtpsreader;
	HistoryKind_t historyKind;
	boost::mutex historyMutex;
private:

	SequenceNumber_t minSeqNum;
	GUID_t minSeqNumGuid;
	SequenceNumber_t maxSeqNum;
	GUID_t maxSeqNumGuid;
	/**
	 * Update the max and min sequence number after a change in the cache changes.
	 */
	void updateMaxMinSeqNum();


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* HISTORYCACHE_H_ */
