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

using namespace eprosima::fastrtps::rtps;


StatefulWriter::StatefulWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
        WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
    RTPSWriter(pimpl, guid, att, hist, listen),
    mp_periodicHB(nullptr), m_times(att.times),
    all_acked_(false), may_remove_change_(0),
    disableHeartbeatPiggyback_(att.disableHeartbeatPiggyback),
    sendBufferSize_(pimpl->get_min_network_send_buffer_size()),
    currentUsageSendBufferSize_(static_cast<int32_t>(pimpl->get_min_network_send_buffer_size()))
{
    m_heartbeatCount = 0;
    if(guid.entityId == c_EntityId_SEDPPubWriter)
        m_HBReaderEntityId = c_EntityId_SEDPPubReader;
    else if(guid.entityId == c_EntityId_SEDPSubWriter)
        m_HBReaderEntityId = c_EntityId_SEDPSubReader;
    else if(guid.entityId == c_EntityId_WriterLiveliness)
        m_HBReaderEntityId= c_EntityId_ReaderLiveliness;
    else
        m_HBReaderEntityId = c_EntityId_Unknown;
    mp_periodicHB = new PeriodicHeartbeat(this,TimeConv::Time_t2MilliSecondsDouble(m_times.heartbeatPeriod));
}


StatefulWriter::~StatefulWriter()
{
    AsyncWriterThread::removeWriter(*this);

    logInfo(RTPS_WRITER,"StatefulWriter destructor");

    for(std::vector<ReaderProxy*>::iterator it = matched_readers.begin();
            it != matched_readers.end(); ++it)
        (*it)->destroy_timers();

    if(mp_periodicHB !=nullptr)
        delete(mp_periodicHB);

    for(std::vector<ReaderProxy*>::iterator it = matched_readers.begin();
            it!=matched_readers.end();++it)
        delete(*it);
}

/*
 *	CHANGE-RELATED METHODS
 */

void StatefulWriter::unsent_change_added_to_history(CacheChange_t* change)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

#if HAVE_SECURITY
    encrypt_cachechange(change);
#endif

    //TODO Think about when set liveliness assertion when writer is asynchronous.
    this->setLivelinessAsserted(true);

    if(!matched_readers.empty())
    {
        if(!isAsync())
        {
            //TODO(Ricardo) Temporal.
            bool expectsInlineQos = false;

            for(auto it = matched_readers.begin(); it != matched_readers.end(); ++it)
            {
                ChangeForReader_t changeForReader(change);

                // TODO(Ricardo) Study next case: Not push mode, writer reliable and reader besteffort.
                if(m_pushMode)
                {
                    if((*it)->m_att.endpoint.reliabilityKind == RELIABLE)
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

                (*it)->mp_mutex->lock();
                changeForReader.setRelevance((*it)->rtps_is_relevant(change));
                (*it)->addChange(changeForReader);
                expectsInlineQos |= (*it)->m_att.expectsInlineQos;
                (*it)->mp_mutex->unlock();

                if((*it)->mp_nackSupression != nullptr) // It is reliable
                    (*it)->mp_nackSupression->restart_timer();
            }

            RTPSMessageGroup group(mp_RTPSParticipant, this,  RTPSMessageGroup::WRITER, m_cdrmessages);
            if(!group.add_data(*change, mAllRemoteReaders, mAllShrinkedLocatorList, expectsInlineQos))
            {
                logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
            }

            // Heartbeat piggyback.
            if(!disableHeartbeatPiggyback_)
            {
                if(mp_history->isFull())
                {
                    send_heartbeat_nts_(mAllRemoteReaders, mAllShrinkedLocatorList, group);
                }
                else
                {
                    currentUsageSendBufferSize_ -= group.get_current_bytes_processed();

                    if(currentUsageSendBufferSize_ < 0)
                    {
                        send_heartbeat_nts_(mAllRemoteReaders, mAllShrinkedLocatorList, group);
                    }
                }
            }

            this->mp_periodicHB->restart_timer();
            if ( (mp_listener != nullptr) && this->is_acked_by_all(change) )
            {
                mp_listener->onWriterChangeReceivedByAll(this, change);
            }
        }
        else
        {
            for(auto it = matched_readers.begin(); it != matched_readers.end(); ++it)
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

                std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
                changeForReader.setRelevance((*it)->rtps_is_relevant(change));
                (*it)->addChange(changeForReader);
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
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    logInfo(RTPS_WRITER,"Change "<< a_change->sequenceNumber << " to be removed.");

    // Invalidate CacheChange pointer in ReaderProxies.
    for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
            it!=this->matched_readers.end();++it)
    {
        (*it)->setNotValid(a_change);
    }

    std::unique_lock<std::mutex> may_lock(may_remove_change_mutex_);
    may_remove_change_ = 2;
    may_remove_change_cond_.notify_one();

    return true;
}

void StatefulWriter::send_any_unsent_changes()
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    RTPSWriterCollector<ReaderProxy*> relevantChanges;
    StatefulWriterOrganizer notRelevantChanges;

    for(auto remoteReader : matched_readers)
    {
        std::lock_guard<std::recursive_mutex> rguard(*remoteReader->mp_mutex);
        std::vector<ChangeForReader_t*> unsentChanges = remoteReader->get_unsent_changes();

        for(auto unsentChange : unsentChanges)
        {
            if(unsentChange->isRelevant() && unsentChange->isValid())
            {
                if(m_pushMode)
                {
                    relevantChanges.add_change(unsentChange->getChange(), remoteReader, unsentChange->getUnsentFragments());
                }
                else // Change status to UNACKNOWLEDGED
                {
                    remoteReader->set_change_to_status(unsentChange->getSequenceNumber(), UNACKNOWLEDGED);
                }
            }
            else
            {
                notRelevantChanges.add_sequence_number(unsentChange->getSequenceNumber(), remoteReader);
                remoteReader->set_change_to_status(unsentChange->getSequenceNumber(), UNDERWAY); //TODO(Ricardo) Review
            }
        }
    }



    if(m_pushMode)
    {
        // Clear all relevant changes through the local controllers first
        for (auto& controller : m_controllers)
            (*controller)(relevantChanges);

        // Clear all relevant changes through the parent controllers
        for (auto& controller : mp_RTPSParticipant->getFlowControllers())
            (*controller)(relevantChanges);

        RTPSMessageGroup group(mp_RTPSParticipant, this,  RTPSMessageGroup::WRITER, m_cdrmessages);
        bool activateHeartbeatPeriod = false;
        uint32_t lastBytesProcessed = 0;

        while(!relevantChanges.empty())
        {
            RTPSWriterCollector<ReaderProxy*>::Item changeToSend = relevantChanges.pop();
            std::vector<GUID_t> remote_readers;
            std::vector<LocatorList_t> locatorLists;
            bool expectsInlineQos = false;

            for(auto remoteReader : changeToSend.remoteReaders)
            {
                remote_readers.push_back(remoteReader->m_att.guid);
                LocatorList_t locators(remoteReader->m_att.endpoint.unicastLocatorList);
                locators.push_back(remoteReader->m_att.endpoint.multicastLocatorList);
                locatorLists.push_back(locators);
                expectsInlineQos |= remoteReader->m_att.expectsInlineQos;
            }

            // TODO(Ricardo) Flowcontroller has to be used in RTPSMessageGroup. Study.
            // And controllers are notified about the changes being sent
            FlowController::NotifyControllersChangeSent(changeToSend.cacheChange);

            if(changeToSend.fragmentNumber != 0)
            {
                if(group.add_data_frag(*changeToSend.cacheChange, changeToSend.fragmentNumber, remote_readers,
                            mp_RTPSParticipant->network_factory().ShrinkLocatorLists(locatorLists),
                            expectsInlineQos))
                {
                    for(auto remoteReader : changeToSend.remoteReaders)
                    {
                        std::lock_guard<std::recursive_mutex> rguard(*remoteReader->mp_mutex);
                        bool allFragmentsSent = remoteReader->mark_fragment_as_sent_for_change(changeToSend.cacheChange, changeToSend.fragmentNumber);

                        if(remoteReader->m_att.endpoint.reliabilityKind == RELIABLE)
                        {
                            activateHeartbeatPeriod = true;
                            assert(remoteReader->mp_nackSupression != nullptr);
                            if(allFragmentsSent)
                            {
                                remoteReader->set_change_to_status(changeToSend.sequenceNumber, UNDERWAY);
                                remoteReader->mp_nackSupression->restart_timer();
                            }
                        }
                        else
                        {
                            if(allFragmentsSent)
                            {
                                remoteReader->set_change_to_status(changeToSend.sequenceNumber, ACKNOWLEDGED);
                            }
                        }
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
                if(group.add_data(*changeToSend.cacheChange, remote_readers,
                            mp_RTPSParticipant->network_factory().ShrinkLocatorLists(locatorLists),
                            expectsInlineQos))
                {
                    for(auto remoteReader : changeToSend.remoteReaders)
                    {
                        std::lock_guard<std::recursive_mutex> rguard(*remoteReader->mp_mutex);
                        if(remoteReader->m_att.endpoint.reliabilityKind == RELIABLE)
                        {
                            remoteReader->set_change_to_status(changeToSend.sequenceNumber, UNDERWAY);
                            activateHeartbeatPeriod = true;
                            assert(remoteReader->mp_nackSupression != nullptr);
                            remoteReader->mp_nackSupression->restart_timer();
                        }
                        else
                        {
                            remoteReader->set_change_to_status(changeToSend.sequenceNumber, ACKNOWLEDGED);
                        }
                    }
                }
                else
                {
                    logError(RTPS_WRITER, "Error sending change " << changeToSend.sequenceNumber);
                }
            }

            // Heartbeat piggyback.
            if(!disableHeartbeatPiggyback_)
            {
                if(mp_history->isFull())
                {
                    send_heartbeat_nts_(mAllRemoteReaders, mAllShrinkedLocatorList, group);
                }
                else
                {
                    currentUsageSendBufferSize_ -= group.get_current_bytes_processed() - lastBytesProcessed;
                    lastBytesProcessed = group.get_current_bytes_processed();

                    if(currentUsageSendBufferSize_ < 0)
                    {
                        send_heartbeat_nts_(mAllRemoteReaders, mAllShrinkedLocatorList, group);
                    }
                }
            }
        }

        for(auto pair : notRelevantChanges.elements())
        {
            std::vector<GUID_t> remote_readers;
            std::vector<LocatorList_t> locatorLists;

            for(auto remoteReader : pair.first)
            {
                remote_readers.push_back(remoteReader->m_att.guid);
                LocatorList_t locators(remoteReader->m_att.endpoint.unicastLocatorList);
                locators.push_back(remoteReader->m_att.endpoint.multicastLocatorList);
                locatorLists.push_back(locators);
            }
            group.add_gap(pair.second, remote_readers,
                    mp_RTPSParticipant->network_factory().ShrinkLocatorLists(locatorLists));
        }

        if(activateHeartbeatPeriod)
            this->mp_periodicHB->restart_timer();
    }
    else
    {
        RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);
        send_heartbeat_nts_(mAllRemoteReaders, mAllShrinkedLocatorList, group, true);
    }

    // On VOLATILE writers, remove auto-acked (best effort readers) changes
    check_acked_status();

    logInfo(RTPS_WRITER, "Finish sending unsent changes");
}


/*
 *	MATCHED_READER-RELATED METHODS
 */
bool StatefulWriter::matched_reader_add(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    if(rdata.guid == c_Guid_Unknown)
    {
        logError(RTPS_WRITER,"Reliable Writer need GUID_t of matched readers");
        return false;
    }

    std::vector<GUID_t> allRemoteReaders;
    std::vector<LocatorList_t> allLocatorLists;

    // Check if it is already matched.
    for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
        if((*it)->m_att.guid == rdata.guid)
        {
            logInfo(RTPS_WRITER, "Attempting to add existing reader" << endl);
            return false;
        }

        allRemoteReaders.push_back((*it)->m_att.guid);
        LocatorList_t locators((*it)->m_att.endpoint.unicastLocatorList);
        locators.push_back((*it)->m_att.endpoint.multicastLocatorList);
        allLocatorLists.push_back(locators);
    }

    // Add info of new datareader.
    allRemoteReaders.push_back(rdata.guid);
    LocatorList_t locators(rdata.endpoint.unicastLocatorList);
    locators.push_back(rdata.endpoint.multicastLocatorList);
    allLocatorLists.push_back(locators);

    update_cached_info_nts(std::move(allRemoteReaders), allLocatorLists);

    ReaderProxy* rp = new ReaderProxy(rdata, m_times, this);
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
            while((*cit)->sequenceNumber != current_seq)
            {
                ChangeForReader_t changeForReader(current_seq);
                changeForReader.setRelevance(false);
                not_relevant_changes.insert(current_seq);
                changeForReader.setStatus(UNACKNOWLEDGED);
                rp->addChange(changeForReader);
                ++current_seq;
            }

            ChangeForReader_t changeForReader(*cit);

            if(rp->m_att.endpoint.durabilityKind >= TRANSIENT_LOCAL && this->getAttributes()->durabilityKind >= TRANSIENT_LOCAL)
            {
                changeForReader.setRelevance(rp->rtps_is_relevant(*cit));
                if(!rp->rtps_is_relevant(*cit))
                    not_relevant_changes.insert(changeForReader.getSequenceNumber());
            }
            else
            {
                changeForReader.setRelevance(false);
                not_relevant_changes.insert(changeForReader.getSequenceNumber());
            }

            changeForReader.setStatus(UNACKNOWLEDGED);
            rp->addChange(changeForReader);
            ++current_seq;
        }

        assert(last_seq + 1 == current_seq);

        RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);
        LocatorList_t locatorsList(rp->m_att.endpoint.unicastLocatorList);
        locatorsList.push_back(rp->m_att.endpoint.multicastLocatorList);

        // Send initial heartbeat
        send_heartbeat_nts_({rp->m_att.guid}, locatorsList, group, false);

        // Send Gap
        if(!not_relevant_changes.empty())
        {
            group.add_gap(not_relevant_changes, {rp->m_att.guid}, locatorsList);
        }

        // Always activate heartbeat period. We need a confirmation of the reader.
        // The state has to be updated.
        this->mp_periodicHB->restart_timer();
    }
    else if(rdata.is_eprosima_endpoint)
    {
        RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);
        LocatorList_t locatorsList(rp->m_att.endpoint.unicastLocatorList);
        locatorsList.push_back(rp->m_att.endpoint.multicastLocatorList);

        // Send initial heartbeat
        send_heartbeat_nts_({rp->m_att.guid}, locatorsList, group, false, true);
    }

    matched_readers.push_back(rp);

    logInfo(RTPS_WRITER, "Reader Proxy "<< rp->m_att.guid<< " added to " << this->m_guid.entityId << " with "
            <<rp->m_att.endpoint.unicastLocatorList.size()<<"(u)-"
            <<rp->m_att.endpoint.multicastLocatorList.size()<<"(m) locators");

    return true;
}

bool StatefulWriter::matched_reader_remove(const RemoteReaderAttributes& rdata)
{
    ReaderProxy *rproxy = nullptr;
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    std::vector<GUID_t> allRemoteReaders;
    std::vector<LocatorList_t> allLocatorLists;

    auto it = matched_readers.begin();
    while(it != matched_readers.end())
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);

        if((*it)->m_att.guid == rdata.guid)
        {
            logInfo(RTPS_WRITER, "Reader Proxy removed: " << (*it)->m_att.guid);
            rproxy = std::move(*it);
            it = matched_readers.erase(it);

            continue;
        }

        allRemoteReaders.push_back((*it)->m_att.guid);
        LocatorList_t locators((*it)->m_att.endpoint.unicastLocatorList);
        locators.push_back((*it)->m_att.endpoint.multicastLocatorList);
        allLocatorLists.push_back(locators);
        ++it;
    }

    update_cached_info_nts(std::move(allRemoteReaders), allLocatorLists);

    if(matched_readers.size()==0)
        this->mp_periodicHB->cancel_timer();

    lock.unlock();

    if(rproxy != nullptr)
    {
        delete rproxy;

        check_acked_status();

        return true;
    }

    logInfo(RTPS_HISTORY,"Reader Proxy doesn't exist in this writer");
    return false;
}

bool StatefulWriter::matched_reader_is_matched(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
        if((*it)->m_att.guid == rdata.guid)
        {
            return true;
        }
    }
    return false;
}

bool StatefulWriter::matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    std::vector<ReaderProxy*>::iterator it;
    for(it=matched_readers.begin();it!=matched_readers.end();++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
        if((*it)->m_att.guid == readerGuid)
        {
            *RP = *it;
            return true;
        }
    }
    return false;
}

bool StatefulWriter::is_acked_by_all(const CacheChange_t* change) const
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    if(change->writerGUID != this->getGuid())
    {
        logWarning(RTPS_WRITER,"The given change is not from this Writer");
        return false;
    }

    for(auto it = matched_readers.begin(); it!=matched_readers.end(); ++it)
    {
        if(!(*it)->change_is_acked(change->sequenceNumber))
        {
            logInfo(RTPS_WRITER, "Change " << change->sequenceNumber << " not acked." << endl);
            return false;
        }
    }
    return true;
}

bool StatefulWriter::wait_for_all_acked(const Duration_t& max_wait)
{
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);
    std::unique_lock<std::mutex> all_acked_lock(all_acked_mutex_);

    all_acked_ = true;

    for(auto it = matched_readers.begin(); it != matched_readers.end(); ++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
        if((*it)->countChangesForReader() > 0)
        {
            all_acked_ = false;
            break;
        }
    }
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
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    bool all_acked = true;
    SequenceNumber_t min_low_mark;


    for(auto it = matched_readers.begin(); it != matched_readers.end(); ++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);

        if(min_low_mark == SequenceNumber_t() || (*it)->get_low_mark() < min_low_mark)
        {
            min_low_mark = (*it)->get_low_mark();
        }

        if((*it)->countChangesForReader() > 0)
        {
            all_acked = false;
        }
    }

    if(get_seq_num_min() != SequenceNumber_t::unknown())
    {
        // Inform of samples acked.
        if(mp_listener != nullptr)
        {
            std::vector<CacheChange_t*> all_acked_changes;
            for(SequenceNumber_t current_seq = get_seq_num_min(); current_seq <= min_low_mark; ++current_seq)
            {
                for(std::vector<CacheChange_t*>::iterator cit = mp_history->changesBegin();
                        cit != mp_history->changesEnd(); ++cit)
                {
                    if((*cit)->sequenceNumber == current_seq)
                    {
                        all_acked_changes.push_back(*cit);
                    }
                }
            }
            for(auto cit = all_acked_changes.begin(); cit != all_acked_changes.end(); ++cit)
            {
                mp_listener->onWriterChangeReceivedByAll(this, *cit);
            }
        }
        
        SequenceNumber_t calc = min_low_mark < get_seq_num_min() ? SequenceNumber_t() :
            (min_low_mark - get_seq_num_min()) + 1;
        if (calc > SequenceNumber_t())
        {
            std::unique_lock<std::mutex> may_lock(may_remove_change_mutex_);
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

bool StatefulWriter::try_remove_change(std::chrono::microseconds& microseconds,
        std::unique_lock<std::recursive_mutex>& lock)
{
    logInfo(RTPS_WRITER, "Starting process try remove change for writer " << getGuid());

    SequenceNumber_t min_low_mark;

    for(auto it = matched_readers.begin(); it != matched_readers.end(); ++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);

        if(min_low_mark == SequenceNumber_t() || (*it)->get_low_mark() < min_low_mark)
        {
            min_low_mark = (*it)->get_low_mark();
        }
    }

    SequenceNumber_t calc = min_low_mark < get_seq_num_min() ? SequenceNumber_t() :
        (min_low_mark - get_seq_num_min()) + 1;
    unsigned int may_remove_change = 1;

    if(calc <= SequenceNumber_t())
    {
        lock.unlock();
        std::unique_lock<std::mutex> may_lock(may_remove_change_mutex_);
        may_remove_change_ = 0;
        may_remove_change_cond_.wait_for(may_lock, microseconds,
                [&]() { return may_remove_change_ > 0; });
        may_remove_change = may_remove_change_;
        may_lock.unlock();
        lock.lock();
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
void StatefulWriter::updateAttributes(WriterAttributes& att)
{
    this->updateTimes(att.times);
}

void StatefulWriter::updateTimes(WriterTimes& times)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    if(m_times.heartbeatPeriod != times.heartbeatPeriod)
    {
        this->mp_periodicHB->update_interval(times.heartbeatPeriod);
    }
    if(m_times.nackResponseDelay != times.nackResponseDelay)
    {
        for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
                it!=this->matched_readers.end();++it)
        {
            std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);

            if((*it)->mp_nackResponse != nullptr) // It is reliable
                (*it)->mp_nackResponse->update_interval(times.nackResponseDelay);
        }
    }
    if(m_times.heartbeatPeriod != times.heartbeatPeriod)
    {
        this->mp_periodicHB->update_interval(times.heartbeatPeriod);
    }
    if(m_times.nackResponseDelay != times.nackResponseDelay)
    {
        for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
                it!=this->matched_readers.end();++it)
        {
            std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
            (*it)->mp_nackResponse->update_interval(times.nackResponseDelay);
        }
    }
    if(m_times.nackSupressionDuration != times.nackSupressionDuration)
    {
        for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
                it!=this->matched_readers.end();++it)
        {
            std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);

            if((*it)->mp_nackSupression != nullptr) // It is reliable
                (*it)->mp_nackSupression->update_interval(times.nackSupressionDuration);
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

void StatefulWriter::send_heartbeat_to_nts(ReaderProxy& remoteReaderProxy, bool final,
        bool send_empty_history_info)
{
    RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);
    LocatorList_t locators(remoteReaderProxy.m_att.endpoint.unicastLocatorList);
    locators.push_back(remoteReaderProxy.m_att.endpoint.multicastLocatorList);

    send_heartbeat_nts_({remoteReaderProxy.m_att.guid},
            locators, group, final, send_empty_history_info);
}

void StatefulWriter::send_heartbeat_nts_(const std::vector<GUID_t>& remote_readers, const LocatorList_t &locators,
        RTPSMessageGroup& message_group, bool final, bool send_empty_history_info)
{
    SequenceNumber_t firstSeq = get_seq_num_min();
    SequenceNumber_t lastSeq = get_seq_num_max();

    if (firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
    {
        assert(firstSeq == c_SequenceNumber_Unknown && lastSeq == c_SequenceNumber_Unknown);

        if(send_empty_history_info && remote_readers.size() == 1)
        {
            firstSeq = next_sequence_number();
            lastSeq = {0, 0};
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

void StatefulWriter::process_acknack(const GUID_t reader_guid, uint32_t ack_count,
                        const SequenceNumberSet_t& sn_set, bool final_flag)
{
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    for(auto remote_reader : matched_readers)
    {
        std::lock_guard<std::recursive_mutex> reader_guard(*remote_reader->mp_mutex);

        if(remote_reader->m_att.guid == reader_guid)
        {
            if(remote_reader->m_lastAcknackCount < ack_count)
            {
                remote_reader->m_lastAcknackCount = ack_count;
                if(sn_set.base != SequenceNumber_t(0, 0))
                {
                    // Sequence numbers before Base are set as Acknowledged.
                    remote_reader->acked_changes_set(sn_set.base);
                    std::vector<SequenceNumber_t> set_vec = sn_set.get_set();
                    if (remote_reader->requested_changes_set(set_vec) && remote_reader->mp_nackResponse != nullptr)
                    {
                        remote_reader->mp_nackResponse->restart_timer();
                    }
                    else if(!final_flag)
                    {
                        mp_periodicHB->restart_timer();
                    }
                }
                else if(sn_set.isSetEmpty() && !final_flag)
                {
                    send_heartbeat_to_nts(*remote_reader, true, remote_reader->m_att.is_eprosima_endpoint);
                }

                // Check if all CacheChange are acknowledge, because a user could be waiting
                // for this, of if VOLATILE should be removed CacheChanges
                check_acked_status();
            }
            break;
        }
    }
}
