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

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include "../participant/RTPSParticipantImpl.h"
#include "../flowcontrol/FlowController.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/rtps/messages/RTPSMessageGroup.h>

#include <fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h>
#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>

#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>

#include "RTPSWriterCollector.h"
#include "StatefulWriterOrganizer.h"

#include <mutex>
#include <vector>
#include <stdexcept>

using namespace eprosima::fastrtps::rtps;


StatefulWriter::StatefulWriter(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const WriterAttributes& att,
        WriterHistory* hist,
        WriterListener* listen)
    : RTPSWriter(pimpl, guid, att, hist, listen)
    , mp_periodicHB(nullptr)
    , m_times(att.times)
    , matched_readers_(att.matched_readers_allocation)
    , matched_readers_pool_(att.matched_readers_allocation)
    , all_acked_(false)
    , may_remove_change_(0)
    , nack_response_event_(nullptr)
    , disableHeartbeatPiggyback_(att.disableHeartbeatPiggyback)
    , sendBufferSize_(pimpl->get_min_network_send_buffer_size())
    , currentUsageSendBufferSize_(static_cast<int32_t>(pimpl->get_min_network_send_buffer_size()))
{
    m_heartbeatCount = 0;
    m_HBReaderEntityId = 
        (guid.entityId == c_EntityId_SEDPPubWriter)    ? c_EntityId_SEDPPubReader :
        (guid.entityId == c_EntityId_SEDPSubWriter)    ? c_EntityId_SEDPSubReader :
        (guid.entityId == c_EntityId_WriterLiveliness) ? c_EntityId_ReaderLiveliness :
                                                         c_EntityId_Unknown;

    mp_periodicHB = new PeriodicHeartbeat(this,TimeConv::Time_t2MilliSecondsDouble(m_times.heartbeatPeriod));
    nack_response_event_ = new NackResponseDelay(this, TimeConv::Time_t2MilliSecondsDouble(m_times.nackResponseDelay));

    for (size_t n = 0; n < att.matched_readers_allocation.initial; ++n)
    {
        matched_readers_pool_.push_back(new ReaderProxy(m_times, this));
    }
}


StatefulWriter::~StatefulWriter()
{
    AsyncWriterThread::removeWriter(*this);

    logInfo(RTPS_WRITER,"StatefulWriter destructor");

    // Stop all active proxies and pass them to the pool
    while (!matched_readers_.empty())
    {
        ReaderProxy* remote_reader = matched_readers_.back();
        matched_readers_.pop_back();
        remote_reader->stop();
        matched_readers_pool_.push_back(remote_reader);
    }

    if(nack_response_event_ != nullptr)
    {
        delete(nack_response_event_);
        nack_response_event_ = nullptr;
    }

    // Destroy heartbeat event
    if (mp_periodicHB != nullptr)
    {
        delete(mp_periodicHB);
        mp_periodicHB = nullptr;
    }

    // Delete all proxies in the pool
    for (ReaderProxy* remote_reader : matched_readers_pool_)
    {
        delete(remote_reader);
    }
}

/*
 *	CHANGE-RELATED METHODS
 */

void StatefulWriter::unsent_change_added_to_history(CacheChange_t* change)
{
    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);

#if HAVE_SECURITY
    encrypt_cachechange(change);
#endif

    //TODO Think about when set liveliness assertion when writer is asynchronous.
    this->setLivelinessAsserted(true);

    if(!matched_readers_.empty())
    {
        if(!isAsync())
        {
            //TODO(Ricardo) Temporal.
            bool expectsInlineQos = false;

            // First step is to add the new CacheChange_t to all reader proxies.
            // It has to be done before sending, because if a timeout is catched, we will not include the
            // CacheChange_t in some reader proxies.
            for (ReaderProxy* it : matched_readers_)
            {
                ChangeForReader_t changeForReader(change);

                if(m_pushMode)
                {
                    if(it->is_reliable())
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
                it->add_change(changeForReader, true);
                expectsInlineQos |= it->expects_inline_qos();
            }

            try
            {
                //At this point we are sure all information was stores. We now can send data.
                if (!m_separateSendingEnabled)
                {
                    RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages,
                            change->write_params.max_blocking_time_point());
                    if (!group.add_data(*change, all_remote_readers_, mAllShrinkedLocatorList, expectsInlineQos))
                    {
                        logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                    }

                    // Heartbeat piggyback.
                    uint32_t last_processed = 0;
                    send_heartbeat_piggyback_nts_(group, last_processed);
                }
                else
                {
                    for (ReaderProxy* it : matched_readers_)
                    {
                        const std::vector<GUID_t>& guids = it->guid_as_vector();
                        const LocatorList_t& locators = it->remote_locators_shrinked();
                        RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages,
                                locators, guids, change->write_params.max_blocking_time_point());
                        if (!group.add_data(*change, guids, locators, it->expects_inline_qos()))
                        {
                            logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                        }
                        uint32_t last_processed = 0;
                        send_heartbeat_piggyback_nts_(guids, locators, group, last_processed);
                    }
                }

                this->mp_periodicHB->restart_timer();
                if ( (mp_listener != nullptr) && this->is_acked_by_all(change) )
                {
                    mp_listener->onWriterChangeReceivedByAll(this, change);
                }
            }
            catch(const RTPSMessageGroup::timeout&)
            {
                logError(RTPS_WRITER, "Max blocking time reached");
            }
        }
        else
        {
            for(ReaderProxy* it : matched_readers_)
            {
                ChangeForReader_t changeForReader(change);

                if(m_pushMode)
                {
                    changeForReader.setStatus(UNSENT);
                }
                else
                {
                    changeForReader.setStatus(UNACKNOWLEDGED);
                }

                changeForReader.setRelevance(it->rtps_is_relevant(change));
                it->add_change(changeForReader, false);
            }

            if (m_pushMode)
            {
                AsyncWriterThread::wakeUp(this);
            }
        }
    }
    else
    {
        logInfo(RTPS_WRITER,"No reader proxy to add change.");
        if (mp_listener != nullptr)
        {
            mp_listener->onWriterChangeReceivedByAll(this, change);
        }
    }
}


bool StatefulWriter::change_removed_by_history(CacheChange_t* a_change)
{
    SequenceNumber_t sequence_number = a_change->sequenceNumber;

    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);
    logInfo(RTPS_WRITER,"Change "<< sequence_number << " to be removed.");

    // Invalidate CacheChange pointer in ReaderProxies.
    for(ReaderProxy* it : matched_readers_)
    {
        it->change_has_been_removed(sequence_number);
    }

    may_remove_change_ = 2;
    may_remove_change_cond_.notify_one();

    return true;
}

void StatefulWriter::send_any_unsent_changes()
{
    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);

    bool activateHeartbeatPeriod = false;
    SequenceNumber_t max_sequence = mp_history->next_sequence_number();

    // Separate sending for asynchronous writers
    if (m_pushMode && m_separateSendingEnabled)
    {
        if(!isAsync())
        {
            for (ReaderProxy* remoteReader : matched_readers_)
            {
                try
                {
                    // For possible GAP
                    std::set<SequenceNumber_t> irrelevant;

                    // Specific destination message group
                    const std::vector<GUID_t>& guids = remoteReader->guid_as_vector();
                    const LocatorList_t& locators = remoteReader->remote_locators_shrinked();
                    RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages, locators, guids);

                    // Loop all changes
                    bool is_reliable = remoteReader->is_reliable();
                    auto unsent_change_process = [&](const SequenceNumber_t& seqNum, const ChangeForReader_t* unsentChange)
                    {
                        if (unsentChange != nullptr && unsentChange->isRelevant() && unsentChange->isValid())
                        {
                            // As we checked we are not async, we know we cannot have fragments
                            if (group.add_data(*(unsentChange->getChange()), guids, locators, 
                                        remoteReader->expects_inline_qos()))
                            {
                                remoteReader->set_change_to_status(seqNum, UNDERWAY, true);

                                if (is_reliable)
                                {
                                    activateHeartbeatPeriod = true;
                                }
                            }
                            else
                            {
                                logError(RTPS_WRITER, "Error sending change " << seqNum);
                            }
                        }
                        else
                        {
                            if (is_reliable)
                            {
                                irrelevant.emplace(seqNum);
                            }
                            remoteReader->set_change_to_status(seqNum, UNDERWAY, true);
                        } // Relevance
                    };
                    remoteReader->for_each_unsent_change(max_sequence, unsent_change_process);

                    if (!irrelevant.empty())
                    {
                        group.add_gap(irrelevant, guids, locators);
                    }
                }
                catch(const RTPSMessageGroup::timeout&)
                {
                    logError(RTPS_WRITER, "Max blocking time reached");
                }
            } // Readers loop
        }
        else
        {
            // This casuistic is not contemplated yet.
            assert(0);
        }
    }
    else
    {
        RTPSWriterCollector<ReaderProxy*> relevantChanges;
        StatefulWriterOrganizer notRelevantChanges;

        for (ReaderProxy* remoteReader : matched_readers_)
        {
            auto unsent_change_process = [&](const SequenceNumber_t& seq_num, const ChangeForReader_t* unsentChange)
            {
                if (unsentChange != nullptr && unsentChange->isRelevant() && unsentChange->isValid())
                {
                    if (m_pushMode)
                    {
                        relevantChanges.add_change(unsentChange->getChange(), remoteReader, unsentChange->getUnsentFragments());
                    }
                    else // Change status to UNACKNOWLEDGED
                    {
                        remoteReader->set_change_to_status(seq_num, UNACKNOWLEDGED, false);
                    }
                }
                else
                {
                    remoteReader->set_change_to_status(seq_num, UNDERWAY, true);
                    notRelevantChanges.add_sequence_number(seq_num, remoteReader);
                }
            };

            remoteReader->for_each_unsent_change(max_sequence, unsent_change_process);
        }

        if (m_pushMode)
        {
            // Clear all relevant changes through the local controllers first
            for (std::unique_ptr<FlowController>& controller : m_controllers)
                (*controller)(relevantChanges);

            // Clear all relevant changes through the parent controllers
            for (std::unique_ptr<FlowController>& controller : mp_RTPSParticipant->getFlowControllers())
                (*controller)(relevantChanges);

            try
            {
                RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);
                uint32_t lastBytesProcessed = 0;

                while (!relevantChanges.empty())
                {
                    RTPSWriterCollector<ReaderProxy*>::Item changeToSend = relevantChanges.pop();
                    std::vector<GUID_t> remote_readers;
                    std::vector<LocatorList_t> locatorLists;
                    bool expectsInlineQos = false;

                    for (const ReaderProxy* remoteReader : changeToSend.remoteReaders)
                    {
                        remote_readers.push_back(remoteReader->guid());
                        locatorLists.push_back(remoteReader->remote_locators());
                        expectsInlineQos |= remoteReader->expects_inline_qos();
                    }

                    // TODO(Ricardo) Flowcontroller has to be used in RTPSMessageGroup. Study.
                    // And controllers are notified about the changes being sent
                    FlowController::NotifyControllersChangeSent(changeToSend.cacheChange);

                    if (changeToSend.fragmentNumber != 0)
                    {
                        if (group.add_data_frag(*changeToSend.cacheChange, changeToSend.fragmentNumber, remote_readers,
                                    mp_RTPSParticipant->network_factory().ShrinkLocatorLists(locatorLists),
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
                                    if (remoteReader->is_reliable())
                                    {
                                        activateHeartbeatPeriod = true;
                                        if (allFragmentsSent)
                                        {
                                            remoteReader->set_change_to_status(changeToSend.sequenceNumber, UNDERWAY, true);
                                        }
                                    }
                                    else
                                    {
                                        if (allFragmentsSent)
                                        {
                                            remoteReader->set_change_to_status(changeToSend.sequenceNumber, ACKNOWLEDGED, false);
                                        }
                                    }
                                }
                            }

                            if (must_wake_up_async_thread)
                            {
                                AsyncWriterThread::wakeUp(this);
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
                    if (group.add_data(*changeToSend.cacheChange, remote_readers,
                                    mp_RTPSParticipant->network_factory().ShrinkLocatorLists(locatorLists),
                                    expectsInlineQos))
                    {
                        for (ReaderProxy* remoteReader : changeToSend.remoteReaders)
                        {
                            remoteReader->set_change_to_status(changeToSend.sequenceNumber, UNDERWAY, true);

                            if (remoteReader->is_reliable())
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
                    send_heartbeat_piggyback_nts_(group, lastBytesProcessed);
                }

                for (std::pair<std::vector<ReaderProxy*>, std::set<SequenceNumber_t>> pair : notRelevantChanges.elements())
                {
                    std::vector<GUID_t> remote_readers;
                    std::vector<LocatorList_t> locatorLists;

                    for (const ReaderProxy* remoteReader : pair.first)
                    {
                        remote_readers.push_back(remoteReader->guid());
                        locatorLists.push_back(remoteReader->remote_locators());
                    }
                    group.add_gap(pair.second, remote_readers,
                            mp_RTPSParticipant->network_factory().ShrinkLocatorLists(locatorLists));
                }
            }
            catch(const RTPSMessageGroup::timeout&)
            {
                logError(RTPS_WRITER, "Max blocking time reached");
            }
        }
        else
        {
            try
            {
                RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);
                send_heartbeat_nts_(all_remote_readers_, mAllShrinkedLocatorList, group, true);
            }
            catch(const RTPSMessageGroup::timeout&)
            {
                logError(RTPS_WRITER, "Max blocking time reached");
            }
        }
    }

    if (activateHeartbeatPeriod)
        this->mp_periodicHB->restart_timer();

    // On VOLATILE writers, remove auto-acked (best effort readers) changes
    check_acked_status();

    logInfo(RTPS_WRITER, "Finish sending unsent changes");
}


/*
 *	MATCHED_READER-RELATED METHODS
 */
bool StatefulWriter::matched_reader_add(RemoteReaderAttributes& rdata)
{
    if (rdata.guid == c_Guid_Unknown)
    {
        logError(RTPS_WRITER, "Reliable Writer need GUID_t of matched readers");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);

    std::vector<LocatorList_t> allLocatorLists;

    // Check if it is already matched.
    for(ReaderProxy* it : matched_readers_)
    {
        if(it->guid() == rdata.guid)
        {
            logInfo(RTPS_WRITER, "Attempting to add existing reader" << endl);
            return false;
        }

        allLocatorLists.push_back(it->remote_locators());
    }

    // Get a reader proxy from the inactive pool (or create a new one if necessary and allowed)
    ReaderProxy* rp = nullptr;
    if (matched_readers_pool_.empty())
    {
        size_t max_readers = matched_readers_pool_.max_size();
        if (matched_readers_.size() + matched_readers_pool_.size() < max_readers)
        {
            rp = new ReaderProxy(m_times, this);
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
        rp = matched_readers_pool_.back();
        matched_readers_pool_.pop_back();
    }

    // Add info of new datareader.
    all_remote_readers_.push_back(rdata.guid);
    LocatorList_t locators(rdata.endpoint.unicastLocatorList);
    locators.push_back(rdata.endpoint.multicastLocatorList);
    allLocatorLists.push_back(locators);

    update_cached_info_nts(allLocatorLists);

    getRTPSParticipant()->createSenderResources(mAllShrinkedLocatorList, false);

    rdata.endpoint.unicastLocatorList =
        mp_RTPSParticipant->network_factory().ShrinkLocatorLists({rdata.endpoint.unicastLocatorList});

    rp->start(rdata);
    std::set<SequenceNumber_t> not_relevant_changes;

    SequenceNumber_t current_seq = get_seq_num_min();
    SequenceNumber_t last_seq = get_seq_num_max();

    if(current_seq != SequenceNumber_t::unknown())
    {
        (void)last_seq;
        assert(last_seq != SequenceNumber_t::unknown());
        assert(current_seq <= last_seq);

        for(std::vector<CacheChange_t*>::iterator cit = mp_history->changesBegin();
                cit != mp_history->changesEnd(); ++cit)
        {
            ChangeForReader_t changeForReader(*cit);

            if(rp->durability_kind() >= TRANSIENT_LOCAL && this->getAttributes().durabilityKind >= TRANSIENT_LOCAL)
            {
                changeForReader.setRelevance(rp->rtps_is_relevant(*cit));
                if(!rp->rtps_is_relevant(*cit))
                {
                    not_relevant_changes.insert(changeForReader.getSequenceNumber());
                }
            }
            else
            {
                changeForReader.setRelevance(false);
                not_relevant_changes.insert(changeForReader.getSequenceNumber());
            }

            // The ChangeForReader_t status has to be UNACKNOWLEDGED
            changeForReader.setStatus(UNACKNOWLEDGED); /// TODO JOOOODERR TESSSST
            rp->add_change(changeForReader, false);
            ++current_seq;
        }

        assert(last_seq + 1 == current_seq);

        try
        {
            const std::vector<GUID_t>& guids = rp->guid_as_vector();
            const LocatorList_t& locatorsList = rp->remote_locators_shrinked();
            RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages, locatorsList, guids);

            // Send initial heartbeat
            send_heartbeat_nts_(guids, locatorsList, group, false);

            // Send Gap
            if(!not_relevant_changes.empty())
            {
                group.add_gap(not_relevant_changes, guids, locatorsList);
            }
        }
        catch(const RTPSMessageGroup::timeout&)
        {
            logError(RTPS_WRITER, "Max blocking time reached");
        }

        // Always activate heartbeat period. We need a confirmation of the reader.
        // The state has to be updated.
        this->mp_periodicHB->restart_timer();
    }
    else
    {
        send_heartbeat_to_nts(*rp, false);
    }

    matched_readers_.push_back(rp);

    logInfo(RTPS_WRITER, "Reader Proxy "<< rp->guid()<< " added to " << this->m_guid.entityId << " with "
            <<rp->reader_attributes().endpoint.unicastLocatorList.size()<<"(u)-"
            <<rp->reader_attributes().endpoint.multicastLocatorList.size()<<"(m) locators");

    return true;
}

bool StatefulWriter::matched_reader_remove(const RemoteReaderAttributes& rdata)
{
    ReaderProxy *rproxy = nullptr;
    std::unique_lock<std::recursive_timed_mutex> lock(mp_mutex);

    std::vector<LocatorList_t> allLocatorLists;

    ReaderProxyIterator it = matched_readers_.begin();
    while(it != matched_readers_.end())
    {
        if((*it)->guid() == rdata.guid)
        {
            logInfo(RTPS_WRITER, "Reader Proxy removed: " << (*it)->guid());
            rproxy = std::move(*it);
            it = matched_readers_.erase(it);

            continue;
        }

        allLocatorLists.push_back((*it)->remote_locators());
        ++it;
    }

    all_remote_readers_.remove(rdata.guid);
    update_cached_info_nts(allLocatorLists);

    if(matched_readers_.size()==0)
        this->mp_periodicHB->cancel_timer();

    lock.unlock();

    if(rproxy != nullptr)
    {
        rproxy->stop();
        matched_readers_pool_.push_back(rproxy);

        check_acked_status();

        return true;
    }

    logInfo(RTPS_HISTORY,"Reader Proxy doesn't exist in this writer");
    return false;
}

bool StatefulWriter::matched_reader_is_matched(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);
    for(ReaderProxy* it : matched_readers_)
    {
        if(it->guid() == rdata.guid)
        {
            return true;
        }
    }
    return false;
}

bool StatefulWriter::matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP)
{
    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);
    for(ReaderProxy* it : matched_readers_)
    {
        if(it->guid() == readerGuid)
        {
            *RP = it;
            return true;
        }
    }
    return false;
}

bool StatefulWriter::is_acked_by_all(const CacheChange_t* change)
{
    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);

    if(change->writerGUID != this->getGuid())
    {
        logWarning(RTPS_WRITER,"The given change is not from this Writer");
        return false;
    }

    assert(mp_history->next_sequence_number() > change->sequenceNumber);
    return std::all_of(matched_readers_.begin(), matched_readers_.end(),
        [change](const ReaderProxy* reader)
        {
            return reader->change_is_acked(change->sequenceNumber);
        });
}

bool StatefulWriter::wait_for_all_acked(const Duration_t& max_wait)
{
    std::unique_lock<std::recursive_timed_mutex> lock(mp_mutex);
    std::unique_lock<std::mutex> all_acked_lock(all_acked_mutex_);

    all_acked_ = std::none_of(matched_readers_.begin(), matched_readers_.end(),
        [](const ReaderProxy* reader)
        {
            return reader->has_changes();
        });
    lock.unlock();

    if(!all_acked_)
    {
        std::chrono::microseconds max_w(::TimeConv::Time_t2MicroSecondsInt64(max_wait));
        all_acked_cond_.wait_for(all_acked_lock, max_w, [&]() { return all_acked_; });
    }

    return all_acked_;
}

void StatefulWriter::check_acked_status()
{
    std::unique_lock<std::recursive_timed_mutex> lock(mp_mutex);

    bool all_acked = true;
    SequenceNumber_t min_low_mark;

    for(const ReaderProxy* it : matched_readers_)
    {
        SequenceNumber_t reader_low_mark = it->changes_low_mark();
        if(min_low_mark == SequenceNumber_t() || reader_low_mark < min_low_mark)
        {
            min_low_mark = reader_low_mark;
        }

        if(it->has_changes())
        {
            all_acked = false;
        }
    }

    if(get_seq_num_min() != SequenceNumber_t::unknown())
    {
        // Inform of samples acked.
        if(mp_listener != nullptr)
        {
            for(SequenceNumber_t current_seq = get_seq_num_min(); current_seq <= min_low_mark; ++current_seq)
            {
                std::vector<CacheChange_t*>::iterator history_end = mp_history->changesEnd();
                std::vector<CacheChange_t*>::iterator cit = std::find_if(mp_history->changesBegin(), history_end,
                    [current_seq](const CacheChange_t* change)
                    {
                        return change->sequenceNumber == current_seq;
                    });
                if(cit != history_end)
                {
                    mp_listener->onWriterChangeReceivedByAll(this, *cit);
                }
            }
        }

        SequenceNumber_t calc = min_low_mark < get_seq_num_min() ? SequenceNumber_t() :
            (min_low_mark - get_seq_num_min()) + 1;
        if (calc > SequenceNumber_t())
        {
            may_remove_change_ = 1;
            may_remove_change_cond_.notify_one();
        }
    }

    if(all_acked)
    {
        std::unique_lock<std::mutex> all_acked_lock(all_acked_mutex_);
        all_acked_ = true;
        all_acked_cond_.notify_all();
    }
}

bool StatefulWriter::try_remove_change(
        std::chrono::steady_clock::time_point& max_blocking_time_point,
        std::unique_lock<std::recursive_timed_mutex>& lock)
{
    logInfo(RTPS_WRITER, "Starting process try remove change for writer " << getGuid());

    SequenceNumber_t min_low_mark;

    for(ReaderProxy* it : matched_readers_)
    {
        SequenceNumber_t reader_low_mark = it->changes_low_mark();
        if (min_low_mark == SequenceNumber_t() || reader_low_mark < min_low_mark)
        {
            min_low_mark = reader_low_mark;
        }
    }

    SequenceNumber_t calc = min_low_mark < get_seq_num_min() ? SequenceNumber_t() :
        (min_low_mark - get_seq_num_min()) + 1;
    unsigned int may_remove_change = 1;

    if(calc <= SequenceNumber_t())
    {
        may_remove_change_ = 0;
        may_remove_change_cond_.wait_until(lock, max_blocking_time_point,
                [&]() { return may_remove_change_ > 0; });
        may_remove_change = may_remove_change_;
    }

    // Some changes acked
    if(may_remove_change == 1)
    {
        return mp_history->remove_min_change();
    }
    // Waiting a change was removed.
    else if(may_remove_change == 2)
    {
        return true;
    }

    return false;
}

/*
 * PARAMETER_RELATED METHODS
 */
void StatefulWriter::updateAttributes(const WriterAttributes& att)
{
    this->updateTimes(att.times);
}

void StatefulWriter::updateTimes(const WriterTimes& times)
{
    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);
    if(m_times.heartbeatPeriod != times.heartbeatPeriod)
    {
        this->mp_periodicHB->update_interval(times.heartbeatPeriod);
    }
    if(m_times.nackResponseDelay != times.nackResponseDelay)
    {
        if(nack_response_event_ != nullptr)
        {
            nack_response_event_->update_interval(times.nackResponseDelay);
        }
    }
    if(m_times.nackSupressionDuration != times.nackSupressionDuration)
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

void StatefulWriter::add_flow_controller(std::unique_ptr<FlowController> controller)
{
    m_controllers.push_back(std::move(controller));
}

SequenceNumber_t StatefulWriter::next_sequence_number() const
{
    return mp_history->next_sequence_number();
}

bool StatefulWriter::send_periodic_heartbeat()
{
    std::lock_guard<std::recursive_timed_mutex> guardW(mp_mutex);

    bool unacked_changes = false;
    if (m_separateSendingEnabled)
    {
        for (ReaderProxy* it : matched_readers_)
        {
            if (it->has_unacknowledged())
            {
                // FinalFlag is always false because this class is used only by StatefulWriter in Reliable.
                send_heartbeat_to_nts(*it, false);
                unacked_changes = true;
            }
        }
    }
    else
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
                    RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages,
                        mAllShrinkedLocatorList, all_remote_readers_);
                    send_heartbeat_nts_(all_remote_readers_, mAllShrinkedLocatorList, group, false);
                }
                catch(const RTPSMessageGroup::timeout&)
                {
                    logError(RTPS_WRITER, "Max blocking time reached");
                }
            }
        }
    }

    return unacked_changes;
}

void StatefulWriter::send_heartbeat_to_nts(
    ReaderProxy& remoteReaderProxy, 
    bool final)
{
    try
    {
        const std::vector<GUID_t>& guids = remoteReaderProxy.guid_as_vector();
        const LocatorList_t& locators = remoteReaderProxy.remote_locators_shrinked();
        RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages,
            locators, guids);

        send_heartbeat_nts_(guids, locators, group, final);
    }
    catch(const RTPSMessageGroup::timeout&)
    {
        logError(RTPS_WRITER, "Max blocking time reached");
    }
}

void StatefulWriter::send_heartbeat_nts_(
    const std::vector<GUID_t>& remote_readers,
    const LocatorList_t& locators,
    RTPSMessageGroup& message_group,
    bool final)
{
    SequenceNumber_t firstSeq = get_seq_num_min();
    SequenceNumber_t lastSeq = get_seq_num_max();

    if (firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
    {
        assert(firstSeq == c_SequenceNumber_Unknown && lastSeq == c_SequenceNumber_Unknown);

        if(remote_readers.size() == 1)
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

    // FinalFlag is always false because this class is used only by StatefulWriter in Reliable.
    message_group.add_heartbeat(remote_readers,
            firstSeq, lastSeq, m_heartbeatCount, final, false, locators);
    // Update calculate of heartbeat piggyback.
    currentUsageSendBufferSize_ = static_cast<int32_t>(sendBufferSize_);

    logInfo(RTPS_WRITER, getGuid().entityId << " Sending Heartbeat (" << firstSeq << " - " << lastSeq <<")" );
}

void StatefulWriter::send_heartbeat_piggyback_nts_(
    const std::vector<GUID_t>& remote_readers, 
    const LocatorList_t& locators,
    RTPSMessageGroup& message_group,
    uint32_t& last_bytes_processed)
{
    if (!disableHeartbeatPiggyback_)
    {
        if (mp_history->isFull())
        {
            send_heartbeat_nts_(remote_readers, locators, message_group, false);
        }
        else
        {
            uint32_t current_bytes = message_group.get_current_bytes_processed();
            currentUsageSendBufferSize_ -= current_bytes - last_bytes_processed;
            last_bytes_processed = current_bytes;
            if (currentUsageSendBufferSize_ < 0)
            {
                send_heartbeat_nts_(remote_readers, locators, message_group, false);
            }
        }
    }
}

void StatefulWriter::send_heartbeat_piggyback_nts_(
    RTPSMessageGroup& message_group,
    uint32_t& last_bytes_processed)
{
    send_heartbeat_piggyback_nts_(all_remote_readers_, mAllShrinkedLocatorList, message_group, last_bytes_processed);
}

void StatefulWriter::perform_nack_response()
{
    std::unique_lock<std::recursive_timed_mutex> lock(mp_mutex);
    bool must_wake_up_async_thread = false;

    for (ReaderProxy* remote_reader : matched_readers_)
    {
        if (remote_reader->perform_acknack_response())
        {
            must_wake_up_async_thread = true;
        }
    }

    if (must_wake_up_async_thread)
    {
        AsyncWriterThread::wakeUp(this);
    }
}

void StatefulWriter::perform_nack_supression(const GUID_t& reader_guid)
{
    std::unique_lock<std::recursive_timed_mutex> lock(mp_mutex);

    for (ReaderProxy* remote_reader : matched_readers_)
    {
        if (remote_reader->guid() == reader_guid)
        {
            remote_reader->perform_nack_supression();
            mp_periodicHB->restart_timer();
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
        bool &result)
{
    std::unique_lock<std::recursive_timed_mutex> lock(mp_mutex);
    result = (m_guid == writer_guid);
    if (result)
    {
        for (ReaderProxy* remote_reader : matched_readers_)
        {
            if (remote_reader->guid() == reader_guid)
            {
                if (remote_reader->check_and_set_acknack_count(ack_count))
                {
                    if (sn_set.base() > SequenceNumber_t(0, 0))
                    {
                        // Sequence numbers before Base are set as Acknowledged.
                        remote_reader->acked_changes_set(sn_set.base());
                        if (remote_reader->requested_changes_set(sn_set))
                        {
                            nack_response_event_->restart_timer();
                        }
                        else if (!final_flag)
                        {
                            mp_periodicHB->restart_timer();
                        }
                    }
                    else if (SequenceNumber_t() == remote_reader->changes_low_mark() && sn_set.empty() && !final_flag)
                    {
                        send_heartbeat_to_nts(*remote_reader, true);
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
    std::unique_lock<std::recursive_timed_mutex> lock(mp_mutex);
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
