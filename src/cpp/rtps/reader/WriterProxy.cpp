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
 * @file WriterProxy.cpp
 *
 */

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include "WriterProxy.h"
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>

#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include "../participant/RTPSParticipantImpl.h"

#include <foonathan/memory/namespace_alias.hpp>
#include <fastrtps/utils/collections/foonathan_memory_helpers.hpp>

#if !defined(NDEBUG) && defined(FASTRTPS_SOURCE) && defined(__linux__)
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <cassert>
#include <mutex>
#endif

namespace eprosima {
namespace fastrtps {
namespace rtps {

/*!
 * @brief Auxiliary function to change status in a range.
 */
void WriterProxy::for_each_set_status_from(
        ChangeIterator first,
        ChangeIterator last,
        ChangeFromWriterStatus_t status,
        ChangeFromWriterStatus_t new_status)
{
    ChangeIterator it = first;
    while(it != last)
    {
        if(it->getStatus() == status)
        {
            ChangeFromWriter_t& ch = const_cast<ChangeFromWriter_t&>(*it);
            ch.setStatus(new_status);
        }

        ++it;
    }
}

void WriterProxy::for_each_set_status_from_and_maybe_remove(
        ChangeIterator first,
        ChangeIterator last,
        ChangeFromWriterStatus_t status,
        ChangeFromWriterStatus_t or_status,
        ChangeFromWriterStatus_t new_status)
{
    ChangeIterator it = first;
    while(it != last)
    {
        if(it->getStatus() == status || it->getStatus() == or_status)
        {
            if(it != changes_from_writer_.begin())
            {
                ChangeFromWriter_t& ch = const_cast<ChangeFromWriter_t&>(*it);
                ch.setStatus(new_status);
                ++it;
                continue;
            }
        }

        // UNKNOWN or MISSING at the beginning or
        // LOST or RECEIVED at the beginning.
        if(it == changes_from_writer_.begin())
        {
            changes_from_writer_low_mark_ = it->getSequenceNumber();
            it = changes_from_writer_.erase(it);
        }
    }
}

WriterProxy::~WriterProxy()
{
    delete(initial_acknack_);
    delete(writer_proxy_liveliness_);
    delete(heartbeat_response_);
}

constexpr size_t changes_node_size = memory::set_node_size<std::pair<size_t, ChangeFromWriter_t>>::value;

WriterProxy::WriterProxy(
        StatefulReader* reader,
        const RemoteLocatorsAllocationAttributes& loc_alloc,
        const ResourceLimitedContainerConfig& changes_allocation)
    : reader_(reader)
    , attributes_(loc_alloc.max_unicast_locators, loc_alloc.max_multicast_locators)
    , heartbeat_response_(nullptr)
    , writer_proxy_liveliness_(nullptr)
    , initial_acknack_(nullptr)
    , last_heartbeat_count_(0)
    , heartbeat_final_flag_(false)
    , is_alive_(false)
    , changes_pool_(
            changes_node_size,
            memory_pool_block_size<pool_allocator_t>(changes_node_size, changes_allocation))
    , changes_from_writer_(changes_pool_)
    , guid_as_vector_(ResourceLimitedContainerConfig::fixed_size_configuration(1u))
    , guid_prefix_as_vector_(ResourceLimitedContainerConfig::fixed_size_configuration(1u))
{
    //Create Events
    writer_proxy_liveliness_ = new TimedEvent(reader->getRTPSParticipant()->getEventResource(),
            [&](TimedEvent::EventCode code) -> bool
            {
                if (TimedEvent::EVENT_SUCCESS == code)
                {
                    reader_->liveliness_expired(attributes_.guid());
                }

                return false;
            }, 0);

    heartbeat_response_ = new TimedEvent(reader_->getRTPSParticipant()->getEventResource(),
            [&](TimedEvent::EventCode code) -> bool
            {
                if (TimedEvent::EVENT_SUCCESS == code)
                {
                    perform_heartbeat_response();
                }

                return false;
            }, 0);

    initial_acknack_ = new TimedEvent(reader_->getRTPSParticipant()->getEventResource(),
            [&](TimedEvent::EventCode code) -> bool
            {
                if (TimedEvent::EVENT_SUCCESS == code)
                {
                    perform_initial_ack_nack();
                }

                return false;
            }, 0 );

    clear();
    logInfo(RTPS_READER, "Writer Proxy created in reader: " << reader_->getGuid().entityId);
}

void WriterProxy::start(const WriterProxyData& attributes)
{
    heartbeat_response_->update_interval(reader_->getTimes().heartbeatResponseDelay);
    initial_acknack_->update_interval(reader_->getTimes().initialAcknackDelay);

    attributes_ = attributes;
    guid_as_vector_.push_back(attributes_.guid());
    guid_prefix_as_vector_.push_back(attributes_.guid().guidPrefix);
    is_alive_ = true;
    if (attributes_.m_qos.m_liveliness.lease_duration < c_TimeInfinite)
    {
        writer_proxy_liveliness_->cancel_timer();
        writer_proxy_liveliness_->update_interval(attributes_.m_qos.m_liveliness.lease_duration);
        writer_proxy_liveliness_->restart_timer();
    }
    initial_acknack_->restart_timer();
}

void WriterProxy::update(const WriterProxyData& attributes)
{
#if !defined(NDEBUG) && defined(FASTRTPS_SOURCE) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    assert(is_alive_);
    attributes_ = attributes;
}

void WriterProxy::stop()
{
    writer_proxy_liveliness_->cancel_timer();
    initial_acknack_->cancel_timer();
    heartbeat_response_->cancel_timer();

    clear();
}

void WriterProxy::clear()
{
    is_alive_ = false;
    attributes_.guid(c_Guid_Unknown);
    last_heartbeat_count_ = 0;
    heartbeat_final_flag_ = false;
    guid_as_vector_.clear();
    guid_prefix_as_vector_.clear();
    changes_from_writer_.clear();
}

void WriterProxy::loaded_from_storage(const SequenceNumber_t& seq_num)
{
#if !defined(NDEBUG) && defined(FASTRTPS_SOURCE) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    last_notified_ = seq_num;
    changes_from_writer_low_mark_ = seq_num;
}

void WriterProxy::missing_changes_update(const SequenceNumber_t& seq_num)
{
#if !defined(NDEBUG) && defined(FASTRTPS_SOURCE) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    logInfo(RTPS_READER,attributes_.guid().entityId<<": changes up to seq_num: " << seq_num <<" missing.");

    // Check was not removed from container.
    if(seq_num > changes_from_writer_low_mark_)
    {
        if(changes_from_writer_.empty() || changes_from_writer_.rbegin()->getSequenceNumber() < seq_num)
        {
            // Set already values in container.
            for_each_set_status_from(changes_from_writer_.begin(), changes_from_writer_.end(),
                    ChangeFromWriterStatus_t::UNKNOWN, ChangeFromWriterStatus_t::MISSING);

            // Changes only already inserted values.
            bool will_be_the_last = maybe_add_changes_from_writer_up_to(seq_num, ChangeFromWriterStatus_t::MISSING);
            (void)will_be_the_last;
            assert(will_be_the_last);

            // Add requetes sequence number.
            ChangeFromWriter_t newch(seq_num);
            newch.setStatus(ChangeFromWriterStatus_t::MISSING);
            changes_from_writer_.insert(changes_from_writer_.end(), newch);
        }
        else
        {
            // Find it. Must be there.
            ChangeIterator last_it = changes_from_writer_.find(ChangeFromWriter_t(seq_num));
            assert(last_it != changes_from_writer_.end());
            for_each_set_status_from(changes_from_writer_.begin(), ++last_it,
                    ChangeFromWriterStatus_t::UNKNOWN, ChangeFromWriterStatus_t::MISSING);
        }
    }
}

bool WriterProxy::maybe_add_changes_from_writer_up_to(
        const SequenceNumber_t& sequence_number,
        const ChangeFromWriterStatus_t default_status)
{
    bool returnedValue = false;
    // Check if CacheChange_t is in the container or not.
    SequenceNumber_t lastSeqNum = changes_from_writer_low_mark_;

    if (changes_from_writer_.size() > 0)
    {
        lastSeqNum = changes_from_writer_.rbegin()->getSequenceNumber();
    }

    if(sequence_number > lastSeqNum)
    {
        returnedValue = true;

        // If it is not in the container, create info up to its sequence number.
        ++lastSeqNum;
        for(; lastSeqNum < sequence_number; ++lastSeqNum)
        {
            ChangeFromWriter_t newch(lastSeqNum);
            newch.setStatus(default_status);
            changes_from_writer_.insert(changes_from_writer_.end(), newch);
        }
    }

    return returnedValue;
}

void WriterProxy::lost_changes_update(const SequenceNumber_t& seq_num)
{
#if !defined(NDEBUG) && defined(FASTRTPS_SOURCE) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    logInfo(RTPS_READER, attributes_.guid().entityId <<": up to seq_num: "<<seq_num);

    // Check was not removed from container.
    if(seq_num > changes_from_writer_low_mark_)
    {
        if(changes_from_writer_.size() == 0 || changes_from_writer_.rbegin()->getSequenceNumber() < seq_num)
        {
            // Remove all because lost or received.
            changes_from_writer_.clear();
            // Any in container, then not insert new lost.
            changes_from_writer_low_mark_ = seq_num - 1;
        }
        else
        {
            // Find it. Must be there.
            ChangeIterator last_it = changes_from_writer_.find(ChangeFromWriter_t(seq_num));
            assert(last_it != changes_from_writer_.end());
            for_each_set_status_from_and_maybe_remove(changes_from_writer_.begin(), last_it,
                    ChangeFromWriterStatus_t::UNKNOWN, ChangeFromWriterStatus_t::MISSING,
                    ChangeFromWriterStatus_t::LOST);
            // Next could need to be removed.
            cleanup();
        }
    }
}

bool WriterProxy::received_change_set(const SequenceNumber_t& seq_num)
{
    logInfo(RTPS_READER, attributes_.guid().entityId << ": seq_num: " << seq_num);
    return received_change_set(seq_num, true);
}

bool WriterProxy::irrelevant_change_set(const SequenceNumber_t& seq_num)
{
    return received_change_set(seq_num, false);
}

bool WriterProxy::received_change_set(
        const SequenceNumber_t& seq_num,
        bool is_relevance)
{
#if !defined(NDEBUG) && defined(FASTRTPS_SOURCE) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    // Check if CacheChange_t was already and it was already removed from changesFromW container.
    if(seq_num <= changes_from_writer_low_mark_)
    {
        logInfo(RTPS_READER, "Change " << seq_num << " <= than max available sequence number "
            << changes_from_writer_low_mark_);
        return false;
    }

    // Maybe create information because it is not in the changes_from_writer_ container.
    bool will_be_the_last = maybe_add_changes_from_writer_up_to(seq_num);

    // If will be the last element, insert it at the end.
    if(will_be_the_last)
    {
        // There are others.
        if(changes_from_writer_.size() > 0)
        {
            ChangeFromWriter_t chfw(seq_num);
            chfw.setStatus(RECEIVED);
            chfw.setRelevance(is_relevance);
            changes_from_writer_.insert(changes_from_writer_.end(), chfw);
        }
        // Else not insert
        else
        {
            changes_from_writer_low_mark_ = seq_num;
        }
    }
    // Else it has to be found and change state.
    else
    {
        ChangeIterator chit = changes_from_writer_.find(ChangeFromWriter_t(seq_num));

        // Has to be in the container.
        assert(chit != changes_from_writer_.end());

        if(chit != changes_from_writer_.begin())
        {
            if(chit->getStatus() != RECEIVED)
            {
                ChangeFromWriter_t& ch = const_cast<ChangeFromWriter_t&>(*chit);
                ch.setStatus(RECEIVED);
                ch.setRelevance(is_relevance);
            }
            else
            {
                return false;
            }
        }
        else
        {
            assert(chit->getStatus() != RECEIVED);
            changes_from_writer_low_mark_ = seq_num;
            changes_from_writer_.erase(chit);
            cleanup();
        }
    }

    return true;
}


SequenceNumberSet_t WriterProxy::missing_changes() const
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    SequenceNumberSet_t sns(available_changes_max() + 1);

    for(const ChangeFromWriter_t& ch : changes_from_writer_)
    {
        if(ch.getStatus() == MISSING)
        {
            // If MISSING, then is relevant.
            assert(ch.isRelevant());
            if(!sns.add(ch.getSequenceNumber()))
            {
                logInfo(RTPS_READER, "Sequence number " << ch.getSequenceNumber()
                    << " exceeded bitmap limit of AckNack. SeqNumSet Base: " << sns.base());
                break;
            }
        }
    }

    return sns;
}

bool WriterProxy::change_was_received(const SequenceNumber_t& seq_num) const
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    if (seq_num <= changes_from_writer_low_mark_)
    {
        return true;
    }

    ChangeIterator chit = changes_from_writer_.find(ChangeFromWriter_t(seq_num));
    return (chit != changes_from_writer_.end() && chit->getStatus() == RECEIVED);
}

const SequenceNumber_t WriterProxy::available_changes_max() const
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    return changes_from_writer_low_mark_;
}

void WriterProxy::assert_liveliness()
{
    logInfo(RTPS_READER,attributes_.guid().entityId << " Liveliness asserted");

    is_alive_ = true;

    if (attributes_.m_qos.m_liveliness.lease_duration < c_TimeInfinite)
    {
        writer_proxy_liveliness_->cancel_timer();
        writer_proxy_liveliness_->restart_timer();
    }
}

void WriterProxy::change_removed_from_history(const SequenceNumber_t& seq_num)
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    // Check sequence number is in the container, because it was not clean up.
    if (seq_num <= changes_from_writer_low_mark_)
    {
        return;
    }

    ChangeIterator chit = changes_from_writer_.find(ChangeFromWriter_t(seq_num));

    // Element must be in the container. In other case, bug.
    assert(chit != changes_from_writer_.end());
    // If the element will be set not valid, element must be received.
    // In other case, bug.
    assert(chit->getStatus() == RECEIVED);

    // Cannot be in the beginning because process of cleanup
    assert(chit != changes_from_writer_.begin());

    ChangeFromWriter_t& ch = const_cast<ChangeFromWriter_t&>(*chit);
    ch.notValid();
}

void WriterProxy::cleanup()
{
    ChangeIterator chit = changes_from_writer_.begin();

    while(chit != changes_from_writer_.end() &&
            (chit->getStatus() == RECEIVED || chit->getStatus() == LOST))
    {
        changes_from_writer_low_mark_ = chit->getSequenceNumber();
        chit = changes_from_writer_.erase(chit);
    }
}

bool WriterProxy::are_there_missing_changes() const
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    for(const ChangeFromWriter_t& ch : changes_from_writer_)
    {
        if(ch.getStatus() == ChangeFromWriterStatus_t::MISSING)
        {
            return true;
        }
    }

    return false;
}

size_t WriterProxy::unknown_missing_changes_up_to(const SequenceNumber_t& seq_num) const
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    size_t returnedValue = 0;

    if(seq_num > changes_from_writer_low_mark_)
    {
        for(ChangeIterator ch = changes_from_writer_.begin();
            ch != changes_from_writer_.end() && ch->getSequenceNumber() < seq_num;
            ++ch)
        {
            if (ch->getStatus() == ChangeFromWriterStatus_t::UNKNOWN ||
                ch->getStatus() == ChangeFromWriterStatus_t::MISSING)
            {
                ++returnedValue;
            }
        }
    }

    return returnedValue;
}

size_t WriterProxy::number_of_changes_from_writer() const
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    return changes_from_writer_.size();
}

SequenceNumber_t WriterProxy::next_cache_change_to_be_notified()
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    if(last_notified_ < changes_from_writer_low_mark_)
    {
        ++last_notified_;
        return last_notified_;
    }

    return SequenceNumber_t::unknown();
}

void WriterProxy::perform_initial_ack_nack() const
{
    // Send initial NACK.
    SequenceNumberSet_t sns(SequenceNumber_t(0, 0));
    reader_->send_acknack(sns, *this, false);
}

void WriterProxy::perform_heartbeat_response() const
{
    reader_->send_acknack(this, *this, heartbeat_final_flag_);
}

bool WriterProxy::process_heartbeat(
        uint32_t count,
        const SequenceNumber_t& first_seq,
        const SequenceNumber_t& last_seq,
        bool final_flag,
        bool liveliness_flag,
        bool disable_positive)
{
#if defined(__DEBUG) && defined(__linux__)
    assert(get_mutex_owner() == get_thread_id());
#endif

    if (last_heartbeat_count_ < count)
    {
        // If it is the first heartbeat message, we can try to cancel initial ack.
        // TODO: This timer cancelling should be checked if needed with the liveliness implementation.
        // To keep PARTICIPANT_DROPPED event we should add an explicit participant_liveliness QoS.
        // This is now commented to avoid issues #457 and #155
        // initial_acknack_->cancel_timer();

        last_heartbeat_count_ = count;
        lost_changes_update(first_seq);
        missing_changes_update(last_seq);
        heartbeat_final_flag_ = final_flag;

        //Analyze wheter a acknack message is needed:
        if (!final_flag)
        {
            if (!disable_positive || are_there_missing_changes())
            {
                heartbeat_response_->restart_timer();
            }
        }
        else if (final_flag && !liveliness_flag)
        {
            if (are_there_missing_changes())
            {
                heartbeat_response_->restart_timer();
            }
        }

        //TODO: liveliness implementation details
        if (liveliness_flag)
        {
            assert_liveliness();
        }

        return true;
    }

    return false;
}

void WriterProxy::update_heartbeat_response_interval(const Duration_t& interval)
{
    heartbeat_response_->update_interval(interval);
}

bool WriterProxy::send(
        CDRMessage_t* message,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    for (const Locator_t& locator : remote_locators_shrinked())
    {
        if (!reader_->send_sync_nts(message, locator, max_blocking_time_point))
        {
            return false;
        }
    }

    return true;
}

#if !defined(NDEBUG) && defined(FASTRTPS_SOURCE) && defined(__linux__)
int WriterProxy::get_mutex_owner() const
{
    auto mutex = reader_->getMutex().native_handle();
    return mutex->__data.__owner;
}

int WriterProxy::get_thread_id() const
{
    return syscall(__NR_gettid);
}
#endif

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
