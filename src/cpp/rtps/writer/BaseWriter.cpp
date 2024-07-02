// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseWriter.cpp
 */

#include <rtps/writer/BaseWriter.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <rtps/flowcontrol/FlowController.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

BaseWriter::BaseWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& att,
        FlowController* flow_controller,
        WriterHistory* hist,
        WriterListener* listen)
    : RTPSWriter(impl, guid, att, flow_controller, hist, listen)
{
    history_->mp_writer = this;
    history_->mp_mutex = &mp_mutex;

    flow_controller_->register_writer(this);

    EPROSIMA_LOG_INFO(RTPS_WRITER, "RTPSWriter created");
}

BaseWriter::~BaseWriter()
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "RTPSWriter destructor");

    // Deletion of the events has to be made in child destructor.
    // Also at this point all CacheChange_t must have been released by the child destructor

    history_->mp_writer = nullptr;
    history_->mp_mutex = nullptr;
}

#ifdef FASTDDS_STATISTICS

bool BaseWriter::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    return add_statistics_listener_impl(listener);
}

bool BaseWriter::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    return remove_statistics_listener_impl(listener);
}

void BaseWriter::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    set_enabled_statistics_writers_mask_impl(enabled_writers);
}

#endif // FASTDDS_STATISTICS

void BaseWriter::add_statistics_sent_submessage(
        CacheChange_t* change,
        size_t num_locators)
{
    static_cast<void>(change);
    static_cast<void>(num_locators);

#ifdef FASTDDS_STATISTICS
    change->writer_info.num_sent_submessages += num_locators;
    on_data_generated(num_locators);
#endif // ifdef FASTDDS_STATISTICS
}

bool BaseWriter::send_nts(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        const LocatorSelectorSender& locator_selector,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    RTPSParticipantImpl* participant = getRTPSParticipant();

    return locator_selector.locator_selector.selected_size() == 0 ||
           participant->sendSync(buffers, total_bytes, m_guid, locator_selector.locator_selector.begin(),
                   locator_selector.locator_selector.end(), max_blocking_time_point);
}

void BaseWriter::add_guid(
        LocatorSelectorSender& locator_selector,
        const GUID_t& remote_guid)
{
    const GuidPrefix_t& prefix = remote_guid.guidPrefix;
    locator_selector.all_remote_readers.push_back(remote_guid);
    if (std::find(locator_selector.all_remote_participants.begin(),
            locator_selector.all_remote_participants.end(), prefix) ==
            locator_selector.all_remote_participants.end())
    {
        locator_selector.all_remote_participants.push_back(prefix);
    }
}

void BaseWriter::compute_selected_guids(
        LocatorSelectorSender& locator_selector)
{
    locator_selector.all_remote_readers.clear();
    locator_selector.all_remote_participants.clear();

    for (LocatorSelectorEntry* entry : locator_selector.locator_selector.transport_starts())
    {
        if (entry->enabled)
        {
            add_guid(locator_selector, entry->remote_guid);
        }
    }
}

void BaseWriter::update_cached_info_nts(
        LocatorSelectorSender& locator_selector)
{
    locator_selector.locator_selector.reset(true);
    mp_RTPSParticipant->network_factory().select_locators(locator_selector.locator_selector);
}

void BaseWriter::deinit()
{
    // First, unregister changes from FlowController. This action must be protected.
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        for (auto it = history_->changesBegin(); it != history_->changesEnd(); ++it)
        {
            flow_controller_->remove_change(*it, std::chrono::steady_clock::now() + std::chrono::hours(24));
        }

        for (auto it = history_->changesBegin(); it != history_->changesEnd(); ++it)
        {
            history_->release_change(*it);
        }

        history_->m_changes.clear();
    }
    flow_controller_->unregister_writer(this);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
