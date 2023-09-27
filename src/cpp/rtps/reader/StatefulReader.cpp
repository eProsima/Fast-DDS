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
 * @file StatefulReader.cpp
 *
 */

#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/messages/RTPSMessageCreator.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/reader/WriterProxy.h>
#include <fastrtps/utils/TimeConversion.h>
#include <rtps/history/HistoryAttributesExtension.hpp>
#include <rtps/DataSharing/DataSharingListener.hpp>
#include <rtps/DataSharing/ReaderPool.hpp>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>
#include <fastdds/rtps/writer/LivelinessManager.h>

#include "rtps/RTPSDomainImpl.hpp"

#include <mutex>
#include <thread>

#include <cassert>

#define IDSTRING "(ID:" << std::this_thread::get_id() << ") " <<

using namespace eprosima::fastrtps::rtps;

static void send_datasharing_ack(
        StatefulReader* reader,
        ReaderHistory* history,
        WriterProxy* writer,
        const SequenceNumber_t& sequence_number)
{
    // This may not be the change read with highest SN,
    // need to find largest SN to ACK
    for (std::vector<CacheChange_t*>::iterator it = history->changesBegin(); it != history->changesEnd(); ++it)
    {
        if (!(*it)->isRead)
        {
            if ((*it)->writerGUID == writer->guid())
            {
                if ((*it)->sequenceNumber < sequence_number)
                {
                    //There are earlier changes not read yet. Do not send ACK.
                    return;
                }
                SequenceNumberSet_t sns((*it)->sequenceNumber);
                reader->send_acknack(writer, sns, writer, false);
                return;
            }
        }
    }

    // Must ACK all in the writer
    SequenceNumberSet_t sns(writer->available_changes_max() + 1);
    reader->send_acknack(writer, sns, writer, false);
}

static inline void send_ack_if_datasharing(
        StatefulReader* reader,
        ReaderHistory* history,
        WriterProxy* writer,
        const SequenceNumber_t& sequence_number)
{
    // Shall be datasharing, and not on same process
    if (writer && writer->is_datasharing_writer() && !writer->is_on_same_process())
    {
        send_datasharing_ack(reader, history, writer, sequence_number);
    }
}

StatefulReader::~StatefulReader()
{
    EPROSIMA_LOG_INFO(RTPS_READER, "StatefulReader destructor.");

    // Only is_alive_ assignment needs to be protected, as
    // matched_writers_ and matched_writers_pool_ are only used
    // when is_alive_ is true
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        is_alive_ = false;
    }

    // Datasharing listener must be stopped to avoid processing notifications
    // while the reader is being destroyed
    if (is_datasharing_compatible_)
    {
        datasharing_listener_->stop();
    }

    for (WriterProxy* writer : matched_writers_)
    {
        delete(writer);
    }
    for (WriterProxy* writer : matched_writers_pool_)
    {
        delete(writer);
    }
}

StatefulReader::StatefulReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        ReaderHistory* hist,
        ReaderListener* listen)
    : RTPSReader(pimpl, guid, att, hist, listen)
    , acknack_count_(0)
    , nackfrag_count_(0)
    , times_(att.times)
    , matched_writers_(att.matched_writers_allocation)
    , matched_writers_pool_(att.matched_writers_allocation)
    , proxy_changes_config_(resource_limits_from_history(hist->m_att, 0))
    , disable_positive_acks_(att.disable_positive_acks)
    , is_alive_(true)
{
    init(pimpl, att);
}

StatefulReader::StatefulReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
    : RTPSReader(pimpl, guid, att, payload_pool, hist, listen)
    , acknack_count_(0)
    , nackfrag_count_(0)
    , times_(att.times)
    , matched_writers_(att.matched_writers_allocation)
    , matched_writers_pool_(att.matched_writers_allocation)
    , proxy_changes_config_(resource_limits_from_history(hist->m_att, 0))
    , disable_positive_acks_(att.disable_positive_acks)
    , is_alive_(true)
{
    init(pimpl, att);
}

StatefulReader::StatefulReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
    : RTPSReader(pimpl, guid, att, payload_pool, change_pool, hist, listen)
    , acknack_count_(0)
    , nackfrag_count_(0)
    , times_(att.times)
    , matched_writers_(att.matched_writers_allocation)
    , matched_writers_pool_(att.matched_writers_allocation)
    , proxy_changes_config_(resource_limits_from_history(hist->m_att, 0))
    , disable_positive_acks_(att.disable_positive_acks)
    , is_alive_(true)
{
    init(pimpl, att);
}

void StatefulReader::init(
        RTPSParticipantImpl* pimpl,
        const ReaderAttributes& att)
{
    const RTPSParticipantAttributes& part_att = pimpl->getRTPSParticipantAttributes();
    for (size_t n = 0; n < att.matched_writers_allocation.initial; ++n)
    {
        matched_writers_pool_.push_back(new WriterProxy(this, part_att.allocation.locators, proxy_changes_config_));
    }
}

bool StatefulReader::matched_writer_add(
        const WriterProxyData& wdata)
{
    assert(wdata.guid() != c_Guid_Unknown);
    ReaderListener* listener = nullptr;

    {
        std::unique_lock<RecursiveTimedMutex> guard(mp_mutex);

        if (!is_alive_)
        {
            return false;
        }

        listener = mp_listener;
        bool is_same_process = RTPSDomainImpl::should_intraprocess_between(m_guid, wdata.guid());
        bool is_datasharing = is_datasharing_compatible_with(wdata);

        for (WriterProxy* it : matched_writers_)
        {
            if (it->guid() == wdata.guid())
            {
                EPROSIMA_LOG_INFO(RTPS_READER, "Attempting to add existing writer, updating information");
                // If Ownership strength changes then update all history instances.
                if (EXCLUSIVE_OWNERSHIP_QOS == m_att.ownershipKind &&
                        it->ownership_strength() != wdata.m_qos.m_ownershipStrength.value)
                {
                    mp_history->writer_update_its_ownership_strength_nts(
                        it->guid(), wdata.m_qos.m_ownershipStrength.value);
                }
                it->update(wdata);
                if (!is_same_process)
                {
                    for (const Locator_t& locator : it->remote_locators_shrinked())
                    {
                        getRTPSParticipant()->createSenderResources(locator);
                    }
                }

                if (nullptr != listener)
                {
                    // call the listener without the lock taken
                    guard.unlock();
                    listener->on_writer_discovery(this, WriterDiscoveryInfo::CHANGED_QOS_WRITER, wdata.guid(), &wdata);
                }
                return false;
            }
        }

        // Get a writer proxy from the inactive pool (or create a new one if necessary and allowed)
        WriterProxy* wp = nullptr;
        if (matched_writers_pool_.empty())
        {
            size_t max_readers = matched_writers_pool_.max_size();
            if (getMatchedWritersSize() + matched_writers_pool_.size() < max_readers)
            {
                const RTPSParticipantAttributes& part_att = mp_RTPSParticipant->getRTPSParticipantAttributes();
                wp = new WriterProxy(this, part_att.allocation.locators, proxy_changes_config_);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTPS_READER, "Maximum number of reader proxies (" << max_readers << \
                        ") reached for writer " << m_guid);
                return false;
            }
        }
        else
        {
            wp = matched_writers_pool_.back();
            matched_writers_pool_.pop_back();
        }

        SequenceNumber_t initial_sequence;
        add_persistence_guid(wdata.guid(), wdata.persistence_guid());
        initial_sequence = get_last_notified(wdata.guid());

        wp->start(wdata, initial_sequence, is_datasharing);

        if (!is_same_process)
        {
            for (const Locator_t& locator : wp->remote_locators_shrinked())
            {
                getRTPSParticipant()->createSenderResources(locator);
            }
        }

        if (is_datasharing)
        {
            if (datasharing_listener_->add_datasharing_writer(wdata.guid(),
                    m_att.durabilityKind == VOLATILE,
                    mp_history->m_att.maximumReservedCaches))
            {
                matched_writers_.push_back(wp);
                EPROSIMA_LOG_INFO(RTPS_READER, "Writer Proxy " << wdata.guid() << " added to " << this->m_guid.entityId
                                                               << " with data sharing");
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_READER, "Failed to add Writer Proxy " << wdata.guid()
                                                                              << " to " << this->m_guid.entityId
                                                                              << " with data sharing.");
                {
                    // Release reader's lock to avoid deadlock when waiting for event (requiring mutex) to finish
                    guard.unlock();
                    assert(!guard.owns_lock());
                    wp->stop();
                    guard.lock();
                }
                matched_writers_pool_.push_back(wp);
                return false;
            }

            // Intraprocess manages durability itself
            if (VOLATILE == m_att.durabilityKind)
            {
                std::shared_ptr<ReaderPool> pool = datasharing_listener_->get_pool_for_writer(wp->guid());
                SequenceNumber_t last_seq = pool->get_last_read_sequence_number();
                if (SequenceNumber_t::unknown() != last_seq)
                {
                    SequenceNumberSet_t sns(last_seq + 1);
                    send_acknack(wp, sns, wp, false);
                    wp->lost_changes_update(last_seq + 1);
                }
            }
            else
            {
                // simulate a notification to force reading of transient changes
                datasharing_listener_->notify(false);
            }
        }
        else
        {
            matched_writers_.push_back(wp);
            EPROSIMA_LOG_INFO(RTPS_READER, "Writer Proxy " << wp->guid() << " added to " << m_guid.entityId);
        }
    }
    if (liveliness_lease_duration_ < c_TimeInfinite)
    {
        auto wlp = this->mp_RTPSParticipant->wlp();
        if ( wlp != nullptr)
        {
            wlp->sub_liveliness_manager_->add_writer(
                wdata.guid(),
                liveliness_kind_,
                liveliness_lease_duration_);
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS,
                    "Finite liveliness lease duration but WLP not enabled, cannot add writer");
        }
    }

    if (nullptr != listener)
    {
        listener->on_writer_discovery(this, WriterDiscoveryInfo::DISCOVERED_WRITER, wdata.guid(), &wdata);
    }

    return true;
}

bool StatefulReader::matched_writer_remove(
        const GUID_t& writer_guid,
        bool removed_by_lease)
{

    if (is_alive_ && liveliness_lease_duration_ < c_TimeInfinite)
    {
        auto wlp = this->mp_RTPSParticipant->wlp();
        if ( wlp != nullptr)
        {
            wlp->sub_liveliness_manager_->remove_writer(
                writer_guid,
                liveliness_kind_,
                liveliness_lease_duration_);
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS,
                    "Finite liveliness lease duration but WLP not enabled, cannot remove writer");
        }
    }

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    WriterProxy* wproxy = nullptr;
    if (is_alive_)
    {
        //Remove cachechanges belonging to the unmatched writer
        mp_history->writer_unmatched(writer_guid, get_last_notified(writer_guid));

        for (ResourceLimitedVector<WriterProxy*>::iterator it = matched_writers_.begin();
                it != matched_writers_.end();
                ++it)
        {
            if ((*it)->guid() == writer_guid)
            {
                EPROSIMA_LOG_INFO(RTPS_READER, "Writer proxy " << writer_guid << " removed from " << m_guid.entityId);
                wproxy = *it;
                matched_writers_.erase(it);

                break;
            }
        }

        if (wproxy != nullptr)
        {
            remove_persistence_guid(wproxy->guid(), wproxy->persistence_guid(), removed_by_lease);
            if (wproxy->is_datasharing_writer())
            {
                // If it is datasharing, it must be in the listener
                bool removed_from_listener = datasharing_listener_->remove_datasharing_writer(writer_guid);
                assert(removed_from_listener);
                (void)removed_from_listener;
                remove_changes_from(writer_guid, true);
            }
            {
                // Release reader's lock to avoid deadlock when waiting for event (requiring mutex) to finish
                lock.unlock();
                assert(!lock.owns_lock());
                wproxy->stop();
                lock.lock();
            }
            matched_writers_pool_.push_back(wproxy);
            if (nullptr != mp_listener)
            {
                // call the listener without the lock taken
                ReaderListener* listener = mp_listener;
                lock.unlock();
                listener->on_writer_discovery(this, WriterDiscoveryInfo::REMOVED_WRITER, writer_guid, nullptr);
            }
        }
        else
        {
            EPROSIMA_LOG_INFO(RTPS_READER,
                    "Writer Proxy " << writer_guid << " doesn't exist in reader " << this->getGuid().entityId);
        }
    }

    return (wproxy != nullptr);
}

bool StatefulReader::matched_writer_is_matched(
        const GUID_t& writer_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (is_alive_)
    {
        for (WriterProxy* it : matched_writers_)
        {
            if (it->guid() == writer_guid && it->is_alive())
            {
                return true;
            }
        }
    }

    return false;
}

bool StatefulReader::matched_writer_lookup(
        const GUID_t& writerGUID,
        WriterProxy** WP)
{
    assert(WP);

    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (!is_alive_)
    {
        return false;
    }

    bool returnedValue = findWriterProxy(writerGUID, WP);

    if (returnedValue)
    {
        EPROSIMA_LOG_INFO(RTPS_READER, this->getGuid().entityId << " FINDS writerProxy " << writerGUID << " from "
                                                                << getMatchedWritersSize());
    }
    else
    {
        EPROSIMA_LOG_INFO(RTPS_READER, this->getGuid().entityId << " NOT FINDS writerProxy " << writerGUID << " from "
                                                                << getMatchedWritersSize());
    }

    return returnedValue;
}

bool StatefulReader::findWriterProxy(
        const GUID_t& writerGUID,
        WriterProxy** WP) const
{
    assert(WP);

    for (WriterProxy* it : matched_writers_)
    {
        if (it->guid() == writerGUID && it->is_alive())
        {
            *WP = it;
            return true;
        }
    }
    return false;
}

void StatefulReader::assert_writer_liveliness(
        const GUID_t& writer)
{
    if (liveliness_lease_duration_ < c_TimeInfinite)
    {
        auto wlp = this->mp_RTPSParticipant->wlp();
        if (wlp != nullptr)
        {
            wlp->sub_liveliness_manager_->assert_liveliness(
                writer,
                liveliness_kind_,
                liveliness_lease_duration_);
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS, "Finite liveliness lease duration but WLP not enabled");
        }
    }
}

bool StatefulReader::processDataMsg(
        CacheChange_t* change)
{
    WriterProxy* pWP = nullptr;

    assert(change);

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    if (!is_alive_)
    {
        return false;
    }

    if (acceptMsgFrom(change->writerGUID, &pWP))
    {
        // Check if CacheChange was received or is framework data
        if (!pWP || !pWP->change_was_received(change->sequenceNumber))
        {
            // Always assert liveliness on scope exit
            auto assert_liveliness_lambda = [&lock, this, change](void*)
                    {
                        lock.unlock(); // Avoid deadlock with LivelinessManager.
                        assert_writer_liveliness(change->writerGUID);
                    };
            std::unique_ptr<void, decltype(assert_liveliness_lambda)> p{ this, assert_liveliness_lambda };

            EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                    IDSTRING "Trying to add change " << change->sequenceNumber << " TO reader: " << getGuid().entityId);

            size_t unknown_missing_changes_up_to = pWP ? pWP->unknown_missing_changes_up_to(change->sequenceNumber) : 0;
            bool will_never_be_accepted = false;
            if (!mp_history->can_change_be_added_nts(change->writerGUID, change->serializedPayload.length,
                    unknown_missing_changes_up_to, will_never_be_accepted))
            {
                if (will_never_be_accepted && pWP)
                {
                    pWP->irrelevant_change_set(change->sequenceNumber);
                    NotifyChanges(pWP);
                    send_ack_if_datasharing(this, mp_history, pWP, change->sequenceNumber);
                }
                return false;
            }

            if (data_filter_ && !data_filter_->is_relevant(*change, m_guid))
            {
                if (pWP)
                {
                    pWP->irrelevant_change_set(change->sequenceNumber);
                    NotifyChanges(pWP);
                    send_ack_if_datasharing(this, mp_history, pWP, change->sequenceNumber);
                }
                return true;
            }

            // Ask the pool for a cache change
            CacheChange_t* change_to_add = nullptr;
            if (!change_pool_->reserve_cache(change_to_add))
            {
                EPROSIMA_LOG_WARNING(RTPS_MSG_IN,
                        IDSTRING "Reached the maximum number of samples allowed by this reader's QoS. Rejecting change for reader: " <<
                        m_guid );
                return false;
            }

            // Copy metadata to reserved change
            change_to_add->copy_not_memcpy(change);

            // Ask payload pool to copy the payload
            IPayloadPool* payload_owner = change->payload_owner();

            if (is_datasharing_compatible_ && datasharing_listener_->writer_is_matched(change->writerGUID))
            {
                // We may receive the change from the listener (with owner a ReaderPool) or intraprocess (with owner a WriterPool)
                ReaderPool* datasharing_pool = dynamic_cast<ReaderPool*>(payload_owner);
                if (!datasharing_pool)
                {
                    datasharing_pool = datasharing_listener_->get_pool_for_writer(change->writerGUID).get();
                }
                if (!datasharing_pool)
                {
                    EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Problem copying DataSharing CacheChange from writer "
                            << change->writerGUID);
                    change_pool_->release_cache(change_to_add);
                    return false;
                }
                datasharing_pool->get_payload(change->serializedPayload, payload_owner, *change_to_add);
            }
            else if (payload_pool_->get_payload(change->serializedPayload, payload_owner, *change_to_add))
            {
                change->payload_owner(payload_owner);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Problem copying CacheChange, received data is: "
                        << change->serializedPayload.length << " bytes and max size in reader "
                        << m_guid << " is "
                        << (fixed_payload_size_ > 0 ? fixed_payload_size_ : std::numeric_limits<uint32_t>::max()));
                change_pool_->release_cache(change_to_add);
                return false;
            }

            // Perform reception of cache change
            if (!change_received(change_to_add, pWP, unknown_missing_changes_up_to))
            {
                EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                        IDSTRING "Change " << change_to_add->sequenceNumber << " not added to history");
                change_to_add->payload_owner()->release_payload(*change_to_add);
                change_pool_->release_cache(change_to_add);
                return false;
            }
        }

        return true;
    }

    return false;
}

bool StatefulReader::processDataFragMsg(
        CacheChange_t* incomingChange,
        uint32_t sampleSize,
        uint32_t fragmentStartingNum,
        uint16_t fragmentsInSubmessage)
{
    WriterProxy* pWP = nullptr;

    assert(incomingChange);

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    if (!is_alive_)
    {
        return false;
    }

    // TODO: see if we need manage framework fragmented DATA message
    if (acceptMsgFrom(incomingChange->writerGUID, &pWP) && pWP)
    {
        // Always assert liveliness on scope exit
        auto assert_liveliness_lambda = [&lock, this, incomingChange](void*)
                {
                    lock.unlock(); // Avoid deadlock with LivelinessManager.
                    assert_writer_liveliness(incomingChange->writerGUID);
                };
        std::unique_ptr<void, decltype(assert_liveliness_lambda)> p{ this, assert_liveliness_lambda };

        // Check if CacheChange was received.
        if (!pWP->change_was_received(incomingChange->sequenceNumber))
        {
            EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                    IDSTRING "Trying to add fragment " << incomingChange->sequenceNumber.to64long() << " TO reader: " <<
                    getGuid().entityId);

            size_t changes_up_to = pWP->unknown_missing_changes_up_to(incomingChange->sequenceNumber);
            bool will_never_be_accepted = false;
            if (!mp_history->can_change_be_added_nts(incomingChange->writerGUID, sampleSize, changes_up_to,
                    will_never_be_accepted))
            {
                if (will_never_be_accepted)
                {
                    pWP->irrelevant_change_set(incomingChange->sequenceNumber);
                    NotifyChanges(pWP);
                    send_ack_if_datasharing(this, mp_history, pWP, incomingChange->sequenceNumber);
                }
                return false;
            }

            CacheChange_t* change_to_add = incomingChange;

            CacheChange_t* change_created = nullptr;
            CacheChange_t* work_change = nullptr;
            if (!mp_history->get_change(change_to_add->sequenceNumber, change_to_add->writerGUID, &work_change))
            {
                // A new change should be reserved
                if (reserveCache(&work_change, sampleSize))
                {
                    if (work_change->serializedPayload.max_size < sampleSize)
                    {
                        releaseCache(work_change);
                        work_change = nullptr;
                    }
                    else
                    {
                        work_change->copy_not_memcpy(change_to_add);
                        work_change->serializedPayload.length = sampleSize;
                        work_change->setFragmentSize(change_to_add->getFragmentSize(), true);
                        change_created = work_change;
                    }
                }
            }

            if (work_change != nullptr)
            {
                work_change->add_fragments(change_to_add->serializedPayload, fragmentStartingNum,
                        fragmentsInSubmessage);
            }

            // If this is the first time we have received fragments for this change, add it to history
            if (change_created != nullptr)
            {
                if (!change_received(change_created, pWP, changes_up_to))
                {
                    EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                            IDSTRING "MessageReceiver not add change " << change_created->sequenceNumber.to64long());

                    releaseCache(change_created);
                    work_change = nullptr;
                }
            }

            // If change has been fully reassembled, mark as received and add notify user
            if (work_change != nullptr && work_change->is_fully_assembled())
            {
                fastdds::dds::SampleRejectedStatusKind rejection_reason;
                if (mp_history->completed_change(work_change, changes_up_to, rejection_reason))
                {
                    pWP->received_change_set(work_change->sequenceNumber);

                    // Temporarilly assign the inline qos while evaluating the data filter
                    work_change->inline_qos = incomingChange->inline_qos;
                    bool filtered_out = data_filter_ && !data_filter_->is_relevant(*work_change, m_guid);
                    work_change->inline_qos = SerializedPayload_t();

                    if (filtered_out)
                    {
                        mp_history->remove_change(work_change);
                    }

                    NotifyChanges(pWP);
                }
                else
                {
                    bool has_to_notify = false;
                    if (fastdds::dds::NOT_REJECTED != rejection_reason)
                    {
                        if (getListener())
                        {
                            getListener()->on_sample_rejected((RTPSReader*)this, rejection_reason, work_change);
                        }

                        /* Special case: rejected by REJECTED_BY_INSTANCES_LIMIT should never be received again.
                         */
                        if (fastdds::dds::REJECTED_BY_INSTANCES_LIMIT == rejection_reason)
                        {
                            pWP->irrelevant_change_set(work_change->sequenceNumber);
                            has_to_notify = true;
                        }
                    }

                    History::const_iterator chit = mp_history->find_change_nts(work_change);
                    if (chit != mp_history->changesEnd())
                    {
                        mp_history->remove_change_nts(chit);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(RTPS_READER, "Change should exist but didn't find it");
                    }

                    if (has_to_notify)
                    {
                        NotifyChanges(pWP);
                    }
                }
            }
        }
    }

    return true;
}

bool StatefulReader::processHeartbeatMsg(
        const GUID_t& writerGUID,
        uint32_t hbCount,
        const SequenceNumber_t& firstSN,
        const SequenceNumber_t& lastSN,
        bool finalFlag,
        bool livelinessFlag)
{
    WriterProxy* writer = nullptr;

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    if (!is_alive_)
    {
        return false;
    }

    if (acceptMsgFrom(writerGUID, &writer) && writer)
    {
        bool assert_liveliness = false;
        int32_t current_sample_lost = 0;
        if (writer->process_heartbeat(
                    hbCount, firstSN, lastSN, finalFlag, livelinessFlag, disable_positive_acks_, assert_liveliness,
                    current_sample_lost))
        {
            mp_history->remove_fragmented_changes_until(firstSN, writerGUID);

            if (0 < current_sample_lost)
            {
                if (getListener() != nullptr)
                {
                    getListener()->on_sample_lost((RTPSReader*)this, current_sample_lost);
                }
            }

            // Maybe now we have to notify user from new CacheChanges.
            NotifyChanges(writer);

            // Try to assert liveliness if requested by proxy's logic
            if (assert_liveliness)
            {
                if (liveliness_lease_duration_ < c_TimeInfinite)
                {
                    if (liveliness_kind_ == MANUAL_BY_TOPIC_LIVELINESS_QOS ||
                            writer->liveliness_kind() == MANUAL_BY_TOPIC_LIVELINESS_QOS)
                    {
                        auto wlp = this->mp_RTPSParticipant->wlp();
                        if ( wlp != nullptr)
                        {
                            lock.unlock(); // Avoid deadlock with LivelinessManager.
                            wlp->sub_liveliness_manager_->assert_liveliness(
                                writerGUID,
                                liveliness_kind_,
                                liveliness_lease_duration_);
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS, "Finite liveliness lease duration but WLP not enabled");
                        }
                    }
                }
            }
        }

        return true;
    }

    return false;
}

bool StatefulReader::processGapMsg(
        const GUID_t& writerGUID,
        const SequenceNumber_t& gapStart,
        const SequenceNumberSet_t& gapList)
{
    WriterProxy* pWP = nullptr;

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    if (!is_alive_ || gapStart < SequenceNumber_t(0, 1) || gapList.base() <= gapStart)
    {
        return false;
    }

    if (acceptMsgFrom(writerGUID, &pWP) && pWP)
    {
        // TODO (Miguel C): Refactor this inside WriterProxy
        SequenceNumber_t auxSN;
        SequenceNumber_t finalSN = gapList.base();
        History::const_iterator history_iterator = mp_history->changesBegin();
        for (auxSN = gapStart; auxSN < finalSN; auxSN++)
        {
            if (pWP->irrelevant_change_set(auxSN))
            {
                CacheChange_t* to_remove = nullptr;
                auto ret_iterator = findCacheInFragmentedProcess(auxSN, pWP->guid(), &to_remove, history_iterator);
                if (to_remove != nullptr)
                {
                    // we called the History version to avoid callbacks
                    history_iterator = mp_history->History::remove_change_nts(ret_iterator);
                }
                else if (ret_iterator != mp_history->changesEnd())
                {
                    history_iterator = ret_iterator;
                }
            }
        }

        gapList.for_each(
            [&](SequenceNumber_t it)
            {
                if (pWP->irrelevant_change_set(it))
                {
                    CacheChange_t* to_remove = nullptr;
                    auto ret_iterator =
                    findCacheInFragmentedProcess(auxSN, pWP->guid(), &to_remove, history_iterator);
                    if (to_remove != nullptr)
                    {
                        // we called the History version to avoid callbacks
                        history_iterator = mp_history->History::remove_change_nts(ret_iterator);
                    }
                    else if (ret_iterator != mp_history->changesEnd())
                    {
                        history_iterator = ret_iterator;
                    }
                }
            });

        // Maybe now we have to notify user from new CacheChanges.
        NotifyChanges(pWP);

        return true;
    }

    return false;
}

bool StatefulReader::acceptMsgFrom(
        const GUID_t& writerId,
        WriterProxy** wp) const
{
    assert(wp != nullptr);

    for (WriterProxy* it : matched_writers_)
    {
        if (it->guid() == writerId && it->is_alive())
        {
            *wp = it;
            return true;
        }
    }

    // Check if it's a framework's one. In this case, m_acceptMessagesFromUnkownWriters
    // is an enabler for the trusted entity comparison
    if (m_acceptMessagesFromUnkownWriters
            && (writerId.entityId == m_trustedWriterEntityId))
    {
        *wp = nullptr;
        return true;
    }

    return false;
}

bool StatefulReader::change_removed_by_history(
        CacheChange_t* a_change,
        WriterProxy* wp)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (is_alive_)
    {
        if (a_change->is_fully_assembled())
        {
            if (!a_change->isRead &&
                    get_last_notified(a_change->writerGUID) >= a_change->sequenceNumber)
            {
                if (0 < total_unread_)
                {
                    --total_unread_;
                }
            }

            WriterProxy* proxy = wp;

            if (nullptr == proxy)
            {
                if (!findWriterProxy(a_change->writerGUID, &proxy))
                {
                    return false;
                }
            }

            if (nullptr != proxy)
            {
                send_ack_if_datasharing(this, mp_history, proxy, a_change->sequenceNumber);
            }
        }
        else
        {
            /* A not fully assembled fragmented sample may be removed when receiving a newer sample and KEEP_LAST
             * policy. The WriterProxy should consider it as irrelevant to avoid an infinite loop asking for it.
             */
            WriterProxy* proxy = wp;

            if (nullptr == proxy)
            {
                if (!findWriterProxy(a_change->writerGUID, &proxy))
                {
                    return false;
                }

                proxy->irrelevant_change_set(a_change->sequenceNumber);
                send_ack_if_datasharing(this, mp_history, proxy, a_change->sequenceNumber);
            }

        }

        return true;
    }

    //Simulate a datasharing notification to process any pending payloads that were waiting due to full history
    if (is_datasharing_compatible_)
    {
        datasharing_listener_->notify(false);
    }

    return false;
}

bool StatefulReader::change_received(
        CacheChange_t* a_change,
        WriterProxy* prox,
        size_t unknown_missing_changes_up_to)
{
    //First look for WriterProxy in case is not provided
    if (prox == nullptr)
    {
        if (!findWriterProxy(a_change->writerGUID, &prox))
        {
            // discard non framework messages from unknown writer
            if (a_change->writerGUID.entityId != m_trustedWriterEntityId)
            {
                EPROSIMA_LOG_INFO(RTPS_READER,
                        "Writer Proxy " << a_change->writerGUID << " not matched to this Reader " << m_guid.entityId);
                return false;
            }
            else if (a_change->kind != eprosima::fastrtps::rtps::ChangeKind_t::ALIVE)
            {
                EPROSIMA_LOG_INFO(RTPS_READER, "Not alive change " << a_change->writerGUID << " has not WriterProxy");
                return false;
            }
            else
            {
                // handle framework messages in a stateless fashion
                // Only make visible the change if there is not other with bigger sequence number.
                if (get_last_notified(a_change->writerGUID) < a_change->sequenceNumber)
                {
                    if (mp_history->received_change(a_change, 0))
                    {
                        Time_t::now(a_change->reader_info.receptionTimestamp);

                        // If we use the real a_change->sequenceNumber no DATA(p) with a lower one will ever be received.
                        // That happens because the WriterProxy created when the listener matches the PDP endpoints is
                        // initialized using this SequenceNumber_t. Note that on a SERVER the own DATA(p) may be in any
                        // position within the WriterHistory preventing effective data exchange.
                        update_last_notified(a_change->writerGUID, SequenceNumber_t(0, 1));
                        auto listener = getListener();
                        if (listener != nullptr)
                        {
                            bool notify_single = false;
                            auto guid = a_change->writerGUID;
                            auto seq = a_change->sequenceNumber;
                            listener->on_data_available(this, guid, seq, seq, notify_single);
                            if (notify_single)
                            {
                                listener->onNewCacheChangeAdded(this, a_change);
                            }
                        }

                        return true;
                    }
                }

                EPROSIMA_LOG_INFO(RTPS_READER, "Change received from " << a_change->writerGUID << " with sequence number: "
                                                                       << a_change->sequenceNumber <<
                        " skipped. Higher sequence numbers have been received.");
                return false;
            }
        }
        else
        {
            unknown_missing_changes_up_to = prox->unknown_missing_changes_up_to(a_change->sequenceNumber);
        }
    }

    // Update Ownership strength.
    if (EXCLUSIVE_OWNERSHIP_QOS == m_att.ownershipKind)
    {
        a_change->reader_info.writer_ownership_strength = prox->ownership_strength();
    }
    else
    {
        a_change->reader_info.writer_ownership_strength = std::numeric_limits<uint32_t>::max();
    }

    // NOTE: Depending on QoS settings, one change can be removed from history
    // inside the call to mp_history->received_change
    fastdds::dds::SampleRejectedStatusKind rejection_reason;
    if (mp_history->received_change(a_change, unknown_missing_changes_up_to, rejection_reason))
    {
        auto payload_length = a_change->serializedPayload.length;

        bool ret = true;

        if (a_change->is_fully_assembled())
        {
            ret = prox->received_change_set(a_change->sequenceNumber);
        }
        else
        {
            /* Search if the first fragment was stored, because it may have been discarded due to being older and KEEP_LAST
             * policy. In this case this samples should be set as irrelevant.
             */
            if (mp_history->changesEnd() == mp_history->find_change(a_change))
            {
                prox->irrelevant_change_set(a_change->sequenceNumber);
                send_ack_if_datasharing(this, mp_history, prox, a_change->sequenceNumber);
                ret = false;

            }
        }

        Time_t::now(a_change->reader_info.receptionTimestamp);

        // WARNING! This method could destroy a_change
        NotifyChanges(prox);

        // statistics callback
        on_subscribe_throughput(payload_length);

        return ret;
    }
    else
    {
        if (fastdds::dds::NOT_REJECTED != rejection_reason)
        {
            if (getListener() && (a_change->is_fully_assembled() || (a_change->contains_first_fragment())))
            {
                getListener()->on_sample_rejected((RTPSReader*)this, rejection_reason, a_change);
            }

            /* Special case: rejected by REJECTED_BY_INSTANCES_LIMIT should never be received again.
             */
            if (fastdds::dds::REJECTED_BY_INSTANCES_LIMIT == rejection_reason)
            {
                prox->irrelevant_change_set(a_change->sequenceNumber);
                NotifyChanges(prox);
            }
        }
    }

    return false;
}

void StatefulReader::NotifyChanges(
        WriterProxy* prox)
{
    CacheChange_t* aux_ch = nullptr;
    GUID_t proxGUID = prox->guid();
    SequenceNumber_t max_seq = prox->available_changes_max();
    SequenceNumber_t first_seq = prox->next_cache_change_to_be_notified();

    bool new_data_available = false;

    // Update state before notifying
    update_last_notified(proxGUID, max_seq);
    History::const_iterator it = mp_history->changesBegin();
    SequenceNumber_t next_seq = first_seq;
    while (next_seq != c_SequenceNumber_Unknown &&
            mp_history->changesEnd() != (it = mp_history->get_change_nts(next_seq, proxGUID, &aux_ch, it)) &&
            (*it)->sequenceNumber <= max_seq)
    {
        aux_ch = *it;
        assert(false == aux_ch->isRead);
        new_data_available = true;
        ++total_unread_;
        on_data_notify(proxGUID, aux_ch->sourceTimestamp);

        ++it;
        do
        {
            next_seq = prox->next_cache_change_to_be_notified();
        } while (next_seq != c_SequenceNumber_Unknown && next_seq <= aux_ch->sequenceNumber);
    }
    // Ensure correct state of proxy when max_seq is not present in history
    while (c_SequenceNumber_Unknown != prox->next_cache_change_to_be_notified())
    {
    }

    // Notify listener if new data is available
    auto listener = getListener();
    if (new_data_available && (nullptr != listener))
    {
        bool notify_individual = false;
        listener->on_data_available(this, proxGUID, first_seq, max_seq, notify_individual);

        if (notify_individual)
        {
            it = mp_history->changesBegin();
            next_seq = first_seq;
            while (next_seq <= max_seq &&
                    mp_history->changesEnd() != (it = mp_history->get_change_nts(next_seq, proxGUID, &aux_ch, it)) &&
                    (*it)->sequenceNumber <= max_seq)
            {
                aux_ch = *it;
                next_seq = aux_ch->sequenceNumber + 1;
                listener->onNewCacheChangeAdded(this, aux_ch);

                // Reset the iterator to the beginning, since it may be invalidated inside the callback
                it = mp_history->changesBegin();
            }
        }
    }

    // Notify in case someone is waiting for unread messages
    if (new_data_available)
    {
        new_notification_cv_.notify_all();
    }
}

void StatefulReader::remove_changes_from(
        const GUID_t& writerGUID,
        bool is_payload_pool_lost)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    std::vector<CacheChange_t*> toremove;
    for (std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
            it != mp_history->changesEnd(); ++it)
    {
        if ((*it)->writerGUID == writerGUID)
        {
            toremove.push_back((*it));
        }
    }

    for (std::vector<CacheChange_t*>::iterator it = toremove.begin();
            it != toremove.end(); ++it)
    {
        EPROSIMA_LOG_INFO(RTPS_READER,
                "Removing change " << (*it)->sequenceNumber << " from " << (*it)->writerGUID);
        if (is_payload_pool_lost)
        {
            (*it)->serializedPayload.data = nullptr;
            (*it)->payload_owner(nullptr);
        }
        mp_history->remove_change(*it);
    }
}

ResourceEvent& StatefulReader::getEventResource() const
{
    return mp_RTPSParticipant->getEventResource();
}

bool StatefulReader::nextUntakenCache(
        CacheChange_t** change,
        WriterProxy** wpout)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (!is_alive_)
    {
        return false;
    }

    bool takeok = false;
    WriterProxy* wp;
    std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
    while (it != mp_history->changesEnd())
    {
        if (this->matched_writer_lookup((*it)->writerGUID, &wp))
        {
            // TODO Revisar la comprobacion
            SequenceNumber_t seq;
            seq = wp->available_changes_max();
            if (seq < (*it)->sequenceNumber)
            {
                ++it;
                continue;
            }
            takeok = true;
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_READER,
                    "Removing change " << (*it)->sequenceNumber << " from " << (*it)->writerGUID <<
                    " because is no longer paired");
            it = mp_history->remove_change(it);
        }

        if (takeok)
        {

            *change = *it;

            if (wpout != nullptr)
            {
                *wpout = wp;
            }

            break;
        }
    }

    return takeok;
}

// TODO Porque elimina aqui y no cuando hay unpairing
bool StatefulReader::nextUnreadCache(
        CacheChange_t** change,
        WriterProxy** wpout)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (!is_alive_)
    {
        return false;
    }

    std::vector<CacheChange_t*> toremove;
    bool readok = false;
    WriterProxy* wp = nullptr;
    std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
    while (it != mp_history->changesEnd())
    {
        if ((*it)->isRead)
        {
            ++it;
            continue;
        }

        if (matched_writer_lookup((*it)->writerGUID, &wp))
        {
            SequenceNumber_t seq;
            seq = wp->available_changes_max();
            if (seq < (*it)->sequenceNumber)
            {
                ++it;
                continue;
            }
            readok = true;
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_READER,
                    "Removing change " << (*it)->sequenceNumber << " from " << (*it)->writerGUID <<
                    " because is no longer paired");
            it = mp_history->remove_change(it);
            continue;
        }

        if (readok)
        {

            *change = *it;

            if (wpout != nullptr)
            {
                *wpout = wp;
            }

            break;
        }
    }

    return readok;
}

bool StatefulReader::updateTimes(
        const ReaderTimes& ti)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (is_alive_)
    {
        if (times_.heartbeatResponseDelay != ti.heartbeatResponseDelay)
        {
            times_ = ti;
            for (WriterProxy* writer : matched_writers_)
            {
                writer->update_heartbeat_response_interval(times_.heartbeatResponseDelay);
            }
        }
    }
    return true;
}

bool StatefulReader::isInCleanState()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    if (is_alive_)
    {
        for (WriterProxy* wp : matched_writers_)
        {
            if (wp->number_of_changes_from_writer() != 0)
            {
                return false;
            }
        }
    }

    return true;
}

bool StatefulReader::begin_sample_access_nts(
        CacheChange_t* change,
        WriterProxy*& wp,
        bool& is_future_change)
{
    const GUID_t& writer_guid = change->writerGUID;
    is_future_change = false;

    if (matched_writer_lookup(writer_guid, &wp))
    {
        SequenceNumber_t seq;
        seq = wp->available_changes_max();
        if (seq < change->sequenceNumber)
        {
            is_future_change = true;
        }
    }

    return true;
}

void StatefulReader::end_sample_access_nts(
        CacheChange_t* change,
        WriterProxy*& wp,
        bool mark_as_read)
{
    change_read_by_user(change, wp, mark_as_read);
}

void StatefulReader::change_read_by_user(
        CacheChange_t* change,
        WriterProxy* writer,
        bool mark_as_read)
{
    assert(!writer || change->writerGUID == writer->guid());

    // Mark change as read
    if (mark_as_read && !change->isRead)
    {
        change->isRead = true;
        if (0 < total_unread_)
        {
            --total_unread_;
        }
    }

    if (mark_as_read)
    {
        send_ack_if_datasharing(this, mp_history, writer, change->sequenceNumber);
    }
}

void StatefulReader::send_acknack(
        const WriterProxy* writer,
        const SequenceNumberSet_t& sns,
        RTPSMessageSenderInterface* sender,
        bool is_final)
{

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    if (!writer->is_alive())
    {
        return;
    }

    if (writer->is_on_same_process())
    {
        return;
    }

    acknack_count_++;


    EPROSIMA_LOG_INFO(RTPS_READER, "Sending ACKNACK: " << sns);

    RTPSMessageGroup group(getRTPSParticipant(), this, sender);
    group.add_acknack(sns, acknack_count_, is_final);
}

void StatefulReader::send_acknack(
        const WriterProxy* writer,
        RTPSMessageSenderInterface* sender,
        bool heartbeat_was_final)
{
    // Protect reader
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (!writer->is_alive())
    {
        return;
    }

    // ACKNACK for datasharing writers is done for changes with status READ, not on reception
    // This is handled in change_read_by_user
    if (writer->is_datasharing_writer())
    {
        return;
    }

    SequenceNumberSet_t missing_changes = writer->missing_changes();

    try
    {
        RTPSMessageGroup group(getRTPSParticipant(), this, sender);
        if (!missing_changes.empty() || !heartbeat_was_final)
        {
            GUID_t guid = sender->remote_guids().at(0);
            SequenceNumberSet_t sns(writer->available_changes_max() + 1);
            History::const_iterator history_iterator = mp_history->changesBegin();

            missing_changes.for_each(
                [&](const SequenceNumber_t& seq)
                {
                    // Check if the CacheChange_t is uncompleted.
                    CacheChange_t* uncomplete_change = nullptr;
                    auto ret_iterator = findCacheInFragmentedProcess(seq, guid, &uncomplete_change, history_iterator);
                    if (ret_iterator != mp_history->changesEnd())
                    {
                        history_iterator = ret_iterator;
                    }
                    if (uncomplete_change == nullptr)
                    {
                        if (!sns.add(seq))
                        {
                            EPROSIMA_LOG_INFO(RTPS_READER, "Sequence number " << seq
                                                                              <<
                                " exceeded bitmap limit of AckNack. SeqNumSet Base: "
                                                                              << sns.base());
                        }
                    }
                    else
                    {
                        FragmentNumberSet_t frag_sns;
                        uncomplete_change->get_missing_fragments(frag_sns);
                        ++nackfrag_count_;
                        EPROSIMA_LOG_INFO(RTPS_READER, "Sending NACKFRAG for sample" << seq << ": " << frag_sns; );

                        group.add_nackfrag(seq, frag_sns, nackfrag_count_);
                    }

                });

            acknack_count_++;
            EPROSIMA_LOG_INFO(RTPS_READER, "Sending ACKNACK: " << sns; );

            bool final = sns.empty();
            group.add_acknack(sns, acknack_count_, final);
        }
    }
    catch (const RTPSMessageGroup::timeout&)
    {
        EPROSIMA_LOG_ERROR(RTPS_READER, "Max blocking time reached");
    }
}

bool StatefulReader::send_sync_nts(
        CDRMessage_t* message,
        const Locators& locators_begin,
        const Locators& locators_end,
        std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    return mp_RTPSParticipant->sendSync(message, m_guid, locators_begin, locators_end, max_blocking_time_point);
}
