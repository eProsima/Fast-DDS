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
 * @file StatefulWriter.cpp
 *
 */

#include "StatefulWriter.hpp"

#include <mutex>
#include <stdexcept>
#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <rtps/messages/RTPSMessageCreator.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/DataSharing/DataSharingNotifier.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <rtps/DataSharing/WriterPool.hpp>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/messages/RTPSGapBuilder.hpp>
#include <rtps/messages/RTPSMessageGroup.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/LocalReaderPointer.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <rtps/resources/TimedEvent.h>
#include <rtps/RTPSDomainImpl.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/ReaderProxy.hpp>
#include <utils/TimeConversion.hpp>

#ifdef FASTDDS_STATISTICS
#include <statistics/types/monitorservice_types.hpp>
#endif // ifdef FASTDDS_STATISTICS

#include "../builtin/discovery/database/DiscoveryDataBase.hpp"

#include "../flowcontrol/FlowController.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Loops over all the readers in the vector, applying the given routine.
 * The loop continues until the result of the routine is true for any reader
 * or all readers have been processed.
 * The returned value is true if the routine returned true at any point,
 * or false otherwise.
 */
template<typename T>
static bool for_matched_readers(
        ResourceLimitedVector<ReaderProxy*>& reader_vector_1,
        T fun)
{
    for (ReaderProxy* remote_reader : reader_vector_1)
    {
        if (fun(remote_reader))
        {
            return true;
        }
    }

    return false;
}

template<typename T>
static bool for_matched_readers(
        ResourceLimitedVector<ReaderProxy*>& reader_vector_1,
        ResourceLimitedVector<ReaderProxy*>& reader_vector_2,
        T fun)
{
    if (for_matched_readers(reader_vector_1, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_2, fun);
}

template<typename T>
static bool for_matched_readers(
        ResourceLimitedVector<ReaderProxy*>& reader_vector_1,
        ResourceLimitedVector<ReaderProxy*>& reader_vector_2,
        ResourceLimitedVector<ReaderProxy*>& reader_vector_3,
        T fun)
{
    if (for_matched_readers(reader_vector_1, reader_vector_2, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_3, fun);
}

/**
 * Loops over all the readers in the vector, applying the given routine.
 * The loop continues until the result of the routine is true for any reader
 * or all readers have been processes.
 * The returned value is true if the routine returned true at any point,
 * or false otherwise.
 *
 * const version
 */
template<typename T>
static bool for_matched_readers(
        const ResourceLimitedVector<ReaderProxy*>& reader_vector_1,
        T fun)
{
    for (const ReaderProxy* remote_reader : reader_vector_1)
    {
        if (fun(remote_reader))
        {
            return true;
        }
    }

    return false;
}

template<typename T>
static bool for_matched_readers(
        const ResourceLimitedVector<ReaderProxy*>& reader_vector_1,
        const ResourceLimitedVector<ReaderProxy*>& reader_vector_2,
        T fun)
{
    if (for_matched_readers(reader_vector_1, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_2, fun);
}

template<typename T>
static bool for_matched_readers(
        const ResourceLimitedVector<ReaderProxy*>& reader_vector_1,
        const ResourceLimitedVector<ReaderProxy*>& reader_vector_2,
        const ResourceLimitedVector<ReaderProxy*>& reader_vector_3,
        T fun)
{
    if (for_matched_readers(reader_vector_1, reader_vector_2, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_3, fun);
}

using namespace std::chrono;

StatefulWriter::StatefulWriter(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const WriterAttributes& att,
        FlowController* flow_controller,
        WriterHistory* history,
        WriterListener* listener)
    : BaseWriter(pimpl, guid, att, flow_controller, history, listener)
    , periodic_hb_event_(nullptr)
    , nack_response_event_(nullptr)
    , ack_event_(nullptr)
    , heartbeat_count_(0)
    , times_(att.times)
    , matched_remote_readers_(att.matched_readers_allocation)
    , matched_readers_pool_(att.matched_readers_allocation)
    , next_all_acked_notify_sequence_(0, 1)
    , all_acked_(false)
    , may_remove_change_cond_()
    , may_remove_change_(0)
    , disable_heartbeat_piggyback_(att.disable_heartbeat_piggyback)
    , disable_positive_acks_(att.disable_positive_acks)
    , keep_duration_(att.keep_duration)
    , last_sequence_number_()
    , biggest_removed_sequence_number_()
    , sendBufferSize_(pimpl->get_min_network_send_buffer_size())
    , currentUsageSendBufferSize_(static_cast<int32_t>(pimpl->get_min_network_send_buffer_size()))
    , matched_local_readers_(att.matched_readers_allocation)
    , matched_datasharing_readers_(att.matched_readers_allocation)
    , locator_selector_general_(*this, att.matched_readers_allocation)
    , locator_selector_async_(*this, att.matched_readers_allocation)
{
    init(pimpl, att);
}

void StatefulWriter::init(
        RTPSParticipantImpl* pimpl,
        const WriterAttributes& att)
{
    const RTPSParticipantAttributes& part_att = pimpl->get_attributes();

    auto push_mode = PropertyPolicyHelper::find_property(att.endpoint.properties, "fastdds.push_mode");
    push_mode_ = !((nullptr != push_mode) && ("false" == *push_mode));

    periodic_hb_event_ = new TimedEvent(
        pimpl->getEventResource(),
        [&]() -> bool
        {
            return send_periodic_heartbeat();
        },
        fastdds::rtps::TimeConv::Time_t2MilliSecondsDouble(times_.heartbeat_period));

    nack_response_event_ = new TimedEvent(
        pimpl->getEventResource(),
        [&]() -> bool
        {
            perform_nack_response();
            return false;
        },
        fastdds::rtps::TimeConv::Time_t2MilliSecondsDouble(times_.nack_response_delay));

    if (disable_positive_acks_)
    {
        ack_event_ = new TimedEvent(
            pimpl->getEventResource(),
            [&]() -> bool
            {
                return ack_timer_expired();
            },
            att.keep_duration.to_ns() * 1e-6);             // in milliseconds
    }

    for (size_t n = 0; n < att.matched_readers_allocation.initial; ++n)
    {
        matched_readers_pool_.push_back(new ReaderProxy(times_, part_att.allocation.locators, this));
    }
}

StatefulWriter::~StatefulWriter()
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "StatefulWriter destructor");
}

void StatefulWriter::local_actions_on_writer_removed()
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "StatefulWriter local_actions_on_writer_removed");

    // Disable timed events, because their callbacks use cache changes
    if (disable_positive_acks_)
    {
        delete(ack_event_);
        ack_event_ = nullptr;
    }

    if (nack_response_event_ != nullptr)
    {
        delete(nack_response_event_);
        nack_response_event_ = nullptr;
    }

    // This must be the next action, as it frees CacheChange_t from the async thread.
    BaseWriter::local_actions_on_writer_removed();

    // Stop all active proxies and pass them to the pool
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        while (!matched_remote_readers_.empty())
        {
            ReaderProxy* remote_reader = matched_remote_readers_.back();
            matched_remote_readers_.pop_back();
            remote_reader->stop();
            matched_readers_pool_.push_back(remote_reader);
        }
        while (!matched_local_readers_.empty())
        {
            ReaderProxy* remote_reader = matched_local_readers_.back();
            matched_local_readers_.pop_back();
            remote_reader->stop();
            matched_readers_pool_.push_back(remote_reader);
        }
        while (!matched_datasharing_readers_.empty())
        {
            ReaderProxy* remote_reader = matched_datasharing_readers_.back();
            matched_datasharing_readers_.pop_back();
            remote_reader->stop();
            matched_readers_pool_.push_back(remote_reader);
        }
    }

    // PeriodicHeartbeatEvent must be released after releasing all proxies
    // because proxy's NackSuppressionEvent could restart this event.
    if (periodic_hb_event_ != nullptr)
    {
        delete(periodic_hb_event_);
        periodic_hb_event_ = nullptr;
    }

    // Delete all proxies in the pool
    for (ReaderProxy* remote_reader : matched_readers_pool_)
    {
        delete(remote_reader);
    }
}

/*
 * CHANGE-RELATED METHODS
 */
void StatefulWriter::prepare_datasharing_delivery(
        CacheChange_t* change)
{
    auto pool = std::dynamic_pointer_cast<WriterPool>(history_->get_payload_pool());
    assert (pool != nullptr);

    pool->add_to_shared_history(change);
    EPROSIMA_LOG_INFO(RTPS_WRITER, "Notifying readers of cache change with SN " << change->sequenceNumber);
}

void StatefulWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    auto payload_length = change->serializedPayload.length;

    if (liveliness_lease_duration_ < dds::c_TimeInfinite)
    {
        mp_RTPSParticipant->wlp()->assert_liveliness(
            getGuid(),
            liveliness_kind_,
            liveliness_lease_duration_);
    }

    // Prepare the metadata for datasharing
    if (is_datasharing_compatible())
    {
        prepare_datasharing_delivery(change);
    }

    // Now for the rest of readers
    if (!matched_remote_readers_.empty() || !matched_datasharing_readers_.empty() || !matched_local_readers_.empty())
    {
        bool should_be_sent = false;
        for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                [this, &should_be_sent, &change, &max_blocking_time](ReaderProxy* reader)
                {
                    ChangeForReader_t changeForReader(change);
                    bool is_relevant = reader->rtps_is_relevant(change);

                    if (push_mode_ || !reader->is_reliable() || reader->is_local_reader())
                    {
                        //ChangeForReader_t construct sets status to UNSENT.
                        should_be_sent |= is_relevant;
                    }
                    else
                    {
                        changeForReader.setStatus(UNACKNOWLEDGED);
                    }
                    reader->add_change(changeForReader, is_relevant, false, max_blocking_time);

                    return false;
                }
                );

        if (disable_positive_acks_)
        {
            Time_t expiration_ts = change->sourceTimestamp + keep_duration_;
            Time_t current_ts;
            Time_t::now(current_ts);
            assert(expiration_ts >= current_ts);
            auto interval = (expiration_ts - current_ts).to_duration_t();

            ack_event_->update_interval(interval);
            ack_event_->restart_timer(max_blocking_time);
        }

        // After adding the CacheChange_t to flowcontroller, its pointer cannot be used because it may be removed
        // internally before exiting the call. For example if the writer matched with a best-effort reader.
        if (should_be_sent)
        {
            flow_controller_->add_new_sample(this, change, max_blocking_time);
        }
        else
        {
            periodic_hb_event_->restart_timer(max_blocking_time);
        }
    }
    else
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER, "No reader proxy to add change.");
        check_acked_status();
    }

    // Throughput should be notified even if no matches are available
    on_publish_throughput(payload_length);
}

bool StatefulWriter::intraprocess_delivery(
        CacheChange_t* change,
        ReaderProxy* reader_proxy)
{
    LocalReaderPointer::Instance local_reader = reader_proxy->local_reader();
    if (local_reader)
    {
        if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
        {
            change->write_params.sample_identity(change->write_params.related_sample_identity());
        }
        return local_reader->process_data_msg(change);
    }
    return false;
}

bool StatefulWriter::intraprocess_gap(
        ReaderProxy* reader_proxy,
        const SequenceNumber_t& first_seq,
        const SequenceNumber_t& last_seq)
{
    LocalReaderPointer::Instance local_reader = reader_proxy->local_reader();
    if (local_reader)
    {
        return local_reader->process_gap_msg(
            m_guid, first_seq, SequenceNumberSet_t(last_seq), c_VendorId_eProsima);
    }

    return false;
}

bool StatefulWriter::intraprocess_heartbeat(
        ReaderProxy* reader_proxy,
        bool liveliness)
{
    bool returned_value = false;
    LocalReaderPointer::Instance local_reader = reader_proxy->local_reader();

    if (local_reader)
    {
        std::unique_lock<RecursiveTimedMutex> lockW(mp_mutex);
        SequenceNumber_t first_seq = get_seq_num_min();
        SequenceNumber_t last_seq = get_seq_num_max();

        if (first_seq == c_SequenceNumber_Unknown || last_seq == c_SequenceNumber_Unknown)
        {
            if (liveliness)
            {
                first_seq = next_sequence_number();
                last_seq = first_seq - 1;
            }
        }

        if ((first_seq != c_SequenceNumber_Unknown && last_seq != c_SequenceNumber_Unknown) &&
                (liveliness || reader_proxy->has_changes()))
        {
            increment_hb_count();
            Count_t hb_count = heartbeat_count_;
            lockW.unlock();
            returned_value = local_reader->process_heartbeat_msg(
                m_guid, hb_count, first_seq, last_seq, true, liveliness, c_VendorId_eProsima);
        }
    }

    return returned_value;
}

bool StatefulWriter::change_removed_by_history(
        CacheChange_t* a_change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    bool ret_value = false;
    SequenceNumber_t sequence_number = a_change->sequenceNumber;

    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    EPROSIMA_LOG_INFO(RTPS_WRITER, "Change " << sequence_number << " to be removed.");

    if (flow_controller_->remove_change(a_change, max_blocking_time))
    {

        // Take note of biggest removed sequence number to improve sending of gaps
        if (sequence_number > biggest_removed_sequence_number_)
        {
            biggest_removed_sequence_number_ = sequence_number;
        }

        // Invalidate CacheChange pointer in ReaderProxies.
        for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                [sequence_number](ReaderProxy* reader)
                {
                    reader->change_has_been_removed(sequence_number);
                    return false;
                }
                );

        // remove from datasharing pool history
        if (is_datasharing_compatible())
        {
            auto pool = std::dynamic_pointer_cast<WriterPool>(history_->get_payload_pool());
            assert (pool != nullptr);

            pool->remove_from_shared_history(a_change);
            EPROSIMA_LOG_INFO(RTPS_WRITER, "Removing shared cache change with SN " << a_change->sequenceNumber);
        }

        may_remove_change_ = 2;
        may_remove_change_cond_.notify_one();

        ret_value = true;
    }

    return ret_value;
}

void StatefulWriter::send_heartbeat_to_all_readers()
{
    // This method is only called from send_periodic_heartbeat

    if (separate_sending_enabled_)
    {
        for (ReaderProxy* reader : matched_remote_readers_)
        {
            send_heartbeat_to_nts(*reader);
        }
    }
    else
    {
        for (ReaderProxy* reader : matched_local_readers_)
        {
            intraprocess_heartbeat(reader);
        }

        for (ReaderProxy* reader : matched_datasharing_readers_)
        {
            reader->datasharing_notify();
        }

        if (there_are_remote_readers_)
        {
            RTPSMessageGroup group(mp_RTPSParticipant, this, &locator_selector_general_);
            select_all_readers_nts(group, locator_selector_general_);

            assert(
                (SequenceNumber_t::unknown() == get_seq_num_min() && SequenceNumber_t::unknown() == get_seq_num_max()) ||
                (SequenceNumber_t::unknown() != get_seq_num_min() &&
                SequenceNumber_t::unknown() != get_seq_num_max()));

            add_gaps_for_holes_in_history(group);

            send_heartbeat_nts_(locator_selector_general_.all_remote_readers.size(), group, disable_positive_acks_);
        }
    }
}

void StatefulWriter::deliver_sample_to_intraprocesses(
        CacheChange_t* change)
{
    for (ReaderProxy* remoteReader : matched_local_readers_)
    {
        SequenceNumber_t gap_seq;
        FragmentNumber_t dummy = 0;
        bool dumb = false;
        if (remoteReader->change_is_unsent(change->sequenceNumber, dummy, gap_seq, get_seq_num_min(), dumb))
        {
            // If there is a hole (removed from history or not relevants) between previous sample and this one,
            // send it a personal GAP.
            if (SequenceNumber_t::unknown() != gap_seq)
            {
                intraprocess_gap(remoteReader, gap_seq, change->sequenceNumber);
                remoteReader->acked_changes_set(change->sequenceNumber);
            }
            bool delivered = intraprocess_delivery(change, remoteReader);
            if (!remoteReader->is_reliable())
            {
                remoteReader->acked_changes_set(change->sequenceNumber + 1);
            }
            else
            {
                intraprocess_heartbeat(remoteReader, false);
                remoteReader->from_unsent_to_status(
                    change->sequenceNumber,
                    delivered ? ACKNOWLEDGED : UNACKNOWLEDGED,
                    false,
                    delivered);
            }
        }
    }
}

void StatefulWriter::deliver_sample_to_datasharing(
        CacheChange_t* change)
{
    for (ReaderProxy* remoteReader : matched_datasharing_readers_)
    {
        SequenceNumber_t gap_seq;
        FragmentNumber_t dummy = 0;
        bool dumb = false;
        if (remoteReader->change_is_unsent(change->sequenceNumber, dummy, gap_seq, get_seq_num_min(), dumb))
        {
            if (!remoteReader->is_reliable())
            {
                remoteReader->acked_changes_set(change->sequenceNumber + 1);
            }
            else
            {
                remoteReader->from_unsent_to_status(
                    change->sequenceNumber,
                    UNACKNOWLEDGED,
                    false);
            }
            remoteReader->datasharing_notify();
        }
    }
}

DeliveryRetCode StatefulWriter::deliver_sample_to_network(
        CacheChange_t* change,
        RTPSMessageGroup& group,
        LocatorSelectorSender& locator_selector, // Object locked by FlowControllerImpl
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    NetworkFactory& network = mp_RTPSParticipant->network_factory();
    DeliveryRetCode ret_code = DeliveryRetCode::DELIVERED;
    uint32_t n_fragments = change->getFragmentCount();
    FragmentNumber_t min_unsent_fragment = 0;
    bool need_reactivate_periodic_heartbeat = false;

    while (DeliveryRetCode::DELIVERED == ret_code &&
            min_unsent_fragment != n_fragments + 1)
    {
        SequenceNumber_t gap_seq_for_all = SequenceNumber_t::unknown();
        locator_selector.locator_selector.reset(false);
        auto first_relevant_reader = matched_remote_readers_.begin();
        bool inline_qos = false;
        bool should_be_sent = false;
        min_unsent_fragment = n_fragments + 1;

        for (auto remote_reader = first_relevant_reader; remote_reader != matched_remote_readers_.end();
                ++remote_reader)
        {
            SequenceNumber_t gap_seq;
            FragmentNumber_t next_unsent_frag = 0;
            if ((*remote_reader)->change_is_unsent(change->sequenceNumber, next_unsent_frag, gap_seq, get_seq_num_min(),
                    need_reactivate_periodic_heartbeat) &&
                    (0 == n_fragments || min_unsent_fragment >= next_unsent_frag))
            {
                if (min_unsent_fragment > next_unsent_frag)
                {
                    locator_selector.locator_selector.reset(false);
                    first_relevant_reader = remote_reader;
                    min_unsent_fragment = next_unsent_frag;
                }

                (*remote_reader)->active(true);
                locator_selector.locator_selector.enable((*remote_reader)->guid());
                should_be_sent = true;
                inline_qos |= (*remote_reader)->expects_inline_qos();

                // If there is a hole (removed from history or not relevants) between previous sample and this one,
                // send it a personal GAP.
                if (SequenceNumber_t::unknown() != gap_seq)
                {
                    if (SequenceNumber_t::unknown() == gap_seq_for_all)     // Calculate if the hole is for all readers
                    {
                        History::const_iterator chit = history_->find_change_nts(change);

                        if (chit == history_->changesBegin())
                        {
                            gap_seq_for_all = gap_seq;
                        }
                        else
                        {
                            SequenceNumber_t prev = (*std::prev(chit))->sequenceNumber + 1;

                            if (prev == gap_seq)
                            {
                                gap_seq_for_all = gap_seq;
                            }
                        }
                    }

                    if (gap_seq_for_all != gap_seq)     // If it is an individual GAP, sent it to repective reader.
                    {
                        group.sender(this, (*remote_reader)->message_sender());
                        group.add_gap(gap_seq, SequenceNumberSet_t(change->sequenceNumber),
                                (*remote_reader)->guid());
                        send_heartbeat_nts_(1u, group, disable_positive_acks_);
                        group.sender(this, &locator_selector);     // This makes the flush_and_reset().
                    }
                }
            }
            else
            {
                (*remote_reader)->active(false);
            }
        }

        bool should_send_global_gap = SequenceNumber_t::unknown() != gap_seq_for_all;

        if (locator_selector.locator_selector.state_has_changed() &&
                ((should_be_sent && !separate_sending_enabled_) || should_send_global_gap))
        {
            group.flush_and_reset();
            network.select_locators(locator_selector.locator_selector);
            compute_selected_guids(locator_selector);
        }

        if (should_send_global_gap) // Send GAP for all readers
        {
            group.add_gap(gap_seq_for_all, SequenceNumberSet_t(change->sequenceNumber));
        }

        try
        {
            if (should_be_sent)
            {
                uint32_t last_processed = 0;
                if (!separate_sending_enabled_)
                {
                    size_t num_locators = locator_selector.locator_selector.selected_size();
                    if (num_locators > 0)
                    {
                        if (0 < n_fragments)
                        {
                            if (min_unsent_fragment != n_fragments + 1)
                            {
                                if (group.add_data_frag(*change, min_unsent_fragment, inline_qos))
                                {
                                    for (auto remote_reader = first_relevant_reader;
                                            remote_reader != matched_remote_readers_.end();
                                            ++remote_reader)
                                    {
                                        if ((*remote_reader)->active())
                                        {
                                            bool allFragmentsSent = false;
                                            (*remote_reader)->mark_fragment_as_sent_for_change(
                                                change->sequenceNumber,
                                                min_unsent_fragment,
                                                allFragmentsSent);

                                            if (allFragmentsSent)
                                            {
                                                if (!(*remote_reader)->is_reliable())
                                                {
                                                    (*remote_reader)->acked_changes_set(change->sequenceNumber + 1);
                                                }
                                                else
                                                {
                                                    (*remote_reader)->from_unsent_to_status(
                                                        change->sequenceNumber,
                                                        UNDERWAY,
                                                        true);
                                                }
                                            }
                                        }
                                    }
                                    add_statistics_sent_submessage(change, num_locators);
                                }
                                else
                                {
                                    ret_code = DeliveryRetCode::NOT_DELIVERED;
                                }
                            }
                        }
                        else
                        {
                            if (group.add_data(*change, inline_qos))
                            {
                                for (auto remote_reader = first_relevant_reader;
                                        remote_reader != matched_remote_readers_.end();
                                        ++remote_reader)
                                {
                                    if ((*remote_reader)->active())
                                    {
                                        if (!(*remote_reader)->is_reliable())
                                        {
                                            (*remote_reader)->acked_changes_set(change->sequenceNumber + 1);
                                        }
                                        else
                                        {
                                            (*remote_reader)->from_unsent_to_status(
                                                change->sequenceNumber,
                                                UNDERWAY,
                                                true);
                                        }
                                    }
                                }
                                add_statistics_sent_submessage(change, num_locators);
                            }
                            else
                            {
                                ret_code = DeliveryRetCode::NOT_DELIVERED;
                            }
                        }

                        send_heartbeat_piggyback_nts_(group, locator_selector, last_processed);
                    }
                }
                else
                {
                    for (auto remote_reader = first_relevant_reader;
                            remote_reader != matched_remote_readers_.end();
                            ++remote_reader)
                    {
                        if ((*remote_reader)->active())
                        {
                            group.sender(this, (*remote_reader)->message_sender());

                            if (0 < n_fragments)
                            {
                                if (min_unsent_fragment != n_fragments + 1)
                                {
                                    if (group.add_data_frag(*change, min_unsent_fragment, inline_qos))
                                    {
                                        bool allFragmentsSent = false;
                                        (*remote_reader)->mark_fragment_as_sent_for_change(
                                            change->sequenceNumber,
                                            min_unsent_fragment,
                                            allFragmentsSent);

                                        if (allFragmentsSent)
                                        {
                                            if (!(*remote_reader)->is_reliable())
                                            {
                                                (*remote_reader)->acked_changes_set(change->sequenceNumber + 1);
                                            }
                                            else
                                            {
                                                (*remote_reader)->from_unsent_to_status(
                                                    change->sequenceNumber,
                                                    UNDERWAY,
                                                    true);
                                            }
                                        }
                                        add_statistics_sent_submessage(change, (*remote_reader)->locators_size());
                                    }
                                    else
                                    {
                                        ret_code = DeliveryRetCode::NOT_DELIVERED;
                                    }
                                }
                            }
                            else
                            {
                                if (group.add_data(*change, (*remote_reader)->expects_inline_qos()))
                                {
                                    if (!(*remote_reader)->is_reliable())
                                    {
                                        (*remote_reader)->acked_changes_set(change->sequenceNumber + 1);
                                    }
                                    else
                                    {
                                        (*remote_reader)->from_unsent_to_status(
                                            change->sequenceNumber,
                                            UNDERWAY,
                                            true);
                                    }
                                    add_statistics_sent_submessage(change, (*remote_reader)->locators_size());
                                }
                                else
                                {
                                    EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                                    ret_code = DeliveryRetCode::NOT_DELIVERED;
                                }
                            }

                            send_heartbeat_nts_(1u, group, false);
                        }
                    }
                }

                on_sample_datas(change->write_params.sample_identity(), change->writer_info.num_sent_submessages);
                on_data_sent();
            }
        }
        catch (const RTPSMessageGroup::timeout&)
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Max blocking time reached");
            ret_code = DeliveryRetCode::NOT_DELIVERED;
        }
        catch (const RTPSMessageGroup::limit_exceeded&)
        {
            ret_code = DeliveryRetCode::EXCEEDED_LIMIT;
        }

        if (disable_positive_acks_ && last_sequence_number_ == SequenceNumber_t())
        {
            last_sequence_number_ = change->sequenceNumber;
            if ( !(ack_event_->getRemainingTimeMilliSec() > 0))
            {
                // Restart ack_timer
                Time_t expiration_ts = change->sourceTimestamp + keep_duration_;
                Time_t current_ts;
                Time_t::now(current_ts);
                assert(expiration_ts >= current_ts);
                auto interval = (expiration_ts - current_ts).to_duration_t();

                ack_event_->update_interval(interval);
                ack_event_->restart_timer(max_blocking_time);
            }
        }

        // Restore in case a exception was launched by RTPSMessageGroup.
        group.sender(this, &locator_selector);

    }

    if (need_reactivate_periodic_heartbeat)
    {
        periodic_hb_event_->restart_timer(max_blocking_time);
    }

    return ret_code;
}

/*
 * MATCHED_READER-RELATED METHODS
 */
void StatefulWriter::update_reader_info(
        LocatorSelectorSender& locator_selector,
        bool create_sender_resources)
{
    update_cached_info_nts(locator_selector);
    compute_selected_guids(locator_selector);

    if (create_sender_resources)
    {
        RTPSParticipantImpl* part = get_participant_impl();
        locator_selector.locator_selector.for_each([part](const Locator_t& loc)
                {
                    part->createSenderResources(loc);
                });
    }

    // Check if we have local or remote readers
    there_are_remote_readers_ = !matched_remote_readers_.empty();
    there_are_local_readers_ = !matched_local_readers_.empty();
    there_are_datasharing_readers_ = !matched_datasharing_readers_.empty();
}

void StatefulWriter::select_all_readers_nts(
        RTPSMessageGroup& group,
        LocatorSelectorSender& locator_selector)
{
    locator_selector.locator_selector.reset(true);
    if (locator_selector.locator_selector.state_has_changed())
    {
        group.flush_and_reset();
        mp_RTPSParticipant->network_factory().select_locators(locator_selector.locator_selector);
        compute_selected_guids(locator_selector);
    }
}

bool StatefulWriter::matched_reader_add_edp(
        const ReaderProxyData& rdata)
{
    using network::external_locators::filter_remote_locators;

    if (rdata.guid == c_Guid_Unknown)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Reliable Writer need GUID_t of matched readers");
        return false;
    }

    std::unique_lock<RecursiveTimedMutex> guard(mp_mutex);
    std::unique_lock<LocatorSelectorSender> guard_locator_selector_general(locator_selector_general_);
    std::unique_lock<LocatorSelectorSender> guard_locator_selector_async(locator_selector_async_);

    // Check if it is already matched.
    if (for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
            [this, &rdata](ReaderProxy* reader)
            {
                if (reader->guid() == rdata.guid)
                {
                    EPROSIMA_LOG_INFO(RTPS_WRITER, "Attempting to add existing reader, updating information.");
                    if (reader->update(rdata))
                    {
                        filter_remote_locators(*reader->general_locator_selector_entry(),
                        m_att.external_unicast_locators, m_att.ignore_non_matching_locators);
                        filter_remote_locators(*reader->async_locator_selector_entry(),
                        m_att.external_unicast_locators, m_att.ignore_non_matching_locators);
                        mp_RTPSParticipant->createSenderResources(rdata.remote_locators, m_att);
                        update_reader_info(locator_selector_general_, true);
                        update_reader_info(locator_selector_async_, true);
                    }
                    return true;
                }
                return false;
            }))
    {
        if (nullptr != listener_)
        {
            // call the listener without locks taken
            guard_locator_selector_async.unlock();
            guard_locator_selector_general.unlock();
            guard.unlock();

            listener_->on_reader_discovery(this, ReaderDiscoveryStatus::CHANGED_QOS_READER, rdata.guid, &rdata);
        }

#ifdef FASTDDS_STATISTICS
        // notify monitor service so that the connectionlist for this entity
        // could be updated
        if (nullptr != mp_RTPSParticipant->get_connections_observer() && !m_guid.is_builtin())
        {
            mp_RTPSParticipant->get_connections_observer()->on_local_entity_connections_change(m_guid);
        }
#endif //FASTDDS_STATISTICS

        return false;
    }

    // Get a reader proxy from the inactive pool (or create a new one if necessary and allowed)
    ReaderProxy* rp = nullptr;
    if (matched_readers_pool_.empty())
    {
        size_t max_readers = matched_readers_pool_.max_size();
        if (get_matched_readers_size() + matched_readers_pool_.size() < max_readers)
        {
            const RTPSParticipantAttributes& part_att = mp_RTPSParticipant->get_attributes();
            rp = new ReaderProxy(times_, part_att.allocation.locators, this);
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_WRITER, "Maximum number of reader proxies (" << max_readers <<
                    ") reached for writer " << m_guid);
            return false;
        }
    }
    else
    {
        rp = matched_readers_pool_.back();
        matched_readers_pool_.pop_back();
    }

    // Add info of new datareader.
    rp->start(rdata, is_datasharing_compatible_with(rdata.data_sharing));
    filter_remote_locators(*rp->general_locator_selector_entry(),
            m_att.external_unicast_locators, m_att.ignore_non_matching_locators);
    filter_remote_locators(*rp->async_locator_selector_entry(),
            m_att.external_unicast_locators, m_att.ignore_non_matching_locators);
    locator_selector_general_.locator_selector.add_entry(rp->general_locator_selector_entry());
    locator_selector_async_.locator_selector.add_entry(rp->async_locator_selector_entry());

    if (rp->is_local_reader())
    {
        matched_local_readers_.push_back(rp);
        EPROSIMA_LOG_INFO(RTPS_WRITER, "Adding reader " << rdata.guid << " to " << this->m_guid.entityId
                                                        << " as local reader");
    }
    else
    {
        if (rp->is_datasharing_reader())
        {
            matched_datasharing_readers_.push_back(rp);
            EPROSIMA_LOG_INFO(RTPS_WRITER, "Adding reader " << rdata.guid << " to " << this->m_guid.entityId
                                                            << " as data sharing");
        }
        else
        {
            matched_remote_readers_.push_back(rp);
            EPROSIMA_LOG_INFO(RTPS_WRITER, "Adding reader " << rdata.guid << " to " << this->m_guid.entityId
                                                            << " as remote reader");
        }
    }

    mp_RTPSParticipant->createSenderResources(rdata.remote_locators, m_att);
    update_reader_info(locator_selector_general_, true);
    update_reader_info(locator_selector_async_, true);

    if (rp->is_datasharing_reader())
    {
        if (nullptr != listener_)
        {
            // call the listener without locks taken
            guard_locator_selector_async.unlock();
            guard_locator_selector_general.unlock();
            guard.unlock();

            listener_->on_reader_discovery(this, ReaderDiscoveryStatus::DISCOVERED_READER, rdata.guid, &rdata);
        }

#ifdef FASTDDS_STATISTICS
        // notify monitor service so that the connectionlist for this entity
        // could be updated
        if (nullptr != mp_RTPSParticipant->get_connections_observer() && !m_guid.is_builtin())
        {
            mp_RTPSParticipant->get_connections_observer()->on_local_entity_connections_change(m_guid);
        }
#endif //FASTDDS_STATISTICS

        return true;
    }

    bool is_reliable = rp->is_reliable();
    if (is_reliable)
    {
        SequenceNumber_t min_seq = get_seq_num_min();
        SequenceNumber_t last_seq = get_seq_num_max();
        RTPSMessageGroup group(mp_RTPSParticipant, this, rp->message_sender());

        // History not empty
        if (min_seq != SequenceNumber_t::unknown())
        {
            (void)last_seq;
            assert(last_seq != SequenceNumber_t::unknown());
            assert(min_seq <= last_seq);

            try
            {
                // Late-joiner
                if (TRANSIENT_LOCAL <= rp->durability_kind() &&
                        TRANSIENT_LOCAL <= m_att.durabilityKind)
                {
                    for (History::iterator cit = history_->changesBegin(); cit != history_->changesEnd(); ++cit)
                    {
                        // Holes are managed when deliver_sample(), sending GAP messages.
                        if (rp->rtps_is_relevant(*cit))
                        {
                            ChangeForReader_t changeForReader(*cit);

                            // If it is local, maintain in UNSENT status and add to flow controller.
                            if (rp->is_local_reader())
                            {
                                flow_controller_->add_old_sample(this, *cit);
                            }
                            // In other case, set as UNACKNOWLEDGED and expects the reader request them.
                            else
                            {
                                changeForReader.setStatus(UNACKNOWLEDGED);
                            }

                            rp->add_change(changeForReader, true, false);
                        }
                    }
                }
                else
                {
                    if (rp->is_local_reader())
                    {
                        intraprocess_gap(rp, min_seq, history_->next_sequence_number());
                    }
                    else
                    {
                        // Send a GAP of the whole history.
                        group.add_gap(min_seq, SequenceNumberSet_t(history_->next_sequence_number()), rp->guid());
                    }
                }

                // Always activate heartbeat period. We need a confirmation of the reader.
                // The state has to be updated.
                periodic_hb_event_->restart_timer(std::chrono::steady_clock::now() + std::chrono::hours(24));
            }
            catch (const RTPSMessageGroup::timeout&)
            {
                EPROSIMA_LOG_ERROR(RTPS_WRITER, "Max blocking time reached");
            }
        }

        if (rp->is_local_reader())
        {
            intraprocess_heartbeat(rp);
        }
        else
        {
            send_heartbeat_nts_(1u, group, disable_positive_acks_);
            group.flush_and_reset();
        }
    }
    else
    {
        // Acknowledged all for best-effort reader.
        rp->acked_changes_set(history_->next_sequence_number());
    }

    EPROSIMA_LOG_INFO(RTPS_WRITER, "Reader Proxy " << rp->guid() << " added to " << this->m_guid.entityId << " with "
                                                   << rdata.remote_locators.unicast.size() << "(u)-"
                                                   << rdata.remote_locators.multicast.size() <<
            "(m) locators");

    if (nullptr != listener_)
    {
        // call the listener without locks taken
        guard_locator_selector_async.unlock();
        guard_locator_selector_general.unlock();
        guard.unlock();

        listener_->on_reader_discovery(this, ReaderDiscoveryStatus::DISCOVERED_READER, rdata.guid, &rdata);
    }

#ifdef FASTDDS_STATISTICS
    // notify monitor service so that the connectionlist for this entity
    // could be updated
    if (nullptr != mp_RTPSParticipant->get_connections_observer() && !m_guid.is_builtin())
    {
        mp_RTPSParticipant->get_connections_observer()->on_local_entity_connections_change(m_guid);
    }
#endif //FASTDDS_STATISTICS

    return true;
}

bool StatefulWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    ReaderProxy* rproxy = nullptr;
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    {
        std::lock_guard<LocatorSelectorSender> guard_locator_selector_general(locator_selector_general_);
        std::lock_guard<LocatorSelectorSender> guard_locator_selector_async(locator_selector_async_);

        for (ReaderProxyIterator it = matched_local_readers_.begin();
                it != matched_local_readers_.end(); ++it)
        {
            if ((*it)->guid() == reader_guid)
            {
                EPROSIMA_LOG_INFO(RTPS_WRITER, "Reader Proxy removed: " << reader_guid);
                rproxy = std::move(*it);
                it = matched_local_readers_.erase(it);
                break;
            }
        }

        if (rproxy == nullptr)
        {
            for (ReaderProxyIterator it = matched_datasharing_readers_.begin();
                    it != matched_datasharing_readers_.end(); ++it)
            {
                if ((*it)->guid() == reader_guid)
                {
                    EPROSIMA_LOG_INFO(RTPS_WRITER, "Reader Proxy removed: " << reader_guid);
                    rproxy = std::move(*it);
                    it = matched_datasharing_readers_.erase(it);
                    break;
                }
            }
        }

        if (rproxy == nullptr)
        {
            for (ReaderProxyIterator it = matched_remote_readers_.begin();
                    it != matched_remote_readers_.end(); ++it)
            {
                if ((*it)->guid() == reader_guid)
                {
                    EPROSIMA_LOG_INFO(RTPS_WRITER, "Reader Proxy removed: " << reader_guid);
                    rproxy = std::move(*it);
                    it = matched_remote_readers_.erase(it);
                    break;
                }
            }
        }

        locator_selector_general_.locator_selector.remove_entry(reader_guid);
        locator_selector_async_.locator_selector.remove_entry(reader_guid);
        update_reader_info(locator_selector_general_, false);
        update_reader_info(locator_selector_async_, false);
    }

    if (get_matched_readers_size() == 0)
    {
        periodic_hb_event_->cancel_timer();
    }

    if (rproxy != nullptr)
    {
        rproxy->stop();
        matched_readers_pool_.push_back(rproxy);

        check_acked_status();

        if (nullptr != listener_)
        {
            // listener is called without locks taken
            lock.unlock();
            listener_->on_reader_discovery(this, ReaderDiscoveryStatus::REMOVED_READER, reader_guid, nullptr);
        }

#ifdef FASTDDS_STATISTICS
        // notify monitor service so that the connectionlist for this entity
        // could be updated
        if (nullptr != mp_RTPSParticipant->get_connections_observer() && !m_guid.is_builtin())
        {
            mp_RTPSParticipant->get_connections_observer()->on_local_entity_connections_change(m_guid);
        }
#endif //FASTDDS_STATISTICS

        return true;
    }

    EPROSIMA_LOG_INFO(RTPS_HISTORY, "Reader Proxy doesn't exist in this writer");
    return false;
}

bool StatefulWriter::matched_reader_is_matched(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                   [&reader_guid](ReaderProxy* reader)
                   {
                       return (reader->guid() == reader_guid);
                   }
                   );
}

bool StatefulWriter::matched_reader_lookup(
        GUID_t& readerGuid,
        ReaderProxy** RP)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                   [&readerGuid, RP](ReaderProxy* reader)
                   {
                       if (reader->guid() == readerGuid)
                       {
                           *RP = reader;
                           return true;
                       }
                       return false;
                   }
                   );
}

bool StatefulWriter::has_been_fully_delivered(
        const SequenceNumber_t& seq_num) const
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    bool found = false;
    // Sequence number has not been generated by this WriterHistory.
    if (seq_num >= history_->next_sequence_number())
    {
        return false;
    }
    for (auto reader : matched_remote_readers_)
    {
        bool ret_code = reader->has_been_delivered(seq_num, found);
        if (found && !ret_code)
        {
            // The change has not been fully delivered if it is pending delivery on at least one ReaderProxy.
            return false;
        }
    }
    return true;
}

bool StatefulWriter::is_acked_by_all(
        const SequenceNumber_t& seq_num) const
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return is_acked_by_all_nts(seq_num);
}

bool StatefulWriter::is_acked_by_all_nts(
        const SequenceNumber_t seq) const
{
    assert(history_->next_sequence_number() > seq);
    return (seq < next_all_acked_notify_sequence_) ||
           !for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                   [seq](const ReaderProxy* reader)
                   {
                       return !(reader->change_is_acked(seq));
                   });
}

bool StatefulWriter::all_readers_updated()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    return !for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                   [](const ReaderProxy* reader)
                   {
                       return (reader->has_changes());
                   }
                   );
}

bool StatefulWriter::wait_for_all_acked(
        const dds::Duration_t& max_wait)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    std::unique_lock<std::mutex> all_acked_lock(all_acked_mutex_);

    all_acked_ = !for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                    [](const ReaderProxy* reader)
                    {
                        return reader->has_changes();
                    }
                    );
    lock.unlock();

    if (!all_acked_)
    {
        std::chrono::microseconds max_w(fastdds::rtps::TimeConv::Duration_t2MicroSecondsInt64(max_wait));
        all_acked_cond_.wait_for(all_acked_lock, max_w, [&]()
                {
                    return all_acked_;
                });
    }

    return all_acked_;
}

void StatefulWriter::rebuild_status_after_load()
{
    SequenceNumber_t min_seq = get_seq_num_min();
    if (min_seq != SequenceNumber_t::unknown())
    {
        biggest_removed_sequence_number_ = min_seq - 1;
        may_remove_change_ = 1;
    }

    SequenceNumber_t next_seq = history_->next_sequence_number();
    next_all_acked_notify_sequence_ = next_seq;
    min_readers_low_mark_ = next_seq - 1;
    all_acked_ = true;
}

void StatefulWriter::check_acked_status()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    bool all_acked = true;
    bool has_min_low_mark = false;
    // #8945 If no readers matched, notify all old changes.
    SequenceNumber_t min_low_mark = history_->next_sequence_number() - 1;

    for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
            [&all_acked, &has_min_low_mark, &min_low_mark](ReaderProxy* reader)
            {
                SequenceNumber_t reader_low_mark = reader->changes_low_mark();
                if (reader_low_mark < min_low_mark || !has_min_low_mark)
                {
                    has_min_low_mark = true;
                    min_low_mark = reader_low_mark;
                }

                if (reader->has_changes())
                {
                    all_acked = false;
                }

                return false;
            }
            );

    bool something_changed = all_acked;
    SequenceNumber_t min_seq = get_seq_num_min();
    if (min_seq != SequenceNumber_t::unknown())
    {
        // In the case where we haven't received an acknack from a recently matched reader,
        // min_low_mark will be zero, and no change will be notified as received by all
        if (next_all_acked_notify_sequence_ <= min_low_mark)
        {
            if ((listener_ != nullptr) && (min_low_mark >= get_seq_num_min()))
            {
                // We will inform backwards about the changes received by all readers, starting
                // on min_low_mark down until next_all_acked_notify_sequence_. This way we can
                // safely proceed with the traversal, in case a change is removed from the history
                // inside the callback
                History::iterator history_end = history_->changesEnd();
                History::iterator cit =
                        std::lower_bound(history_->changesBegin(), history_end, min_low_mark,
                                [](
                                    const CacheChange_t* change,
                                    const SequenceNumber_t& seq)
                                {
                                    return change->sequenceNumber < seq;
                                });
                if (cit != history_end && (*cit)->sequenceNumber == min_low_mark)
                {
                    ++cit;
                }

                SequenceNumber_t seq{};
                SequenceNumber_t end_seq = min_seq > next_all_acked_notify_sequence_ ?
                        min_seq : next_all_acked_notify_sequence_;

                // The iterator starts pointing to the change inmediately after min_low_mark
                --cit;

                do
                {
                    // Avoid notifying changes before next_all_acked_notify_sequence_
                    CacheChange_t* change = *cit;
                    seq = change->sequenceNumber;
                    if (seq < next_all_acked_notify_sequence_)
                    {
                        break;
                    }

                    // Change iterator before it possibly becomes invalidated
                    if (cit != history_->changesBegin())
                    {
                        --cit;
                    }

                    // Notify reception of change (may remove that change on VOLATILE writers)
                    listener_->on_writer_change_received_by_all(this, change);

                    // Stop if we got to either next_all_acked_notify_sequence_ or the first change
                } while (seq > end_seq);
            }

            next_all_acked_notify_sequence_ = min_low_mark + 1;
        }

        if (min_low_mark >= get_seq_num_min())
        {
            // get_seq_num_min() returns SequenceNumber_t::unknown() when the history is empty.
            // Thus, it is set to 2 to indicate that all samples have been removed.
            may_remove_change_ = (get_seq_num_min() == SequenceNumber_t::unknown()) ? 2 : 1;
        }

        min_readers_low_mark_ = min_low_mark;
        something_changed = true;
    }

    if (all_acked)
    {
        std::unique_lock<std::mutex> all_acked_lock(all_acked_mutex_);
        SequenceNumber_t next_seq = history_->next_sequence_number();
        next_all_acked_notify_sequence_ = next_seq;
        min_readers_low_mark_ = next_seq - 1;
        all_acked_ = true;
        all_acked_cond_.notify_all();
    }

    if (something_changed)
    {
        may_remove_change_cond_.notify_one();
    }
}

bool StatefulWriter::try_remove_change(
        const std::chrono::steady_clock::time_point& max_blocking_time_point,
        std::unique_lock<RecursiveTimedMutex>& lock)
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "Starting process try remove change for writer " << getGuid());

    SequenceNumber_t min_low_mark;

    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        min_low_mark = next_all_acked_notify_sequence_ - 1;
    }

    SequenceNumber_t calc = min_low_mark < get_seq_num_min() ? SequenceNumber_t() :
            (min_low_mark - get_seq_num_min()) + 1;
    unsigned int may_remove_change = 1;

    if (calc <= SequenceNumber_t())
    {
        may_remove_change_ = 0;
        may_remove_change_cond_.wait_until(lock, max_blocking_time_point,
                [&]()
                {
                    return may_remove_change_ > 0;
                });
        may_remove_change = may_remove_change_;
    }

    // Some changes acked
    if (may_remove_change == 1)
    {
        return history_->remove_min_change();
    }
    // Waiting a change was removed.
    else if (may_remove_change == 2)
    {
        return true;
    }

    return false;
}

bool StatefulWriter::wait_for_acknowledgement(
        const SequenceNumber_t& seq,
        const std::chrono::steady_clock::time_point& max_blocking_time_point,
        std::unique_lock<RecursiveTimedMutex>& lock)
{
    return may_remove_change_cond_.wait_until(lock, max_blocking_time_point,
                   [this, &seq]()
                   {
                       return is_acked_by_all_nts(seq);
                   });
}

/*
 * PARAMETER_RELATED METHODS
 */
void StatefulWriter::update_attributes(
        const WriterAttributes& att)
{
    this->update_times(att.times);
    if (this->get_disable_positive_acks())
    {
        this->update_positive_acks_times(att);
    }
}

bool StatefulWriter::matched_readers_guids(
        std::vector<GUID_t>& guids) const
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    guids.clear();
    guids.reserve(matched_local_readers_.size() + matched_datasharing_readers_.size() +
            matched_remote_readers_.size());
    for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
            [&guids](const ReaderProxy* reader)
            {
                guids.emplace_back(reader->guid());
                return false;
            }
            );

    return true;
}

void StatefulWriter::update_positive_acks_times(
        const WriterAttributes& att)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    keep_duration_ = att.keep_duration;
    // Restart ack timer with new duration
    ack_event_->update_interval(keep_duration_);
    ack_event_->restart_timer();
}

void StatefulWriter::update_times(
        const WriterTimes& times)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (times_.heartbeat_period != times.heartbeat_period)
    {
        periodic_hb_event_->update_interval(times.heartbeat_period);
    }
    if (times_.nack_response_delay != times.nack_response_delay)
    {
        if (nack_response_event_ != nullptr)
        {
            nack_response_event_->update_interval(times.nack_response_delay);
        }
    }
    if (times_.nack_supression_duration != times.nack_supression_duration)
    {
        for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                [&times](ReaderProxy* reader)
                {
                    reader->update_nack_supression_interval(times.nack_supression_duration);
                    return false;
                }
                );

        for (ReaderProxy* it : matched_readers_pool_)
        {
            it->update_nack_supression_interval(times.nack_supression_duration);
        }
    }
    times_ = times;
}

SequenceNumber_t StatefulWriter::next_sequence_number() const
{
    return history_->next_sequence_number();
}

bool StatefulWriter::send_periodic_heartbeat(
        bool final,
        bool liveliness)
{
    std::lock_guard<RecursiveTimedMutex> guardW(mp_mutex);
    std::lock_guard<LocatorSelectorSender> guard_locator_selector_general(locator_selector_general_);

    bool unacked_changes = false;
    if (!liveliness)
    {
        SequenceNumber_t first_seq_to_check_acknowledge = get_seq_num_min();
        if (SequenceNumber_t::unknown() == first_seq_to_check_acknowledge)
        {
            first_seq_to_check_acknowledge = history_->next_sequence_number() - 1;
        }

        unacked_changes = for_matched_readers(matched_local_readers_, matched_datasharing_readers_,
                        matched_remote_readers_,
                        [first_seq_to_check_acknowledge](ReaderProxy* reader)
                        {
                            return reader->has_unacknowledged(first_seq_to_check_acknowledge);
                        }
                        );

        if (unacked_changes)
        {
            try
            {
                //TODO if separating, here sends periodic for all readers, instead of ones needed it.
                send_heartbeat_to_all_readers();
            }
            catch (const RTPSMessageGroup::timeout&)
            {
                EPROSIMA_LOG_ERROR(RTPS_WRITER, "Max blocking time reached");
            }
        }
    }
    else if (separate_sending_enabled_)
    {
        // Send individual liveliness heartbeat to each reader
        for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                [this, &liveliness, &unacked_changes](ReaderProxy* reader)
                {
                    send_heartbeat_to_nts(*reader, liveliness);
                    unacked_changes = true;
                    return false;
                }
                );
    }
    else
    {
        // This is a liveliness heartbeat, we don't care about checking sequence numbers
        try
        {
            for (ReaderProxy* reader : matched_local_readers_)
            {
                intraprocess_heartbeat(reader, true);
                unacked_changes = true;
            }

            for (ReaderProxy* reader : matched_datasharing_readers_)
            {
                auto pool = std::dynamic_pointer_cast<WriterPool>(history_->get_payload_pool());
                assert(pool);
                pool->assert_liveliness();
                reader->datasharing_notify();
                unacked_changes = true;
            }

            if (there_are_remote_readers_)
            {
                unacked_changes = true;
                RTPSMessageGroup group(mp_RTPSParticipant, this, &locator_selector_general_);
                send_heartbeat_nts_(locator_selector_general_.all_remote_readers.size(), group, final, liveliness);
            }
        }
        catch (const RTPSMessageGroup::timeout&)
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Max blocking time reached");
        }
    }

    return unacked_changes;
}

void StatefulWriter::send_heartbeat_to_nts(
        ReaderProxy& remoteReaderProxy,
        bool liveliness,
        bool force /* = false */)
{
    SequenceNumber_t first_seq_to_check_acknowledge = get_seq_num_min();
    if (SequenceNumber_t::unknown() == first_seq_to_check_acknowledge)
    {
        first_seq_to_check_acknowledge = history_->next_sequence_number() - 1;
    }
    if (remoteReaderProxy.is_reliable() &&
            (force || liveliness || remoteReaderProxy.has_unacknowledged(first_seq_to_check_acknowledge)))
    {
        if (remoteReaderProxy.is_local_reader())
        {
            intraprocess_heartbeat(&remoteReaderProxy, liveliness);
        }
        else if (remoteReaderProxy.is_datasharing_reader())
        {
            remoteReaderProxy.datasharing_notify();
        }
        else
        {
            try
            {
                RTPSMessageGroup group(mp_RTPSParticipant, this, remoteReaderProxy.message_sender());
                SequenceNumber_t firstSeq = get_seq_num_min();
                SequenceNumber_t lastSeq = get_seq_num_max();

                if (firstSeq != c_SequenceNumber_Unknown && lastSeq != c_SequenceNumber_Unknown)
                {
                    assert(firstSeq <= lastSeq);
                    if (!liveliness)
                    {
                        add_gaps_for_holes_in_history(group);
                    }
                }

                send_heartbeat_nts_(1u, group, disable_positive_acks_, liveliness);
            }
            catch (const RTPSMessageGroup::timeout&)
            {
                EPROSIMA_LOG_ERROR(RTPS_WRITER, "Max blocking time reached");
            }
        }
    }
}

void StatefulWriter::send_heartbeat_nts_(
        size_t number_of_readers,
        RTPSMessageGroup& message_group,
        bool final,
        bool liveliness)
{
    if (!number_of_readers)
    {
        return;
    }

    SequenceNumber_t firstSeq = get_seq_num_min();
    SequenceNumber_t lastSeq = get_seq_num_max();

    if (firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
    {
        assert(firstSeq == c_SequenceNumber_Unknown && lastSeq == c_SequenceNumber_Unknown);

        if (number_of_readers == 1 || liveliness)
        {
            firstSeq = next_sequence_number();
            lastSeq = firstSeq - 1;
        }
        else
        {
            return;
        }
    }
    else
    {
        assert(firstSeq <= lastSeq);
    }

    increment_hb_count();
    message_group.add_heartbeat(firstSeq, lastSeq, heartbeat_count_, final, liveliness);
    // Update calculate of heartbeat piggyback.
    currentUsageSendBufferSize_ = static_cast<int32_t>(sendBufferSize_);

    EPROSIMA_LOG_INFO(RTPS_WRITER,
            getGuid().entityId << " Sending Heartbeat (" << firstSeq << " - " << lastSeq << ")" );
}

void StatefulWriter::send_heartbeat_piggyback_nts_(
        RTPSMessageGroup& message_group,
        LocatorSelectorSender& locator_selector,
        uint32_t& last_bytes_processed)
{
    if (!disable_heartbeat_piggyback_)
    {
        if (history_->isFull() || next_all_acked_notify_sequence_ < get_seq_num_min())
        {
            select_all_readers_nts(message_group, locator_selector);
            size_t number_of_readers = locator_selector.all_remote_readers.size();
            send_heartbeat_nts_(number_of_readers, message_group, disable_positive_acks_);
        }
        else
        {
            uint32_t current_bytes = message_group.get_current_bytes_processed();
            currentUsageSendBufferSize_ -= current_bytes - last_bytes_processed;
            last_bytes_processed = current_bytes;
            if (currentUsageSendBufferSize_ < 0)
            {
                select_all_readers_nts(message_group, locator_selector);
                size_t number_of_readers = locator_selector.all_remote_readers.size();
                send_heartbeat_nts_(number_of_readers, message_group, disable_positive_acks_);
            }
        }
    }
}

void StatefulWriter::perform_nack_response()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    uint32_t changes_to_resend = 0;
    for (ReaderProxy* reader : matched_remote_readers_)
    {
        changes_to_resend += reader->perform_acknack_response([&](ChangeForReader_t& change)
                        {
                            // This labmda is called if the ChangeForReader_t pass from REQUESTED to UNSENT.
                            assert(nullptr != change.getChange());
                            flow_controller_->add_old_sample(this, change.getChange());
                        }
                        );
    }

    lock.unlock();

    // Notify the statistics module
    on_resent_data(changes_to_resend);
}

void StatefulWriter::perform_nack_supression(
        const GUID_t& reader_guid)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
            [this, &reader_guid](ReaderProxy* reader)
            {
                if (reader->guid() == reader_guid)
                {
                    reader->perform_nack_supression();
                    periodic_hb_event_->restart_timer();
                    return true;
                }
                return false;
            }
            );
}

bool StatefulWriter::process_acknack(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        uint32_t ack_count,
        const SequenceNumberSet_t& sn_set,
        bool final_flag,
        bool& result,
        fastdds::rtps::VendorId_t /*origin_vendor_id*/)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    result = (m_guid == writer_guid);

    if (result)
    {
        SequenceNumber_t received_sequence_number = sn_set.empty() ? sn_set.base() : sn_set.max();
        if (received_sequence_number <= next_sequence_number())
        {
            for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                    [&](ReaderProxy* remote_reader)
                    {
                        if (remote_reader->guid() == reader_guid)
                        {
                            if (remote_reader->check_and_set_acknack_count(ack_count))
                            {
                                // Sequence numbers before Base are set as Acknowledged.
                                remote_reader->acked_changes_set(sn_set.base());
                                if (sn_set.base() > SequenceNumber_t(0, 0))
                                {
                                    // Prepare GAP for requested  samples that are not in history or are irrelevants.
                                    RTPSMessageGroup group(mp_RTPSParticipant, this, remote_reader->message_sender());
                                    RTPSGapBuilder gap_builder(group);

                                    if (remote_reader->requested_changes_set(sn_set, gap_builder, get_seq_num_min()))
                                    {
                                        nack_response_event_->restart_timer();
                                    }
                                    else if (!final_flag)
                                    {
                                        periodic_hb_event_->restart_timer();
                                    }

                                    gap_builder.flush();
                                }
                                else if (sn_set.empty() && !final_flag)
                                {
                                    // This is the preemptive acknack.
                                    if (remote_reader->process_initial_acknack([&](ChangeForReader_t& change_reader)
                                    {
                                        assert(nullptr != change_reader.getChange());
                                        flow_controller_->add_old_sample(this, change_reader.getChange());
                                    }))
                                    {
                                        if (remote_reader->is_remote_and_reliable())
                                        {
                                            // Send heartbeat if requested
                                            send_heartbeat_to_nts(*remote_reader, false, true);
                                            periodic_hb_event_->restart_timer();
                                        }
                                    }

                                    if (remote_reader->is_local_reader() && !remote_reader->is_datasharing_reader())
                                    {
                                        intraprocess_heartbeat(remote_reader);
                                    }
                                }

                                // Check if all CacheChange are acknowledge, because a user could be waiting
                                // for this, or some CacheChanges could be removed if we are VOLATILE
                                check_acked_status();
                            }
                            return true;
                        }

                        return false;
                    }
                    );
        }
        else
        {
            print_inconsistent_acknack(writer_guid, reader_guid, sn_set.base(), received_sequence_number,
                    next_sequence_number());
        }
    }

    return result;
}

bool StatefulWriter::process_nack_frag(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        uint32_t ack_count,
        const SequenceNumber_t& seq_num,
        const FragmentNumberSet_t& fragments_state,
        bool& result,
        fastdds::rtps::VendorId_t /*origin_vendor_id*/)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    result = false;
    if (m_guid == writer_guid)
    {
        result = true;
        for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                [this, &reader_guid, &ack_count, &seq_num, &fragments_state](ReaderProxy* reader)
                {
                    if (reader->guid() == reader_guid)
                    {
                        if (reader->process_nack_frag(reader_guid, ack_count, seq_num, fragments_state))
                        {
                            nack_response_event_->restart_timer();
                        }
                        return true;
                    }
                    return false;
                }
                );
    }

    return result;
}

bool StatefulWriter::ack_timer_expired()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    // The timer has expired so the earliest non-acked change must be marked as acknowledged
    // This will be done in the first while iteration, as we start with a negative interval

    Time_t expiration_ts;
    Time_t current_ts;
    Time_t::now(current_ts);

    // On the other hand, we've seen in the tests that if samples are sent very quickly with little
    // time between consecutive samples, the timer interval could end up being negative
    // In this case, we keep marking changes as acknowledged until the timer is able to keep up, hence the while
    // loop

    do
    {
        bool acks_flag = false;
        for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                [this, &acks_flag](ReaderProxy* reader)
                {
                    if (reader->disable_positive_acks())
                    {
                        reader->acked_changes_set(last_sequence_number_ + 1);
                        acks_flag = true;
                    }
                    return false;
                }
                );
        if (acks_flag)
        {
            check_acked_status();
        }

        CacheChange_t* change;

        // Skip removed changes until reaching the last change
        do
        {
            last_sequence_number_++;
        } while (!history_->get_change(
            last_sequence_number_,
            getGuid(),
            &change) && last_sequence_number_ < next_sequence_number());

        if (!history_->get_change(
                    last_sequence_number_,
                    getGuid(),
                    &change))
        {
            // Stop ack_timer
            return false;
        }

        Time_t::now(current_ts);
        expiration_ts = change->sourceTimestamp + keep_duration_;
    }
    while (expiration_ts < current_ts);

    auto interval = (expiration_ts - current_ts).to_duration_t();
    ack_event_->update_interval(interval);
    return true;
}

void StatefulWriter::print_inconsistent_acknack(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        const SequenceNumber_t& min_requested_sequence_number,
        const SequenceNumber_t& max_requested_sequence_number,
        const SequenceNumber_t& next_sequence_number)
{
    EPROSIMA_LOG_WARNING(RTPS_WRITER, "Inconsistent acknack received. Local Writer "
            << writer_guid << " next SequenceNumber " << next_sequence_number << ". Remote Reader "
            << reader_guid << " requested range is  [" << min_requested_sequence_number
            << ", " << max_requested_sequence_number << "].");
    // This is necessary to avoid Warning of unused variable in case warning log level is disable
    static_cast<void>(writer_guid);
    static_cast<void>(reader_guid);
    static_cast<void>(min_requested_sequence_number);
    static_cast<void>(max_requested_sequence_number);
    static_cast<void>(next_sequence_number);
}

void StatefulWriter::reader_data_filter(
        fastdds::rtps::IReaderDataFilter* reader_data_filter)
{
    reader_data_filter_ = reader_data_filter;
}

const fastdds::rtps::IReaderDataFilter* StatefulWriter::reader_data_filter() const
{
    return reader_data_filter_;
}

DeliveryRetCode StatefulWriter::deliver_sample_nts(
        CacheChange_t* cache_change,
        RTPSMessageGroup& group,
        LocatorSelectorSender& locator_selector, // Object locked by FlowControllerImpl
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    DeliveryRetCode ret_code = DeliveryRetCode::DELIVERED;

    if (there_are_local_readers_)
    {
        deliver_sample_to_intraprocesses(cache_change);
    }

    // Process datasharing then
    if (there_are_datasharing_readers_)
    {
        deliver_sample_to_datasharing(cache_change);
    }

    if (there_are_remote_readers_)
    {
        ret_code = deliver_sample_to_network(cache_change, group, locator_selector, max_blocking_time);
    }

    check_acked_status();

    return ret_code;
}

#ifdef FASTDDS_STATISTICS

bool StatefulWriter::get_connections(
        fastdds::statistics::rtps::ConnectionList& connection_list)
{
    connection_list.reserve(matched_local_readers_.size() +
            matched_datasharing_readers_.size() +
            matched_remote_readers_.size());

    fastdds::statistics::Connection connection;

    {
        std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

        //! intraprocess
        for_matched_readers(matched_local_readers_, [&connection, &connection_list](ReaderProxy*& reader)
                {
                    connection.guid(fastdds::statistics::to_statistics_type(reader->guid()));
                    connection.mode(fastdds::statistics::ConnectionMode::INTRAPROCESS);
                    connection_list.push_back(connection);

                    return false;
                });
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

        //! datasharing
        for_matched_readers(matched_datasharing_readers_, [&connection, &connection_list](ReaderProxy*& reader)
                {
                    connection.guid(fastdds::statistics::to_statistics_type(reader->guid()));
                    connection.mode(fastdds::statistics::ConnectionMode::DATA_SHARING);
                    connection_list.push_back(connection);

                    return false;
                });
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

        //! remote
        for_matched_readers(matched_remote_readers_, [&connection, &connection_list](ReaderProxy*& reader)
                {
                    //! Announced locators is, for the moment,
                    //! equal to the used_locators
                    LocatorSelectorEntry* loc_selector_entry = reader->general_locator_selector_entry();

                    connection.announced_locators().reserve(reader->locators_size());
                    connection.used_locators().reserve(reader->locators_size());

                    std::vector<fastdds::statistics::detail::Locator_s> statistics_locators;
                    std::for_each(loc_selector_entry->multicast.begin(), loc_selector_entry->multicast.end(),
                    [&statistics_locators](const Locator_t& locator)
                    {
                        statistics_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

                    std::for_each(loc_selector_entry->unicast.begin(), loc_selector_entry->unicast.end(),
                    [&statistics_locators](const Locator_t& locator)
                    {
                        statistics_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

                    connection.guid(fastdds::statistics::to_statistics_type(reader->guid()));
                    connection.mode(fastdds::statistics::ConnectionMode::TRANSPORT);
                    connection.announced_locators(statistics_locators);
                    connection.used_locators(statistics_locators);
                    connection_list.push_back(connection);

                    return false;
                });
    }

    return true;

}

#endif // ifdef FASTDDS_STATISTICS

void StatefulWriter::add_gaps_for_holes_in_history(
        RTPSMessageGroup& group)
{
    SequenceNumber_t firstSeq = get_seq_num_min();
    SequenceNumber_t lastSeq = get_seq_num_max();

    if (SequenceNumber_t::unknown() != firstSeq &&
            lastSeq.to64long() - firstSeq.to64long() + 1 != history_->getHistorySize())
    {
        RTPSGapBuilder gaps(group);
        // There are holes in the history.
        History::const_iterator cit = history_->changesBegin();
        SequenceNumber_t prev = (*cit)->sequenceNumber + 1;
        ++cit;
        while (cit != history_->changesEnd())
        {
            while (prev != (*cit)->sequenceNumber)
            {
                gaps.add(prev);
                ++prev;
            }
            ++prev;
            ++cit;
        }
        gaps.flush();
    }
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
