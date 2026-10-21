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

#include <rtps/reader/WriterProxy.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include "rtps/domain/RTPSDomainImpl.hpp"
#include "utils/collections/node_size_helpers.hpp"
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/messages/RTPSMessageCreator.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/BaseWriter.hpp>

#if !defined(NDEBUG) && !defined(ANDROID) && defined(FASTDDS_SOURCE) && defined(__unix__)
#define SHOULD_DEBUG_LINUX
#endif // SHOULD_DEBUG_LINUX

#ifdef SHOULD_DEBUG_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <cassert>
#include <mutex>
#endif // SHOULD_DEBUG_LINUX

namespace eprosima {
namespace fastdds {
namespace rtps {

WriterProxy::~WriterProxy()
{
    if (is_alive_ && is_on_same_process_)
    {
        EPROSIMA_LOG_WARNING(RTPS_READER, "Automatically unmatching on ~WriterProxy");
        BaseWriter* writer = RTPSDomainImpl::get_instance()->find_writer(guid());
        if (writer)
        {
            writer->matched_reader_remove(reader_->getGuid());
        }
    }

    delete(initial_acknack_);
    delete(heartbeat_response_);
}

using set_helper = utilities::collections::set_size_helper<SequenceNumber_t>;

WriterProxy::WriterProxy(
        StatefulReader* reader,
        const RemoteLocatorsAllocationAttributes& loc_alloc,
        const ResourceLimitedContainerConfig& changes_allocation)
    : reader_(reader)
    , heartbeat_response_(nullptr)
    , initial_acknack_(nullptr)
    , last_heartbeat_count_(0)
    , heartbeat_final_flag_(false)
    , is_alive_(false)
    , changes_pool_(
        set_helper::node_size,
        set_helper::min_pool_size<pool_allocator_t>(changes_allocation.initial))
    , changes_received_(changes_pool_)
    , guid_as_vector_(ResourceLimitedContainerConfig::fixed_size_configuration(1u))
    , guid_prefix_as_vector_(ResourceLimitedContainerConfig::fixed_size_configuration(1u))
    , is_on_same_process_(false)
    , ownership_strength_(0)
    , liveliness_kind_(dds::AUTOMATIC_LIVELINESS_QOS)
    , locators_entry_(loc_alloc.max_unicast_locators, loc_alloc.max_multicast_locators)
    , is_datasharing_writer_(false)
    , received_at_least_one_heartbeat_(false)
    , state_(StateCode::STOPPED)
{
    //Create Events
    ResourceEvent& event_manager = reader_->getEventResource();
    auto heartbeat_lambda = [this]() -> bool
            {
                perform_heartbeat_response();
                return false;
            };
    auto acknack_lambda = [this]() -> bool
            {
                return perform_initial_ack_nack();
            };

    heartbeat_response_ = new TimedEvent(event_manager, heartbeat_lambda, 0);
    initial_acknack_ = new TimedEvent(event_manager, acknack_lambda, 0);

    clear();
    EPROSIMA_LOG_INFO(RTPS_READER, "Writer Proxy created in reader: " << reader_->getGuid().entityId);
}

void WriterProxy::start(
        const WriterProxyData& attributes,
        const SequenceNumber_t& initial_sequence)
{
    start(attributes, initial_sequence, false);
}

void WriterProxy::start(
        const WriterProxyData& attributes,
        const SequenceNumber_t& initial_sequence,
        bool is_datasharing)
{
    using network::external_locators::filter_remote_locators;

#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    heartbeat_response_->update_interval(reader_->getTimes().heartbeat_response_delay);
    initial_acknack_->update_interval(reader_->getTimes().initial_acknack_delay);

    locators_entry_.remote_guid = attributes.guid;
    guid_as_vector_.push_back(attributes.guid);
    guid_prefix_as_vector_.push_back(attributes.guid.guidPrefix);
    persistence_guid_ = attributes.persistence_guid;
    is_alive_ = true;
    is_on_same_process_ = RTPSDomainImpl::should_intraprocess_between(reader_->getGuid(), attributes.guid);
    ownership_strength_ = attributes.ownership_strength.value;
    liveliness_kind_ = attributes.liveliness.kind;
    locators_entry_.unicast = attributes.remote_locators.unicast;
    locators_entry_.multicast = attributes.remote_locators.multicast;
    filter_remote_locators(locators_entry_,
            reader_->getAttributes().external_unicast_locators, reader_->getAttributes().ignore_non_matching_locators);
    is_datasharing_writer_ = is_datasharing;
    state_.store(StateCode::IDLE);
    initial_acknack_->restart_timer();
    loaded_from_storage(initial_sequence);
    received_at_least_one_heartbeat_ = false;
}

void WriterProxy::update(
        const WriterProxyData& attributes)
{
    using network::external_locators::filter_remote_locators;

#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    assert(is_alive_);
    ownership_strength_ = attributes.ownership_strength.value;
    locators_entry_.unicast = attributes.remote_locators.unicast;
    locators_entry_.multicast = attributes.remote_locators.multicast;
    filter_remote_locators(locators_entry_,
            reader_->getAttributes().external_unicast_locators, reader_->getAttributes().ignore_non_matching_locators);
}

void WriterProxy::stop()
{
    StateCode prev_code;
    if ((prev_code = state_.exchange(StateCode::STOPPED)) == StateCode::BUSY)
    {
        // TimedEvent being performed, wait for it to finish.
        // It does not matter which of the two events is the one on execution, but we must wait on initial_acknack_ as
        // it could be restarted if only cancelled while its callback is being triggered.
        initial_acknack_->recreate_timer();
    }
    else
    {
        initial_acknack_->cancel_timer();
    }
    heartbeat_response_->cancel_timer();

    clear();
}

void WriterProxy::clear()
{
    is_alive_ = false;
    locators_entry_.unicast.clear();
    locators_entry_.multicast.clear();
    locators_entry_.remote_guid = c_Guid_Unknown;
    last_heartbeat_count_ = 0;
    heartbeat_final_flag_.store(false);
    guid_as_vector_.clear();
    guid_prefix_as_vector_.clear();
    changes_received_.clear();
    is_on_same_process_ = false;
    loaded_from_storage(SequenceNumber_t());
}

void WriterProxy::loaded_from_storage(
        const SequenceNumber_t& seq_num)
{
    last_notified_ = seq_num;
    changes_from_writer_low_mark_ = seq_num;
    max_sequence_number_ = seq_num;
}

void WriterProxy::missing_changes_update(
        const SequenceNumber_t& seq_num)
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    EPROSIMA_LOG_INFO(RTPS_READER, guid().entityId << ": changes up to seq_num: " << seq_num << " missing.");

    // Check was not removed from container.
    if (seq_num > changes_from_writer_low_mark_)
    {
        if (seq_num > max_sequence_number_)
        {
            max_sequence_number_ = seq_num;
        }
    }
}

int32_t WriterProxy::lost_changes_update(
        const SequenceNumber_t& seq_num)
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    EPROSIMA_LOG_INFO(RTPS_READER, guid().entityId << ": up to seq_num: " << seq_num);
    int32_t current_sample_lost = 0;

    // Check was not removed from container.
    if (seq_num > (changes_from_writer_low_mark_ + 1))
    {
        // Remove all received changes with a sequence lower than seq_num
        ChangeIterator it = std::lower_bound(changes_received_.begin(), changes_received_.end(), seq_num);
        if (!changes_received_.empty())
        {
            uint64_t tmp = (*changes_received_.begin()).to64long() - (changes_from_writer_low_mark_.to64long() + 1);
            auto distance = std::distance(changes_received_.begin(), it);
            tmp += seq_num.to64long() - (*changes_received_.begin()).to64long() - distance;
            current_sample_lost = tmp > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()) ?
                    std::numeric_limits<int32_t>::max() : static_cast<int32_t>(tmp);
        }
        else
        {
            uint64_t tmp = seq_num.to64long() - (changes_from_writer_low_mark_.to64long() + 1);
            current_sample_lost = tmp > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()) ?
                    std::numeric_limits<int32_t>::max() : static_cast<int32_t>(tmp);
        }
        changes_received_.erase(changes_received_.begin(), it);

        // Update low mark
        changes_from_writer_low_mark_ = seq_num - 1;
        if (changes_from_writer_low_mark_ > max_sequence_number_)
        {
            max_sequence_number_ = changes_from_writer_low_mark_;
        }

        // Next could need to be removed.
        cleanup();
    }

    return current_sample_lost;
}

bool WriterProxy::received_change_set(
        const SequenceNumber_t& seq_num)
{
    EPROSIMA_LOG_INFO(RTPS_READER, guid().entityId << ": seq_num: " << seq_num);
    return received_change_set(seq_num, true);
}

bool WriterProxy::irrelevant_change_set(
        const SequenceNumber_t& seq_num)
{
    return received_change_set(seq_num, false);
}

bool WriterProxy::received_change_set(
        const SequenceNumber_t& seq_num,
        bool /* is_relevance */ )
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    // Check if CacheChange_t was already and it was already removed from changesFromW container.
    if (seq_num <= changes_from_writer_low_mark_)
    {
        EPROSIMA_LOG_INFO(RTPS_READER, "Change " << seq_num << " <= than max available sequence number "
                                                 << changes_from_writer_low_mark_);
        return false;
    }

    // If will be the last element, insert it at the end.
    if (seq_num > max_sequence_number_)
    {
        // If it is the next to be acknowledeg, not insert
        if (seq_num == changes_from_writer_low_mark_ + 1)
        {
            changes_from_writer_low_mark_ = seq_num;
            cleanup();
        }
        else
        {
            changes_received_.insert(changes_received_.end(), seq_num);
        }
        max_sequence_number_ = seq_num;
    }
    else
    {
        // Check if it is next to the last acknowledged
        if (changes_from_writer_low_mark_ + 1 == seq_num)
        {
            changes_from_writer_low_mark_ = seq_num;
            cleanup();
        }
        else
        {
            // Check if already received
            if (changes_received_.find(seq_num) != changes_received_.end())
            {
                return false;
            }

            changes_received_.insert(seq_num);
        }
    }

    return true;
}

SequenceNumberSet_t WriterProxy::missing_changes() const
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    SequenceNumber_t first_missing = changes_from_writer_low_mark_ + 1;
    SequenceNumber_t max_missing = std::min(first_missing + 256UL, max_sequence_number_ + 1);
    SequenceNumberSet_t sns(first_missing);

    for (SequenceNumber_t seq : changes_received_)
    {
        seq = std::min(seq, max_missing);
        sns.add_range(first_missing, seq);
        first_missing = seq + 1;
        if (first_missing >= max_missing)
        {
            break;
        }
    }

    if (first_missing < max_missing)
    {
        sns.add_range(first_missing, max_missing);
    }

    return sns;
}

bool WriterProxy::change_was_received(
        const SequenceNumber_t& seq_num) const
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    if (seq_num <= changes_from_writer_low_mark_)
    {
        return true;
    }

    ChangeIterator chit = changes_received_.find(seq_num);
    return chit != changes_received_.end();
}

const SequenceNumber_t WriterProxy::available_changes_max() const
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    return changes_from_writer_low_mark_;
}

void WriterProxy::cleanup()
{
    ChangeIterator chit = changes_received_.begin();

    // Jump over all consecutive received changes starting on the next to low_mark
    while (chit != changes_received_.end() && *chit == changes_from_writer_low_mark_ + 1)
    {
        chit++;
        changes_from_writer_low_mark_++;
    }

    // Remove all those changes
    changes_received_.erase(changes_received_.begin(), chit);
}

bool WriterProxy::are_there_missing_changes() const
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    return changes_from_writer_low_mark_ < max_sequence_number_;
}

size_t WriterProxy::unknown_missing_changes_up_to(
        const SequenceNumber_t& seq_num) const
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    uint32_t returnedValue = 0;

    if (seq_num > changes_from_writer_low_mark_)
    {
        SequenceNumber_t first_missing = changes_from_writer_low_mark_ + 1;
        SequenceNumberSet_t sns(first_missing);
        SequenceNumberDiff d_fun;

        for (SequenceNumber_t seq : changes_received_)
        {
            seq = std::min(seq, seq_num);
            if (first_missing < seq)
            {
                returnedValue += d_fun(seq, first_missing);
            }
            first_missing = seq + 1;
            if (first_missing >= seq_num)
            {
                break;
            }
        }

        if (first_missing < seq_num)
        {
            returnedValue += d_fun(seq_num, first_missing);
        }
    }

    return returnedValue;
}

size_t WriterProxy::number_of_changes_from_writer() const
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    if (max_sequence_number_ > changes_from_writer_low_mark_)
    {
        SequenceNumberDiff d_fun;
        return d_fun(max_sequence_number_, changes_from_writer_low_mark_);
    }

    return 0;
}

SequenceNumber_t WriterProxy::next_cache_change_to_be_notified()
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    if (last_notified_ < changes_from_writer_low_mark_)
    {
        ++last_notified_;
        return last_notified_;
    }

    return SequenceNumber_t::unknown();
}

void WriterProxy::consider_all_notified()
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    if (last_notified_ < changes_from_writer_low_mark_)
    {
        last_notified_ = changes_from_writer_low_mark_;
    }
}

void WriterProxy::consider_all_notified_up_to(
        const SequenceNumber_t& seq_num)
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    if (seq_num > last_notified_ && seq_num <= changes_from_writer_low_mark_)
    {
        last_notified_ = seq_num;
    }
}

bool WriterProxy::perform_initial_ack_nack()
{
    bool ret_value = false;

    StateCode expected = StateCode::IDLE;
    if (!state_.compare_exchange_strong(expected, StateCode::BUSY))
    {
        // Stopped from another thread -> abort
        return ret_value;
    }

    if (!is_datasharing_writer_)
    {
        // Send initial NACK.
        SequenceNumberSet_t sns(SequenceNumber_t(0, 0));
        if (is_on_same_process_)
        {
            BaseWriter* writer = RTPSDomainImpl::get_instance()->find_writer(guid());
            if (writer)
            {
                bool tmp;
                writer->process_acknack(guid(), reader_->getGuid(), 1,
                        SequenceNumberSet_t(), false, tmp, c_VendorId_eProsima);
            }
        }
        else
        {
            if (0 == last_heartbeat_count_)
            {
                reader_->send_acknack(this, sns, this, false);
                double time_ms = initial_acknack_->getIntervalMilliSec();
                constexpr double max_ms = 60 * 60 * 1000; // Limit to 1 hour
                if (time_ms < max_ms)
                {
                    initial_acknack_->update_interval_millisec(time_ms * 2);
                    ret_value = true;
                }
            }
        }
    }

    expected = StateCode::BUSY;
    state_.compare_exchange_strong(expected, StateCode::IDLE);

    return ret_value;
}

void WriterProxy::perform_heartbeat_response()
{
    StateCode expected = StateCode::IDLE;
    if (!state_.compare_exchange_strong(expected, StateCode::BUSY))
    {
        // Stopped from another thread -> abort
        return;
    }

    reader_->send_acknack(this, this, heartbeat_final_flag_.load());

    expected = StateCode::BUSY;
    state_.compare_exchange_strong(expected, StateCode::IDLE);
}

bool WriterProxy::process_heartbeat(
        uint32_t count,
        const SequenceNumber_t& first_seq,
        const SequenceNumber_t& last_seq,
        bool final_flag,
        bool liveliness_flag,
        bool disable_positive,
        bool& assert_liveliness,
        int32_t& current_sample_lost)
{
#ifdef SHOULD_DEBUG_LINUX
    assert(get_mutex_owner() == get_thread_id());
#endif // SHOULD_DEBUG_LINUX

    assert_liveliness = false;
    if (state_ != StateCode::STOPPED && last_heartbeat_count_ < count)
    {
        // If it is the first heartbeat message, we can try to cancel initial ack.
        // TODO: This timer cancelling should be checked if needed with the liveliness implementation.
        // To keep PARTICIPANT_DROPPED event we should add an explicit participant_liveliness QoS.
        // This is now commented to avoid issues #457 and #155
        // initial_acknack_->cancel_timer();

        last_heartbeat_count_ = count;
        current_sample_lost = lost_changes_update(first_seq);
        missing_changes_update(last_seq);
        heartbeat_final_flag_.store(final_flag);

        //Analyze whether a acknack message is needed:
        if (!is_on_same_process_)
        {
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
            else
            {
                assert_liveliness = liveliness_flag;
            }
        }
        else
        {
            assert_liveliness = liveliness_flag;
        }

        if (!received_at_least_one_heartbeat_)
        {
            current_sample_lost = 0;
            received_at_least_one_heartbeat_ = true;
        }

        return true;
    }

    return false;
}

bool WriterProxy::process_gap(
        const SequenceNumber_t& gap_start,
        const SequenceNumberSet_t& gap_list,
        SequenceNumber_t& validated_start,
        SequenceNumberSet_t& validated_gap_list)
{
    // First sequence number that can be considered for GAP processing
    SequenceNumber_t first_allowed_gap = changes_from_writer_low_mark_ + 1u;

    // Cap start sequence number
    SequenceNumber_t initial_seq = std::max(gap_start, first_allowed_gap);

    // Cap the GAP to the maximum sequence number the writer has announced through a
    // Heartbeat, which is is guaranteed to precede every GAP.
    // If no Heartbeat has been processed yet, max_sequence_number_ equals
    // changes_from_writer_low_mark_, so initial_seq >= max_allowed_gap and nothing is
    // processed (the GAP is ignored until a Heartbeat sets the range).
    SequenceNumber_t max_allowed_gap = max_sequence_number_ + 1u;

    if (initial_seq == first_allowed_gap)
    {
        // Special case for datasharing where no GAP is emitted from the writer's side,
        // but created locally by the reader for initial positioning
        max_allowed_gap = std::max(max_allowed_gap, gap_list.base());
    }

    // Early exit if initial_seq is beyond the allowed range
    if (initial_seq > max_allowed_gap)
    {
        return false;
    }

    // Maximum sequence number to be considered for contiguous GAP processing
    SequenceNumber_t finalSN = std::min(gap_list.base(), max_allowed_gap);
    validated_start = initial_seq;
    validated_gap_list.base(finalSN);

    if ((initial_seq == first_allowed_gap) && (finalSN > initial_seq))
    {
        // Special case when receiving a GAP starting at the next expected sequence number,
        // which means that all changes below the first sequence number in the gap_list are irrelevant.
        // We know that irrelevant_change_set would just increment the low mark, so we can just advance
        // it directly and remove all irrelevant changes in one go.

        // Advance low mark to the last contiguous sequence number before the first sequence number in the gap_list.
        changes_from_writer_low_mark_ = finalSN - 1u;

        // Update max_sequence_number_ if needed, as irrelevant_change_set would do.
        if (changes_from_writer_low_mark_ > max_sequence_number_)
        {
            max_sequence_number_ = changes_from_writer_low_mark_;
        }

        // Remove every already received change up to the first sequence number in the gap_list.
        ChangeIterator first_relevant = std::lower_bound(
            changes_received_.begin(),
            changes_received_.end(),
            finalSN);
        changes_received_.erase(changes_received_.begin(), first_relevant);

        cleanup();
    }
    else
    {
        // Cap the final sequence number to avoid CPU denial-of-service attacks by sending a GAP with a huge range.
        max_allowed_gap = std::min(max_allowed_gap, initial_seq + 512u);
        finalSN = std::min(finalSN, max_allowed_gap);
        // Iterate through all sequence numbers in [initial_seq, final_seq)
        SequenceNumber_t auxSN;
        for (auxSN = initial_seq; auxSN < finalSN; auxSN++)
        {
            irrelevant_change_set(auxSN);
        }
    }

    // Early exit if the entire gap_list is beyond the announced range
    if (gap_list.base() > max_allowed_gap)
    {
        return true;
    }

    // Iterate through all sequence numbers in the gap_list
    gap_list.for_each(
        [&](SequenceNumber_t it)
        {
            if ((it < max_allowed_gap) && irrelevant_change_set(it))
            {
                validated_gap_list.add(it);
            }
        });

    return true;
}

void WriterProxy::update_heartbeat_response_interval(
        const dds::Duration_t& interval)
{
    heartbeat_response_->update_interval(interval);
}

bool WriterProxy::send(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        std::chrono::steady_clock::time_point max_blocking_time_point)
{
    if (is_on_same_process_)
    {
        return true;
    }

    const ResourceLimitedVector<Locator_t>& remote_locators = remote_locators_shrinked();

    return reader_->send_sync_nts(buffers,
                   total_bytes,
                   Locators(remote_locators.begin()),
                   Locators(remote_locators.end()),
                   max_blocking_time_point);
}

#ifdef SHOULD_DEBUG_LINUX
int WriterProxy::get_mutex_owner() const
{
    auto mutex = reader_->getMutex().native_handle();
    return mutex->__data.__owner;
}

int WriterProxy::get_thread_id() const
{
    return syscall(__NR_gettid);
}

#endif // SHOULD_DEBUG_LINUX

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
