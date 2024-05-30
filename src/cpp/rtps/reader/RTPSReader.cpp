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
#include <fastdds/rtps/reader/RTPSReader.h>

#include <typeinfo>
#include <algorithm>
#include <chrono>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/reader/ReaderListener.h>
#include <foonathan/memory/namespace_alias.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/DataSharing/DataSharingListener.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/reader/ReaderHistoryState.hpp>
#include <statistics/rtps/StatisticsBase.hpp>


namespace eprosima {
namespace fastrtps {
namespace rtps {

RTPSReader::RTPSReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        ReaderHistory* hist,
        ReaderListener* rlisten)
    : Endpoint(pimpl, guid, att.endpoint)
    , mp_history(hist)
    , mp_listener(rlisten)
    , m_acceptMessagesToUnknownReaders(true)
    , m_acceptMessagesFromUnkownWriters(false)
    , m_expectsInlineQos(att.expectsInlineQos)
{
    PoolConfig cfg = PoolConfig::from_history_attributes(hist->m_att);
    std::shared_ptr<IChangePool> change_pool;
    std::shared_ptr<IPayloadPool> payload_pool;
    payload_pool = BasicPayloadPool::get(cfg, change_pool);

    init(payload_pool, change_pool);
}

RTPSReader::RTPSReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* hist,
        ReaderListener* rlisten)
    : RTPSReader(
        pimpl, guid, att, payload_pool,
        std::make_shared<CacheChangePool>(PoolConfig::from_history_attributes(hist->m_att)),
        hist, rlisten)
{
}

RTPSReader::RTPSReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        ReaderHistory* hist,
        ReaderListener* rlisten)
    : Endpoint(pimpl, guid, att.endpoint)
    , mp_history(hist)
    , mp_listener(rlisten)
    , m_acceptMessagesToUnknownReaders(true)
    , m_acceptMessagesFromUnkownWriters(false)
    , m_expectsInlineQos(att.expectsInlineQos)
{
    init(payload_pool, change_pool);
}

void RTPSReader::init(
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool)
{
    payload_pool_ = payload_pool;
    change_pool_ = change_pool;
    fixed_payload_size_ = 0;
    if (mp_history->m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE)
    {
        fixed_payload_size_ = mp_history->m_att.payloadMaxSize;
    }

    mp_history->mp_reader = this;
    mp_history->mp_mutex = &mp_mutex;

    EPROSIMA_LOG_INFO(RTPS_READER, "RTPSReader created correctly");
}

RTPSReader::~RTPSReader()
{
    mp_history->mp_reader = nullptr;
    mp_history->mp_mutex = nullptr;
}

ReaderListener* RTPSReader::getListener() const
{
    return mp_listener;
}

bool RTPSReader::setListener(
        ReaderListener* target)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    mp_listener = target;
    return true;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
