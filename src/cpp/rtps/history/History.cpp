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


#include <fastdds/rtps/history/History.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>

#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

History::History(
        const HistoryAttributes& att)
    : m_att(att)
{
    uint32_t initial_size = static_cast<uint32_t>(att.initialReservedCaches < 0 ? 0 : att.initialReservedCaches);
    m_changes.reserve(static_cast<size_t>(initial_size));
}

History::~History()
{
    EPROSIMA_LOG_INFO(RTPS_HISTORY, "");
}

History::const_iterator History::find_change_nts(
        CacheChange_t* ch)
{
    if ( nullptr == mp_mutex )
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a RTPS Entity with this History before using it");
        return const_iterator();
    }

    return std::find_if(changesBegin(), changesEnd(), [this, ch](const CacheChange_t* chi)
                   {
                       // use the derived classes comparison criteria for searching
                       return this->matches_change(chi, ch);
                   });
}

bool History::matches_change(
        const CacheChange_t* ch_inner,
        CacheChange_t* ch_outer)
{
    return ch_inner->sequenceNumber == ch_outer->sequenceNumber;
}

History::iterator History::remove_change_nts(
        const_iterator removal,
        bool release)
{
    if (nullptr == mp_mutex)
    {
        return remove_iterator_constness(removal);
    }

    if (removal == changesEnd())
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY, "Trying to remove without a proper CacheChange_t referenced");
        return changesEnd();
    }

    CacheChange_t* change = *removal;
    m_isHistoryFull = false;

    if (release)
    {
        do_release_cache(change);
    }

    return m_changes.erase(removal);
}

History::iterator History::remove_change_nts(
        const_iterator removal,
        const std::chrono::time_point<std::chrono::steady_clock>&,
        bool release)
{
    return History::remove_change_nts(removal, release);
}

bool History::remove_change(
        CacheChange_t* ch)
{
    return History::remove_change(ch, std::chrono::steady_clock::now() + std::chrono::hours(24));
}

bool History::remove_change(
        CacheChange_t* ch,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(*mp_mutex, std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        EPROSIMA_LOG_ERROR(PUBLISHER, "Cannot lock the DataWriterHistory mutex");
        return false;
    }
#else
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
#endif // if HAVE_STRICT_REALTIME

    const_iterator it = find_change_nts(ch);

    if (it == changesEnd())
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY, "Trying to remove a change not in history");
        return false;
    }

    // Dummy change just used to compare original change with change returned from remove_change_nts function
    CacheChange_t dummy_change;
    dummy_change.writerGUID = (*it)->writerGUID;
    dummy_change.sequenceNumber = (*it)->sequenceNumber;

    // Remove using the virtual method
    History::iterator history_it = remove_change_nts(it, max_blocking_time);

    // If remove_change_nts returns a valid iterator (not end()) and this is the same iterator means that it
    // could not remove it so this function should fail
    if (history_it != changesEnd() && matches_change(&dummy_change, *history_it))
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY, "Failed to remove a change from history");
        return false;
    }

    return true;
}

bool History::remove_all_changes()
{
    if (mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a RTPS Entity with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (!m_changes.empty())
    {
        while (!m_changes.empty())
        {
            remove_change(m_changes.front());
        }
        m_changes.clear();
        m_isHistoryFull = false;
        return true;
    }
    return false;
}

bool History::get_min_change(
        CacheChange_t** min_change)
{
    if (!m_changes.empty())
    {
        *min_change = m_changes.front();
        return true;
    }
    return false;

}

bool History::get_max_change(
        CacheChange_t** max_change)
{
    if (!m_changes.empty())
    {
        *max_change = m_changes.back();
        return true;
    }
    return false;
}

bool History::get_change(
        const SequenceNumber_t& seq,
        const GUID_t& guid,
        CacheChange_t** change) const
{

    if (mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a RTPS Entity with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    get_change_nts(seq, guid, change, m_changes.cbegin());
    return *change != nullptr;
}

History::const_iterator History::get_change_nts(
        const SequenceNumber_t& seq,
        const GUID_t& guid,
        CacheChange_t** change,
        History::const_iterator hint) const
{
    const_iterator returned_value = hint;
    *change = nullptr;

    for (; returned_value != m_changes.end(); ++returned_value)
    {
        if ((*returned_value)->writerGUID == guid)
        {
            if ((*returned_value)->sequenceNumber == seq)
            {
                *change = *returned_value;
                break;
            }
            else if ((*returned_value)->sequenceNumber > seq)
            {
                break;
            }
        }
    }

    return returned_value;
}

bool History::get_earliest_change(
        CacheChange_t** change)
{
    if (mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a RTPS Entity with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    if (m_changes.empty())
    {
        return false;
    }

    *change = m_changes.front();
    return true;
}

History::iterator History::remove_iterator_constness(
        const_iterator c_it)
{
    History::iterator it = changesBegin();
    std::advance(it, std::distance<const_iterator>(m_changes.cbegin(), c_it));
    return it;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima


//TODO Remove if you want.
#include <sstream>

namespace eprosima {
namespace fastdds {
namespace rtps {

void History::print_changes_seqNum2()
{
    std::stringstream ss;
    for (std::vector<CacheChange_t*>::iterator it = m_changes.begin();
            it != m_changes.end(); ++it)
    {
        ss << (*it)->sequenceNumber << "-";
    }
    ss << std::endl;
    std::cout << ss.str();
}

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */
