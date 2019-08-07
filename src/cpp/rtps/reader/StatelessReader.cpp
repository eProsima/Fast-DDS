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

/*
 * @file StatelessReader.cpp
 *
 */

#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>
#include <fastrtps/rtps/writer/LivelinessManager.h>
#include "../participant/RTPSParticipantImpl.h"
#include "FragmentedChangePitStop.h"

#include <mutex>
#include <thread>

#include <cassert>

#define IDSTRING "(ID:" << std::this_thread::get_id() << ") " <<

using namespace eprosima::fastrtps::rtps;


StatelessReader::~StatelessReader()
{
    logInfo(RTPS_READER, "Removing reader " << this->getGuid());
}

StatelessReader::StatelessReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        ReaderHistory* hist,
        ReaderListener* listen)
    : RTPSReader(pimpl, guid, att, hist, listen)
    , matched_writers_(att.matched_writers_allocation)
{
}

bool StatelessReader::matched_writer_add(
        const WriterProxyData& wdata)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    for (const RemoteWriterInfo_t& writer : matched_writers_)
    {
        if (writer.guid == wdata.guid())
        {
            logWarning(RTPS_READER, "Attempting to add existing writer");
            return false;
        }
    }

    RemoteWriterInfo_t info;
    info.guid = wdata.guid();
    info.persistence_guid = wdata.persistence_guid();
    info.has_manual_topic_liveliness = (MANUAL_BY_TOPIC_LIVELINESS_QOS == wdata.m_qos.m_liveliness.kind);
    RemoteWriterInfo_t* att = matched_writers_.emplace_back(info);
    if (att != nullptr)
    {
        add_persistence_guid(info.guid, info.persistence_guid);

        m_acceptMessagesFromUnkownWriters = false;
        logInfo(RTPS_READER, "Writer " << info.guid << " added to " << m_guid.entityId);

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
                logError(RTPS_LIVELINESS, "Finite liveliness lease duration but WLP not enabled");
            }
        }
        
        return true;
    }

    logWarning(RTPS_READER, "No space to add writer " << wdata.guid() << " to reader " << m_guid.entityId);
    return false;
}

bool StatelessReader::matched_writer_remove(const GUID_t& writer_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    ResourceLimitedVector<RemoteWriterInfo_t>::iterator it;
    for (it = matched_writers_.begin(); it != matched_writers_.end(); ++it)
    {
        if (it->guid == writer_guid)
        {
            logInfo(RTPS_READER, "Writer " << writer_guid << " removed from " << m_guid.entityId);

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

            remove_persistence_guid(it->guid, it->persistence_guid);
            matched_writers_.erase(it);

            return true;
        }
    }

    return false;
}

bool StatelessReader::matched_writer_is_matched(const GUID_t& writer_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return std::any_of(matched_writers_.begin(), matched_writers_.end(), 
        [writer_guid](const RemoteWriterInfo_t& item)
        {
            return item.guid == writer_guid;
        });
}

bool StatelessReader::change_received(CacheChange_t* change)
{
    // Only make visible the change if there is not other with bigger sequence number.
    // TODO Revisar si no hay que incluirlo.
    if(!thereIsUpperRecordOf(change->writerGUID, change->sequenceNumber))
    {
        if(mp_history->received_change(change, 0))
        {
            update_last_notified(change->writerGUID, change->sequenceNumber);
            ++total_unread_;

            if(getListener() != nullptr)
            {
                getListener()->onNewCacheChangeAdded(this, change);
            }

            new_notification_cv_.notify_all();

            return true;
        }
    }

    return false;
}

bool StatelessReader::nextUntakenCache(
        CacheChange_t** change, 
        WriterProxy** /*wpout*/)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    bool ret = mp_history->get_min_change(change);

    if(ret)
    {
        if(!(*change)->isRead)
        {
            if (0 < total_unread_)
            {
                --total_unread_;
            }
        }

        (*change)->isRead = true;
    }

    return ret;
}


bool StatelessReader::nextUnreadCache(
        CacheChange_t** change,
        WriterProxy** /*wpout*/)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    bool found = false;
    std::vector<CacheChange_t*>::iterator it;

    for(it = mp_history->changesBegin();
            it!=mp_history->changesEnd();++it)
    {
        if(!(*it)->isRead)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        *change = *it;
        if (0 < total_unread_)
        {
            --total_unread_;
        }
        (*change)->isRead = true;

        return true;
    }

    logInfo(RTPS_READER, "No Unread elements left");
    return false;
}


bool StatelessReader::change_removed_by_history(
        CacheChange_t* ch,
        WriterProxy* /*prox*/)
{
    if(!ch->isRead)
    {
        if (0 < total_unread_)
        {
            --total_unread_;
        }
    }

    return true;
}

bool StatelessReader::processDataMsg(CacheChange_t *change)
{
    assert(change);

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    if(acceptMsgFrom(change->writerGUID))
    {
        logInfo(RTPS_MSG_IN, IDSTRING "Trying to add change " << change->sequenceNumber << " TO reader: "
            << getGuid().entityId);

        if (liveliness_lease_duration_ < c_TimeInfinite)
        {
            if (liveliness_kind_ == MANUAL_BY_TOPIC_LIVELINESS_QOS ||
                writer_has_manual_liveliness(change->writerGUID))
            {
                auto wlp = this->mp_RTPSParticipant->wlp();
                if ( wlp != nullptr)
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

        CacheChange_t* change_to_add;

        //Reserve a new cache from the corresponding cache pool
        if(reserveCache(&change_to_add, change->serializedPayload.length))
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
                    logWarning(RTPS_MSG_IN, IDSTRING "Problem copying CacheChange, received data is: " 
                        << change->serializedPayload.length << " bytes and max size in reader " 
                        << getGuid().entityId << " is " << change_to_add->serializedPayload.max_size);
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

        if(!change_received(change_to_add))
        {
            logInfo(RTPS_MSG_IN, IDSTRING "MessageReceiver not add change " << change_to_add->sequenceNumber);
            releaseCache(change_to_add);

            if(getGuid().entityId == c_EntityId_SPDPReader)
            {
                mp_RTPSParticipant->assertRemoteRTPSParticipantLiveliness(change->writerGUID.guidPrefix);
            }
        }
    }

    return true;
}

bool StatelessReader::processDataFragMsg(
        CacheChange_t* incomingChange, 
        uint32_t sampleSize, 
        uint32_t fragmentStartingNum)
{
    assert(incomingChange);

    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

    if (acceptMsgFrom(incomingChange->writerGUID))
    {
        if (liveliness_lease_duration_ < c_TimeInfinite)
        {
            if (liveliness_kind_ == MANUAL_BY_TOPIC_LIVELINESS_QOS ||
                writer_has_manual_liveliness(incomingChange->writerGUID))
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
        if(!thereIsUpperRecordOf(incomingChange->writerGUID, incomingChange->sequenceNumber))
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

            // Try to remove previous CacheChange_t from PitStop.
            fragmentedChangePitStop_->try_to_remove_until(incomingChange->sequenceNumber, incomingChange->writerGUID);

            // Fragments manager has to process incomming fragments.
            // If CacheChange_t is completed, it will be returned;
            CacheChange_t* change_completed = fragmentedChangePitStop_->process(change_to_add, sampleSize, fragmentStartingNum);

#if HAVE_SECURITY
            if(getAttributes().security_attributes().is_payload_protected)
                releaseCache(change_to_add);
#endif

            // If the change was completed, process it.
            if(change_completed != nullptr)
            {
                if (!change_received(change_completed))
                {
                    logInfo(RTPS_MSG_IN, IDSTRING"MessageReceiver not add change " << change_completed->sequenceNumber.to64long());

                    // Assert liveliness because if it is a participant discovery info.
                    if (getGuid().entityId == c_EntityId_SPDPReader)
                    {
                        mp_RTPSParticipant->assertRemoteRTPSParticipantLiveliness(incomingChange->writerGUID.guidPrefix);
                    }

                    // Release CacheChange_t.
                    releaseCache(change_completed);
                }
            }
        }
    }

    return true;
}

bool StatelessReader::processHeartbeatMsg(
        const GUID_t& /*writerGUID*/,
        uint32_t /*hbCount*/,
        const SequenceNumber_t& /*firstSN*/,
        const SequenceNumber_t& /*lastSN*/,
        bool /*finalFlag*/,
        bool /*livelinessFlag*/)
{
    return true;
}

bool StatelessReader::processGapMsg(
        const GUID_t& /*writerGUID*/,
        const SequenceNumber_t& /*gapStart*/,
        const SequenceNumberSet_t& /*gapList*/)
{
    return true;
}

bool StatelessReader::acceptMsgFrom(const GUID_t& writerId)
{
    if(this->m_acceptMessagesFromUnkownWriters)
    {
        return true;
    }
    else if (writerId.entityId == this->m_trustedWriterEntityId)
    {
        return true;
    }

    return std::any_of(matched_writers_.begin(), matched_writers_.end(), 
        [& writerId](const RemoteWriterInfo_t& writer)
        {
            return writer.guid == writerId;
        });
}

bool StatelessReader::thereIsUpperRecordOf(
        const GUID_t& guid, 
        const SequenceNumber_t& seq)
{
    return get_last_notified(guid) >= seq;
}

bool StatelessReader::writer_has_manual_liveliness(const GUID_t& guid)
{
    for (const RemoteWriterInfo_t& writer : matched_writers_)
    {
        if (writer.guid == guid)
        {
            return writer.has_manual_topic_liveliness;
        }
    }
    return false;
}
