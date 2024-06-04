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
    , history_(hist)
    , listener_(rlisten)
    , accept_messages_from_unkown_writers_(att.accept_messages_from_unkown_writers)
    , expects_inline_qos_(att.expects_inline_qos)
{
    history_->mp_reader = this;
    history_->mp_mutex = &mp_mutex;
}

RTPSReader::~RTPSReader()
{
    history_->mp_reader = nullptr;
    history_->mp_mutex = nullptr;
}

ReaderListener* RTPSReader::get_listener() const
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return listener_;
}

bool RTPSReader::set_listener(
        ReaderListener* target)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    listener_ = target;
    return true;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
