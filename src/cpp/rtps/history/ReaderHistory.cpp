/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.cpp
 *
 */

#include "fastrtps/rtps/history/ReaderHistory.h"

#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/reader/ReaderListener.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "ReaderHistory";

//typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_pairKeyChanges;
//typedef std::vector<t_pairKeyChanges> t_vectorPairKeyChanges;

inline bool sort_ReaderHistoryCache(CacheChange_t*c1, CacheChange_t*c2)
{
	return c1->sequenceNumber < c2->sequenceNumber;
}


ReaderHistory::ReaderHistory(const HistoryAttributes& att):
						History(att),
						mp_reader(nullptr),
						mp_semaphore(new boost::interprocess::interprocess_semaphore(0))

{

}

ReaderHistory::~ReaderHistory()
{
	// TODO Auto-generated destructor stub
}

bool ReaderHistory::received_change(CacheChange_t* change, WriterProxy* wp)
{
	return add_change(change,wp);
}

bool ReaderHistory::add_change(CacheChange_t* a_change,WriterProxy* wp)
{
	const char* const METHOD_NAME = "add_change";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(mp_reader == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Reader with this History before adding any changes");
		return false;
	}
	if(a_change->serializedPayload.length > m_att.payloadMaxSize)
	{
		logError(RTPS_HISTORY,"The Payload length is larger than the maximum payload size");
		return false;
	}
	if(a_change->writerGUID == c_Guid_Unknown)
	{
		logError(RTPS_HISTORY,"The Writer GUID_t must be defined");
	}
	m_changes.push_back(a_change);
	updateMaxMinSeqNum();
	logInfo(RTPS_HISTORY,"Change "<< a_change->sequenceNumber.to64long() << " added with "<<a_change->serializedPayload.length<< " bytes");
	mp_reader->getListener()->onNewCacheChangeAdded(mp_reader,a_change);
	return true;
}

bool ReaderHistory::remove_change(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "remove_change";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(a_change == nullptr)
	{
		logError(RTPS_HISTORY,"Pointer is not valid")
		return false;
	}
	for(std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
			chit!=m_changes.end();++chit)
	{
		if((*chit)->sequenceNumber == a_change->sequenceNumber &&
				(*chit)->writerGUID == a_change->writerGUID)
		{
			m_changePool.release_Cache(a_change);
			m_changes.erase(chit);
			sortCacheChanges();
			updateMaxMinSeqNum();
			mp_reader->change_removed_by_history(a_change);
			return true;
		}
	}
	logWarning(RTPS_HISTORY,"SequenceNumber "<<a_change->sequenceNumber.to64long()<< " not found");
	return false;
}



void ReaderHistory::sortCacheChanges()
{
	std::sort(m_changes.begin(),m_changes.end(),sort_ReaderHistoryCache);
}

void ReaderHistory::updateMaxMinSeqNum()
{
	if(m_changes.size()==0)
	{
		mp_minSeqCacheChange = mp_invalidCache;
		mp_maxSeqCacheChange = mp_invalidCache;
	}
	else
	{
		mp_minSeqCacheChange = m_changes.front();
		mp_maxSeqCacheChange = m_changes.back();
	}
}

void ReaderHistory::postSemaphore()
{
	return mp_semaphore->post();
}

void ReaderHistory::waitSemaphore()
{
	return mp_semaphore->wait();
}

//
//bool ReaderHistory::isUnreadCache()
//{
//	if(m_unreadCacheCount>0)
//		return true;
//	else
//		return false;
//}
//
//bool ReaderHistory::get_last_added_cache(CacheChange_t** change)
//{
//	if(mp_lastAddedCacheChange->sequenceNumber != mp_invalidCache->sequenceNumber)
//	{
//		*change = mp_lastAddedCacheChange;
//		return true;
//	}
//	return false;
//}
//
//
//bool ReaderHistory::removeCacheChangesByKey(InstanceHandle_t& key)
//{
//	const char* const METHOD_NAME = "removeCacheChangesByKey";
//	logError(RTPS_HISTORY,"Not Implemented yet";);
//	return false;
//}

}
} /* namespace rtps */
} /* namespace eprosima */
