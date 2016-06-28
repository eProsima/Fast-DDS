// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file ReaderHistory.cpp
 *
 */

#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {


//typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_pairKeyChanges;
//typedef std::vector<t_pairKeyChanges> t_vectorPairKeyChanges;

inline bool sort_ReaderHistoryCache(CacheChange_t*c1, CacheChange_t*c2)
{
	return c1->sequenceNumber < c2->sequenceNumber;
}


ReaderHistory::ReaderHistory(const HistoryAttributes& att):
						History(att),
						mp_reader(nullptr),
						mp_semaphore(new boost::interprocess::interprocess_semaphore(0)),
                  m_cachedRecordLocation(nullptr),
                  m_cachedGUID()

{

}

ReaderHistory::~ReaderHistory()
{
	// TODO Auto-generated destructor stub
	delete(mp_semaphore);
}

bool ReaderHistory::received_change(CacheChange_t* change, size_t)
{
	return add_change(change);
}

static void CleanSequentials(std::set<SequenceNumber_t>& set)
{
  auto end = set.end();
  auto set_it = set.begin();
  if (set_it == end)
   return;

  while ( next(set_it) != end &&
         *next(set_it) == (*set_it + 1))
      set_it++;

   set.erase(set.begin(), set_it);
}

bool ReaderHistory::add_change(CacheChange_t* a_change)
{

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

   
   if (a_change->writerGUID != m_cachedGUID || !m_cachedRecordLocation)
   {
      m_cachedRecordLocation = &m_historyRecord[a_change->writerGUID];
      if (m_cachedRecordLocation->empty())
         m_cachedRecordLocation->insert(SequenceNumber_t());
      m_cachedGUID = a_change->writerGUID;
   }

	if(*m_cachedRecordLocation->begin() < a_change->sequenceNumber && m_cachedRecordLocation->insert(a_change->sequenceNumber).second)
	{
		m_changes.push_back(a_change);
        sortCacheChanges();
		updateMaxMinSeqNum();
		logInfo(RTPS_HISTORY, "Change " << a_change->sequenceNumber << " added with " << a_change->serializedPayload.length << " bytes");

      CleanSequentials(*m_cachedRecordLocation);

		return true;
	}

    logInfo(RTPS_HISTORY, "Change "<<  a_change->sequenceNumber << " from "<< a_change->writerGUID << " not added.");
	return false;
}

bool ReaderHistory::remove_change(CacheChange_t* a_change)
{

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

bool ReaderHistory::remove_changes_with_guid(GUID_t* a_guid)
{
	std::vector<CacheChange_t*> changes_to_remove;

	if(mp_reader == nullptr || mp_mutex == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Reader with History before removing any changes");
		return false;
	}
	
	{//Lock scope
		boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
		if(a_guid == nullptr)
		{
			logError(RTPS_HISTORY, "Target Guid for Cachechange deletion is not valid");
			return false;
		}
		for(std::vector<CacheChange_t*>::iterator chit = m_changes.begin(); chit!=m_changes.end();++chit)
		{
			bool matches = true;
			unsigned int size = a_guid->guidPrefix.size;
			if( !std::equal( (*chit)->writerGUID.guidPrefix.value , (*chit)->writerGUID.guidPrefix.value + size -1, a_guid->guidPrefix.value ) )
				matches = false;
			size = a_guid->entityId.size;
			if( !std::equal( (*chit)->writerGUID.entityId.value , (*chit)->writerGUID.entityId.value + size -1, a_guid->entityId.value ) )
			        matches = false;	
			if(matches)	
				changes_to_remove.push_back( (*chit) );
		}
	}//End lock scope

	for(std::vector<CacheChange_t*>::iterator chit = changes_to_remove.begin(); chit != changes_to_remove.end(); ++chit)
		if(!remove_change( (*chit) )){
			logError(RTPS_HISTORY,"One of the cachechanged in the GUID removal bulk could not be removed");
			return false;
		}
	return true;
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
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
   if (guid == m_cachedGUID)
      return m_cachedRecordLocation->find(seq) != m_cachedRecordLocation->end();
      
	return m_historyRecord.find(guid) != m_historyRecord.end() && m_historyRecord[guid].find(seq) != m_historyRecord[guid].end();
}

bool ReaderHistory::thereIsUpperRecordOf(GUID_t& guid, SequenceNumber_t& seq)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
   if (guid == m_cachedGUID)
      return m_cachedRecordLocation->upper_bound(seq) != m_cachedRecordLocation->end();

   return m_historyRecord.find(guid) != m_historyRecord.end() && m_historyRecord[guid].upper_bound(seq) != m_historyRecord[guid].end();
}

}
} /* namespace rtps */
} /* namespace eprosima */
