/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <rtps/congestion-control/basic/CongestionControlBasic.hpp>

#include <cstdint>
#include <map>
#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>

#include <rtps/congestion-control/CongestionControlParameters.hpp>
#include <rtps/congestion-control/ICongestionControl.hpp>
#include <rtps/flowcontrol/FlowController.hpp>
#include <rtps/flowcontrol/FlowControllerFactory.hpp>
#include <rtps/flowcontrol/GrainedFlowController.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/writer/StatefulWriterListener.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

CongestionControlBasic::~CongestionControlBasic()
{
    periodic_timer_.reset();
}

bool CongestionControlBasic::initialize(
        RTPSParticipantImpl& participant,
        FlowControllerFactory& flow_controller_factory,
        const CongestionControlParameters& params)
{
    std::lock_guard<std::mutex> lock(readers_mutex_);

    if (flow_controller_ != nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "CongestionControlBasic already initialized.");
        return false;
    }

    uint32_t max_data_size = participant.getMaxDataSize();
    GrainedFlowController* gfc = flow_controller_factory.register_flow_controller_type<GrainedFlowController>(
        grained_flow_controller_name,
        participant.get_attributes().builtin_controllers_sender_thread, max_data_size, 1000);

    // If the flow controller could not be created, it may have already been registered
    if (gfc == nullptr)
    {
        FlowController* fc = flow_controller_factory.retrieve_flow_controller(
            grained_flow_controller_name, WriterAttributes{});
        gfc = dynamic_cast<GrainedFlowController*>(fc);
    }

    // Log error and return false if we could not obtain the flow controller
    if (gfc == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Error obtaining GrainedFlowController.");
        return false;
    }

    TimedEvent* timer = new TimedEvent(participant.getEventResource(),
                    [this]() -> bool
                    {
                        bool reschedule_timer = true;
                        flow_controller_limitations_update(reschedule_timer);
                        return reschedule_timer;
                    },
                    params.period_duration_ms);
    periodic_timer_.reset(timer);
    parameters_ = params;
    minimum_bandwidth_limitation_ = max_data_size;
    flow_controller_ = gfc;

    return true;
}

WriterAttributes CongestionControlBasic::prepare_writer_attributes(
        const WriterAttributes& original_attributes,
        const EntityId_t& /*entity_id*/,
        bool is_builtin)
{
    // Only modify attributes for reliable non-builtin writers
    const EndpointAttributes& endpoint = original_attributes.endpoint;
    if ((RELIABLE != endpoint.reliabilityKind) || is_builtin)
    {
        return original_attributes;
    }

    // Warn if the original attributes already specify a flow controller
    if (!original_attributes.flow_controller_name.empty() &&
            (original_attributes.flow_controller_name != grained_flow_controller_name) &&
            (original_attributes.flow_controller_name != fastdds::rtps::FASTDDS_FLOW_CONTROLLER_DEFAULT))
    {
        EPROSIMA_LOG_WARNING(RTPS_PARTICIPANT, "Specified flow controller '"
                << original_attributes.flow_controller_name
                << "' will be overridden with builtin congestion-control flow controller.");
    }

    WriterAttributes attr = original_attributes;
    attr.flow_controller_name = grained_flow_controller_name;
    attr.mode = ASYNCHRONOUS_WRITER;
    return attr;
}

void CongestionControlBasic::notify_reader_discovery(
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info)
{
    if (flow_controller_ == nullptr)
    {
        // Not initialized, do nothing
        return;
    }

    switch (reason)
    {
        case ReaderDiscoveryStatus::DISCOVERED_READER:
        case ReaderDiscoveryStatus::CHANGED_QOS_READER:
            add_or_update_reader(info);
            periodic_timer_->restart_timer();
            break;
        default:
            remove_reader(info);
            break;
    }
}

void CongestionControlBasic::on_writer_resend_data(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        const SequenceNumber_t& sequence_number,
        uint32_t resent_bytes,
        const LocatorSelectorEntry& locators)
{
    static_cast<void>(locators);

    if (flow_controller_ == nullptr)
    {
        // Not initialized, do nothing
        return;
    }

    std::lock_guard<std::mutex> lock(readers_mutex_);
    auto it = readers_.find(reader_guid);
    if (it == readers_.end())
    {
        // Reader not found, do nothing
        return;
    }

    // Don't account resent bytes for sequence numbers the first time they are resent
    auto& last_seq_num = it->second.writer_last_sequence_number[writer_guid];
    if (sequence_number <= last_seq_num)
    {
        it->second.total_resent_bytes += resent_bytes;
    }
    else
    {
        last_seq_num = sequence_number;
    }
}

void CongestionControlBasic::on_writer_data_acknowledged(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        const SequenceNumber_t& sequence_number,
        uint32_t payload_length,
        const std::chrono::steady_clock::duration& ack_duration,
        const LocatorSelectorEntry& locators)
{
    static_cast<void>(ack_duration);
    static_cast<void>(locators);

    if (flow_controller_ == nullptr)
    {
        // Not initialized, do nothing
        return;
    }

    std::lock_guard<std::mutex> lock(readers_mutex_);
    auto it = readers_.find(reader_guid);
    if (it == readers_.end())
    {
        // Reader not found, do nothing
        return;
    }
    it->second.total_acked_bytes += payload_length;
    auto& last_seq_num = it->second.writer_last_sequence_number[writer_guid];
    if (sequence_number > last_seq_num)
    {
        last_seq_num = sequence_number;
    }
}

void CongestionControlBasic::add_or_update_reader(
        const SubscriptionBuiltinTopicData& info)
{
    std::lock_guard<std::mutex> lock(readers_mutex_);
    if (readers_.find(info.guid) != readers_.end())
    {
        // Reader already exists, no need to add
        return;
    }

    ReaderInfo reader_info;
    reader_info.current_limitation = parameters_.initial_target_bytes_per_second;
    reader_info.last_update_time = std::chrono::steady_clock::now();
    readers_.emplace(info.guid, reader_info);

    // Register the reader in the flow controller
    flow_controller_.load()->register_remote_reader(info.guid, reader_info.current_limitation);
}

void CongestionControlBasic::remove_reader(
        const SubscriptionBuiltinTopicData& info)
{
    std::lock_guard<std::mutex> lock(readers_mutex_);
    if (readers_.erase(info.guid) > 0)
    {
        // Unregister the reader from the flow controller
        flow_controller_.load()->unregister_remote_reader(info.guid);
    }
}

void CongestionControlBasic::flow_controller_limitations_update(
        bool& reschedule_timer)
{
    std::lock_guard<std::mutex> lock(readers_mutex_);

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    for (auto& reader : readers_)
    {
        // Calculate the new limitation based on acked and resent bytes
        ReaderInfo& reader_info = reader.second;
        bool increase = false;
        if (should_update_limitation(reader_info, now, increase))
        {
            if (increase)
            {
                float factor = parameters_.increase_factor;
                uint32_t new_limitation = static_cast<uint32_t>(reader_info.current_limitation * factor);
                if (new_limitation > reader_info.current_limitation)
                {
                    reader_info.current_limitation = new_limitation;
                }
            }
            else
            {
                float factor = parameters_.decrease_factor;
                uint32_t new_limitation = static_cast<uint32_t>(reader_info.current_limitation * factor);
                if (new_limitation < minimum_bandwidth_limitation_)
                {
                    new_limitation = minimum_bandwidth_limitation_;
                }
                reader_info.current_limitation = new_limitation;
            }

            // Update the flow controller with the new limitation
            GrainedFlowController* fc = flow_controller_.load();
            fc->update_remote_reader_bytes_per_period(reader.first, reader_info.current_limitation);
        }

        reader_info.total_acked_bytes = 0;
        reader_info.total_resent_bytes = 0;
        reader_info.last_update_time = now;
    }

    reschedule_timer = !readers_.empty();
}

bool CongestionControlBasic::should_update_limitation(
        const ReaderInfo& reader_info,
        const std::chrono::steady_clock::time_point& now,
        bool& increase) const
{
    if (reader_info.total_resent_bytes > 0)
    {
        // Always decrease if there were resent bytes
        increase = false;
        return true;
    }

    auto total_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - reader_info.last_update_time).count();
    if (total_seconds == 0)
    {
        return false;
    }

    // Increase if acked bytes exceed 80% of current limitation
    uint32_t acked_bps = static_cast<uint32_t>(reader_info.total_acked_bytes / total_seconds);
    if (acked_bps >= reader_info.current_limitation * 0.8f)
    {
        increase = true;
        return true;
    }

    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
