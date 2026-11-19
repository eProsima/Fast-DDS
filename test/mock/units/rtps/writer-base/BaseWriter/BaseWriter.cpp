// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of BaseWriter: replaces src/cpp/rtps/writer/BaseWriter.cpp. The writer is not registered in
// its flow controller, nothing is sent, statistics are not collected, and the queries tests control are forwarded to
// the BaseWriterMock companion of the instance.

#include <rtps/writer/BaseWriter.hpp>

#include <algorithm>

#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>
#include <mock/rtps/writer/BaseWriterMock.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<BaseWriter>;

BaseWriter::BaseWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& att,
        FlowController* flow_controller,
        WriterHistory* hist,
        WriterListener* listen)
    : RTPSWriter(impl, guid, att)
    , flow_controller_(flow_controller)
    , history_(hist)
    , listener_(listen)
    , is_async_(att.mode == SYNCHRONOUS_WRITER ? false : true)
    , separate_sending_enabled_(att.separate_sending)
    , liveliness_kind_(att.liveliness_kind)
    , liveliness_lease_duration_(att.liveliness_lease_duration)
    , liveliness_announcement_period_(att.liveliness_announcement_period)
    , transport_priority_(att.transport_priority)
{
    Companion::attach(this);
    if (nullptr != history_)
    {
        history_->mp_writer = this;
    }
}

BaseWriter::~BaseWriter()
{
    if (nullptr != history_ && this == history_->mp_writer)
    {
        history_->mp_writer = nullptr;
    }
    Companion::detach(this);
}

BaseWriter* BaseWriter::downcast(
        RTPSWriter* writer)
{
    return static_cast<BaseWriter*>(writer);
}

BaseWriter* BaseWriter::downcast(
        Endpoint* endpoint)
{
    return static_cast<BaseWriter*>(endpoint);
}

bool BaseWriter::matched_reader_add(
        const SubscriptionBuiltinTopicData&)
{
    return false;
}

WriterListener* BaseWriter::get_listener() const
{
    return listener_;
}

bool BaseWriter::set_listener(
        WriterListener* listener)
{
    listener_ = listener;
    return true;
}

bool BaseWriter::is_async() const
{
    return is_async_;
}

int32_t BaseWriter::get_transport_priority() const
{
    return transport_priority_;
}

void BaseWriter::update_attributes(
        const WriterAttributes& att)
{
    transport_priority_ = att.transport_priority;
}

void BaseWriter::local_actions_on_writer_removed()
{
}

bool BaseWriter::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener>)
{
    return true;
}

bool BaseWriter::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener>)
{
    return true;
}

void BaseWriter::set_enabled_statistics_writers_mask(
        uint32_t)
{
}

bool BaseWriter::send_nts(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        const LocatorSelectorSender& locator_selector,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    return Companion::get(this).send_nts(buffers, total_bytes, locator_selector, max_blocking_time_point);
}

uint32_t BaseWriter::get_max_allowed_payload_size()
{
    return Companion::get(this).get_max_allowed_payload_size();
}

const dds::LivelinessQosPolicyKind& BaseWriter::get_liveliness_kind() const
{
    return Companion::get(this).get_liveliness_kind();
}

const dds::Duration_t& BaseWriter::get_liveliness_lease_duration() const
{
    return Companion::get(this).get_liveliness_lease_duration();
}

const dds::Duration_t& BaseWriter::get_liveliness_announcement_period() const
{
    return Companion::get(this).get_liveliness_announcement_period();
}

void BaseWriter::liveliness_lost()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);
    liveliness_lost_status_.total_count++;
    liveliness_lost_status_.total_count_change++;
    if (listener_ != nullptr)
    {
        listener_->on_liveliness_lost(this, liveliness_lost_status_);
    }
    liveliness_lost_status_.total_count_change = 0u;
}

bool BaseWriter::is_datasharing_compatible() const
{
    return Companion::get(this).is_datasharing_compatible();
}

bool BaseWriter::is_datasharing_compatible_with(
        const dds::DataSharingQosPolicy& qos) const
{
    return Companion::get(this).is_datasharing_compatible_with(qos);
}

SequenceNumber_t BaseWriter::get_seq_num_min()
{
    return Companion::get(this).get_seq_num_min();
}

SequenceNumber_t BaseWriter::get_seq_num_max()
{
    return Companion::get(this).get_seq_num_max();
}

void BaseWriter::init(
        const WriterAttributes&)
{
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
        LocatorSelectorSender&)
{
}

void BaseWriter::add_statistics_sent_submessage(
        CacheChange_t*,
        size_t)
{
}

uint32_t BaseWriter::calculate_max_payload_size(
        uint32_t datagram_length)
{
    return datagram_length;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
