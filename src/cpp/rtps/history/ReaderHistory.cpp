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

#include <fastdds/rtps/history/ReaderHistory.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

#include <rtps/common/ChangeComparison.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <utils/collections/sorted_vector_insert.hpp>
#include <utils/Semaphore.hpp>

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

using BaseReader = fastdds::rtps::BaseReader;

ReaderHistory::ReaderHistory(
        const HistoryAttributes& att)
    : History(att)
    , mp_reader(nullptr)
{
}

ReaderHistory::~ReaderHistory()
{
}

bool ReaderHistory::can_change_be_added_nts(
        const GUID_t& writer_guid,
        uint32_t total_payload_size,
        size_t unknown_missing_changes_up_to,
        bool& will_never_be_accepted) const
{
    static_cast<void>(unknown_missing_changes_up_to);

    will_never_be_accepted = false;

    if (m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE && total_payload_size > m_att.payloadMaxSize)
    {
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY,
                "Change payload size of '" << total_payload_size <<
                "' bytes is larger than the history payload size of '" << m_att.payloadMaxSize <<
                "' bytes and cannot be resized.");
        will_never_be_accepted = true;
        return false;
    }

    if (writer_guid == c_Guid_Unknown)
    {
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY, "The Writer GUID_t must be defined");
        will_never_be_accepted = true;
        return false;
    }

    return true;
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
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY,
                "You need to create a Reader with this History before adding any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE && a_change->serializedPayload.length > m_att.payloadMaxSize)
    {
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY,
                "Change payload size of '" << a_change->serializedPayload.length <<
                "' bytes is larger than the history payload size of '" << m_att.payloadMaxSize <<
                "' bytes and cannot be resized.");
        return false;
    }
    if (a_change->writerGUID == c_Guid_Unknown)
    {
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY, "The Writer GUID_t must be defined");
    }

    eprosima::utilities::collections::sorted_vector_insert(m_changes, a_change, fastdds::rtps::history_order_cmp);
    EPROSIMA_LOG_INFO(RTPS_READER_HISTORY,
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
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY, "Pointer is not valid");
        return false;
    }

    return inner_change->sequenceNumber == outer_change->sequenceNumber &&
           inner_change->writerGUID == outer_change->writerGUID;
}

History::iterator ReaderHistory::remove_change_nts(
        const_iterator removal,
        bool release)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "You need to create a Writer with this History before removing any changes");
        return remove_iterator_constness(removal);
    }

    if (removal == changesEnd())
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY, "Trying to remove without a proper CacheChange_t referenced");
        return changesEnd();
    }

    CacheChange_t* change = *removal;
    auto ret_val = m_changes.erase(removal);
    m_isHistoryFull = false;

    auto base_reader = BaseReader::downcast(mp_reader);
    base_reader->change_removed_by_history(change);
    if (release)
    {
        base_reader->release_cache(change);
    }

    return ret_val;
}

History::iterator ReaderHistory::remove_change_nts(
        const_iterator removal,
        const std::chrono::time_point<std::chrono::steady_clock>&,
        bool release)
{
    return ReaderHistory::remove_change_nts(removal, release);
}

void ReaderHistory::writer_unmatched(
        const GUID_t& writer_guid,
        const SequenceNumber_t& last_notified_seq)
{
    static_cast<void>(last_notified_seq);
    remove_changes_with_pred(
        [&writer_guid](CacheChange_t* ch)
        {
            return writer_guid == ch->writerGUID;
        });
}

bool ReaderHistory::remove_changes_with_guid(
        const GUID_t& a_guid)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY, "You need to create a Reader with History before removing any changes");
        return false;
    }

    remove_changes_with_pred(
        [a_guid](CacheChange_t* ch)
        {
            return a_guid == ch->writerGUID;
        });

    return true;
}

bool ReaderHistory::remove_fragmented_changes_until(
        const SequenceNumber_t& seq_num,
        const GUID_t& writer_guid)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_READER_HISTORY, "You need to create a Reader with History before removing any changes");
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
                    EPROSIMA_LOG_INFO(RTPS_READER_HISTORY, "Removing change " << item->sequenceNumber);
                    chit = remove_change_nts(chit);
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

void ReaderHistory::do_release_cache(
        CacheChange_t* ch)
{
    BaseReader::downcast(mp_reader)->release_cache(ch);
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
