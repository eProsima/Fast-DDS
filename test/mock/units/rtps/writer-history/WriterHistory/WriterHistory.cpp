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

// Link-seam implementation of WriterHistory: replaces src/cpp/rtps/history/WriterHistory.cpp and forwards the
// creation, addition, removal and release of changes to the WriterHistoryMock companion of the instance. Changes the
// companion accepts are kept in the history, so the History methods can find them, but they are never released to a
// pool and the writer is never notified.

#include <fastdds/rtps/history/WriterHistory.hpp>

#include <algorithm>
#include <chrono>

#include <fastdds/rtps/common/WriteParams.hpp>
#include <mock/fastdds/rtps/history/WriterHistoryMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<WriterHistory>;

WriteParams WriteParams::WRITE_PARAM_DEFAULT;

WriterHistory::WriterHistory(
        const HistoryAttributes& att)
    : History(att)
{
    Companion::attach(this);
    mp_mutex = &Companion::get(this).mutex;
}

WriterHistory::WriterHistory(
        const HistoryAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool)
    : History(att)
    , payload_pool_(payload_pool)
{
    Companion::attach(this);
    mp_mutex = &Companion::get(this).mutex;
}

WriterHistory::WriterHistory(
        const HistoryAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool)
    : History(att)
    , change_pool_(change_pool)
    , payload_pool_(payload_pool)
{
    Companion::attach(this);
    mp_mutex = &Companion::get(this).mutex;
}

WriterHistory::~WriterHistory()
{
    Companion::detach(this);
}

const std::shared_ptr<IPayloadPool>& WriterHistory::get_payload_pool() const
{
    return payload_pool_;
}

const std::shared_ptr<IChangePool>& WriterHistory::get_change_pool() const
{
    return change_pool_;
}

CacheChange_t* WriterHistory::create_change(
        ChangeKind_t change_kind,
        InstanceHandle_t handle)
{
    return Companion::get(this).create_change(change_kind, handle);
}

CacheChange_t* WriterHistory::create_change(
        uint32_t payload_size,
        ChangeKind_t change_kind,
        InstanceHandle_t handle)
{
    return Companion::get(this).create_change(payload_size, change_kind, handle);
}

bool WriterHistory::add_change(
        CacheChange_t* a_change)
{
    WriteParams wparams;
    return Companion::get(this).add_change(a_change) && prepare_and_add_change(a_change, wparams);
}

bool WriterHistory::add_change(
        CacheChange_t* a_change,
        WriteParams& wparams)
{
    return Companion::get(this).add_change(a_change, wparams) && prepare_and_add_change(a_change, wparams);
}

bool WriterHistory::add_change_(
        CacheChange_t* a_change,
        WriteParams& wparams,
        std::chrono::time_point<std::chrono::steady_clock>)
{
    return Companion::get(this).add_change(a_change, wparams) && prepare_and_add_change(a_change, wparams);
}

bool WriterHistory::prepare_and_add_change(
        CacheChange_t* a_change,
        WriteParams&)
{
    a_change->sequenceNumber = next_sequence_number();
    m_changes.push_back(a_change);
    m_lastCacheChangeSeqNum = a_change->sequenceNumber;
    Companion::get(this).sample_added();
    return true;
}

void WriterHistory::notify_writer(
        CacheChange_t*,
        const std::chrono::time_point<std::chrono::steady_clock>&)
{
}

void WriterHistory::set_fragments(
        CacheChange_t*)
{
}

bool WriterHistory::matches_change(
        const CacheChange_t* inner_change,
        CacheChange_t* outer_change)
{
    return nullptr != inner_change && nullptr != outer_change &&
           inner_change->sequenceNumber == outer_change->sequenceNumber;
}

History::iterator WriterHistory::remove_change_nts(
        const_iterator removal,
        bool release)
{
    return WriterHistory::remove_change_nts(removal, std::chrono::steady_clock::now() + std::chrono::hours(24),
                   release);
}

History::iterator WriterHistory::remove_change_nts(
        const_iterator removal,
        const std::chrono::time_point<std::chrono::steady_clock>&,
        bool)
{
    if (removal == changesEnd())
    {
        return changesEnd();
    }

    if (!Companion::get(this).remove_change(*removal))
    {
        return remove_iterator_constness(removal);
    }

    m_isHistoryFull = false;
    return m_changes.erase(removal);
}

bool WriterHistory::remove_change_g(
        CacheChange_t* a_change)
{
    return remove_change(a_change);
}

bool WriterHistory::remove_change_g(
        CacheChange_t* a_change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    return remove_change(a_change, max_blocking_time);
}

static void erase_sequence_number(
        std::vector<CacheChange_t*>& changes,
        const SequenceNumber_t& sequence_number)
{
    changes.erase(std::remove_if(changes.begin(), changes.end(), [&sequence_number](const CacheChange_t* change)
            {
                return change->sequenceNumber == sequence_number;
            }), changes.end());
}

bool WriterHistory::remove_change(
        const SequenceNumber_t& sequence_number)
{
    if (!Companion::get(this).remove_change(sequence_number))
    {
        return false;
    }

    erase_sequence_number(m_changes, sequence_number);
    return true;
}

CacheChange_t* WriterHistory::remove_change_and_reuse(
        const SequenceNumber_t& sequence_number)
{
    CacheChange_t* removal = Companion::get(this).remove_change_and_reuse(sequence_number);
    if (nullptr != removal)
    {
        erase_sequence_number(m_changes, sequence_number);
    }
    return removal;
}

bool WriterHistory::remove_min_change()
{
    return remove_min_change(std::chrono::steady_clock::now() + std::chrono::hours(24));
}

bool WriterHistory::remove_min_change(
        const std::chrono::time_point<std::chrono::steady_clock>&)
{
    if (!Companion::get(this).remove_min_change())
    {
        return false;
    }

    if (!m_changes.empty())
    {
        m_changes.erase(m_changes.begin());
    }
    return true;
}

bool WriterHistory::release_change(
        CacheChange_t* ch)
{
    return Companion::get(this).release_change(ch);
}

void WriterHistory::do_release_cache(
        CacheChange_t*)
{
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
