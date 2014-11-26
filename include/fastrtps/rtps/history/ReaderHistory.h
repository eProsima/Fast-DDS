/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.h
 *
 */

#ifndef READERHISTORY_H_
#define READERHISTORY_H_

#include "fastrtps/rtps/history/History.h"
#include "fastrtps/rtps/common/CacheChange.h"

namespace boost
{
	namespace interprocess{ class interprocess_semaphore; }
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

struct FirstLastSeqNum
{
	GUID_t wGuid;
	SequenceNumber_t first;
	SequenceNumber_t last;
};

class WriterProxy;
class RTPSReader;

class ReaderHistory : public History {
	friend class RTPSReader;
public:
	RTPS_DllAPI ReaderHistory(const HistoryAttributes& att);
	RTPS_DllAPI virtual ~ReaderHistory();

	RTPS_DllAPI virtual bool received_change(CacheChange_t* change, WriterProxy*prox = nullptr);

	/**
	 * Add a CacheChange_t to the ReaderHistory.
	 * @param a_change Pointer to the CacheChange to add.
	 * @param prox Pointer to the writerProxy associated with the change that is going to be added.
	 * @return True if added.
	 */
	RTPS_DllAPI bool add_change(CacheChange_t* a_change, WriterProxy*prox = nullptr);

	RTPS_DllAPI bool remove_change(CacheChange_t* a_change);

	/**
	 * Sort the CacheChange_t from the History.
	 */
	RTPS_DllAPI void sortCacheChanges();
	/**
	 * Update the maximum and minimum sequenceNumber cacheChanges.
	 */
	RTPS_DllAPI void updateMaxMinSeqNum();

	RTPS_DllAPI void postSemaphore();

	RTPS_DllAPI void waitSemaphore();
//	/**
//	 * Method to know whether there are unread CacheChange_t.
//	 * @return True if there are unread.
//	 */
//	bool isUnreadCache();
//	//!Increase the unread count.
//	void increaseUnreadCount()
//	{
//		++m_unreadCacheCount;
//	}
//	//!Decrease the unread count.
//	void decreaseUnreadCount()
//	{
//		if(m_unreadCacheCount>0)
//			--m_unreadCacheCount;
//	}
//	//!Get the unread count.
//	uint64_t getUnreadCount()
//	{
//		return m_unreadCacheCount;
//	}
//	//!Get the last added cacheChange.
//	bool get_last_added_cache(CacheChange_t** change);
//	//!Remove all the changes of a specific InstanceHandle_t.
//	bool removeCacheChangesByKey(InstanceHandle_t& key);


protected:
	RTPSReader* mp_reader;
	boost::interprocess::interprocess_semaphore* mp_semaphore;

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERHISTORY_H_ */
