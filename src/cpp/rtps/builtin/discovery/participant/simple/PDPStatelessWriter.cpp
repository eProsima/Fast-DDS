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
#include <chrono>
#include <mutex>

#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/utils/TimedMutex.hpp>

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
{
}

bool PDPStatelessWriter::matched_reader_add_edp(
        const ReaderProxyData& data)
{
    bool ret = StatelessWriter::matched_reader_add_edp(data);
    if (ret)
    {
        // Mark new reader as interested
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
    }
    return ret;
}

void PDPStatelessWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    StatelessWriter::unsent_change_added_to_history(change, max_blocking_time);
    // mark_all_readers_as_interested();
}

bool PDPStatelessWriter::set_fixed_locators(
        const LocatorList_t& locator_list)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    fixed_locators_.push_back(locator_list);
    mp_RTPSParticipant->createSenderResources(fixed_locators_);

    return true;
}

void PDPStatelessWriter::unsent_changes_reset()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    std::for_each(history_->changesBegin(), history_->changesEnd(), [&](CacheChange_t* change)
    {
        flow_controller_->add_new_sample(this, change,
            std::chrono::steady_clock::now() + std::chrono::hours(24));
    });
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
