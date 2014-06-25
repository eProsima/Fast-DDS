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

	bool add_change(CacheChange_t* a_change);

	void sortCacheChanges();

	void updateMaxMinSeqNum();

	bool isUnreadCache();

	void increaseUnreadCount()
	{
		++m_unreadCacheCount;
	}
	void decreaseUnreadCount()
	{
		if(m_unreadCacheCount>0)
			--m_unreadCacheCount;
	}

	uint64_t getUnreadCount()
	{
		return m_unreadCacheCount;
	}

	bool get_last_added_cache(CacheChange_t** change);


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
