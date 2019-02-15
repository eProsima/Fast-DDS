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

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/timedevent/HeartbeatResponseDelay.h>
#include <fastrtps/rtps/reader/timedevent/InitialAckNack.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include "../participant/RTPSParticipantImpl.h"
#include "FragmentedChangePitStop.h"
#include <fastrtps/utils/TimeConversion.h>

#include <mutex>
#include <thread>

#include <cassert>

#define IDSTRING "(ID:"<< std::this_thread::get_id() <<") "<<

using namespace eprosima::fastrtps::rtps;



StatefulReader::~StatefulReader()
{
    logInfo(RTPS_READER,"StatefulReader destructor.";);
    for(WriterProxy* writer : matched_writers)
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
    : RTPSReader(pimpl,guid,att,hist, listen)
    , m_acknackCount(0)
    , m_nackfragCount(0)
    , m_times(att.times)
{
}


bool StatefulReader::matched_writer_add(const RemoteWriterAttributes& wdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    for(WriterProxy* it : matched_writers)
    {
        if(it->m_att.guid == wdata.guid)
        {
            logInfo(RTPS_READER,"Attempting to add existing writer");
            return false;
        }
    }

    RemoteWriterAttributes att(wdata);
    getRTPSParticipant()->createSenderResources(att.endpoint.remoteLocatorList, false);


    att.endpoint.unicastLocatorList =
        mp_RTPSParticipant->network_factory().ShrinkLocatorLists({att.endpoint.unicastLocatorList});
    WriterProxy* wp = new WriterProxy(att, this);

    wp->mp_initialAcknack->restart_timer();

    add_persistence_guid(att);
    wp->loaded_from_storage_nts(get_last_notified(att.guid));
    matched_writers.push_back(wp);
    logInfo(RTPS_READER,"Writer Proxy " <<wp->m_att.guid <<" added to " <<m_guid.entityId);
    return true;
}

bool StatefulReader::matched_writer_remove(const RemoteWriterAttributes& wdata)
{
    WriterProxy *wproxy = nullptr;
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    //Remove cachechanges belonging to the unmatched writer
    mp_history->remove_changes_with_guid(wdata.guid);

    for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
    {
        if((*it)->m_att.guid == wdata.guid)
        {
            logInfo(RTPS_READER,"Writer Proxy removed: " <<(*it)->m_att.guid);
            wproxy = *it;
            matched_writers.erase(it);
            remove_persistence_guid(wdata);
            break;
        }
    }

    lock.unlock();

    if(wproxy != nullptr)
    {
        delete wproxy;
        return true;
    }

    logInfo(RTPS_READER,"Writer Proxy " << wdata.guid << " doesn't exist in reader "<<this->getGuid().entityId);
    return false;
}

bool StatefulReader::liveliness_expired(const GUID_t& writer_guid)
{
    WriterProxy* wproxy = nullptr;
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    //Remove cachechanges belonging to the unmatched writer
    mp_history->remove_changes_with_guid(writer_guid);

    for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
    {
        if((*it)->m_att.guid == writer_guid)
        {
            logInfo(RTPS_READER,"Writer Proxy removed: " <<(*it)->m_att.guid);
            wproxy = *it;
            matched_writers.erase(it);
            remove_persistence_guid(wproxy->m_att);
			if(mp_listener != nullptr)
			{
				MatchingInfo info(REMOVED_MATCHING, writer_guid);
                mp_listener->onReaderMatched(this, info);
			}

            wproxy->liveliness_expired();
			delete(wproxy);
            return true;
        }
    }

    logInfo(RTPS_READER,"Writer Proxy " << writer_guid << " doesn't exist in reader "<<this->getGuid().entityId);
    return false;
}

bool StatefulReader::matched_writer_is_matched(const RemoteWriterAttributes& wdata) const
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    for(WriterProxy* it : matched_writers)
    {
        if(it->m_att.guid == wdata.guid)
        {
            return true;
        }
    }
    return false;
}

bool StatefulReader::matched_writer_lookup(
        const GUID_t& writerGUID, 
        WriterProxy** WP)
{
    assert(WP);

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    bool returnedValue = findWriterProxy(writerGUID, WP);

    if(returnedValue)
    {
        logInfo(RTPS_READER,this->getGuid().entityId<<" FINDS writerProxy "<< writerGUID<<" from "<< matched_writers.size());
    }
    else
    {
        logInfo(RTPS_READER,this->getGuid().entityId<<" NOT FINDS writerProxy "<< writerGUID<<" from "<< matched_writers.size());
    }

    return returnedValue;
}

bool StatefulReader::findWriterProxy(const GUID_t& writerGUID, WriterProxy** WP) const
{
    assert(WP);

    for(WriterProxy* it : matched_writers)
    {
        if(it->m_att.guid == writerGUID)
        {
            *WP = it;
            return true;
        }
    }
    return false;
}

bool StatefulReader::processDataMsg(CacheChange_t *change)
{
    WriterProxy *pWP = nullptr;

    assert(change);

    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(change->writerGUID, &pWP))
    {
        // Check if CacheChange was received.
        if(!pWP->change_was_received(change->sequenceNumber))
        {
            logInfo(RTPS_MSG_IN,IDSTRING"Trying to add change " << change->sequenceNumber <<" TO reader: "<< getGuid().entityId);

            CacheChange_t* change_to_add;

            if(reserveCache(&change_to_add, change->serializedPayload.length)) //Reserve a new cache from the corresponding cache pool
            {
#if HAVE_SECURITY
                if(getAttributes().security_attributes().is_payload_protected)
                {
                    change_to_add->copy_not_memcpy(change);
                    if(!getRTPSParticipant()->security_manager().decode_serialized_payload(change->serializedPayload,
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
                        logWarning(RTPS_MSG_IN,IDSTRING"Problem copying CacheChange, received data is: " << change->serializedPayload.length
                                << " bytes and max size in reader " << getGuid().entityId << " is " << change_to_add->serializedPayload.max_size);
                        releaseCache(change_to_add);
                        return false;
                    }
#if HAVE_SECURITY
                }
#endif
            }
            else
            {
                logError(RTPS_MSG_IN,IDSTRING"Problem reserving CacheChange in reader: " << getGuid().entityId);
                return false;
            }

            // Assertion has to be done before call change_received,
            // because this function can unlock the StatefulReader mutex.
            if(pWP != nullptr)
            {
                pWP->assert_liveliness(); //Asser liveliness since you have received a DATA MESSAGE.
            }

            if(!change_received(change_to_add, pWP))
            {
                logInfo(RTPS_MSG_IN,IDSTRING"MessageReceiver not add change "<<change_to_add->sequenceNumber);
                releaseCache(change_to_add);

                if(pWP == nullptr && getGuid().entityId == c_EntityId_SPDPReader)
                {
                    mp_RTPSParticipant->assertRemoteRTPSParticipantLiveliness(change->writerGUID.guidPrefix);
                }
            }
        }
    }

    return true;
}

bool StatefulReader::processDataFragMsg(
        CacheChange_t* incomingChange, 
        uint32_t sampleSize, 
        uint32_t fragmentStartingNum)
{
    WriterProxy *pWP = nullptr;

    assert(incomingChange);

    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(incomingChange->writerGUID, &pWP))
    {
        // Check if CacheChange was received.
        if(!pWP->change_was_received(incomingChange->sequenceNumber))
        {
            logInfo(RTPS_MSG_IN, IDSTRING"Trying to add fragment " << incomingChange->sequenceNumber.to64long() << " TO reader: " << getGuid().entityId);

            CacheChange_t* change_to_add = incomingChange;

#if HAVE_SECURITY
            if(getAttributes().security_attributes().is_payload_protected)
            {
                if(reserveCache(&change_to_add, incomingChange->serializedPayload.length)) //Reserve a new cache from the corresponding cache pool
                {
                    change_to_add->copy_not_memcpy(incomingChange);
                    if(!getRTPSParticipant()->security_manager().decode_serialized_payload(incomingChange->serializedPayload,
                                change_to_add->serializedPayload, m_guid, incomingChange->writerGUID))
                    {
                        releaseCache(change_to_add);
                        logWarning(RTPS_MSG_IN, "Cannont decode serialized payload");
                        return false;
                    }
                }
            }
#endif

            // Fragments manager has to process incomming fragments.
            // If CacheChange_t is completed, it will be returned;
            CacheChange_t* change_completed = fragmentedChangePitStop_->process(change_to_add, sampleSize, fragmentStartingNum);

#if HAVE_SECURITY
            if(getAttributes().security_attributes().is_payload_protected)
                releaseCache(change_to_add);
#endif

            // Assertion has to be done before call change_received,
            // because this function can unlock the StatefulReader mutex.
            if(pWP != nullptr)
            {
                pWP->assert_liveliness(); //Asser liveliness since you have received a DATA MESSAGE.
            }

            if(change_completed != nullptr)
            {
                if(!change_received(change_completed, pWP))
                {
                    logInfo(RTPS_MSG_IN, IDSTRING"MessageReceiver not add change " << change_completed->sequenceNumber.to64long());

                    // Assert liveliness because it is a participant discovery info.
                    if(pWP == nullptr && getGuid().entityId == c_EntityId_SPDPReader)
                    {
                        mp_RTPSParticipant->assertRemoteRTPSParticipantLiveliness(incomingChange->writerGUID.guidPrefix);
                    }

                    releaseCache(change_completed);
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
    WriterProxy *pWP = nullptr;

    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(writerGUID, &pWP))
    {
        std::unique_lock<std::recursive_mutex> wpLock(*pWP->get_mutex());

        if(pWP->m_lastHeartbeatCount < hbCount)
        {
            // If it is the first heartbeat message, we can try to cancel initial ack.
            pWP->mp_initialAcknack->cancel_timer();

            pWP->m_lastHeartbeatCount = hbCount;
            pWP->lost_changes_update(firstSN);
            fragmentedChangePitStop_->try_to_remove_until(firstSN, pWP->m_att.guid);
            pWP->missing_changes_update(lastSN);
            pWP->m_heartbeatFinalFlag = finalFlag;

            //Analyze wheter a acknack message is needed:
            if(!finalFlag)
            {
                pWP->mp_heartbeatResponse->restart_timer();
            }
            else if(finalFlag && !livelinessFlag)
            {
                if(pWP->are_there_missing_changes())
                    pWP->mp_heartbeatResponse->restart_timer();
            }

            //FIXME: livelinessFlag
            if(livelinessFlag )//TODOG && WP->m_att->m_qos.m_liveliness.kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
            {
                pWP->assert_liveliness();
            }

            wpLock.unlock();

            // Maybe now we have to notify user from new CacheChanges.
            NotifyChanges(pWP);
        }
    }

    return true;
}

bool StatefulReader::processGapMsg(
        const GUID_t& writerGUID, 
        const SequenceNumber_t& gapStart, 
        const SequenceNumberSet_t& gapList)
{
    WriterProxy *pWP = nullptr;

    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(writerGUID, &pWP))
    {
        std::lock_guard<std::recursive_mutex> guardWriterProxy(*pWP->get_mutex());
        SequenceNumber_t auxSN;
        SequenceNumber_t finalSN = gapList.base() - 1;
        for(auxSN = gapStart; auxSN<=finalSN;auxSN++)
        {
            if(pWP->irrelevant_change_set(auxSN))
                fragmentedChangePitStop_->try_to_remove(auxSN, pWP->m_att.guid);
        }

        gapList.for_each([&](SequenceNumber_t it)
        {
            if(pWP->irrelevant_change_set(it))
                fragmentedChangePitStop_->try_to_remove(it, pWP->m_att.guid);
        });
    }

    return true;
}

bool StatefulReader::acceptMsgFrom(
        const GUID_t& writerId, 
        WriterProxy **wp) const
{
    assert(wp != nullptr);

    for(WriterProxy* it : matched_writers)
    {
        if(it->m_att.guid == writerId)
        {
            *wp = it;
            return true;
        }
    }

    return false;
}

bool StatefulReader::change_removed_by_history(
        CacheChange_t* a_change, 
        WriterProxy* wp)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    if(wp != nullptr || matched_writer_lookup(a_change->writerGUID,&wp))
    {
        wp->change_removed_from_history(a_change->sequenceNumber);
        return true;
    }
    else
    {
        logError(RTPS_READER," You should always find the WP associated with a change, something is very wrong");
    }
    return false;
}

bool StatefulReader::change_received(
        CacheChange_t* a_change, 
        WriterProxy* prox)
{
    //First look for WriterProxy in case is not provided
    if(prox == nullptr)
    {
        if(!findWriterProxy(a_change->writerGUID, &prox))
        {
            logInfo(RTPS_READER, "Writer Proxy " << a_change->writerGUID <<" not matched to this Reader "<< m_guid.entityId);
            return false;
        }
    }

    std::unique_lock<std::recursive_mutex> writerProxyLock(*prox->get_mutex());

    size_t unknown_missing_changes_up_to = prox->unknown_missing_changes_up_to(a_change->sequenceNumber);

    if(this->mp_history->received_change(a_change, unknown_missing_changes_up_to))
    {
        bool ret = prox->received_change_set(a_change->sequenceNumber);

        GUID_t proxGUID = prox->m_att.guid;

        // If KEEP_LAST and history full, make older changes as lost.
        CacheChange_t* aux_change = nullptr;
        if(this->mp_history->isFull() && mp_history->get_min_change_from(&aux_change, proxGUID))
        {
            prox->lost_changes_update(aux_change->sequenceNumber);
            fragmentedChangePitStop_->try_to_remove_until(aux_change->sequenceNumber, proxGUID);
        }

        writerProxyLock.unlock();

        NotifyChanges(prox);

        return ret;
    }

    return false;
}

void StatefulReader::NotifyChanges(WriterProxy* prox)
{
    GUID_t proxGUID = prox->m_att.guid;
    update_last_notified(proxGUID, prox->available_changes_max());
    SequenceNumber_t nextChangeToNotify = prox->next_cache_change_to_be_notified();
    while (nextChangeToNotify != SequenceNumber_t::unknown())
    {
        mp_history->postSemaphore();

        if (getListener() != nullptr)
        {
            CacheChange_t* ch_to_give = nullptr;

            if (mp_history->get_change(nextChangeToNotify, proxGUID, &ch_to_give))
            {
                if (!ch_to_give->isRead)
                {
                    getListener()->onNewCacheChangeAdded((RTPSReader*)this, ch_to_give);
                }
            }

            // Search again the WriterProxy because could be removed after the unlock.
            if (!findWriterProxy(proxGUID, &prox))
                break;
        }

        nextChangeToNotify = prox->next_cache_change_to_be_notified();
    }
}

bool StatefulReader::nextUntakenCache(
        CacheChange_t** change,
        WriterProxy** wpout)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    std::vector<CacheChange_t*> toremove;
    bool takeok = false;
    for(std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
            it!=mp_history->changesEnd();++it)
    {
        WriterProxy* wp;
        if(this->matched_writer_lookup((*it)->writerGUID, &wp))
        {
            // TODO Revisar la comprobacion
            SequenceNumber_t seq = wp->available_changes_max();
            if(seq >= (*it)->sequenceNumber)
            {
                *change = *it;
                if(wpout !=nullptr)
                    *wpout = wp;

                takeok = true;
                break;
                //				if((*it)->kind == ALIVE)
                //				{
                //					this->mp_type->deserialize(&(*it)->serializedPayload,data);
                //				}
                //				(*it)->isRead = true;
                //				if(info!=NULL)
                //				{
                //					info->sampleKind = (*it)->kind;
                //					info->writerGUID = (*it)->writerGUID;
                //					info->sourceTimestamp = (*it)->sourceTimestamp;
                //					info->iHandle = (*it)->instanceHandle;
                //					if(this->m_qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
                //						info->ownershipStrength = wp->m_data->m_qos.m_ownershipStrength.value;
                //				}
                //				m_reader_cache.decreaseUnreadCount();
                //				logInfo(RTPS_READER,this->getGuid().entityId<<": reading change "<< (*it)->sequenceNumber.to64long());
                //				readok = true;
                //				break;
            }
        }
        else
        {
            toremove.push_back((*it));
        }
    }

    for(std::vector<CacheChange_t*>::iterator it = toremove.begin();
            it!=toremove.end();++it)
    {
        logWarning(RTPS_READER,"Removing change "<<(*it)->sequenceNumber << " from " << (*it)->writerGUID << " because is no longer paired");
        mp_history->remove_change(*it);
    }
    return takeok;
}

// TODO Porque elimina aqui y no cuando hay unpairing
bool StatefulReader::nextUnreadCache(
        CacheChange_t** change,
        WriterProxy** wpout)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    std::vector<CacheChange_t*> toremove;
    bool readok = false;
    for(std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
            it!=mp_history->changesEnd();++it)
    {
        if((*it)->isRead)
            continue;
        WriterProxy* wp;
        if(this->matched_writer_lookup((*it)->writerGUID,&wp))
        {
            SequenceNumber_t seq;
            seq = wp->available_changes_max();
            if(seq >= (*it)->sequenceNumber)
            {
                *change = *it;
                if(wpout !=nullptr)
                    *wpout = wp;

                readok = true;
                break;
                //				if((*it)->kind == ALIVE)
                //				{
                //					this->mp_type->deserialize(&(*it)->serializedPayload,data);
                //				}
                //				(*it)->isRead = true;
                //				if(info!=NULL)
                //				{
                //					info->sampleKind = (*it)->kind;
                //					info->writerGUID = (*it)->writerGUID;
                //					info->sourceTimestamp = (*it)->sourceTimestamp;
                //					info->iHandle = (*it)->instanceHandle;
                //					if(this->m_qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
                //						info->ownershipStrength = wp->m_data->m_qos.m_ownershipStrength.value;
                //				}
                //				m_reader_cache.decreaseUnreadCount();
                //				logInfo(RTPS_READER,this->getGuid().entityId<<": reading change "<< (*it)->sequenceNumber.to64long());
                //				readok = true;
                //				break;
            }
        }
        else
        {
            toremove.push_back((*it));
        }
    }

    for(std::vector<CacheChange_t*>::iterator it = toremove.begin();
            it!=toremove.end();++it)
    {
        logWarning(RTPS_READER,"Removing change "<<(*it)->sequenceNumber << " from " << (*it)->writerGUID << " because is no longer paired");
        mp_history->remove_change(*it);
    }

    return readok;
}

bool StatefulReader::updateTimes(const ReaderTimes& ti)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    if(m_times.heartbeatResponseDelay != ti.heartbeatResponseDelay)
    {
        m_times = ti;
        for(std::vector<WriterProxy*>::iterator wit = this->matched_writers.begin();
                wit!=this->matched_writers.end();++wit)
        {
            (*wit)->mp_heartbeatResponse->update_interval(m_times.heartbeatResponseDelay);
        }
    }
    return true;
}

bool StatefulReader::isInCleanState() const
{
    bool cleanState = true;
    std::unique_lock<std::recursive_mutex> lock(*mp_mutex);

    for (WriterProxy* wp : matched_writers)
    {
        if (wp->number_of_changes_from_writer() != 0)
        {
            cleanState = false;
            break;
        }
    }

    return cleanState;
}

void StatefulReader::send_acknack(
        const SequenceNumberSet_t& sns,
        RTPSMessageGroup_t& buffer,
        const LocatorList_t& locators,
        const std::vector<GUID_t>& guids,
        bool is_final)
{

    Count_t acknackCount = 0;

    {//BEGIN PROTECTION
        std::lock_guard<std::recursive_mutex> guard_reader(*mp_mutex);
        m_acknackCount++;
        acknackCount = m_acknackCount;
    }


    logInfo(RTPS_READER, "Sending ACKNACK: " << sns);

    RTPSMessageGroup group(getRTPSParticipant(), this, RTPSMessageGroup::READER, buffer, locators, guids);

    group.add_acknack(guids, sns, acknackCount, is_final, locators);

}
