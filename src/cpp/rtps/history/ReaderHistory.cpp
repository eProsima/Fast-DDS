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
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>

#include <mutex>

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
                        mp_semaphore(new Semaphore(0))
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

bool ReaderHistory::add_change(CacheChange_t* a_change)
{

    if(mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a Reader with this History before adding any changes");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    if(m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE && a_change->serializedPayload.length > m_att.payloadMaxSize)
    {
        logError(RTPS_HISTORY,
            "Change payload size of '" << a_change->serializedPayload.length <<
            "' bytes is larger than the history payload size of '" << m_att.payloadMaxSize <<
            "' bytes and cannot be resized.");
        return false;
    }
    if(a_change->writerGUID == c_Guid_Unknown)
    {
        logError(RTPS_HISTORY,"The Writer GUID_t must be defined");
    }

    m_changes.push_back(a_change);
    sortCacheChanges();
    updateMaxMinSeqNum();
    logInfo(RTPS_HISTORY, "Change " << a_change->sequenceNumber << " added with " << a_change->serializedPayload.length << " bytes");

    return true;
}

bool ReaderHistory::remove_change(CacheChange_t* a_change)
{

    if(mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a Reader with this History before removing any changes");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
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

bool ReaderHistory::remove_changes_with_guid(const GUID_t& a_guid)
{
    std::vector<CacheChange_t*> changes_to_remove;

    if(mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a Reader with History before removing any changes");
        return false;
    }

    {//Lock scope
        std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
        for(std::vector<CacheChange_t*>::iterator chit = m_changes.begin(); chit!=m_changes.end();++chit)
        {
            bool matches = true;
            unsigned int size = a_guid.guidPrefix.size;
            if( !std::equal( (*chit)->writerGUID.guidPrefix.value , (*chit)->writerGUID.guidPrefix.value + size -1, a_guid.guidPrefix.value ) )
                matches = false;
            size = a_guid.entityId.size;
            if( !std::equal( (*chit)->writerGUID.entityId.value , (*chit)->writerGUID.entityId.value + size -1, a_guid.entityId.value ) )
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

bool ReaderHistory::get_min_change_from(CacheChange_t** min_change, const GUID_t& writerGuid)
{
    bool ret = false;
    *min_change = nullptr;

    for(auto it = m_changes.begin(); it != m_changes.end(); ++it)
    {
        if((*it)->writerGUID == writerGuid)
        {
            *min_change = *it;
            ret = true;
            break;
        }
    }

    return ret;
}

}
} /* namespace rtps */
} /* namespace eprosima */
