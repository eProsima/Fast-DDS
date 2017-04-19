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
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include "../participant/RTPSParticipantImpl.h"
#include "../flowcontrol/FlowController.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/rtps/messages/RTPSMessageGroup.h>

#include <fastrtps/utils/TimeConversion.h>

#include <fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h>
#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>
#include <fastrtps/rtps/writer/timedevent/InitialHeartbeat.h>

#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>

#include <mutex>

using namespace eprosima::fastrtps::rtps;


StatefulWriter::StatefulWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
        WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
    RTPSWriter(pimpl,guid,att,hist,listen),
    mp_periodicHB(nullptr), m_times(att.times),
    all_acked_mutex_(nullptr), all_acked_(false), all_acked_cond_(nullptr)
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
    all_acked_mutex_ = new std::mutex();
    all_acked_cond_ = new std::condition_variable();
    m_reader_iterator = matched_readers.begin();
}


StatefulWriter::~StatefulWriter()
{
    AsyncWriterThread::removeWriter(*this);

    logInfo(RTPS_WRITER,"StatefulWriter destructor");

    delete all_acked_cond_;
    delete all_acked_mutex_;

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

    //TODO Think about when set liveliness assertion when writer is asynchronous.
    this->setLivelinessAsserted(true);

    if(!matched_readers.empty())
    {
        if(!isAsync())
        {
            //TODO(Ricardo) Temporal.
            LocatorList_t unilocList;
            LocatorList_t multilocList;
            bool expectsInlineQos = false;
            std::vector<GuidPrefix_t> remote_participants;
            std::vector<GUID_t> remote_readers;

            // TODO (Ricardo) Temporal
            LocatorList_t locators;

            for(auto it = matched_readers.begin(); it != matched_readers.end(); ++it)
            {
                ChangeForReader_t changeForReader(change);

                // TODO(Ricardo) Study next case: Not push mode, writer reiable and reader besteffort.
                if(m_pushMode)
                {
                    if((*it)->m_att.endpoint.reliabilityKind == RELIABLE)
                        changeForReader.setStatus(UNDERWAY);
                    else
                        changeForReader.setStatus(ACKNOWLEDGED);
                }
                else
                    changeForReader.setStatus(UNACKNOWLEDGED);

                (*it)->mp_mutex->lock();
                changeForReader.setRelevance((*it)->rtps_is_relevant(change));
                (*it)->addChange(changeForReader);
                locators.push_back((*it)->m_att.endpoint.unicastLocatorList);
                locators.push_back((*it)->m_att.endpoint.multicastLocatorList);
                expectsInlineQos |= (*it)->m_att.expectsInlineQos;
                remote_participants.push_back((*it)->m_att.guid.guidPrefix);
                remote_readers.push_back((*it)->m_att.guid);
                (*it)->mp_mutex->unlock();

                if((*it)->mp_nackSupression != nullptr) // It is reliable
                    (*it)->mp_nackSupression->restart_timer();
            }

            RTPSMessageGroup group(mp_RTPSParticipant, this,  RTPSMessageGroup::WRITER, m_cdrmessages);
            if(!group.add_data(*change, remote_readers, locators, expectsInlineQos))
            {
                logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
            }

            this->mp_periodicHB->restart_timer();
        }
        else
        {
            for(auto it = matched_readers.begin(); it != matched_readers.end(); ++it)
            {
                ChangeForReader_t changeForReader(change);

                if(m_pushMode)
                    changeForReader.setStatus(UNSENT);
                else
                    changeForReader.setStatus(UNACKNOWLEDGED);

                std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
                changeForReader.setRelevance((*it)->rtps_is_relevant(change));
                (*it)->addChange(changeForReader);
            }
        }
    }
    else
    {
        logInfo(RTPS_WRITER,"No reader proxy to add change.");
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

    return true;
}

bool StatefulWriter::wrap_around_readers()
{
    // We loop the reader iterator around until
    // we reach the wrapping point

    if (m_readers_to_walk == 0)
        return false;

    m_readers_to_walk--;
    m_reader_iterator++;
    if (m_reader_iterator == matched_readers.end())
        m_reader_iterator = matched_readers.begin();

    return true;
}

void StatefulWriter::send_any_unsent_changes()
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    // TODO(Ricardo) Change this while when implement Collector class.
    // Collector needs to know about fragments too.
    m_readers_to_walk = matched_readers.size();
    // The reader proxy vector is walked in a different order each time
    // to prevent persistent prioritization of a single reader
    while(wrap_around_readers())
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*m_reader_iterator)->mp_mutex);

        //std::vector<const ChangeForReader_t*> ch_vec = (*m_reader_iterator)->get_unsent_changes();
        std::vector<ChangeForReader_t*> ch_vec = (*m_reader_iterator)->get_unsent_changes();

        std::vector<CacheChange_t*> relevant_changes;
        std::vector<SequenceNumber_t> not_relevant_changes;

        for(auto cit = ch_vec.begin(); cit != ch_vec.end(); ++cit)
        {
            //cout << "EXPECTSINLINE: "<< (*m_reader_iterator)->m_att.expectsInlineQos<< endl;
            if((*cit)->isRelevant() && (*cit)->isValid())
            {
                relevant_changes.push_back((*cit)->getChange());

                // TODO(Ricardo) Change in future with collector.
                if((*cit)->getChange()->getFragmentSize() > 0)
                {
                    (*cit)->getChange()->getDataFragments()->assign((*cit)->getChange()->getDataFragments()->size(),
                            NOT_PRESENT);
                    FragmentNumberSet_t frag_sns = (*cit)->getUnsentFragments();

                    for(auto sn = frag_sns.get_begin(); sn != frag_sns.get_end(); ++sn)
                    {
                        assert(*sn <= (*cit)->getChange()->getDataFragments()->size());
                        (*cit)->getChange()->getDataFragments()->at(*sn - 1) = PRESENT;
                    }
                }
            }
            else
            {
                not_relevant_changes.push_back((*cit)->getSequenceNumber());
                (*m_reader_iterator)->set_change_to_status((*cit)->getSequenceNumber(), UNDERWAY);
            }
        }

        if(m_pushMode)
        {
            // Clear all relevant changes through the local controllers first
            for (auto& controller : m_controllers)
                (*controller)(relevant_changes);

            // Clear all relevant changes through the parent controllers
            for (auto& controller : mp_RTPSParticipant->getFlowControllers())
                (*controller)(relevant_changes);

            // TODO Temporal
            LocatorList_t locators((*m_reader_iterator)->m_att.endpoint.unicastLocatorList);
            locators.push_back((*m_reader_iterator)->m_att.endpoint.multicastLocatorList);

            RTPSMessageGroup group(mp_RTPSParticipant, this,  RTPSMessageGroup::WRITER, m_cdrmessages);

            if(!relevant_changes.empty())
            {
                //TODO (Ricardo) Temporal.
                std::vector<GuidPrefix_t> remote_participants{(*m_reader_iterator)->m_att.guid.guidPrefix};
                std::vector<GUID_t> remote_readers{(*m_reader_iterator)->m_att.guid};

                for (auto* change : relevant_changes)
                {
                    // TODO(Ricardo) Flowcontroller has to be used in RTPSMessageGroup. Study.
                    // And controllers are notified about the changes being sent
                    FlowController::NotifyControllersChangeSent(change);

                    if(change->getFragmentSize() != 0)
                    {
                        for(uint32_t fragment  = 0; fragment < change->getDataFragments()->size(); ++fragment)
                        {
                            if(change->getDataFragments()->at(fragment) == PRESENT)
                            {
                                if(group.add_data_frag(*change, fragment+1, remote_readers,
                                            locators, (*m_reader_iterator)->m_att.expectsInlineQos))
                                    (*m_reader_iterator)->mark_fragment_as_sent_for_change(change, fragment + 1);
                                else
                                {
                                    logError(RTPS_WRITER, "Error sending fragment (" << change->sequenceNumber <<
                                            ", " << fragment + 1 << ")");
                                }
                            }
                        }
                    }
                    else
                    {
                        if(group.add_data(*change, remote_readers,
                                locators, (*m_reader_iterator)->m_att.expectsInlineQos))
                        {
                            if((*m_reader_iterator)->m_att.endpoint.reliabilityKind == RELIABLE)
                                (*m_reader_iterator)->set_change_to_status(change->sequenceNumber, UNDERWAY);
                            else
                                (*m_reader_iterator)->set_change_to_status(change->sequenceNumber, ACKNOWLEDGED);
                        }
                        else
                        {
                            logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                        }
                    }
                }
            }

            if(!not_relevant_changes.empty())
                group.add_gap(not_relevant_changes, (*m_reader_iterator)->m_att.guid, locators);

            if((*m_reader_iterator)->m_att.endpoint.reliabilityKind == RELIABLE)
            {
                this->mp_periodicHB->restart_timer();
            }

            if((*m_reader_iterator)->mp_nackSupression != nullptr) // It is reliable
                (*m_reader_iterator)->mp_nackSupression->restart_timer();
        }
        else
        {
            // Change status to UNACKNOWLEDGED
            for(const auto* change : relevant_changes)
                (*m_reader_iterator)->set_change_to_status(change->sequenceNumber, UNACKNOWLEDGED);

            SequenceNumber_t firstSeq = this->get_seq_num_min();
            SequenceNumber_t lastSeq = this->get_seq_num_max();

            LocatorList_t locators((*m_reader_iterator)->m_att.endpoint.unicastLocatorList);
            locators.push_back((*m_reader_iterator)->m_att.endpoint.multicastLocatorList);

            if(firstSeq != c_SequenceNumber_Unknown && lastSeq != c_SequenceNumber_Unknown && lastSeq >= firstSeq)
            {
                RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);

                this->incrementHBCount();
                // TODO(Ricardo) This is a StatefulWriter in Reliable. Hast the FinalFlag be true? Check.
                group.add_heartbeat(std::vector<GUID_t>{(*m_reader_iterator)->m_att.guid},
                        firstSeq, lastSeq, m_heartbeatCount, true, false, locators);
            }
        }
    }

    logInfo(RTPS_WRITER, "Finish sending unsent changes");
}


/*
 *	MATCHED_READER-RELATED METHODS
 */
bool StatefulWriter::matched_reader_add(RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    if(rdata.guid == c_Guid_Unknown)
    {
        logError(RTPS_WRITER,"Reliable Writer need GUID_t of matched readers");
        return false;
    }

    // Check if it is already matched.
    for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
        if((*it)->m_att.guid == rdata.guid)
        {
            logInfo(RTPS_WRITER, "Attempting to add existing reader" << endl);
            return false;
        }
    }

    ReaderProxy* rp = new ReaderProxy(rdata,m_times,this);
    std::vector<SequenceNumber_t> not_relevant_changes;

    for(std::vector<CacheChange_t*>::iterator cit = mp_history->changesBegin();
            cit != mp_history->changesEnd(); ++cit)
    {
        ChangeForReader_t changeForReader(*cit);

        if(rp->m_att.endpoint.durabilityKind >= TRANSIENT_LOCAL && this->getAttributes()->durabilityKind == TRANSIENT_LOCAL)
        {
            changeForReader.setRelevance(rp->rtps_is_relevant(*cit));
            if(!rp->rtps_is_relevant(*cit))
                not_relevant_changes.push_back(changeForReader.getSequenceNumber());
        }
        else
        {
            changeForReader.setRelevance(false);
            not_relevant_changes.push_back(changeForReader.getSequenceNumber());
        }

        changeForReader.setStatus(UNACKNOWLEDGED);
        rp->addChange(changeForReader);
    }

    // Send a initial heartbeat
    if(rp->mp_initialHeartbeat != nullptr) // It is reliable
        rp->mp_initialHeartbeat->restart_timer();

    // TODO(Ricardo) In the heartbeat event?
    // Send Gap
    if(!not_relevant_changes.empty())
    {
        RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);
        //TODO (Ricardo) Temporal
        LocatorList_t locators(rp->m_att.endpoint.unicastLocatorList);
        locators.push_back(rp->m_att.endpoint.multicastLocatorList);

        group.add_gap(not_relevant_changes, rp->m_att.guid, locators);
    }

    // Always activate heartbeat period. We need a confirmation of the reader.
    // The state has to be updated.
    this->mp_periodicHB->restart_timer();

    matched_readers.push_back(rp);
    // Invalidate persistent iterator
    m_reader_iterator = matched_readers.begin();

    logInfo(RTPS_WRITER, "Reader Proxy "<< rp->m_att.guid<< " added to " << this->m_guid.entityId << " with "
            <<rp->m_att.endpoint.unicastLocatorList.size()<<"(u)-"
            <<rp->m_att.endpoint.multicastLocatorList.size()<<"(m) locators");

    return true;
}

bool StatefulWriter::matched_reader_remove(RemoteReaderAttributes& rdata)
{
    ReaderProxy *rproxy = nullptr;
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
    {
        std::lock_guard<std::recursive_mutex> rguard(*(*it)->mp_mutex);
        if((*it)->m_att.guid == rdata.guid)
        {
            logInfo(RTPS_WRITER, "Reader Proxy removed: " << (*it)->m_att.guid);
            rproxy = *it;
            matched_readers.erase(it);
            // Invalidate persistent iterator
            m_reader_iterator = matched_readers.begin();

            if(matched_readers.size()==0)
                this->mp_periodicHB->cancel_timer();

            break;
        }
    }

    lock.unlock();

    if(rproxy != nullptr)
    {
        delete rproxy;

        if(this->getAttributes()->durabilityKind == VOLATILE)
            clean_history();

        return true;
    }

    logInfo(RTPS_HISTORY,"Reader Proxy doesn't exist in this writer");
    return false;
}

bool StatefulWriter::matched_reader_is_matched(RemoteReaderAttributes& rdata)
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

bool StatefulWriter::is_acked_by_all(CacheChange_t* change)
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
    std::unique_lock<std::mutex> all_lock(*all_acked_mutex_);

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
        if(all_acked_cond_->wait_for(all_lock, max_w)  == std::cv_status::no_timeout)
            all_acked_ = true;
    }

    return all_acked_;
}

void StatefulWriter::check_for_all_acked()
{
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);
    std::unique_lock<std::mutex> all_lock(*all_acked_mutex_);

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

    if(all_acked_)
    {
        all_lock.unlock();
        all_acked_cond_->notify_all();
    }
}

bool StatefulWriter::clean_history(unsigned int max)
{
    logInfo(RTPS_WRITER, "Starting process clean_history for writer " << getGuid());
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    std::vector<CacheChange_t*> ackca;
    bool limit = (max != 0);

    for(std::vector<CacheChange_t*>::iterator cit = mp_history->changesBegin();
            cit != mp_history->changesEnd() && (!limit || ackca.size() < max); ++cit)
    {
        bool acknowledge = true;

        for(std::vector<ReaderProxy*>::iterator it = matched_readers.begin(); it != matched_readers.end(); ++it)
        {
            if(!(*it)->change_is_acked((*cit)->sequenceNumber))
            {
                acknowledge = false;
                break;
            }
        }

        if(acknowledge)
            ackca.push_back(*cit);
    }

    for(std::vector<CacheChange_t*>::iterator cit = ackca.begin();
            cit != ackca.end(); ++cit)
    {
        mp_history->remove_change_g(*cit);
    }

    return (ackca.size() > 0);
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

void StatefulWriter::send_heartbeat_to(ReaderProxy& remoteReaderProxy)
{
    SequenceNumber_t firstSeq = this->get_seq_num_min();
    SequenceNumber_t lastSeq = this->get_seq_num_max();

    if(firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
    {
        firstSeq = mp_history->next_sequence_number();
        lastSeq = SequenceNumber_t(0, 0);
    }
    else
    {
        (void)firstSeq;
        assert(firstSeq <= lastSeq);
    }

    RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);

    this->incrementHBCount();

    LocatorList_t locators(remoteReaderProxy.m_att.endpoint.unicastLocatorList);
    locators.push_back(remoteReaderProxy.m_att.endpoint.multicastLocatorList);

    // FinalFlag is always false because this is a StatefulWriter in Reliable.
    group.add_heartbeat(std::vector<GUID_t>{remoteReaderProxy.m_att.guid},
            firstSeq, lastSeq, m_heartbeatCount, true, false, locators);

    logInfo(RTPS_WRITER, m_guid.entityId << " Sending Heartbeat (" << firstSeq << " - " << lastSeq << ")");
}
