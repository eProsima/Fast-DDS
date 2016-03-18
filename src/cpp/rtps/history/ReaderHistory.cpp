/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.cpp
 *
 */

#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>

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
	delete(mp_semaphore);
}

bool ReaderHistory::received_change(CacheChange_t* change)
{
	return add_change(change);
}

bool ReaderHistory::add_change(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "add_change";

	if(mp_reader == nullptr || mp_mutex == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Reader with this History before adding any changes");
		return false;
	}

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(a_change->serializedPayload.length > m_att.payloadMaxSize)
	{
		logError(RTPS_HISTORY,"The Payload length is larger than the maximum payload size");
		return false;
	}
	if(a_change->writerGUID == c_Guid_Unknown)
	{
		logError(RTPS_HISTORY,"The Writer GUID_t must be defined");
	}
	m_historyRecord.insert(std::make_pair(a_change->writerGUID,std::set<SequenceNumber_t>()));
	if ((m_historyRecord[a_change->writerGUID].insert(a_change->sequenceNumber)).second)
	{
		m_changes.push_back(a_change);
        sortCacheChanges();
		updateMaxMinSeqNum();
		logInfo(RTPS_HISTORY, "Change " << a_change->sequenceNumber << " added with " << a_change->serializedPayload.length << " bytes");
		return true;
	}
	logInfo(RTPS_HISTORY, "Change "<<  a_change->sequenceNumber << " from "<< a_change->writerGUID << " not added.");
	return false;
}

bool ReaderHistory::remove_change(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "remove_change";

	if(mp_reader == nullptr || mp_mutex == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Reader with this History before removing any changes");
		return false;
	}

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
			logInfo(RTPS_HISTORY,"Removing change "<< a_change->sequenceNumber);
			mp_reader->change_removed_by_history(a_change);
			m_changePool.release_Cache(a_change);
			m_changes.erase(chit);
			sortCacheChanges();
			updateMaxMinSeqNum();
			return true;
		}
	}
	logWarning(RTPS_HISTORY,"SequenceNumber "<<a_change->sequenceNumber << " not found");
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

void ReaderHistory::waitSemaphore() //TODO CAMBIAR NOMBRE PARA que el usuario sepa que es para esperar a un cachechange nuevo
{
	return mp_semaphore->wait();
}

bool ReaderHistory::thereIsRecordOf(GUID_t& guid, SequenceNumber_t& seq)
{
    return m_historyRecord[guid].find(seq) != m_historyRecord[guid].end();
}

}
} /* namespace rtps */
} /* namespace eprosima */
