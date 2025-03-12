// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPStatelessWriter.cpp
 */

#include <rtps/builtin/discovery/participant/simple/PDPStatelessWriter.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <set>
#include <vector>

#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/writer/StatelessWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

PDPStatelessWriter::PDPStatelessWriter(
        RTPSParticipantImpl* participant,
        const GUID_t& guid,
        const WriterAttributes& attributes,
        FlowController* flow_controller,
        WriterHistory* history,
        WriterListener* listener)
    : StatelessWriter(participant, guid, attributes, flow_controller, history, listener)
    , interested_readers_(participant->get_attributes().allocation.participants)
{
}

bool PDPStatelessWriter::matched_reader_add_edp(
        const ReaderProxyData& data)
{
    bool ret = StatelessWriter::matched_reader_add_edp(data);
    if (ret)
    {
        // Mark new reader as interested
        add_interested_reader(data.guid);
        // Send announcement to new reader
        reschedule_all_samples();
    }
    return ret;
}

bool PDPStatelessWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    bool ret = StatelessWriter::matched_reader_remove(reader_guid);
    if (ret)
    {
        // Mark reader as not interested
        remove_interested_reader(reader_guid);
    }
    return ret;
}

void PDPStatelessWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    mark_all_readers_interested();
    StatelessWriter::unsent_change_added_to_history(change, max_blocking_time);
}

void PDPStatelessWriter::set_initial_peers(
        const LocatorList& locator_list)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    initial_peers_.push_back(locator_list);
    mp_RTPSParticipant->createSenderResources(initial_peers_);
}

void PDPStatelessWriter::send_periodic_announcement()
{
    mark_all_readers_interested();
    reschedule_all_samples();
}

bool PDPStatelessWriter::send_to_fixed_locators(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    bool ret = true;

    if (should_reach_all_destinations_)
    {
        ret = initial_peers_.empty() ||
                mp_RTPSParticipant->sendSync(buffers, total_bytes, m_guid,
                        Locators(initial_peers_.begin()), Locators(initial_peers_.end()),
                        max_blocking_time_point);

        if (ret)
        {
            fixed_locators_.clear();
            should_reach_all_destinations_ = false;
        }
    }
    else
    {
        interested_readers_.clear();
    }

    return ret;
}

bool PDPStatelessWriter::is_relevant(
        const fastdds::rtps::CacheChange_t& /* change */,
        const fastdds::rtps::GUID_t& reader_guid) const
{
    return interested_readers_.end() !=
           std::find(interested_readers_.begin(), interested_readers_.end(), reader_guid);
}

void PDPStatelessWriter::mark_all_readers_interested()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    should_reach_all_destinations_ = true;
    interested_readers_.clear();
    fixed_locators_.clear();
    fixed_locators_.push_back(initial_peers_);
    reader_data_filter(nullptr);
}

void PDPStatelessWriter::add_interested_reader(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    if (!should_reach_all_destinations_)
    {
        auto it = std::find(interested_readers_.begin(), interested_readers_.end(), reader_guid);
        if (it == interested_readers_.end())
        {
            interested_readers_.emplace_back(reader_guid);
            reader_data_filter(this);
        }
    }
}

void PDPStatelessWriter::remove_interested_reader(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    interested_readers_.remove(reader_guid);
}

void PDPStatelessWriter::reschedule_all_samples()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    size_t n_samples = history_->getHistorySize();
    if (0 < n_samples)
    {
        assert(1 == n_samples);
        auto it = history_->changesBegin();
        CacheChange_t* change = *it;
        flow_controller_->add_new_sample(this, change, std::chrono::steady_clock::now() + std::chrono::hours(24));
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
