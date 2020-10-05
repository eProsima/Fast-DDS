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
 * @file WriterHistory.cpp
 *
 */

#include <fastdds/rtps/history/WriterHistory.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/common/WriteParams.h>

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

WriteParams WriteParams::WRITE_PARAM_DEFAULT;

WriterHistory::WriterHistory(
        const HistoryAttributes& att)
    : History(att)
    , mp_writer(nullptr)
{

}

WriterHistory::~WriterHistory()
{
    // TODO Auto-generated destructor stub
}

bool WriterHistory::add_change(
        CacheChange_t* a_change)
{
    WriteParams wparams;
    return add_change_(a_change, wparams);
}

bool WriterHistory::add_change(
        CacheChange_t* a_change,
        WriteParams& wparams)
{
    return add_change_(a_change, wparams);
}

bool WriterHistory::add_change_(
        CacheChange_t* a_change,
        WriteParams& wparams,
        std::chrono::time_point<std::chrono::steady_clock> max_blocking_time)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before adding any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (a_change->writerGUID != mp_writer->getGuid())
    {
        logError(RTPS_HISTORY,
                "Change writerGUID " << a_change->writerGUID << " different than Writer GUID " <<
                mp_writer->getGuid());
        return false;
    }
    if ((m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE) && a_change->serializedPayload.length > m_att.payloadMaxSize)
    {
        logError(RTPS_HISTORY,
                "Change payload size of '" << a_change->serializedPayload.length <<
                "' bytes is larger than the history payload size of '" << m_att.payloadMaxSize <<
                "' bytes and cannot be resized.");
        return false;
    }

    if (m_isHistoryFull)
    {
        logWarning(RTPS_HISTORY, "History full for writer " << a_change->writerGUID);
        return false;
    }

    ++m_lastCacheChangeSeqNum;
    a_change->sequenceNumber = m_lastCacheChangeSeqNum;
    Time_t::now(a_change->sourceTimestamp);

    a_change->write_params = wparams;
    // Updated sample identity
    wparams.sample_identity().writer_guid(a_change->writerGUID);
    wparams.sample_identity().sequence_number(a_change->sequenceNumber);

    m_changes.push_back(a_change);

    if (static_cast<int32_t>(m_changes.size()) == m_att.maximumReservedCaches)
    {
        m_isHistoryFull = true;
    }

    logInfo(RTPS_HISTORY,
            "Change " << a_change->sequenceNumber << " added with " << a_change->serializedPayload.length << " bytes");

    mp_writer->unsent_change_added_to_history(a_change, max_blocking_time);

    return true;
}

bool WriterHistory::remove_change(
        CacheChange_t* a_change)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before removing any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (a_change == nullptr)
    {
        logError(RTPS_HISTORY, "Pointer is not valid")
        return false;
    }
    if (a_change->writerGUID != mp_writer->getGuid())
    {
        // cout << "a change " << a_change->sequenceNumber<< endl;
        // cout << "a change "<< a_change->writerGUID << endl;
        // cout << "writer: "<< mp_writer->getGuid()<<endl;
        logError(RTPS_HISTORY,
                "Change writerGUID " << a_change->writerGUID << " different than Writer GUID " <<
                mp_writer->getGuid());
        return false;
    }

    for (std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
            chit != m_changes.end(); ++chit)
    {
        if ((*chit)->sequenceNumber == a_change->sequenceNumber)
        {
            // Remove from history
            m_changes.erase(chit);
            m_isHistoryFull = false;

            // Inform writer
            mp_writer->change_removed_by_history(a_change);

            // Release from pools
            mp_writer->release_change(a_change);
            return true;
        }
    }
    logWarning(RTPS_HISTORY, "SequenceNumber " << a_change->sequenceNumber << " not found");
    return false;
}

bool WriterHistory::remove_change_g(
        CacheChange_t* a_change)
{
    return remove_change(a_change);
}

bool WriterHistory::remove_change(
        const SequenceNumber_t& sequence_number)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before removing any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    for (std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
            chit != m_changes.end(); ++chit)
    {
        if ((*chit)->sequenceNumber == sequence_number)
        {
            CacheChange_t* change = *chit;

            // Remove from history
            m_changes.erase(chit);
            m_isHistoryFull = false;

            // Inform writer
            mp_writer->change_removed_by_history(change);

            // Release from pools
            mp_writer->release_change(change);
            return true;
        }
    }

    logWarning(RTPS_HISTORY, "SequenceNumber " <<  sequence_number << " not found");
    return false;
}

CacheChange_t* WriterHistory::remove_change_and_reuse(
        const SequenceNumber_t& sequence_number)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before removing any changes");
        return nullptr;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    for (std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
            chit != m_changes.end(); ++chit)
    {
        if ((*chit)->sequenceNumber == sequence_number)
        {
            CacheChange_t* change = *chit;

            // Remove from history
            m_changes.erase(chit);
            m_isHistoryFull = false;

            // Inform writer
            mp_writer->change_removed_by_history(change);

            // Do not release, but return for reuse
            return change;
        }
    }

    logWarning(RTPS_HISTORY, "SequenceNumber " <<  sequence_number << " not found");
    return nullptr;
}

bool WriterHistory::remove_min_change()
{

    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before removing any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (m_changes.size() > 0 && remove_change_g(m_changes.front()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//TODO Hacer metodos de remove_all_changes. y hacer los metodos correspondientes en los writers y publishers.

void WriterHistory::do_release_cache(
        CacheChange_t* ch)
{
    mp_writer->release_change(ch);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
