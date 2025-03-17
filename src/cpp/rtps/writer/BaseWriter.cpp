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

#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/DataSharing/WriterPool.hpp>
#include <rtps/flowcontrol/FlowController.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>
#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>

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
    : RTPSWriter(impl, guid, att)
    , flow_controller_(flow_controller)
    , history_(hist)
    , listener_(listen)
    , is_async_(att.mode == SYNCHRONOUS_WRITER ? false : true)
    , separate_sending_enabled_(att.separate_sending)
    , liveliness_kind_(att.liveliness_kind)
    , liveliness_lease_duration_(att.liveliness_lease_duration)
    , liveliness_announcement_period_(att.liveliness_announcement_period)
{
    init(att);

    history_->mp_writer = this;
    history_->mp_mutex = &mp_mutex;

    flow_controller_->register_writer(this);

    EPROSIMA_LOG_INFO(RTPS_WRITER, "RTPSWriter created");
}

BaseWriter* BaseWriter::downcast(
        RTPSWriter* writer)
{
    assert(nullptr != dynamic_cast<BaseWriter*>(writer));
    return static_cast<BaseWriter*>(writer);
}

BaseWriter* BaseWriter::downcast(
        Endpoint* endpoint)
{
    assert(nullptr != dynamic_cast<BaseWriter*>(endpoint));
    return static_cast<BaseWriter*>(endpoint);
}

BaseWriter::~BaseWriter()
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "RTPSWriter destructor");

    // Deletion of the events has to be made in child destructor.
    // Also at this point all CacheChange_t must have been released by the child destructor

    history_->mp_writer = nullptr;
    history_->mp_mutex = nullptr;
}

bool BaseWriter::matched_reader_add(
        const SubscriptionBuiltinTopicData& rqos)
{
    const auto& alloc = mp_RTPSParticipant->get_attributes().allocation;
    ReaderProxyData rdata(alloc.data_limits, rqos);

    return matched_reader_add_edp(rdata);
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

uint32_t BaseWriter::get_max_allowed_payload_size()
{
    uint32_t flow_max = flow_controller_->get_max_payload();
    uint32_t part_max = mp_RTPSParticipant->getMaxMessageSize();
    uint32_t max_size = flow_max > part_max ? part_max : flow_max;
    if (max_output_message_size_ < max_size)
    {
        max_size = max_output_message_size_;
    }

    max_size = calculate_max_payload_size(max_size);
    return max_size &= ~3;
}

uint32_t BaseWriter::calculate_max_payload_size(
        uint32_t datagram_length)
{
    constexpr uint32_t info_dst_message_length = 16;
    constexpr uint32_t info_ts_message_length = 12;
    constexpr uint32_t data_frag_submessage_header_length = 36;
    constexpr uint32_t heartbeat_message_length = 32;

    uint32_t max_data_size = mp_RTPSParticipant->calculateMaxDataSize(datagram_length);
    uint32_t overhead = info_dst_message_length +
            info_ts_message_length +
            data_frag_submessage_header_length +
            heartbeat_message_length;

#if HAVE_SECURITY
    if (getAttributes().security_attributes().is_submessage_protected)
    {
        overhead += mp_RTPSParticipant->security_manager().calculate_extra_size_for_rtps_submessage(m_guid);
    }

    if (getAttributes().security_attributes().is_payload_protected)
    {
        overhead += mp_RTPSParticipant->security_manager().calculate_extra_size_for_encoded_payload(m_guid);
    }
#endif // if HAVE_SECURITY

#ifdef FASTDDS_STATISTICS
    overhead += eprosima::fastdds::statistics::rtps::statistics_submessage_length;
#endif // FASTDDS_STATISTICS

    constexpr uint32_t min_fragment_size = 4;
    if ((overhead + min_fragment_size) > max_data_size)
    {
        auto min_datagram_length = overhead + min_fragment_size + 1 + (datagram_length - max_data_size);
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Datagram length '" << datagram_length << "' is too small." <<
                "At least " << min_datagram_length << " bytes are needed to send a message. Fixing fragments to " <<
                min_fragment_size << " bytes.");
        return min_fragment_size;
    }

    max_data_size -= overhead;
    return max_data_size;
}

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
    RTPSParticipantImpl* participant = get_participant_impl();

    return locator_selector.locator_selector.selected_size() == 0 ||
           participant->sendSync(buffers, total_bytes, m_guid, locator_selector.locator_selector.begin(),
                   locator_selector.locator_selector.end(), max_blocking_time_point);
}

const dds::LivelinessQosPolicyKind& BaseWriter::get_liveliness_kind() const
{
    return liveliness_kind_;
}

const dds::Duration_t& BaseWriter::get_liveliness_lease_duration() const
{
    return liveliness_lease_duration_;
}

const dds::Duration_t& BaseWriter::get_liveliness_announcement_period() const
{
    return liveliness_announcement_period_;
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
    return (m_att.data_sharing_configuration().kind() != dds::OFF);
}

bool BaseWriter::is_datasharing_compatible_with(
        const dds::DataSharingQosPolicy& qos) const
{
    if (!is_datasharing_compatible() || qos.kind() == fastdds::dds::OFF)
    {
        return false;
    }

    for (auto id : qos.domain_ids())
    {
        if (std::find(m_att.data_sharing_configuration().domain_ids().begin(),
                m_att.data_sharing_configuration().domain_ids().end(), id)
                != m_att.data_sharing_configuration().domain_ids().end())
        {
            return true;
        }
    }
    return false;
}

SequenceNumber_t BaseWriter::get_seq_num_min()
{
    CacheChange_t* change;
    if (history_->get_min_change(&change) && change != nullptr)
    {
        return change->sequenceNumber;
    }
    else
    {
        return c_SequenceNumber_Unknown;
    }
}

SequenceNumber_t BaseWriter::get_seq_num_max()
{
    CacheChange_t* change;
    if (history_->get_max_change(&change) && change != nullptr)
    {
        return change->sequenceNumber;
    }
    else
    {
        return c_SequenceNumber_Unknown;
    }
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

void BaseWriter::init(
        const WriterAttributes& att)
{
    {
        const std::string* max_size_property =
                PropertyPolicyHelper::find_property(att.endpoint.properties, "fastdds.max_message_size");
        if (max_size_property != nullptr)
        {
            try
            {
                max_output_message_size_ = std::stoul(*max_size_property);
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error parsing max_message_size property: " << e.what());
            }
        }
    }

    fixed_payload_size_ = 0;
    if (history_->m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE)
    {
        fixed_payload_size_ = history_->m_att.payloadMaxSize;
    }

    if (att.endpoint.data_sharing_configuration().kind() != dds::OFF)
    {
        std::shared_ptr<WriterPool> pool = std::dynamic_pointer_cast<WriterPool>(history_->get_payload_pool());
        if (!pool || !pool->init_shared_memory(this, att.endpoint.data_sharing_configuration().shm_directory()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Could not initialize DataSharing writer pool");
        }
    }
}

void BaseWriter::local_actions_on_writer_removed()
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
