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

#include <fastdds/rtps/history/ReaderHistory.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/Semaphore.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/ReaderListener.h>

#include <utils/collections/sorted_vector_insert.hpp>

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

ReaderHistory::ReaderHistory(
        const HistoryAttributes& att)
    : History(att)
    , mp_reader(nullptr)
{
}

ReaderHistory::~ReaderHistory()
{
}

bool ReaderHistory::received_change(
        CacheChange_t* change,
        size_t)
{
    return add_change(change);
}

bool ReaderHistory::add_change(
        CacheChange_t* a_change)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_READER_HISTORY, "You need to create a Reader with this History before adding any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE && a_change->serializedPayload.length > m_att.payloadMaxSize)
    {
        logError(RTPS_READER_HISTORY,
                "Change payload size of '" << a_change->serializedPayload.length <<
                "' bytes is larger than the history payload size of '" << m_att.payloadMaxSize <<
                "' bytes and cannot be resized.");
        return false;
    }
    if (a_change->writerGUID == c_Guid_Unknown)
    {
        logError(RTPS_READER_HISTORY, "The Writer GUID_t must be defined");
    }

    eprosima::utilities::collections::sorted_vector_insert(m_changes, a_change,
            [](const CacheChange_t* lhs, const CacheChange_t* rhs)
            {
                return lhs->sourceTimestamp < rhs->sourceTimestamp;
            });
    logInfo(RTPS_READER_HISTORY,
            "Change " << a_change->sequenceNumber << " added with " << a_change->serializedPayload.length << " bytes");

    return true;
}

bool ReaderHistory::matches_change(
        const CacheChange_t* inner_change,
        CacheChange_t* outer_change)
{
    if (nullptr == outer_change
            || nullptr == inner_change)
    {
        logError(RTPS_READER_HISTORY, "Pointer is not valid")
        return false;
    }

    return inner_change->sequenceNumber == outer_change->sequenceNumber &&
           inner_change->writerGUID == outer_change->writerGUID;
}

History::iterator ReaderHistory::remove_change_nts(
        const_iterator removal,
        bool release)
{
    if ( mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_WRITER_HISTORY, "You need to create a Writer with this History before removing any changes");
        return changesEnd();
    }

    if ( removal == changesEnd())
    {
        logInfo(RTPS_WRITER_HISTORY, "Trying to remove without a proper CacheChange_t referenced");
        return changesEnd();
    }

    CacheChange_t* change = *removal;
    mp_reader->change_removed_by_history(change);
    if ( release )
    {
        mp_reader->releaseCache(change);
    }

    return m_changes.erase(removal);
}

bool ReaderHistory::remove_changes_with_guid(
        const GUID_t& a_guid)
{
    std::vector<CacheChange_t*> changes_to_remove;

    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_READER_HISTORY, "You need to create a Reader with History before removing any changes");
        return false;
    }

    {
        //Lock scope
        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        for (std::vector<CacheChange_t*>::iterator chit = m_changes.begin(); chit != m_changes.end(); ++chit)
        {
            if ((*chit)->writerGUID == a_guid)
            {
                changes_to_remove.push_back((*chit));
            }
        }
    }//End lock scope

    for (std::vector<CacheChange_t*>::iterator chit = changes_to_remove.begin(); chit != changes_to_remove.end();
            ++chit)
    {
        if (!remove_change(*chit))
        {
            logError(RTPS_READER_HISTORY, "One of the cachechanged in the GUID removal bulk could not be removed");
            return false;
        }
    }
    return true;
}

bool ReaderHistory::remove_fragmented_changes_until(
        const SequenceNumber_t& seq_num,
        const GUID_t& writer_guid)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_READER_HISTORY, "You need to create a Reader with History before removing any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
    while (chit != m_changes.end())
    {
        CacheChange_t* item = *chit;
        if (item->writerGUID == writer_guid)
        {
            if (item->sequenceNumber < seq_num)
            {
                if (item->is_fully_assembled() == false)
                {
                    logInfo(RTPS_READER_HISTORY, "Removing change " << item->sequenceNumber);
                    mp_reader->change_removed_by_history(item);
                    mp_reader->releaseCache(item);
                    chit = m_changes.erase(chit);
                    continue;
                }
            }
            else
            {
                break;
            }
        }
        ++chit;
    }

    return true;
}

bool ReaderHistory::get_min_change_from(
        CacheChange_t** min_change,
        const GUID_t& writerGuid)
{
    bool ret = false;
    *min_change = nullptr;

    for (auto it = m_changes.begin(); it != m_changes.end(); ++it)
    {
        if ((*it)->writerGUID == writerGuid)
        {
            *min_change = *it;
            ret = true;
            break;
        }
    }

    return ret;
}

bool ReaderHistory::do_reserve_cache(
        CacheChange_t** change,
        uint32_t size)
{
    return mp_reader->reserveCache(change, size);
}

void ReaderHistory::do_release_cache(
        CacheChange_t* ch)
{
    mp_reader->releaseCache(ch);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
