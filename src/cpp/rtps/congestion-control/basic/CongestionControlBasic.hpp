/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLBASIC_HPP_
#define RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLBASIC_HPP_

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>

#include <rtps/congestion-control/CongestionControlParameters.hpp>
#include <rtps/congestion-control/ICongestionControl.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/writer/StatefulWriterListener.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class FlowControllerFactory;
class GrainedFlowController;
class RTPSParticipantImpl;

/**
 * @brief Interface class for Congestion Control implementations.
 *
 * They must implement StatefulWriterListener to receive writer events, and additionally
 * provide a method to receive reader discovery notifications.
 */
class CongestionControlBasic : public ICongestionControl
{

public:

    ~CongestionControlBasic() override;

    /**
     * @brief Method called to initialize the congestion control with the participant and parameters.
     *
     * @param participant              Reference to the RTPSParticipantImplPro instance.
     * @param flow_controller_factory  Reference to the flow controller factory.
     * @param params                   Congestion control parameters.
     *
     * @return True if initialization was successful, false otherwise.
     */
    bool initialize(
            RTPSParticipantImpl& participant,
            FlowControllerFactory& flow_controller_factory,
            const CongestionControlParameters& params) override;

    /**
     * @brief Method called to prepare writer attributes before creating a writer under congestion control.
     *
     * @param original_attributes  The original writer attributes.
     * @param entity_id            The entity ID of the writer being created.
     * @param is_builtin           Whether the writer is a built-in endpoint.
     *
     * @return Modified writer attributes suitable for congestion control.
     */
    WriterAttributes prepare_writer_attributes(
            const WriterAttributes& original_attributes,
            const EntityId_t& entity_id,
            bool is_builtin) override;

    /**
     * @brief Method called when a reader is discovered, updated or removed.
     *
     * @param reason  The reason for the notification.
     * @param info    Information about the discovered reader.
     */
    void notify_reader_discovery(
            ReaderDiscoveryStatus reason,
            const SubscriptionBuiltinTopicData& info) override;

    /**
     * @brief Method called when a writer resends data to a reader.
     *
     * @param writer_guid      GUID of the writer resending the data.
     * @param reader_guid      GUID of the reader receiving the resent data.
     * @param sequence_number  Sequence number of the data being resent.
     * @param resent_bytes     Number of bytes being resent.
     * @param locators         LocatorSelectorEntry containing the locators where the reader can be reached.
     */
    void on_writer_resend_data(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            const SequenceNumber_t& sequence_number,
            uint32_t resent_bytes,
            const LocatorSelectorEntry& locators) override;

    /**
     * @brief Method called when a writer receives an acknowledgment for data sent to a reader.
     *
     * @param writer_guid      GUID of the writer whose data has been acknowledged.
     * @param reader_guid      GUID of the reader that acknowledged the data.
     * @param sequence_number  Sequence number of the data that has been acknowledged.
     * @param payload_length   Length of the payload that has been acknowledged.
     * @param ack_duration     Duration taken by the reader to acknowledge the data.
     * @param locators         LocatorSelectorEntry containing the locators where the reader can be reached.
     */
    void on_writer_data_acknowledged(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            const SequenceNumber_t& sequence_number,
            uint32_t payload_length,
            const std::chrono::steady_clock::duration& ack_duration,
            const LocatorSelectorEntry& locators) override;

private:

    CongestionControlParameters parameters_ {};
    uint32_t minimum_bandwidth_limitation_ {0};
    std::unique_ptr<TimedEvent> periodic_timer_ {nullptr};
    std::atomic<GrainedFlowController*> flow_controller_ {nullptr};

    struct ReaderInfo
    {
        uint32_t current_limitation {0};
        std::map<GUID_t, SequenceNumber_t> writer_last_sequence_number {};
        uint64_t total_acked_bytes  {0};
        uint64_t total_resent_bytes {0};
        std::chrono::steady_clock::time_point last_update_time {};
    };

    std::mutex readers_mutex_;
    std::map<GUID_t, ReaderInfo> readers_;

    /*!
     * @brief Add or update reader information based on discovery data.
     *
     * This method is called when a reader is discovered or updated, and it updates the internal
     * state of the congestion control accordingly.
     *
     * @param info Information about the discovered or updated reader.
     */
    void add_or_update_reader(
            const SubscriptionBuiltinTopicData& info);

    /*!
     * @brief Remove reader information based on discovery data.
     *
     * This method is called when a reader is removed, and it updates the internal
     * state of the congestion control accordingly.
     *
     * @param info Information about the removed reader.
     */
    void remove_reader(
            const SubscriptionBuiltinTopicData& info);

    /**
     * @brief Update flow controller limitations based on current conditions.
     *
     * This is called periodically to adjust the flow controller's limitations.
     *
     * @param reschedule_timer  Output parameter indicating whether to reschedule the timer.
     */
    void flow_controller_limitations_update(
            bool& reschedule_timer);

    /**
     * @brief Determine if the limitation for a reader should be updated.
     *
     * @param reader_info  Information about the reader.
     * @param now          Current time point.
     * @param increase     Output parameter indicating if the limitation should be increased.
     */
    bool should_update_limitation(
            const ReaderInfo& reader_info,
            const std::chrono::steady_clock::time_point& now,
            bool& increase) const;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLBASIC_HPP_
