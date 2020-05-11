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

#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/writer/WriterListener.h>
#include <fastdds/rtps/writer/ReaderProxy.h>
#include <fastdds/rtps/resources/AsyncWriterThread.h>

#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/flowcontrol/FlowController.h>

#include <fastdds/rtps/messages/RTPSMessageCreator.h>
#include <fastdds/rtps/messages/RTPSMessageGroup.h>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <fastdds/rtps/history/WriterHistory.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/TimeConversion.h>

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <rtps/writer/RTPSWriterCollector.h>
#include "rtps/RTPSDomainImpl.hpp"
#include "../messages/RTPSGapBuilder.hpp"

#include <mutex>
#include <vector>
#include <stdexcept>

namespace eprosima {
namespace fastrtps {
namespace rtps {

template<typename UnaryFun>
bool send_data_or_fragments(
        RTPSMessageGroup& group,
        CacheChange_t* change,
        bool inline_qos,
        UnaryFun sent_fun)
{
    bool sent_ok = true;

    if (change->getFragmentSize() > 0)
    {
        for (FragmentNumber_t frag = 1; frag <= change->getFragmentCount(); frag++)
        {
            sent_ok &= group.add_data_frag(*change, frag, inline_qos);
            if (sent_ok)
            {
                sent_fun(frag);
            }
            else
            {
                logError(RTPS_WRITER, "Error sending fragment ("
                    << change->sequenceNumber << ", " << frag << ")");
                break;
            }
        }
    }
    else
    {
        sent_ok = group.add_data(*change, inline_qos);
        if (sent_ok)
        {
            sent_fun(0);
        }
        else
        {
            logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
        }
    }

    return sent_ok;
}
    
static void null_sent_fun(
        FragmentNumber_t /*frag*/)
{
}

using namespace std::chrono;

StatefulWriter::StatefulWriter(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const WriterAttributes& att,
        WriterHistory* hist,
        WriterListener* listen)
    : RTPSWriter(pimpl, guid, att, hist, listen)
    , periodic_hb_event_(nullptr)
    , nack_response_event_(nullptr)
    , ack_event_(nullptr)
    , m_heartbeatCount(0)
    , m_times(att.times)
    , matched_readers_(att.matched_readers_allocation)
    , matched_readers_pool_(att.matched_readers_allocation)
    , next_all_acked_notify_sequence_(0, 1)
    , all_acked_(false)
    , may_remove_change_cond_()
    , may_remove_change_(0)
    , disable_heartbeat_piggyback_(att.disable_heartbeat_piggyback)
    , disable_positive_acks_(att.disable_positive_acks)
    , keep_duration_us_(att.keep_duration.to_ns() * 1e-3)
    , last_sequence_number_()
    , biggest_removed_sequence_number_()
    , sendBufferSize_(pimpl->get_min_network_send_buffer_size())
    , currentUsageSendBufferSize_(static_cast<int32_t>(pimpl->get_min_network_send_buffer_size()))
    , m_controllers()
{
    m_heartbeatCount = 0;

    const RTPSParticipantAttributes& part_att = pimpl->getRTPSParticipantAttributes();

    periodic_hb_event_ = new TimedEvent(pimpl->getEventResource(), [&]() -> bool
            {
                return send_periodic_heartbeat();
            },
            TimeConv::Time_t2MilliSecondsDouble(m_times.heartbeatPeriod));

    nack_response_event_ = new TimedEvent(pimpl->getEventResource(), [&]() -> bool
            {
                perform_nack_response();
                return false;
            },
            TimeConv::Time_t2MilliSecondsDouble(m_times.nackResponseDelay));

    if (disable_positive_acks_)
    {
        ack_event_ = new TimedEvent(pimpl->getEventResource(), [&]() -> bool
                {
                        return ack_timer_expired();
                },
                att.keep_duration.to_ns() * 1e-6); // in milliseconds
    }

    for (size_t n = 0; n < att.matched_readers_allocation.initial; ++n)
    {
        matched_readers_pool_.push_back(new ReaderProxy(m_times, part_att.allocation.locators, this));
    }
}

StatefulWriter::~StatefulWriter()
{
    logInfo(RTPS_WRITER, "StatefulWriter destructor");

    for (std::unique_ptr<FlowController>& controller : m_controllers)
    {
        controller->disable();
    }

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

    mp_RTPSParticipant->async_thread().unregister_writer(this);

    // After unregistering writer from AsyncWriterThread, delete all flow_controllers because they register the writer in
    // the AsyncWriterThread.
    m_controllers.clear();

    // Stop all active proxies and pass them to the pool
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        while (!matched_readers_.empty())
        {
            ReaderProxy* remote_reader = matched_readers_.back();
            matched_readers_.pop_back();
            remote_reader->stop();
            matched_readers_pool_.push_back(remote_reader);
        }
    }

    // Destroy heartbeat event
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

void StatefulWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (liveliness_lease_duration_ < c_TimeInfinite)
    {
        mp_RTPSParticipant->wlp()->assert_liveliness(
            getGuid(),
            liveliness_kind_,
            liveliness_lease_duration_);
    }

#if HAVE_SECURITY
    encrypt_cachechange(change);
#endif

    if (!matched_readers_.empty())
    {
        if (!isAsync())
        {
            //TODO(Ricardo) Temporal.
            bool expectsInlineQos = false;

            // First step is to add the new CacheChange_t to all reader proxies.
            // It has to be done before sending, because if a timeout is catched, we will not include the
            // CacheChange_t in some reader proxies.
            for (ReaderProxy* it : matched_readers_)
            {
                ChangeForReader_t changeForReader(change);

                if (m_pushMode)
                {
                    if (it->is_reliable())
                    {
                        changeForReader.setStatus(UNDERWAY);
                    }
                    else
                    {
                        changeForReader.setStatus(ACKNOWLEDGED);
                    }
                }
                else
                {
                    changeForReader.setStatus(UNACKNOWLEDGED);
                }

                changeForReader.setRelevance(it->rtps_is_relevant(change));
                it->add_change(changeForReader, true, max_blocking_time);
                expectsInlineQos |= it->expects_inline_qos();
            }

            try
            {
                //At this point we are sure all information was stored. We now can send data.
                if (!m_separateSendingEnabled)
                {
                    if (locator_selector_.selected_size() > 0)
                    {
                        RTPSMessageGroup group(mp_RTPSParticipant, this, *this, max_blocking_time);

                        auto sent_fun = [this, change](
                                FragmentNumber_t frag)
                        {
                            if (frag > 0)
                            {
                                for (ReaderProxy* it : matched_readers_)
                                {
                                    if (!it->is_local_reader())
                                    {
                                        bool allFragmentsSent = false;
                                        it->mark_fragment_as_sent_for_change(
                                            change->sequenceNumber,
                                            frag,
                                            allFragmentsSent);
                                    }
                                }
                            }
                        };

                        send_data_or_fragments(group, change, expectsInlineQos, sent_fun);
                        send_heartbeat_nts_(all_remote_readers_.size(), group, disable_positive_acks_);
                    }

                    for (ReaderProxy* it : matched_readers_)
                    {
                        if (it->is_local_reader())
                        {
                            intraprocess_heartbeat(it, false);
                            bool delivered = intraprocess_delivery(change, it);
                            it->set_change_to_status(
                                change->sequenceNumber,
                                delivered ? ACKNOWLEDGED : UNDERWAY,
                                false);
                        }
                    }
                }
                else
                {
                    for (ReaderProxy* it : matched_readers_)
                    {
                        if (it->is_local_reader())
                        {
                            intraprocess_heartbeat(it, false);
                            bool delivered = intraprocess_delivery(change, it);
                            it->set_change_to_status(
                                change->sequenceNumber,
                                delivered ? ACKNOWLEDGED : UNDERWAY,
                                false);
                        }
                        else
                        {
                            RTPSMessageGroup group(mp_RTPSParticipant, this, it->message_sender(),
                                    max_blocking_time);

                            if (change->getFragmentCount() > 0)
                            {
                                logError(RTPS_WRITER, "Cannot send large messages on separate sending mode");
                            }
                            else
                            {
                                if (!group.add_data(*change, it->expects_inline_qos()))
                                {
                                    logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                                }
                            }
                            uint32_t last_processed = 0;
                            send_heartbeat_piggyback_nts_(it, group, last_processed);
                        }
                    }
                }

                if (there_are_remote_readers_)
                {
                    periodic_hb_event_->restart_timer(max_blocking_time);
                }

                if ( (mp_listener != nullptr) && this->is_acked_by_all(change) )
                {
                    mp_listener->onWriterChangeReceivedByAll(this, change);
                }

                if (disable_positive_acks_ && last_sequence_number_ == SequenceNumber_t())
                {
                    last_sequence_number_ = change->sequenceNumber;
                }
            }
            catch (const RTPSMessageGroup::timeout&)
            {
                logError(RTPS_WRITER, "Max blocking time reached");
            }
        }
        else
        {
            for (ReaderProxy* it : matched_readers_)
            {
                ChangeForReader_t changeForReader(change);

                if (m_pushMode)
                {
                    changeForReader.setStatus(UNSENT);
                }
                else
                {
                    changeForReader.setStatus(UNACKNOWLEDGED);
                }
                changeForReader.setRelevance(it->rtps_is_relevant(change));
                it->add_change(changeForReader, false, max_blocking_time);
            }

            if (m_pushMode)
            {
                mp_RTPSParticipant->async_thread().wake_up(this, max_blocking_time);
            }
        }

        if (disable_positive_acks_)
        {
            auto source_timestamp = system_clock::time_point() + nanoseconds(change->sourceTimestamp.to_ns());
            auto now = system_clock::now();
            auto interval = source_timestamp - now + keep_duration_us_;
            assert(interval.count() >= 0);

            ack_event_->update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
            ack_event_->restart_timer(max_blocking_time);
        }
    }
    else
    {
        logInfo(RTPS_WRITER, "No reader proxy to add change.");
        if (mp_listener != nullptr)
        {
            mp_listener->onWriterChangeReceivedByAll(this, change);
        }
    }
}

bool StatefulWriter::intraprocess_delivery(
        CacheChange_t* change,
        ReaderProxy* reader_proxy)
{
    RTPSReader* reader = reader_proxy->local_reader();
    if (reader)
    {
        if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
        {
            change->write_params.sample_identity(change->write_params.related_sample_identity());
        }
        return reader->processDataMsg(change);
    }
    return false;
}

bool StatefulWriter::intraprocess_gap(
        ReaderProxy* reader_proxy,
        const SequenceNumber_t& seq_num)
{
    RTPSReader* reader = reader_proxy->local_reader();
    if (reader)
    {
        return reader->processGapMsg(m_guid, seq_num, SequenceNumberSet_t(seq_num + 1));
    }

    return false;
}

bool StatefulWriter::intraprocess_heartbeat(
        ReaderProxy* reader_proxy,
        bool liveliness)
{
    bool returned_value = false;

    std::lock_guard<RecursiveTimedMutex> guardW(mp_mutex);
    RTPSReader* reader = RTPSDomainImpl::find_local_reader(reader_proxy->guid());

    if (reader)
    {
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

        if (first_seq != c_SequenceNumber_Unknown && last_seq != c_SequenceNumber_Unknown)
        {
            incrementHBCount();
            if (true == (returned_value =
                    reader->processHeartbeatMsg(m_guid, m_heartbeatCount, first_seq, last_seq, true, liveliness)))
            {
                if (reader_proxy->durability_kind() < TRANSIENT_LOCAL ||
                        this->getAttributes().durabilityKind < TRANSIENT_LOCAL)
                {
                    SequenceNumber_t last_irrelevance = reader_proxy->changes_low_mark();
                    if (first_seq <= last_irrelevance)
                    {
                        reader->processGapMsg(m_guid, first_seq, SequenceNumberSet_t(last_irrelevance + 1));
                    }
                }
            }
        }
    }

    return returned_value;
}

bool StatefulWriter::change_removed_by_history(
        CacheChange_t* a_change)
{
    SequenceNumber_t sequence_number = a_change->sequenceNumber;

    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    logInfo(RTPS_WRITER, "Change " << sequence_number << " to be removed.");

    // Take note of biggest removed sequence number to improve sending of gaps
    if (sequence_number > biggest_removed_sequence_number_)
    {
        biggest_removed_sequence_number_ = sequence_number;
    }

    // Invalidate CacheChange pointer in ReaderProxies.
    for (ReaderProxy* it : matched_readers_)
    {
        it->change_has_been_removed(sequence_number);
    }

    may_remove_change_ = 2;
    may_remove_change_cond_.notify_one();

    return true;
}

void StatefulWriter::send_any_unsent_changes()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    bool activateHeartbeatPeriod = false;
    SequenceNumber_t max_sequence = mp_history->next_sequence_number();

    if (!m_pushMode || mp_history->getHistorySize() == 0 || matched_readers_.empty())
    {
        send_heartbeat_to_all_readers();
    }
    else if (m_separateSendingEnabled)
    {
        send_changes_separatedly(max_sequence, activateHeartbeatPeriod);
    }
    else
    {
        bool no_flow_controllers = m_controllers.empty() && mp_RTPSParticipant->getFlowControllers().empty();
        if (no_flow_controllers || !there_are_remote_readers_)
        {
            send_all_unsent_changes(max_sequence, activateHeartbeatPeriod);
        }
        else
        {
            send_unsent_changes_with_flow_control(max_sequence, activateHeartbeatPeriod);
        }
    }

    if (activateHeartbeatPeriod)
    {
        periodic_hb_event_->restart_timer();
    }

    // On VOLATILE writers, remove auto-acked (best effort readers) changes
    check_acked_status();

    logInfo(RTPS_WRITER, "Finish sending unsent changes");
}

void StatefulWriter::send_heartbeat_to_all_readers()
{
    // This version is called when any of the following conditions is satisfied:
    // a) push mode is false
    // b) history is empty
    // c) there are no matched readers

    if (m_separateSendingEnabled)
    {
        for (ReaderProxy* reader : matched_readers_)
        {
            if (reader->is_local_reader())
            {
                intraprocess_heartbeat(reader);
            }
            else
            {
                send_heartbeat_to_nts(*reader);
            }
        }
    }
    else
    {
        for (ReaderProxy* reader : matched_readers_)
        {
            if (reader->is_local_reader())
            {
                intraprocess_heartbeat(reader);
            }
        }

        if (there_are_remote_readers_)
        {
            RTPSMessageGroup group(mp_RTPSParticipant, this, *this);
            send_heartbeat_nts_(all_remote_readers_.size(), group, disable_positive_acks_);
        }
    }
}

void StatefulWriter::send_changes_separatedly(
        SequenceNumber_t max_sequence,
        bool& activateHeartbeatPeriod)
{
    // This version is called when all of the following conditions are satisfied:
    // a) push mode is true
    // b) history is not empty
    // c) there is at least one matched reader
    // d) separate sending is enabled

    for (ReaderProxy* remoteReader : matched_readers_)
    {
        if (remoteReader->is_local_reader())
        {
            SequenceNumber_t max_ack_seq = SequenceNumber_t::unknown();
            auto unsent_change_process =
                [&](const SequenceNumber_t& seqNum, const ChangeForReader_t* unsentChange)
            {
                if (unsentChange != nullptr && unsentChange->isRelevant() && unsentChange->isValid())
                {
                    if (intraprocess_delivery(unsentChange->getChange(), remoteReader))
                    {
                        max_ack_seq = seqNum;
                    }
                    else
                    {
                        remoteReader->set_change_to_status(seqNum, UNDERWAY, false);
                    }
                }
                else
                {
                    if (intraprocess_gap(remoteReader, seqNum))
                    {
                        max_ack_seq = seqNum;
                    }
                    else
                    {
                        remoteReader->set_change_to_status(seqNum, UNDERWAY, true);
                    }
                }
            };
            remoteReader->for_each_unsent_change(max_sequence, unsent_change_process);
            if (max_ack_seq != SequenceNumber_t::unknown())
            {
                remoteReader->acked_changes_set(max_ack_seq + 1);
            }
        }
        else
        {
            // Specific destination message group
            RTPSMessageGroup group(mp_RTPSParticipant, this, remoteReader->message_sender());
            SequenceNumber_t min_history_seq = get_seq_num_min();
            if (remoteReader->is_reliable())
            {
                if (remoteReader->are_there_gaps())
                {
                    send_heartbeat_nts_(1u, group, true);
                }

                RTPSGapBuilder gaps(group);

                uint32_t lastBytesProcessed = 0;
                auto sent_fun = [this, remoteReader, &lastBytesProcessed, &group](
                        FragmentNumber_t /*frag*/)
                {
                    // Heartbeat piggyback.
                    send_heartbeat_piggyback_nts_(remoteReader, group, lastBytesProcessed);
                };

                auto unsent_change_process =
                    [&](const SequenceNumber_t& seqNum, const ChangeForReader_t* unsentChange)
                {
                    if (unsentChange != nullptr && unsentChange->isRelevant() && unsentChange->isValid())
                    {
                        bool sent_ok = send_data_or_fragments(
                            group,
                            unsentChange->getChange(),
                            remoteReader->expects_inline_qos(),
                            sent_fun);
                        if (sent_ok)
                        {
                            remoteReader->set_change_to_status(seqNum, UNDERWAY, true);
                            activateHeartbeatPeriod = true;
                        }
                    }
                    else
                    {
                        if (seqNum >= min_history_seq)
                        {
                            gaps.add(seqNum);
                        }
                        remoteReader->set_change_to_status(seqNum, UNDERWAY, true);
                    }
                };
                remoteReader->for_each_unsent_change(max_sequence, unsent_change_process);
            }
            else
            {
                SequenceNumber_t max_ack_seq = SequenceNumber_t::unknown();
                auto unsent_change_process =
                    [&](const SequenceNumber_t& seqNum, const ChangeForReader_t* unsentChange)
                {
                    if (unsentChange != nullptr && unsentChange->isRelevant() && unsentChange->isValid())
                    {
                        bool sent_ok = send_data_or_fragments(
                            group,
                            unsentChange->getChange(),
                            remoteReader->expects_inline_qos(),
                            null_sent_fun);
                        if (sent_ok)
                        {
                            max_ack_seq = seqNum;
                        }
                    }
                    else
                    {
                        max_ack_seq = seqNum;
                    }
                };
                remoteReader->for_each_unsent_change(max_sequence, unsent_change_process);
                if (max_ack_seq != SequenceNumber_t::unknown())
                {
                    remoteReader->acked_changes_set(max_ack_seq + 1);
                }
            }
        }
    } // Readers loop
}

void StatefulWriter::send_all_intraprocess_changes(
        SequenceNumber_t max_sequence)
{
    for (ReaderProxy* remoteReader : matched_readers_)
    {
        if (remoteReader->is_local_reader())
        {
            intraprocess_heartbeat(remoteReader, false);
            SequenceNumber_t max_ack_seq = SequenceNumber_t::unknown();
            auto unsent_change_process = [&](const SequenceNumber_t& seq_num, const ChangeForReader_t* unsentChange)
            {
                if (unsentChange != nullptr && unsentChange->isRelevant() && unsentChange->isValid())
                {
                    if (intraprocess_delivery(unsentChange->getChange(), remoteReader))
                    {
                        max_ack_seq = seq_num;
                    }
                    else
                    {
                        remoteReader->set_change_to_status(seq_num, UNDERWAY, false);
                    }
                }
                else
                {
                    if (intraprocess_gap(remoteReader, seq_num))
                    {
                        max_ack_seq = seq_num;
                    }
                    else
                    {
                        remoteReader->set_change_to_status(seq_num, UNDERWAY, true);
                    }
                }
            };

            remoteReader->for_each_unsent_change(max_sequence, unsent_change_process);
            if (max_ack_seq != SequenceNumber_t::unknown())
            {
                remoteReader->acked_changes_set(max_ack_seq + 1);
            }
        }
    }
}

void StatefulWriter::send_all_unsent_changes(
        SequenceNumber_t max_sequence,
        bool& activateHeartbeatPeriod)
{
    // This version is called when all of the following conditions are satisfied:
    // a) push mode is true
    // b) history is not empty
    // c) there is at least one matched reader
    // d) separate sending is disabled
    // e) either all matched readers are local or no flow controllers are configured

    // Process intraprocess first
    if (there_are_local_readers_)
    {
        send_all_intraprocess_changes(max_sequence);
    }

    if (there_are_remote_readers_)
    {
        static constexpr uint32_t implicit_flow_controller_size = RTPSMessageGroup::get_max_fragment_payload_size();

        NetworkFactory& network = mp_RTPSParticipant->network_factory();
        locator_selector_.reset(true);
        network.select_locators(locator_selector_);
        compute_selected_guids();

        bool acknack_required = next_all_acked_notify_sequence_ < get_seq_num_min();;
        bool heartbeat_has_been_sent = false;

        RTPSMessageGroup group(mp_RTPSParticipant, this, *this);

        heartbeat_has_been_sent = send_hole_gaps_to_group(group);

        uint32_t lastBytesProcessed = 0;
        auto sent_fun = [this, &lastBytesProcessed, &group](
            FragmentNumber_t /*frag*/)
        {
            // Heartbeat piggyback.
            send_heartbeat_piggyback_nts_(nullptr, group, lastBytesProcessed);
        };

        RTPSGapBuilder gap_builder(group);
        uint32_t total_sent_size = 0;

        History::iterator cit;
        for (cit = mp_history->changesBegin();
             cit != mp_history->changesEnd() && (total_sent_size < implicit_flow_controller_size);
             cit++)
        {
            SequenceNumber_t seq = (*cit)->sequenceNumber;

            // Deselect all entries on the locator selector (we will only activate the
            // readers for which this sequence number is pending)
            locator_selector_.reset(false);
            
            bool is_irrelevant = true;   // Will turn to false if change is relevant for at least one reader
            bool should_be_sent = false;
            bool inline_qos = false;
            for (ReaderProxy* remoteReader : matched_readers_)
            {
                if (!remoteReader->is_local_reader())
                {
                    if(remoteReader->change_is_unsent(seq, is_irrelevant))
                    {
                        should_be_sent = true;
                        locator_selector_.enable(remoteReader->guid());
                        inline_qos |= remoteReader->expects_inline_qos();
                        if (is_irrelevant)
                        {
                            remoteReader->set_change_to_status(seq, UNDERWAY, true);
                        }
                    }
                }
            }

            if (locator_selector_.state_has_changed())
            {
                group.flush_and_reset();
                network.select_locators(locator_selector_);
                compute_selected_guids();
            }

            if (should_be_sent)
            {
                if (is_irrelevant)
                {
                    gap_builder.add(seq);
                }
                else
                {
                    bool sent_ok = send_data_or_fragments(group, *cit, inline_qos, sent_fun);
                    if (sent_ok)
                    {
                        total_sent_size += (*cit)->serializedPayload.length;
                        bool tmp_bool = false;
                        for (ReaderProxy* remoteReader : matched_readers_)
                        {
                            if (!remoteReader->is_local_reader())
                            {
                                if(remoteReader->change_is_unsent(seq, tmp_bool))
                                {
                                    remoteReader->set_change_to_status(seq, UNDERWAY, true);
                                    if (remoteReader->is_reliable())
                                    {
                                        activateHeartbeatPeriod = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Heartbeat piggyback.
        if (!heartbeat_has_been_sent && acknack_required)
        {
            send_heartbeat_nts_(all_remote_readers_.size(), group, false);
        }

        group.flush_and_reset();

        locator_selector_.reset(true);
        network.select_locators(locator_selector_);
        compute_selected_guids();

        if (cit != mp_history->changesEnd())
        {
            mp_RTPSParticipant->async_thread().wake_up(this);
        }
    }
}

void StatefulWriter::send_unsent_changes_with_flow_control(
        SequenceNumber_t max_sequence,
        bool& activateHeartbeatPeriod)
{
    // This version is called when all of the following conditions are satisfied:
    // a) push mode is true
    // b) history is not empty
    // c) there is at least one matched reader
    // d) separate sending is disabled
    // e) there is at least one remote matched reader and flow controllers are configured

    // Process intraprocess first
    if (there_are_local_readers_)
    {
        send_all_intraprocess_changes(max_sequence);
    }

    // From here onwards, only remote readers should be accessed

    RTPSWriterCollector<ReaderProxy*> relevantChanges;
    bool heartbeat_has_been_sent = false;

    NetworkFactory& network = mp_RTPSParticipant->network_factory();
    locator_selector_.reset(true);
    network.select_locators(locator_selector_);
    compute_selected_guids();

    RTPSMessageGroup group(mp_RTPSParticipant, this, *this);

    heartbeat_has_been_sent = send_hole_gaps_to_group(group);

    for (ReaderProxy* remoteReader : matched_readers_)
    {
        // Skip local readers (were processed before)
        if (remoteReader->is_local_reader())
        {
            continue;
        }

        if (!heartbeat_has_been_sent && remoteReader->are_there_gaps())
        {
            send_heartbeat_nts_(all_remote_readers_.size(), group, true);
            heartbeat_has_been_sent = true;
        }

        RTPSGapBuilder gaps(group, remoteReader->guid());
        auto unsent_change_process = [&](const SequenceNumber_t& seq_num, const ChangeForReader_t* unsentChange)
            {
                if (unsentChange != nullptr && unsentChange->isRelevant() && unsentChange->isValid())
                {
                    relevantChanges.add_change(
                        unsentChange->getChange(), remoteReader, unsentChange->getUnsentFragments());
                }
                else
                {
                    // Skip holes in history, as they were added before
                    if (unsentChange != nullptr && remoteReader->is_reliable())
                    {
                        gaps.add(seq_num);
                    }

                    remoteReader->set_change_to_status(seq_num, UNDERWAY, true);
                }
            };

        remoteReader->for_each_unsent_change(max_sequence, unsent_change_process);
    }

    // Clear all relevant changes through the local controllers first
    for (std::unique_ptr<FlowController>& controller : m_controllers)
    {
        (*controller)(relevantChanges);
    }

    // Clear all relevant changes through the parent controllers
    for (std::unique_ptr<FlowController>& controller : mp_RTPSParticipant->getFlowControllers())
    {
        (*controller)(relevantChanges);
    }

    try
    {
        uint32_t lastBytesProcessed = 0;

        while (!relevantChanges.empty())
        {
            RTPSWriterCollector<ReaderProxy*>::Item changeToSend = relevantChanges.pop();
            bool expectsInlineQos = false;
            locator_selector_.reset(false);

            for (const ReaderProxy* remoteReader : changeToSend.remoteReaders)
            {
                locator_selector_.enable(remoteReader->guid());
                expectsInlineQos |= remoteReader->expects_inline_qos();
            }

            if (locator_selector_.state_has_changed())
            {
                group.flush_and_reset();
                network.select_locators(locator_selector_);
                compute_selected_guids();
            }

            // TODO(Ricardo) Flowcontroller has to be used in RTPSMessageGroup. Study.
            // And controllers are notified about the changes being sent
            FlowController::NotifyControllersChangeSent(changeToSend.cacheChange);

            if (changeToSend.fragmentNumber != 0)
            {
                if (group.add_data_frag(*changeToSend.cacheChange, changeToSend.fragmentNumber,
                        expectsInlineQos))
                {
                    bool must_wake_up_async_thread = false;
                    for (ReaderProxy* remoteReader : changeToSend.remoteReaders)
                    {
                        bool allFragmentsSent = false;
                        if (remoteReader->mark_fragment_as_sent_for_change(
                                    changeToSend.sequenceNumber,
                                    changeToSend.fragmentNumber,
                                    allFragmentsSent))
                        {
                            must_wake_up_async_thread |= !allFragmentsSent;
                            if (remoteReader->is_remote_and_reliable())
                            {
                                activateHeartbeatPeriod = true;
                                if (allFragmentsSent)
                                {
                                    remoteReader->set_change_to_status(changeToSend.sequenceNumber,
                                            UNDERWAY,
                                            true);
                                }
                            }
                            else
                            {
                                if (allFragmentsSent)
                                {
                                    remoteReader->set_change_to_status(changeToSend.sequenceNumber,
                                            ACKNOWLEDGED, false);
                                }
                            }
                        }
                    }

                    if (must_wake_up_async_thread)
                    {
                        mp_RTPSParticipant->async_thread().wake_up(this);
                    }
                }
                else
                {
                    logError(RTPS_WRITER, "Error sending fragment (" << changeToSend.sequenceNumber <<
                            ", " << changeToSend.fragmentNumber << ")");
                }
            }
            else
            {
                if (group.add_data(*changeToSend.cacheChange, expectsInlineQos))
                {
                    for (ReaderProxy* remoteReader : changeToSend.remoteReaders)
                    {
                        remoteReader->set_change_to_status(changeToSend.sequenceNumber, UNDERWAY, true);

                        if (remoteReader->is_remote_and_reliable())
                        {
                            activateHeartbeatPeriod = true;
                        }
                    }
                }
                else
                {
                    logError(RTPS_WRITER, "Error sending change " << changeToSend.sequenceNumber);
                }
            }

            // Heartbeat piggyback.
            send_heartbeat_piggyback_nts_(nullptr, group, lastBytesProcessed);
        }

        group.flush_and_reset();
    }
    catch (const RTPSMessageGroup::timeout&)
    {
        logError(RTPS_WRITER, "Max blocking time reached");
    }

    locator_selector_.reset(true);
    network.select_locators(locator_selector_);
    compute_selected_guids();
}

bool StatefulWriter::send_hole_gaps_to_group(
        RTPSMessageGroup& group)
{
    bool ret_val = false;

    // Add holes in history and send them to all readers in group
    SequenceNumber_t max_removed = biggest_removed_sequence_number_;
    SequenceNumber_t last_sequence = mp_history->next_sequence_number();
    SequenceNumber_t min_history_seq = get_seq_num_min();
    uint32_t history_size = static_cast<uint32_t>(mp_history->getHistorySize());
    if ((next_all_acked_notify_sequence_ < max_removed) &&    // some holes pending acknowledgement
        (min_history_seq + history_size != last_sequence))    // There is a hole in the history
    {
        try
        {
            send_heartbeat_nts_(all_remote_readers_.size(), group, true);
            ret_val = true;

            // Find holes in history from min_history_seq to last_sequence - 1
            RTPSGapBuilder gap_builder(group);

            // Algorithm starts in min_history_seq
            SequenceNumber_t seq = min_history_seq;

            // Loop all history
            for (auto cit = mp_history->changesBegin(); cit != mp_history->changesEnd(); cit++)
            {
                // Add all sequence numbers until the change's sequence number
                while (seq < (*cit)->sequenceNumber)
                {
                    gap_builder.add(seq);
                    seq++;
                }

                // Skip change's sequence number
                seq++;
            }

            // Add all sequence numbers above last change
            while (seq < last_sequence)
            {
                gap_builder.add(seq);
                seq++;
            }
        }
        catch (const RTPSMessageGroup::timeout&)
        {
            logError(RTPS_WRITER, "Max blocking time reached");
        }
    }

    return ret_val;
}

/*
 * MATCHED_READER-RELATED METHODS
 */
void StatefulWriter::update_reader_info(
        bool create_sender_resources)
{
    update_cached_info_nts();
    compute_selected_guids();

    if (create_sender_resources)
    {
        RTPSParticipantImpl* part = getRTPSParticipant();
        locator_selector_.for_each([part](const Locator_t& loc)
        {
            part->createSenderResources(loc);
        });
    }

    // Check if we have local or remote readers
    size_t n_readers = matched_readers_.size();
    there_are_remote_readers_ = false;
    there_are_local_readers_ = false;

    size_t i = 0;
    for (; i < n_readers && !there_are_remote_readers_; ++i)
    {
        bool is_local = matched_readers_.at(i)->is_local_reader();
        there_are_remote_readers_ |= !is_local;
        there_are_local_readers_ |= is_local;
    }

    for (; i < n_readers && !there_are_local_readers_; ++i)
    {
        bool is_local = matched_readers_.at(i)->is_local_reader();
        there_are_local_readers_ |= is_local;
    }
}

bool StatefulWriter::matched_reader_add(
        const ReaderProxyData& rdata)
{
    if (rdata.guid() == c_Guid_Unknown)
    {
        logError(RTPS_WRITER, "Reliable Writer need GUID_t of matched readers");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    // Check if it is already matched.
    for (ReaderProxy* it : matched_readers_)
    {
        if (it->guid() == rdata.guid())
        {
            logInfo(RTPS_WRITER, "Attempting to add existing reader, updating information.");
            if (it->update(rdata))
            {
                update_reader_info(true);
            }
            return false;
        }
    }

    // Get a reader proxy from the inactive pool (or create a new one if necessary and allowed)
    ReaderProxy* rp = nullptr;
    if (matched_readers_pool_.empty())
    {
        size_t max_readers = matched_readers_pool_.max_size();
        if (matched_readers_.size() + matched_readers_pool_.size() < max_readers)
        {
            const RTPSParticipantAttributes& part_att = mp_RTPSParticipant->getRTPSParticipantAttributes();
            rp = new ReaderProxy(m_times, part_att.allocation.locators, this);
        }
        else
        {
            logWarning(RTPS_WRITER, "Maximum number of reader proxies (" << max_readers << \
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
    rp->start(rdata);
    locator_selector_.add_entry(rp->locator_selector_entry());
    matched_readers_.push_back(rp);
    update_reader_info(true);

    RTPSMessageGroup group(mp_RTPSParticipant, this, rp->message_sender());

    // Add initial heartbeat to message group
    send_heartbeat_nts_(1u, group, disable_positive_acks_);

    SequenceNumber_t current_seq = get_seq_num_min();
    SequenceNumber_t last_seq = get_seq_num_max();

    if (current_seq != SequenceNumber_t::unknown())
    {
        (void)last_seq;
        assert(last_seq != SequenceNumber_t::unknown());
        assert(current_seq <= last_seq);

        RTPSGapBuilder gap_builder(group);
        bool is_reliable = rp->is_reliable();

        for (History::iterator cit = mp_history->changesBegin(); cit != mp_history->changesEnd(); ++cit)
        {
            // This is to cover the case when there are holes in the history
            if (is_reliable)
            {
                while (current_seq != (*cit)->sequenceNumber)
                {
                    if (rp->is_local_reader())
                    {
                        intraprocess_gap(rp, current_seq);
                    }
                    else
                    {
                        try
                        {
                            gap_builder.add(current_seq);
                        }
                        catch (const RTPSMessageGroup::timeout&)
                        {
                            logError(RTPS_WRITER, "Max blocking time reached");
                        }
                    }
                    ++current_seq;
                }
            }
            else
            {
                current_seq = (*cit)->sequenceNumber;
            }

            ChangeForReader_t changeForReader(*cit);
            bool relevance = 
                rp->durability_kind() >= TRANSIENT_LOCAL &&
                m_att.durabilityKind >= TRANSIENT_LOCAL &&
                rp->rtps_is_relevant(*cit);
            changeForReader.setRelevance(relevance);
            if (!relevance && is_reliable)
            {
                if (rp->is_local_reader())
                {
                    intraprocess_gap(rp, current_seq);
                }
                else
                {
                    try
                    {
                        gap_builder.add(current_seq);
                    }
                    catch (const RTPSMessageGroup::timeout&)
                    {
                        logError(RTPS_WRITER, "Max blocking time reached");
                    }
                }
            }

            // The ChangeForReader_t status has to be UNACKNOWLEDGED
            if (!rp->is_local_reader() || !changeForReader.isRelevant())
            {
                changeForReader.setStatus(UNACKNOWLEDGED);
            }
            rp->add_change(changeForReader, false);
            ++current_seq;
        }

        // This is to cover the case where the last changes have been removed from the history
        if (is_reliable)
        {
            while (current_seq < next_sequence_number())
            {
                if (rp->is_local_reader())
                {
                    intraprocess_gap(rp, current_seq);
                }
                else
                {
                    try
                    {
                        gap_builder.add(current_seq);
                    }
                    catch (const RTPSMessageGroup::timeout&)
                    {
                        logError(RTPS_WRITER, "Max blocking time reached");
                    }
                }
                ++current_seq;
            }
        }

        try
        {
            if (rp->is_local_reader())
            {
                mp_RTPSParticipant->async_thread().wake_up(this);
            }
            else if (is_reliable)
            {
                // Send Gap
                gap_builder.flush();
            }
        }
        catch (const RTPSMessageGroup::timeout&)
        {
            logError(RTPS_WRITER, "Max blocking time reached");
        }

        // Always activate heartbeat period. We need a confirmation of the reader.
        // The state has to be updated.
        periodic_hb_event_->restart_timer();
    }

    try
    {
        // Send all messages
        group.flush_and_reset();
    }
    catch (const RTPSMessageGroup::timeout&)
    {
        logError(RTPS_WRITER, "Max blocking time reached");
    }

    logInfo(RTPS_WRITER, "Reader Proxy " << rp->guid() << " added to " << this->m_guid.entityId << " with "
                                         << rdata.remote_locators().unicast.size() << "(u)-"
                                         << rdata.remote_locators().multicast.size() <<
            "(m) locators");

    return true;
}

bool StatefulWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    ReaderProxy* rproxy = nullptr;
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    ReaderProxyIterator it = matched_readers_.begin();
    while (it != matched_readers_.end())
    {
        if ((*it)->guid() == reader_guid)
        {
            logInfo(RTPS_WRITER, "Reader Proxy removed: " << reader_guid);
            rproxy = std::move(*it);
            it = matched_readers_.erase(it);

            continue;
        }

        ++it;
    }

    locator_selector_.remove_entry(reader_guid);
    update_reader_info(false);

    if (matched_readers_.size() == 0)
    {
        periodic_hb_event_->cancel_timer();
    }

    if (rproxy != nullptr)
    {
        rproxy->stop();
        matched_readers_pool_.push_back(rproxy);

        lock.unlock();
        check_acked_status();

        return true;
    }

    logInfo(RTPS_HISTORY, "Reader Proxy doesn't exist in this writer");
    return false;
}

bool StatefulWriter::matched_reader_is_matched(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    for (ReaderProxy* it : matched_readers_)
    {
        if (it->guid() == reader_guid)
        {
            return true;
        }
    }
    return false;
}

bool StatefulWriter::matched_reader_lookup(
        GUID_t& readerGuid,
        ReaderProxy** RP)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    for (ReaderProxy* it : matched_readers_)
    {
        if (it->guid() == readerGuid)
        {
            *RP = it;
            return true;
        }
    }
    return false;
}

bool StatefulWriter::is_acked_by_all(
        const CacheChange_t* change) const
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (change->writerGUID != this->getGuid())
    {
        logWarning(RTPS_WRITER, "The given change is not from this Writer");
        return false;
    }

    assert(mp_history->next_sequence_number() > change->sequenceNumber);
    return std::all_of(matched_readers_.begin(), matched_readers_.end(),
                   [change](const ReaderProxy* reader)
    {
        return reader->change_is_acked(change->sequenceNumber);
    });
}

bool StatefulWriter::all_readers_updated()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    for (auto it = matched_readers_.begin(); it != matched_readers_.end(); ++it)
    {
        if ((*it)->has_changes())
        {
            return false;
        }
    }

    return true;
}

bool StatefulWriter::wait_for_all_acked(
        const Duration_t& max_wait)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    std::unique_lock<std::mutex> all_acked_lock(all_acked_mutex_);

    all_acked_ = std::none_of(matched_readers_.begin(), matched_readers_.end(),
                    [](const ReaderProxy* reader)
    {
        return reader->has_changes();
    });
    lock.unlock();

    if (!all_acked_)
    {
        std::chrono::microseconds max_w(TimeConv::Duration_t2MicroSecondsInt64(max_wait));
        all_acked_cond_.wait_for(all_acked_lock, max_w, [&]() {
            return all_acked_;
        });
    }

    return all_acked_;
}

void StatefulWriter::check_acked_status()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    bool all_acked = true;
    bool has_min_low_mark = false;
    SequenceNumber_t min_low_mark;

    for (const ReaderProxy* it : matched_readers_)
    {
        SequenceNumber_t reader_low_mark = it->changes_low_mark();
        if (reader_low_mark < min_low_mark || !has_min_low_mark)
        {
            has_min_low_mark = true;
            min_low_mark = reader_low_mark;
        }

        if (it->has_changes())
        {
            all_acked = false;
        }
    }

    SequenceNumber_t min_seq = get_seq_num_min();
    if (min_seq != SequenceNumber_t::unknown())
    {
        // In the case where we haven't received an acknack from a recently matched reader,
        // min_low_mark will be zero, and no change will be notified as received by all
        if (next_all_acked_notify_sequence_ <= min_low_mark)
        {
            if ( (mp_listener != nullptr) && (min_low_mark >= get_seq_num_min()))
            {
                // We will inform backwards about the changes received by all readers, starting
                // on min_low_mark down until next_all_acked_notify_sequence_. This way we can
                // safely proceed with the traversal, in case a change is removed from the history
                // inside the callback
                History::iterator history_end = mp_history->changesEnd();
                History::iterator cit =
                    std::lower_bound(mp_history->changesBegin(), history_end, min_low_mark,
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
                    if (cit != mp_history->changesBegin())
                    {
                        --cit;
                    }

                    // Notify reception of change (may remove that change on VOLATILE writers)
                    mp_listener->onWriterChangeReceivedByAll(this, change);

                    // Stop if we got to either next_all_acked_notify_sequence_ or the first change
                } while (seq > end_seq);
            }

            next_all_acked_notify_sequence_ = min_low_mark + 1;
        }

        if (min_low_mark >= get_seq_num_min())
        {
            may_remove_change_ = 1;
            may_remove_change_cond_.notify_one();
        }
    }

    if (all_acked)
    {
        std::unique_lock<std::mutex> all_acked_lock(all_acked_mutex_);
        all_acked_ = true;
        all_acked_cond_.notify_all();
    }
}

bool StatefulWriter::try_remove_change(
        std::chrono::steady_clock::time_point& max_blocking_time_point,
        std::unique_lock<RecursiveTimedMutex>& lock)
{
    logInfo(RTPS_WRITER, "Starting process try remove change for writer " << getGuid());

    SequenceNumber_t min_low_mark;

    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        for (ReaderProxy* it : matched_readers_)
        {
            SequenceNumber_t reader_low_mark = it->changes_low_mark();
            if (min_low_mark == SequenceNumber_t() || reader_low_mark < min_low_mark)
            {
                min_low_mark = reader_low_mark;
            }
        }
    }

    SequenceNumber_t calc = min_low_mark < get_seq_num_min() ? SequenceNumber_t() :
            (min_low_mark - get_seq_num_min()) + 1;
    unsigned int may_remove_change = 1;

    if (calc <= SequenceNumber_t())
    {
        may_remove_change_ = 0;
        may_remove_change_cond_.wait_until(lock, max_blocking_time_point,
                [&]() {
            return may_remove_change_ > 0;
        });
        may_remove_change = may_remove_change_;
    }

    // Some changes acked
    if (may_remove_change == 1)
    {
        return mp_history->remove_min_change();
    }
    // Waiting a change was removed.
    else if (may_remove_change == 2)
    {
        return true;
    }

    return false;
}

/*
 * PARAMETER_RELATED METHODS
 */
void StatefulWriter::updateAttributes(
        const WriterAttributes& att)
{
    this->updateTimes(att.times);
}

void StatefulWriter::updateTimes(
        const WriterTimes& times)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (m_times.heartbeatPeriod != times.heartbeatPeriod)
    {
        periodic_hb_event_->update_interval(times.heartbeatPeriod);
    }
    if (m_times.nackResponseDelay != times.nackResponseDelay)
    {
        if (nack_response_event_ != nullptr)
        {
            nack_response_event_->update_interval(times.nackResponseDelay);
        }
    }
    if (m_times.nackSupressionDuration != times.nackSupressionDuration)
    {
        for (ReaderProxy* it : matched_readers_)
        {
            it->update_nack_supression_interval(times.nackSupressionDuration);
        }
        for (ReaderProxy* it : matched_readers_pool_)
        {
            it->update_nack_supression_interval(times.nackSupressionDuration);
        }
    }
    m_times = times;
}

void StatefulWriter::add_flow_controller(
        std::unique_ptr<FlowController> controller)
{
    m_controllers.push_back(std::move(controller));
}

SequenceNumber_t StatefulWriter::next_sequence_number() const
{
    return mp_history->next_sequence_number();
}

bool StatefulWriter::send_periodic_heartbeat(
        bool final,
        bool liveliness)
{
    std::lock_guard<RecursiveTimedMutex> guardW(mp_mutex);

    bool unacked_changes = false;
    if (m_separateSendingEnabled)
    {
        for (ReaderProxy* it : matched_readers_)
        {
            if (it->has_unacknowledged() && !it->is_local_reader())
            {
                send_heartbeat_to_nts(*it, liveliness);
                unacked_changes = true;
            }
        }
    }
    else if (!liveliness)
    {
        SequenceNumber_t firstSeq, lastSeq;

        firstSeq = get_seq_num_min();
        lastSeq = get_seq_num_max();

        if (firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
        {
            return false;
        }
        else
        {
            assert(firstSeq <= lastSeq);

            unacked_changes = std::any_of(matched_readers_.begin(), matched_readers_.end(),
                            [](const ReaderProxy* reader)
            {
                return reader->has_unacknowledged();
            });

            if (unacked_changes)
            {
                try
                {
                    RTPSMessageGroup group(mp_RTPSParticipant, this, *this);
                    send_heartbeat_nts_(all_remote_readers_.size(), group, disable_positive_acks_, liveliness);
                }
                catch (const RTPSMessageGroup::timeout&)
                {
                    logError(RTPS_WRITER, "Max blocking time reached");
                }
            }
        }
    }
    else
    {
        // This is a liveliness heartbeat, we don't care about checking sequence numbers
        try
        {
            for (ReaderProxy* it : matched_readers_)
            {
                if (it->is_local_reader())
                {
                    intraprocess_heartbeat(it, true);
                }
            }

            RTPSMessageGroup group(mp_RTPSParticipant, this, *this);
            send_heartbeat_nts_(all_remote_readers_.size(), group, final, liveliness);
        }
        catch (const RTPSMessageGroup::timeout&)
        {
            logError(RTPS_WRITER, "Max blocking time reached");
        }
    }

    return unacked_changes;
}

void StatefulWriter::send_heartbeat_to_nts(
        ReaderProxy& remoteReaderProxy,
        bool liveliness)
{
    try
    {
        RTPSMessageGroup group(mp_RTPSParticipant, this, remoteReaderProxy.message_sender());
        send_heartbeat_nts_(1u, group, disable_positive_acks_, liveliness);
        SequenceNumber_t first_seq = get_seq_num_min();
        if (first_seq != c_SequenceNumber_Unknown)
        {
            if (remoteReaderProxy.durability_kind() < TRANSIENT_LOCAL ||
                getAttributes().durabilityKind < TRANSIENT_LOCAL)
            {
                SequenceNumber_t last_irrelevance = remoteReaderProxy.changes_low_mark();
                if (first_seq <= last_irrelevance)
                {
                    group.add_gap(first_seq, SequenceNumberSet_t(last_irrelevance + 1));
                }
            }

        }
    }
    catch (const RTPSMessageGroup::timeout&)
    {
        logError(RTPS_WRITER, "Max blocking time reached");
    }
}

void StatefulWriter::send_heartbeat_nts_(
        size_t number_of_readers,
        RTPSMessageGroup& message_group,
        bool final,
        bool liveliness)
{

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

    incrementHBCount();
    message_group.add_heartbeat(firstSeq, lastSeq, m_heartbeatCount, final, liveliness);
    // Update calculate of heartbeat piggyback.
    currentUsageSendBufferSize_ = static_cast<int32_t>(sendBufferSize_);

    logInfo(RTPS_WRITER, getGuid().entityId << " Sending Heartbeat (" << firstSeq << " - " << lastSeq << ")" );
}

void StatefulWriter::send_heartbeat_piggyback_nts_(
        ReaderProxy* reader,
        RTPSMessageGroup& message_group,
        uint32_t& last_bytes_processed)
{
    if (!disable_heartbeat_piggyback_)
    {
        size_t number_of_readers = reader == nullptr ? all_remote_readers_.size() : 1u;
        if (mp_history->isFull())
        {
            if (reader == nullptr)
            {
                locator_selector_.reset(true);
                if (locator_selector_.state_has_changed())
                {
                    message_group.flush_and_reset();
                    getRTPSParticipant()->network_factory().select_locators(locator_selector_);
                    compute_selected_guids();
                }
            }
            send_heartbeat_nts_(number_of_readers, message_group, disable_positive_acks_);
        }
        else
        {
            uint32_t current_bytes = message_group.get_current_bytes_processed();
            currentUsageSendBufferSize_ -= current_bytes - last_bytes_processed;
            last_bytes_processed = current_bytes;
            if (currentUsageSendBufferSize_ < 0)
            {
                send_heartbeat_nts_(number_of_readers, message_group, disable_positive_acks_);
            }
        }
    }
}

void StatefulWriter::perform_nack_response()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    bool must_wake_up_async_thread = false;

    for (ReaderProxy* remote_reader : matched_readers_)
    {
        if (remote_reader->perform_acknack_response() || remote_reader->are_there_gaps())
        {
            must_wake_up_async_thread = true;
        }
    }

    if (must_wake_up_async_thread)
    {
        mp_RTPSParticipant->async_thread().wake_up(this);
    }
}

void StatefulWriter::perform_nack_supression(
        const GUID_t& reader_guid)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    for (ReaderProxy* remote_reader : matched_readers_)
    {
        if (remote_reader->guid() == reader_guid)
        {
            remote_reader->perform_nack_supression();
            periodic_hb_event_->restart_timer();
            return;
        }
    }
}

bool StatefulWriter::process_acknack(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        uint32_t ack_count,
        const SequenceNumberSet_t& sn_set,
        bool final_flag,
        bool& result)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    result = (m_guid == writer_guid);
    if (result)
    {
        for (ReaderProxy* remote_reader : matched_readers_)
        {
            if (remote_reader->guid() == reader_guid)
            {
                if (remote_reader->check_and_set_acknack_count(ack_count))
                {
                    // Sequence numbers before Base are set as Acknowledged.
                    remote_reader->acked_changes_set(sn_set.base());
                    if (sn_set.base() > SequenceNumber_t(0, 0))
                    {
                        if (remote_reader->requested_changes_set(sn_set) || remote_reader->are_there_gaps())
                        {
                            nack_response_event_->restart_timer();
                        }
                        else if (!final_flag)
                        {
                            periodic_hb_event_->restart_timer();
                        }
                    }
                    else if (sn_set.empty() && !final_flag)
                    {
                        // This is the preemptive acknack.
                        if (remote_reader->process_initial_acknack())
                        {
                            if (remote_reader->is_local_reader())
                            {
                                mp_RTPSParticipant->async_thread().wake_up(this);
                            }
                            else
                            {
                                // Send heartbeat if requested
                                send_heartbeat_to_nts(*remote_reader);
                            }
                        }

                        if (remote_reader->is_local_reader())
                        {
                            intraprocess_heartbeat(remote_reader);
                        }
                    }

                    // Check if all CacheChange are acknowledge, because a user could be waiting
                    // for this, of if VOLATILE should be removed CacheChanges
                    check_acked_status();
                }
                break;
            }
        }
    }

    return result;
}

bool StatefulWriter::process_nack_frag(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        uint32_t ack_count,
        const SequenceNumber_t& seq_num,
        const FragmentNumberSet_t fragments_state,
        bool& result)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    result = false;
    if (m_guid == writer_guid)
    {
        result = true;
        for (ReaderProxy* remote_reader : matched_readers_)
        {
            if (remote_reader->guid() == reader_guid)
            {
                if (remote_reader->process_nack_frag(reader_guid, ack_count, seq_num, fragments_state))
                {
                    nack_response_event_->restart_timer();
                }
                break;
            }
        }
    }

    return result;
}

bool StatefulWriter::ack_timer_expired()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    // The timer has expired so the earliest non-acked change must be marked as acknowledged
    // This will be done in the first while iteration, as we start with a negative interval

    auto interval = -keep_duration_us_;

    // On the other hand, we've seen in the tests that if samples are sent very quickly with little
    // time between consecutive samples, the timer interval could end up being negative
    // In this case, we keep marking changes as acknowledged until the timer is able to keep up, hence the while
    // loop

    while (interval.count() < 0)
    {
        for (ReaderProxy* remote_reader : matched_readers_)
        {
            if (remote_reader->disable_positive_acks())
            {
                remote_reader->acked_changes_set(last_sequence_number_ + 1);
            }
        }
        last_sequence_number_++;

        // Get the next cache change from the history
        CacheChange_t* change;

        if (!mp_history->get_change(
                    last_sequence_number_,
                    getGuid(),
                    &change))
        {
            return false;
        }

        auto source_timestamp = system_clock::time_point() + nanoseconds(change->sourceTimestamp.to_ns());
        auto now = system_clock::now();
        interval = source_timestamp - now + keep_duration_us_;
    }
    assert(interval.count() >= 0);

    ack_event_->update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
    return true;
}

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
