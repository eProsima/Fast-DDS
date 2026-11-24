// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of ReaderHistory: replaces src/cpp/rtps/history/ReaderHistory.cpp. Changes are added and
// looked up as in the real history, but the reader is never involved, and removals are forwarded to the
// ReaderHistoryMock companion of the instance. Removed changes are never released.

#include <fastdds/rtps/history/ReaderHistory.hpp>

#include <chrono>
#include <mutex>

#include <mock/fastdds/rtps/history/ReaderHistoryMock.hpp>
#include <rtps/common/ChangeComparison.hpp>
#include <utils/collections/sorted_vector_insert.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<ReaderHistory>;

ReaderHistory::ReaderHistory(
        const HistoryAttributes& att)
    : History(att)
    , mp_reader(nullptr)
{
    Companion::attach(this);
    mp_mutex = &Companion::get(this).mutex;
}

ReaderHistory::~ReaderHistory()
{
    Companion::detach(this);
}

bool ReaderHistory::can_change_be_added_nts(
        const GUID_t& writer_guid,
        uint32_t total_payload_size,
        size_t,
        bool& will_never_be_accepted) const
{
    will_never_be_accepted = (m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE &&
            total_payload_size > m_att.payloadMaxSize) || writer_guid == c_Guid_Unknown;
    return !will_never_be_accepted;
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
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    eprosima::utilities::collections::sorted_vector_insert(m_changes, a_change, fastdds::rtps::history_order_cmp);
    return true;
}

bool ReaderHistory::matches_change(
        const CacheChange_t* inner_change,
        CacheChange_t* outer_change)
{
    return nullptr != inner_change && nullptr != outer_change &&
           inner_change->sequenceNumber == outer_change->sequenceNumber &&
           inner_change->writerGUID == outer_change->writerGUID;
}

History::iterator ReaderHistory::remove_change_nts(
        const_iterator removal,
        bool release)
{
    return ReaderHistory::remove_change_nts(removal, std::chrono::steady_clock::now() + std::chrono::hours(24),
                   release);
}

History::iterator ReaderHistory::remove_change_nts(
        const_iterator removal,
        const std::chrono::time_point<std::chrono::steady_clock>&,
        bool)
{
    if (removal == changesEnd())
    {
        return changesEnd();
    }

    Companion::get(this).remove_change(*removal);
    m_isHistoryFull = false;
    return m_changes.erase(removal);
}

void ReaderHistory::writer_unmatched(
        const GUID_t& writer_guid,
        const SequenceNumber_t&)
{
    remove_changes_with_guid(writer_guid);
}

bool ReaderHistory::remove_changes_with_guid(
        const GUID_t& a_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
    while (chit != m_changes.end())
    {
        if (a_guid == (*chit)->writerGUID)
        {
            chit = remove_change_nts(chit);
        }
        else
        {
            ++chit;
        }
    }
    return true;
}

bool ReaderHistory::remove_fragmented_changes_until(
        const SequenceNumber_t& seq_num,
        const GUID_t& writer_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
    while (chit != m_changes.end())
    {
        CacheChange_t* item = *chit;
        if (item->writerGUID == writer_guid)
        {
            if (item->sequenceNumber >= seq_num)
            {
                break;
            }
            if (!item->is_fully_assembled())
            {
                chit = remove_change_nts(chit);
                continue;
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
    *min_change = nullptr;
    for (CacheChange_t* change : m_changes)
    {
        if (change->writerGUID == writerGuid)
        {
            *min_change = change;
            return true;
        }
    }
    return false;
}

void ReaderHistory::do_release_cache(
        CacheChange_t*)
{
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
