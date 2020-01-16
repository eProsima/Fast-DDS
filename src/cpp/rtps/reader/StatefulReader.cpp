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
#include <fastrtps/log/Log.h>
#include <fastdds/rtps/messages/RTPSMessageCreator.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/reader/WriterProxy.h>
#include <fastrtps/utils/TimeConversion.h>
#include <rtps/history/HistoryAttributesExtension.hpp>

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>
#include <fastdds/rtps/writer/LivelinessManager.h>

#include "rtps/RTPSDomainImpl.hpp"

#include <mutex>
#include <thread>

#include <cassert>

#define IDSTRING "(ID:" << std::this_thread::get_id() << ") " <<

using namespace eprosima::fastrtps::rtps;

StatefulReader::~StatefulReader()
{
    logInfo(RTPS_READER, "StatefulReader destructor.");

    // Only is_alive_ assignment needs to be protected, as
    // matched_writers_ and matched_writers_pool_ are only used
    // when is_alive_ is true
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        is_alive_ = false;
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

    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (!is_alive_)
    {
        return false;
    }

    bool is_same_process = RTPSDomainImpl::should_intraprocess_between(m_guid, wdata.guid());

    for (WriterProxy* it : matched_writers_)
    {
        if (it->guid() == wdata.guid())
        {
            logInfo(RTPS_READER, "Attempting to add existing writer, updating information");
            it->update(wdata);
            if (!is_same_process)
            {
                for (const Locator_t& locator : it->remote_locators_shrinked())
                {
                    getRTPSParticipant()->createSenderResources(locator);
                }
            }
            return false;
        }
    }

    // Get a writer proxy from the inactive pool (or create a new one if necessary and allowed)
    WriterProxy* wp = nullptr;
    if (matched_writers_pool_.empty())
    {
        size_t max_readers = matched_writers_pool_.max_size();
        if (matched_writers_.size() + matched_writers_pool_.size() < max_readers)
        {
            const RTPSParticipantAttributes& part_att = mp_RTPSParticipant->getRTPSParticipantAttributes();
            wp = new WriterProxy(this, part_att.allocation.locators, proxy_changes_config_);
        }
        else
        {
            logWarning(RTPS_WRITER, "Maximum number of reader proxies (" << max_readers << \
                    ") reached for writer " << m_guid << endl);
            return false;
        }
    }
    else
    {
        wp = matched_writers_pool_.back();
        matched_writers_pool_.pop_back();
    }

    if (!is_same_process)
    {
        for (const Locator_t& locator : wp->remote_locators_shrinked())
        {
            getRTPSParticipant()->createSenderResources(locator);
        }
    }

    SequenceNumber_t initial_sequence;
    add_persistence_guid(wdata.guid(), wdata.persistence_guid());
    initial_sequence = get_last_notified(wdata.guid());

    wp->start(wdata, initial_sequence);

    matched_writers_.push_back(wp);

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
            logError(RTPS_LIVELINESS, "Finite liveliness lease duration but WLP not enabled, cannot add writer");
        }
    }

    logInfo(RTPS_READER, "Writer Proxy " << wp->guid() << " added to " << m_guid.entityId);
    return true;
}

bool StatefulReader::matched_writer_remove(
        const GUID_t& writer_guid)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    if (is_alive_)
    {
        WriterProxy* wproxy = nullptr;

        //Remove cachechanges belonging to the unmatched writer
        mp_history->remove_changes_with_guid(writer_guid);

        for (ResourceLimitedVector<WriterProxy*>::iterator it = matched_writers_.begin(); it != matched_writers_.end();
                ++it)
        {
            if ((*it)->guid() == writer_guid)
            {
                logInfo(RTPS_READER, "Writer proxy " << writer_guid << " removed from " << m_guid.entityId);

                if (liveliness_lease_duration_ < c_TimeInfinite)
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
                        logError(RTPS_LIVELINESS,
                                "Finite liveliness lease duration but WLP not enabled, cannot remove writer");
                    }
                }

                wproxy = *it;
                matched_writers_.erase(it);
                remove_persistence_guid(wproxy->guid(), wproxy->attributes().persistence_guid());
                break;
            }
        }

        if (wproxy != nullptr)
        {
            wproxy->stop();
            matched_writers_pool_.push_back(wproxy);
            return true;
        }

        logInfo(RTPS_READER, "Writer Proxy " << writer_guid << " doesn't exist in reader " << this->getGuid().entityId);
    }
    return false;
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
        logInfo(RTPS_READER, this->getGuid().entityId << " FINDS writerProxy " << writerGUID << " from "
                                                      << matched_writers_.size());
    }
    else
    {
        logInfo(RTPS_READER, this->getGuid().entityId << " NOT FINDS writerProxy " << writerGUID << " from "
                                                      << matched_writers_.size());
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

bool StatefulReader::processDataMsg(
        CacheChange_t* change)
{
    WriterProxy* pWP = nullptr;

    assert(change);

    std::lock_guard<RecursiveTimedMutex> lock(mp_mutex);
    if (!is_alive_)
    {
        return false;
    }

    if (acceptMsgFrom(change->writerGUID, &pWP))
    {
        if (liveliness_lease_duration_ < c_TimeInfinite)
        {
            if (liveliness_kind_ == MANUAL_BY_TOPIC_LIVELINESS_QOS ||
                    pWP->attributes().m_qos.m_liveliness.kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
            {
                auto wlp = this->mp_RTPSParticipant->wlp();
                if (wlp != nullptr)
                {
                    wlp->sub_liveliness_manager_->assert_liveliness(
                        change->writerGUID,
                        liveliness_kind_,
                        liveliness_lease_duration_);
                }
                else
                {
                    logError(RTPS_LIVELINESS, "Finite liveliness lease duration but WLP not enabled");
                }
            }
        }

        // Check if CacheChange was received or is framework data
        if (!pWP || !pWP->change_was_received(change->sequenceNumber))
        {
            logInfo(RTPS_MSG_IN,
                    IDSTRING "Trying to add change " << change->sequenceNumber << " TO reader: " << getGuid().entityId);

            CacheChange_t* change_to_add;

            if (reserveCache(&change_to_add, change->serializedPayload.length)) //Reserve a new cache from the corresponding cache pool
            {
#if HAVE_SECURITY
                if (getAttributes().security_attributes().is_payload_protected)
                {
                    change_to_add->copy_not_memcpy(change);
                    if (!getRTPSParticipant()->security_manager().decode_serialized_payload(change->serializedPayload,
                            change_to_add->serializedPayload, m_guid, change->writerGUID))
                    {
                        releaseCache(change_to_add);
                        logWarning(RTPS_MSG_IN, "Cannont decode serialized payload");
                        return false;
                    }
                }
                else
                {
#endif
                if (!change_to_add->copy(change))
                {
                    logWarning(RTPS_MSG_IN, IDSTRING "Problem copying CacheChange, received data is: " << change->serializedPayload.length
                                                                                                       << " bytes and max size in reader " << getGuid().entityId << " is " <<
                            change_to_add->serializedPayload.max_size);
                    releaseCache(change_to_add);
                    return false;
                }
#if HAVE_SECURITY
            }
#endif
            }
            else
            {
                logError(RTPS_MSG_IN, IDSTRING "Problem reserving CacheChange in reader: " << getGuid().entityId);
                return false;
            }

            if (!change_received(change_to_add, pWP))
            {
                logInfo(RTPS_MSG_IN, IDSTRING "MessageReceiver not add change " << change_to_add->sequenceNumber);
                releaseCache(change_to_add);
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
        if (liveliness_lease_duration_ < c_TimeInfinite)
        {
            if (liveliness_kind_ == MANUAL_BY_TOPIC_LIVELINESS_QOS ||
                    pWP->attributes().m_qos.m_liveliness.kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
            {
                auto wlp = this->mp_RTPSParticipant->wlp();
                if ( wlp != nullptr)
                {
                    wlp->sub_liveliness_manager_->assert_liveliness(
                        incomingChange->writerGUID,
                        liveliness_kind_,
                        liveliness_lease_duration_);
                }
                else
                {
                    logError(RTPS_LIVELINESS, "Finite liveliness lease duration but WLP not enabled");
                }
            }
        }

        // Check if CacheChange was received.
        if (!pWP->change_was_received(incomingChange->sequenceNumber))
        {
            logInfo(RTPS_MSG_IN,
                    IDSTRING "Trying to add fragment " << incomingChange->sequenceNumber.to64long() << " TO reader: " <<
                    getGuid().entityId);

            CacheChange_t* change_to_add = incomingChange;

#if HAVE_SECURITY
            if (getAttributes().security_attributes().is_payload_protected)
            {
                if (reserveCache(&change_to_add, incomingChange->serializedPayload.length)) //Reserve a new cache from the corresponding cache pool
                {
                    change_to_add->copy_not_memcpy(incomingChange);
                    if (!getRTPSParticipant()->security_manager().decode_serialized_payload(incomingChange->
                            serializedPayload,
                            change_to_add->serializedPayload, m_guid, incomingChange->writerGUID))
                    {
                        releaseCache(change_to_add);
                        logWarning(RTPS_MSG_IN, "Cannont decode serialized payload");
                        return false;
                    }
                }
            }
#endif

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

#if HAVE_SECURITY
            if (getAttributes().security_attributes().is_payload_protected)
            {
                releaseCache(change_to_add);
            }
#endif

            // If this is the first time we have received fragments for this change, add it to history
            if (change_created != nullptr)
            {
                if (!change_received(change_created, pWP))
                {

                    logInfo(RTPS_MSG_IN,
                            IDSTRING "MessageReceiver not add change " << change_created->sequenceNumber.to64long());

                    releaseCache(change_created);
                    work_change = nullptr;
                }
            }

            // If change has been fully reassembled, mark as received and add notify user
            if (work_change != nullptr && work_change->is_fully_assembled())
            {
                pWP->received_change_set(work_change->sequenceNumber);
                NotifyChanges(pWP);
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
        new_lost_sample(firstSN, writer->get_low_mark());
        if (writer->process_heartbeat(
                    hbCount, firstSN, lastSN, finalFlag, livelinessFlag, disable_positive_acks_, assert_liveliness))
        {
            mp_history->remove_fragmented_changes_until(firstSN, writerGUID);

            // Try to assert liveliness if requested by proxy's logic
            if (assert_liveliness)
            {
                if (liveliness_lease_duration_ < c_TimeInfinite)
                {
                    if (liveliness_kind_ == MANUAL_BY_TOPIC_LIVELINESS_QOS ||
                            writer->attributes().m_qos.m_liveliness.kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
                    {
                        auto wlp = this->mp_RTPSParticipant->wlp();
                        if ( wlp != nullptr)
                        {
                            wlp->sub_liveliness_manager_->assert_liveliness(
                                writerGUID,
                                liveliness_kind_,
                                liveliness_lease_duration_);
                        }
                        else
                        {
                            logError(RTPS_LIVELINESS, "Finite liveliness lease duration but WLP not enabled");
                        }
                    }
                }
            }

            // Maybe now we have to notify user from new CacheChanges.
            NotifyChanges(writer);
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
    if (!is_alive_)
    {
        return false;
    }

    if (acceptMsgFrom(writerGUID, &pWP) && pWP)
    {
        // TODO (Miguel C): Refactor this inside WriterProxy
        SequenceNumber_t auxSN;
        SequenceNumber_t finalSN = gapList.base() - 1;
        History::const_iterator history_iterator = mp_history->changesBegin();
        for (auxSN = gapStart; auxSN <= finalSN; auxSN++)
        {
            if (pWP->irrelevant_change_set(auxSN))
            {
                CacheChange_t* to_remove = nullptr;
                auto ret_iterator = findCacheInFragmentedProcess(auxSN, pWP->guid(), &to_remove, history_iterator);
                if (to_remove != nullptr)
                {
                    history_iterator = mp_history->remove_change_nts(to_remove, ret_iterator);
                }
                else if (ret_iterator != mp_history->changesEnd())
                {
                    history_iterator = ret_iterator;
                }
            }
        }

        gapList.for_each([&](SequenceNumber_t it)
        {
            if (pWP->irrelevant_change_set(it))
            {
                CacheChange_t* to_remove = nullptr;
                auto ret_iterator = findCacheInFragmentedProcess(auxSN, pWP->guid(), &to_remove, history_iterator);
                if (to_remove != nullptr)
                {
                    history_iterator = mp_history->remove_change_nts(to_remove, ret_iterator);
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
        if (wp != nullptr || matched_writer_lookup(a_change->writerGUID, &wp))
        {
            if (a_change->is_fully_assembled())
            {
                if (!a_change->isRead && wp->available_changes_max() >= a_change->sequenceNumber)
                {
                    if (0 < total_unread_)
                    {
                        --total_unread_;
                    }
                }


                wp->change_removed_from_history(a_change->sequenceNumber);
            }
            return true;
        }
        else if (a_change->writerGUID.entityId != this->m_trustedWriterEntityId)
        {
            // trusted entities messages mean no havoc
            logError(RTPS_READER, " You should always find the WP associated with a change, something is very wrong");
        }
    }

    return false;
}

bool StatefulReader::change_received(
        CacheChange_t* a_change,
        WriterProxy* prox)
{
    //First look for WriterProxy in case is not provided
    if (prox == nullptr)
    {
        if (!findWriterProxy(a_change->writerGUID, &prox))
        {
            // discard non framework messages from unknown writer
            if (a_change->writerGUID.entityId != m_trustedWriterEntityId)
            {
                logInfo(RTPS_READER,
                        "Writer Proxy " << a_change->writerGUID << " not matched to this Reader " << m_guid.entityId);
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
                        update_last_notified(a_change->writerGUID, a_change->sequenceNumber);
                        if (getListener() != nullptr)
                        {
                            getListener()->onNewCacheChangeAdded((RTPSReader*)this, a_change);
                        }

                        return true;
                    }
                }

                return false;
            }
        }
    }

    // TODO (Miguel C): Refactor this inside WriterProxy
    size_t unknown_missing_changes_up_to = prox->unknown_missing_changes_up_to(a_change->sequenceNumber);

    // NOTE: Depending on QoS settings, one change can be removed from history
    // inside the call to mp_history->received_change
    if (mp_history->received_change(a_change, unknown_missing_changes_up_to))
    {
        GUID_t proxGUID = prox->guid();

        // If KEEP_LAST and history full, make older changes as lost.
        CacheChange_t* aux_change = nullptr;
        if (mp_history->isFull() && mp_history->get_min_change_from(&aux_change, proxGUID))
        {
            prox->lost_changes_update(aux_change->sequenceNumber);
        }

        bool ret = true;

        if (a_change->is_fully_assembled())
        {
            ret = prox->received_change_set(a_change->sequenceNumber);
        }

        NotifyChanges(prox);

        return ret;
    }

    return false;
}

void StatefulReader::NotifyChanges(
        WriterProxy* prox)
{
    GUID_t proxGUID = prox->guid();
    update_last_notified(proxGUID, prox->available_changes_max());
    SequenceNumber_t nextChangeToNotify = prox->next_cache_change_to_be_notified();
    while (nextChangeToNotify != SequenceNumber_t::unknown())
    {
        CacheChange_t* ch_to_give = nullptr;

        if (mp_history->get_change(nextChangeToNotify, proxGUID, &ch_to_give))
        {
            if (!ch_to_give->isRead)
            {
                ++total_unread_;

                if (getListener() != nullptr)
                {
                    getListener()->onNewCacheChangeAdded((RTPSReader*)this, ch_to_give);
                }

                new_notification_cv_.notify_all();
            }
        }

        // Search again the WriterProxy because could be removed after the unlock.
        if (!findWriterProxy(proxGUID, &prox))
        {
            break;
        }

        nextChangeToNotify = prox->next_cache_change_to_be_notified();
    }
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

    std::vector<CacheChange_t*> toremove;
    bool takeok = false;
    for (std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
            it != mp_history->changesEnd(); ++it)
    {
        WriterProxy* wp;
        if (this->matched_writer_lookup((*it)->writerGUID, &wp))
        {
            // TODO Revisar la comprobacion
            SequenceNumber_t seq = wp->available_changes_max();
            if (seq >= (*it)->sequenceNumber)
            {
                *change = *it;

                if (!(*change)->isRead)
                {
                    if (0 < total_unread_)
                    {
                        --total_unread_;
                    }
                }

                (*change)->isRead = true;

                if (wpout != nullptr)
                {
                    *wpout = wp;
                }

                takeok = true;
                break;
            }
        }
        else
        {
            toremove.push_back((*it));
        }
    }

    for (std::vector<CacheChange_t*>::iterator it = toremove.begin();
            it != toremove.end(); ++it)
    {
        logWarning(RTPS_READER,
                "Removing change " << (*it)->sequenceNumber << " from " << (*it)->writerGUID <<
                " because is no longer paired");
        mp_history->remove_change(*it);
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
    for (std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
            it != mp_history->changesEnd(); ++it)
    {
        if ((*it)->isRead)
        {
            continue;
        }

        WriterProxy* wp;
        if (this->matched_writer_lookup((*it)->writerGUID, &wp))
        {
            SequenceNumber_t seq;
            seq = wp->available_changes_max();
            if (seq >= (*it)->sequenceNumber)
            {
                *change = *it;

                if (0 < total_unread_)
                {
                    --total_unread_;
                }

                (*change)->isRead = true;

                if (wpout != nullptr)
                {
                    *wpout = wp;
                }

                readok = true;
                break;
            }
        }
        else
        {
            toremove.push_back((*it));
        }
    }

    for (std::vector<CacheChange_t*>::iterator it = toremove.begin();
            it != toremove.end(); ++it)
    {
        logWarning(RTPS_READER,
                "Removing change " << (*it)->sequenceNumber << " from " << (*it)->writerGUID <<
                " because is no longer paired");
        mp_history->remove_change(*it);
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
    bool cleanState = true;
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    if (is_alive_)
    {
        for (WriterProxy* wp : matched_writers_)
        {
            if (wp->number_of_changes_from_writer() != 0)
            {
                cleanState = false;
                break;
            }
        }
    }

    return cleanState;
}

void StatefulReader::send_acknack(
        const WriterProxy* writer,
        const SequenceNumberSet_t& sns,
        const RTPSMessageSenderInterface& sender,
        bool is_final)
{

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    if (!writer->is_alive())
    {
        return;
    }

    acknack_count_++;


    logInfo(RTPS_READER, "Sending ACKNACK: " << sns);

    if (!writer->is_on_same_process())
    {
        RTPSMessageGroup group(getRTPSParticipant(), this, sender);
        group.add_acknack(sns, acknack_count_, is_final);
    }
    else
    {
        GUID_t reader_guid = m_guid;
        uint32_t acknack_count = acknack_count_;
        lock.unlock(); //For local writers only call when initial ack, and we have to avoid deadlock with common
                       //calls writer -> reader
        RTPSWriter* writer_ptr = RTPSDomainImpl::find_local_writer(writer->guid());

        if (writer_ptr)
        {
            bool result;
            writer_ptr->process_acknack(writer->guid(), reader_guid, acknack_count, sns, is_final, result);
        }
    }
}

void StatefulReader::send_acknack(
        const WriterProxy* writer,
        const RTPSMessageSenderInterface& sender,
        bool heartbeat_was_final)
{
    // Protect reader
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (!writer->is_alive())
    {
        return;
    }

    SequenceNumberSet_t missing_changes = writer->missing_changes();

    try
    {
        RTPSMessageGroup group(getRTPSParticipant(), this, sender);
        if (!missing_changes.empty() || !heartbeat_was_final)
        {
            GUID_t guid = sender.remote_guids().at(0);
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
                        logInfo(RTPS_READER, "Sequence number " << seq
                                                                << " exceeded bitmap limit of AckNack. SeqNumSet Base: "
                                                                << sns.base());
                    }
                }
                else
                {
                    FragmentNumberSet_t frag_sns;
                    uncomplete_change->get_missing_fragments(frag_sns);
                    ++nackfrag_count_;
                    logInfo(RTPS_READER, "Sending NACKFRAG for sample" << seq << ": " << frag_sns; );

                    group.add_nackfrag(seq, frag_sns, nackfrag_count_);
                }

            });

            acknack_count_++;
            logInfo(RTPS_READER, "Sending ACKNACK: " << sns; );

            bool final = sns.empty();
            group.add_acknack(sns, acknack_count_, final);
        }
    }
    catch (const RTPSMessageGroup::timeout&)
    {
        logError(RTPS_WRITER, "Max blocking time reached");
    }
}

bool StatefulReader::send_sync_nts(
        CDRMessage_t* message,
        const Locator_t& locator,
        std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    return mp_RTPSParticipant->sendSync(message, locator, max_blocking_time_point);
}

void StatefulReader::new_lost_sample(
        SequenceNumber_t seq_num,
        SequenceNumber_t low_mark)
{
    if (seq_num > (low_mark + 1))
    {
        uint64_t num_samples = (seq_num - (low_mark + 1)).to64long();
        sample_lost_status_.total_count += static_cast<uint32_t>(num_samples);
        sample_lost_status_.total_count_change += static_cast<uint32_t>(num_samples);
        getListener()->on_sample_lost(this, sample_lost_status_);
    }
}
