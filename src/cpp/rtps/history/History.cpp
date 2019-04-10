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
 * @file History.cpp
 *
 */


#include <fastrtps/rtps/history/History.h>

#include <fastrtps/rtps/common/CacheChange.h>


#include <fastrtps/log/Log.h>

#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps {


typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_pairKeyChanges;
typedef std::vector<t_pairKeyChanges> t_vectorPairKeyChanges;

History::History(const HistoryAttributes & att):
    m_att(att),
    m_isHistoryFull(false),
    mp_invalidCache(nullptr),
    m_changePool(att.initialReservedCaches,att.payloadMaxSize,att.maximumReservedCaches,att.memoryPolicy),
    mp_minSeqCacheChange(nullptr),
    mp_maxSeqCacheChange(nullptr),
    mp_mutex(nullptr)

    {
        m_changes.reserve((uint32_t)abs(att.initialReservedCaches));
        mp_invalidCache = new CacheChange_t();
        mp_invalidCache->writerGUID = c_Guid_Unknown;
        mp_invalidCache->sequenceNumber = c_SequenceNumber_Unknown;
        mp_minSeqCacheChange = mp_invalidCache;
        mp_maxSeqCacheChange = mp_invalidCache;
        //logInfo(RTPS_HISTORY,"History created");

    }

History::~History()
{
    logInfo(RTPS_HISTORY,"");
    delete(mp_invalidCache);
}


bool History::remove_all_changes()
{

    if(mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a RTPS Entity with this History before using it");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    if(!m_changes.empty())
    {
        while(!m_changes.empty())
        {
            remove_change(m_changes.front());
        }
        m_changes.clear();
        m_isHistoryFull = false;
        updateMaxMinSeqNum();
        return true;
    }
    return false;
}

bool History::get_min_change(CacheChange_t** min_change)
{
    if(mp_minSeqCacheChange->sequenceNumber != mp_invalidCache->sequenceNumber)
    {
        *min_change = mp_minSeqCacheChange;
        return true;
    }
    return false;

}
bool History::get_max_change(CacheChange_t** max_change)
{
    if(mp_maxSeqCacheChange->sequenceNumber != mp_invalidCache->sequenceNumber)
    {
        *max_change = mp_maxSeqCacheChange;
        return true;
    }
    return false;
}

bool History::get_change(const SequenceNumber_t& seq, const GUID_t& guid,CacheChange_t** change)
{

    if(mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a RTPS Entity with this History before using it");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    for(std::vector<CacheChange_t*>::iterator it = m_changes.begin();
            it!=m_changes.end();++it)
    {
        if((*it)->sequenceNumber == seq && (*it)->writerGUID == guid)
        {
            *change = *it;
            return true;
        }
        else if((*it)->sequenceNumber > seq)
            break;
    }
    return false;
}
}
}
}


//TODO Remove if you want.
#include <sstream>

namespace eprosima{
namespace fastrtps{
namespace rtps{

void History::print_changes_seqNum2()
{
    std::stringstream ss;
    for(std::vector<CacheChange_t*>::iterator it = m_changes.begin();
            it!=m_changes.end();++it)
    {
        ss << (*it)->sequenceNumber << "-";
    }
    ss << std::endl;
    std::cout << ss.str();
}


}
} /* namespace rtps */
} /* namespace eprosima */
