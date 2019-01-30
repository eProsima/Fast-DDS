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
 * @file ReaderProxy.cpp
 *
 */


#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>
#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/utils/TimeConversion.h>

#include <mutex>

#include <cassert>


namespace eprosima {
namespace fastrtps {
namespace rtps {


ReaderProxy::ReaderProxy(
        const RemoteReaderAttributes& reader_attributes,
        const WriterTimes& times, 
        StatefulWriter* writer) 
    : reader_attributes_(reader_attributes)
    , writer_(writer)
    , nack_response_event_(nullptr)
    , nack_supression_event_(nullptr)
    , last_acknack_count_(0)
    , last_nackfrag_count_(0)
{
    if (reader_attributes.endpoint.reliabilityKind == RELIABLE)
    {
        nack_response_event_ = new NackResponseDelay(writer_, reader_attributes_.guid,
            TimeConv::Time_t2MilliSecondsDouble(times.nackResponseDelay));
        nack_supression_event_ = new NackSupressionDuration(writer_, reader_attributes_.guid,
            TimeConv::Time_t2MilliSecondsDouble(times.nackSupressionDuration));
    }

    // Use remoteLocatorList as joint unicast + multicast locators
    reader_attributes_.endpoint.remoteLocatorList.assign(reader_attributes_.endpoint.unicastLocatorList);
    reader_attributes_.endpoint.remoteLocatorList.push_back(reader_attributes_.endpoint.multicastLocatorList);

    logInfo(RTPS_WRITER, "Reader Proxy created");
}

ReaderProxy::~ReaderProxy()
{
    destroy_timers();
}

void ReaderProxy::destroy_timers()
{
    if (nack_response_event_ != nullptr)
    {
        delete(nack_response_event_);
        nack_response_event_ = nullptr;
    }

    if (nack_supression_event_ != nullptr)
    {
        delete(nack_supression_event_);
        nack_supression_event_ = nullptr;
    }
}

void ReaderProxy::update_nack_response_interval(const Duration_t& interval)
{
    if (nack_response_event_ != nullptr)
    {
        nack_response_event_->update_interval(interval);
    }
}

void ReaderProxy::update_nack_supression_interval(const Duration_t& interval)
{
    if (nack_supression_event_ != nullptr)
    {
        nack_supression_event_->update_interval(interval);
    }
}

void ReaderProxy::add_change(
        const ChangeForReader_t& change, 
        bool restart_nack_supression)
{
    assert(change.getSequenceNumber() > changes_low_mark_);
    assert(m_changesForReader.empty() ? true :
        change.getSequenceNumber() > m_changesForReader.rbegin()->getSequenceNumber());

    if (restart_nack_supression && nack_supression_event_ != nullptr)
    {
        nack_supression_event_->restart_timer();
    }

    // For best effort readers, changes are acked when being sent
    if (m_changesForReader.empty() && change.getStatus() == ACKNOWLEDGED)
    {
        changes_low_mark_ = change.getSequenceNumber();
        return;
    }

    m_changesForReader.insert(change);
}

bool ReaderProxy::has_changes() const
{
    return !m_changesForReader.empty();
}

bool ReaderProxy::change_is_acked(const SequenceNumber_t& seq_num) const
{
    if (seq_num <= changes_low_mark_)
    {
        return true;
    }

    auto chit = m_changesForReader.find(ChangeForReader_t(seq_num));
    assert(chit != m_changesForReader.end());

    return !chit->isRelevant() || chit->getStatus() == ACKNOWLEDGED;
}

void ReaderProxy::acked_changes_set(const SequenceNumber_t& seq_num)
{
    SequenceNumber_t future_low_mark = seq_num;

    if (seq_num > changes_low_mark_)
    {
        auto chit = m_changesForReader.find(seq_num);
        m_changesForReader.erase(m_changesForReader.begin(), chit);
    }
    else
    {
        // Special case. Currently only used on Builtin StatefulWriters
        // after losing lease duration.

        SequenceNumber_t current_sequence = seq_num;
        if (seq_num < writer_->get_seq_num_min())
        {
            current_sequence = writer_->get_seq_num_min();
        }
        future_low_mark = current_sequence;

        for (; current_sequence <= changes_low_mark_; ++current_sequence)
        {
            CacheChange_t* change = nullptr;

            if (writer_->mp_history->get_change(current_sequence, writer_->getGuid(), &change))
            {
                ChangeForReader_t cr(change);
                cr.setStatus(UNACKNOWLEDGED);
                m_changesForReader.insert(cr);
            }
            else
            {
                ChangeForReader_t cr(current_sequence);
                cr.setStatus(UNACKNOWLEDGED);
                cr.notValid();
                m_changesForReader.insert(cr);
            }
        }
    }

    changes_low_mark_ = future_low_mark - 1;
}

bool ReaderProxy::requested_changes_set(const SequenceNumberSet_t& seq_num_set)
{
    bool isSomeoneWasSetRequested = false;

    seq_num_set.for_each([&](SequenceNumber_t sit)
    {
        auto chit = m_changesForReader.find(ChangeForReader_t(sit));
        if (chit != m_changesForReader.end())
        {
            ChangeForReader_t& newch = const_cast<ChangeForReader_t&>(*chit);
            newch.setStatus(REQUESTED);
            newch.markAllFragmentsAsUnsent();
            isSomeoneWasSetRequested = true;
        }
    });

    if (isSomeoneWasSetRequested)
    {
        logInfo(RTPS_WRITER, "Requested Changes: " << seq_num_set);
        if (nack_response_event_ != nullptr)
        {
            nack_response_event_->restart_timer();
        }
    }
    else if (!seq_num_set.empty())
    {
        logWarning(RTPS_WRITER, "Requested Changes: " << seq_num_set
            << " not found (low mark: " << changes_low_mark_ << ")");
    }

    return isSomeoneWasSetRequested;
}

bool ReaderProxy::set_change_to_status(
        const SequenceNumber_t& seq_num, 
        ChangeForReaderStatus_t status,
        bool restart_nack_supression)
{
    if (restart_nack_supression && is_reliable())
    {
        assert(nack_supression_event_ != nullptr);
        nack_supression_event_->restart_timer();
    }

    if (seq_num <= changes_low_mark_)
    {
        return false;
    }

    auto it = m_changesForReader.find(ChangeForReader_t(seq_num));
    bool change_was_modified = false;

    if (it != m_changesForReader.end())
    {
        if (status == ACKNOWLEDGED && it == m_changesForReader.begin())
        {
            m_changesForReader.erase(it);
            changes_low_mark_ = seq_num;
            change_was_modified = true;
        }
        else
        {
            if (it->getStatus() != status)
            {
                ChangeForReader_t& newch = const_cast<ChangeForReader_t&>(*it);
                newch.setStatus(status);
                change_was_modified = true;
            }
        }
    }

    return change_was_modified;
}

bool ReaderProxy::mark_fragment_as_sent_for_change(
        const SequenceNumber_t& seq_num, 
        FragmentNumber_t frag_num,
        bool& was_last_fragment)
{
    was_last_fragment = false;

    if (seq_num <= changes_low_mark_)
    {
        return false;
    }

    bool change_found = false;
    auto it = m_changesForReader.find(ChangeForReader_t(seq_num));

    if (it != m_changesForReader.end())
    {
        change_found = true;
        ChangeForReader_t& newch = const_cast<ChangeForReader_t&>(*it);
        newch.markFragmentsAsSent(frag_num);
        was_last_fragment = newch.getUnsentFragments().empty();
    }

    return change_found;

}

bool ReaderProxy::convert_status_on_all_changes(
        ChangeForReaderStatus_t previous, 
        ChangeForReaderStatus_t next)
{
    if (previous == next)
    {
        return false;
    }

    bool at_least_one_modified = false;

    auto it = m_changesForReader.begin();
    while (it != m_changesForReader.end())
    {
        if (it->getStatus() == previous)
        {
            at_least_one_modified = true;

            if (next == ACKNOWLEDGED && it == m_changesForReader.begin())
            {
                changes_low_mark_ = it->getSequenceNumber();
                it = m_changesForReader.erase(it);
                continue;
            }

            // Note: we can perform this cast as we are not touching the sorting field (seq_num)
            const_cast<ChangeForReader_t&>(*it).setStatus(next);
        }

        ++it;
    }

    return at_least_one_modified;
}

void ReaderProxy::change_has_been_removed(const SequenceNumber_t& seq_num)
{
    // Check sequence number is in the container, because it was not clean up.
    if (m_changesForReader.empty() || seq_num < m_changesForReader.begin()->getSequenceNumber())
    {
        return;
    }

    auto chit = m_changesForReader.find(ChangeForReader_t(seq_num));

    // Element must be in the container. In other case, bug.
    assert(chit != m_changesForReader.end());

    // Note: we can perform this cast as we are not touching the sorting field (seq_num)
    auto & newch = const_cast<ChangeForReader_t&>(*chit);

    if (chit == m_changesForReader.begin())
    {
        assert(chit->getStatus() != ACKNOWLEDGED);

        // if it is the first element, set state to unacknowledge because from now reader has to confirm
        // it will not be expecting it.
        newch.setStatus(UNACKNOWLEDGED);
    }
    else
    {
        // In case its state is not ACKNOWLEDGED, set it to UNACKNOWLEDGE because from now reader has to confirm
        // it will not be expecting it.
        if (newch.getStatus() != ACKNOWLEDGED)
        {
            newch.setStatus(UNACKNOWLEDGED);
        }
    }
    newch.notValid();
}

bool ReaderProxy::has_unacknowledged() const
{
    for (const ChangeForReader_t& it : m_changesForReader)
    {
        if (it.getStatus() == UNACKNOWLEDGED)
        {
            return true;
        }
    }

    return false;
}

bool change_min(
        const ChangeForReader_t* ch1, 
        const ChangeForReader_t* ch2)
{
    return ch1->getSequenceNumber() < ch2->getSequenceNumber();
}

bool ReaderProxy::requested_fragment_set(
        const SequenceNumber_t& seq_num, 
        const FragmentNumberSet_t& frag_set)
{
    // Locate the outbound change referenced by the NACK_FRAG
    auto changeIter = std::find_if(m_changesForReader.begin(), m_changesForReader.end(),
        [seq_num](const ChangeForReader_t& change)
    {
        return change.getSequenceNumber() == seq_num;
    });
    if (changeIter == m_changesForReader.end())
    {
        return false;
    }

    ChangeForReader_t& newch = const_cast<ChangeForReader_t&>(*changeIter);
    newch.markFragmentsAsUnsent(frag_set);

    // If it was UNSENT, we shouldn't switch back to REQUESTED to prevent stalling.
    if (newch.getStatus() != UNSENT)
    {
        newch.setStatus(REQUESTED);
    }

    return true;
}

bool ReaderProxy::process_nack_frag(
        const GUID_t& reader_guid, 
        uint32_t nack_count,
        const SequenceNumber_t& seq_num,
        const FragmentNumberSet_t& fragments_state)
{
    if (reader_attributes_.guid == reader_guid)
    {
        if (last_nackfrag_count_ < nack_count)
        {
            last_nackfrag_count_ = nack_count;
            // TODO Not doing Acknowledged.
            if (requested_fragment_set(seq_num, fragments_state))
            {
                nack_response_event_->restart_timer();
            }
        }

        return true;
    }

    return false;
}

}   // namespace rtps
}   // namespace fastrtps
}   // namespace eprosima