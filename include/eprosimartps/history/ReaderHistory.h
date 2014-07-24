/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.h
 *
 */

#ifndef READERHISTORY_H_
#define READERHISTORY_H_

#include "eprosimartps/history/History.h"

namespace eprosima {
namespace rtps {

struct FirstLastSeqNum
{
	GUID_t wGuid;
	SequenceNumber_t first;
	SequenceNumber_t last;
};

class ReaderHistory: public History {
public:
	ReaderHistory(Endpoint* endp,uint32_t payload_max_size=5000);
	virtual ~ReaderHistory();
	/**
	 * Add a CacheChange_t to the ReaderHistory.
	 * @param a_change Pointer to the CacheChange to add.
	 * @return True if added.
	 */
	bool add_change(CacheChange_t* a_change);
	/**
	 * Sort the CacheChange_t from the History.
	 */
	void sortCacheChanges();
	/**
	 * Update the maximum and minimum sequenceNumber cacheChanges.
	 */
	void updateMaxMinSeqNum();
	/**
	 * Method to know whether there are unread CacheChange_t.
	 * @return True if there are unread.
	 */
	bool isUnreadCache();
	//!Increase the unread count.
	void increaseUnreadCount()
	{
		++m_unreadCacheCount;
	}
	//!Decrease the unread count.
	void decreaseUnreadCount()
	{
		if(m_unreadCacheCount>0)
			--m_unreadCacheCount;
	}
	//!Get the unread count.
	uint64_t getUnreadCount()
	{
		return m_unreadCacheCount;
	}
	//!Get the last added cacheChange.
	bool get_last_added_cache(CacheChange_t** change);
	//!Remove all the changes of a specific InstanceHandle_t.
	bool removeCacheChangesByKey(InstanceHandle_t& key);


protected:
	std::vector<FirstLastSeqNum> m_firstlastSeqNum;
	CacheChange_t* mp_lastAddedCacheChange;
	RTPSReader* mp_reader;
	uint64_t m_unreadCacheCount;
	CacheChange_t* mp_getKeyCache;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERHISTORY_H_ */
