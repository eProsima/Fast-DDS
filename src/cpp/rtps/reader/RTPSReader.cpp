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
 * RTPSReader.cpp
 *
*/

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/log/Log.h>
#include "FragmentedChangePitStop.h"

#include <fastrtps/rtps/reader/ReaderListener.h>

#include <typeinfo>

namespace eprosima {
namespace fastrtps{
namespace rtps {

RTPSReader::RTPSReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        ReaderHistory* hist,
        ReaderListener* rlisten)
    : Endpoint(pimpl,guid,att.endpoint)
    , mp_history(hist)
    , mp_listener(rlisten)
    , m_acceptMessagesToUnknownReaders(true)
    , m_acceptMessagesFromUnkownWriters(true)
    , m_expectsInlineQos(att.expectsInlineQos)
    , fragmentedChangePitStop_(nullptr)
{
    mp_history->mp_reader = this;
    mp_history->mp_mutex = mp_mutex;
    fragmentedChangePitStop_ = new FragmentedChangePitStop(this);
    logInfo(RTPS_READER,"RTPSReader created correctly");
}

RTPSReader::~RTPSReader()
{
    logInfo(RTPS_READER,"Removing reader "<<this->getGuid().entityId;);
    delete fragmentedChangePitStop_;
    mp_history->mp_reader = nullptr;
    mp_history->mp_mutex = nullptr;
}

bool RTPSReader::acceptMsgDirectedTo(const EntityId_t& entityId) const
{
    if(entityId == m_guid.entityId)
        return true;
    if(m_acceptMessagesToUnknownReaders && entityId == c_EntityId_Unknown)
        return true;
    else
        return false;
}

bool RTPSReader::reserveCache(
        CacheChange_t** change, 
        uint32_t dataCdrSerializedSize)
{
    return mp_history->reserve_Cache(change, dataCdrSerializedSize);
}

void RTPSReader::releaseCache(CacheChange_t* change)
{
    return mp_history->release_Cache(change);
}

ReaderListener* RTPSReader::getListener() const
{
    return mp_listener;
}

bool RTPSReader::setListener(ReaderListener *target)
{
    mp_listener = target;
    return true;
}

CacheChange_t* RTPSReader::findCacheInFragmentedCachePitStop(
        const SequenceNumber_t& sequence_number,
        const GUID_t& writer_guid) const
{
    return fragmentedChangePitStop_->find(sequence_number, writer_guid);
}

void RTPSReader::add_persistence_guid(const RemoteWriterAttributes& wdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    persistence_guid_map_[wdata.guid] = wdata.endpoint.persistence_guid;
    persistence_guid_count_[wdata.endpoint.persistence_guid]++;
}

void RTPSReader::remove_persistence_guid(const RemoteWriterAttributes& wdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    persistence_guid_map_.erase(wdata.guid);
    auto count = --persistence_guid_count_[wdata.endpoint.persistence_guid];
    if (count == 0)
    {
        if (m_att.durabilityKind < TRANSIENT)
        {
            history_record_.erase(wdata.endpoint.persistence_guid);
        }
    }
}

SequenceNumber_t RTPSReader::update_last_notified(
        const GUID_t& guid, 
        const SequenceNumber_t& seq)
{
    SequenceNumber_t ret_val;
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    GUID_t guid_to_look = guid;
    auto p_guid = persistence_guid_map_.find(guid);
    if (p_guid != persistence_guid_map_.end())
    {
        guid_to_look = p_guid->second;
    }

    auto p_seq = history_record_.find(guid_to_look);
    if (p_seq != history_record_.end())
    {
        ret_val = p_seq->second;
    }

    if (ret_val < seq)
    {
        set_last_notified(guid_to_look, seq);
    }

    return ret_val;
}

SequenceNumber_t RTPSReader::get_last_notified(const GUID_t& guid) const
{
    SequenceNumber_t ret_val;
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    GUID_t guid_to_look = guid;
    auto p_guid = persistence_guid_map_.find(guid);
    if (p_guid != persistence_guid_map_.end())
    {
        guid_to_look = p_guid->second;
    }

    auto p_seq = history_record_.find(guid_to_look);
    if (p_seq != history_record_.end())
    {
        ret_val = p_seq->second;
    }

    return ret_val;
}

void RTPSReader::set_last_notified(
        const GUID_t& peristence_guid, 
        const SequenceNumber_t& seq)
{
    history_record_[peristence_guid] = seq;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
