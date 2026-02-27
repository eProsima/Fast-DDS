/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef RTPS_CONGESTION_CONTROL__ICONGESTIONCONTROL_HPP_
#define RTPS_CONGESTION_CONTROL__ICONGESTIONCONTROL_HPP_

#include <cstdint>

#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>

#include <rtps/congestion-control/CongestionControlParameters.hpp>
#include <rtps/writer/StatefulWriterListener.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class FlowControllerFactory;
class RTPSParticipantImpl;

/**
 * @brief Interface class for Congestion Control implementations.
 *
 * They must implement StatefulWriterListener to receive writer events, and additionally
 * provide a method to receive reader discovery notifications.
 */
class ICongestionControl : public StatefulWriterListener
{
protected:

    // Protected default constructor to prevent direct instantiation
    ICongestionControl() = default;

public:

    // Virtual destructor to ensure proper cleanup of derived classes
    virtual ~ICongestionControl() = default;

    /**
     * @brief Method called to initialize the congestion control with the participant and parameters.
     *
     * @param participant              Reference to the RTPSParticipantImplPro instance.
     * @param flow_controller_factory  Reference to the flow controller factory.
     * @param params                   Congestion control parameters.
     *
     * @return True if initialization was successful, false otherwise.
     */
    virtual bool initialize(
            RTPSParticipantImpl& participant,
            FlowControllerFactory& flow_controller_factory,
            const CongestionControlParameters& params) = 0;

    /**
     * @brief Method called to prepare writer attributes before creating a writer under congestion control.
     *
     * @param original_attributes  The original writer attributes.
     * @param entity_id            The entity ID of the writer being created.
     * @param is_builtin           Whether the writer is a built-in endpoint.
     *
     * @return Modified writer attributes suitable for congestion control.
     */
    virtual WriterAttributes prepare_writer_attributes(
            const WriterAttributes& original_attributes,
            const EntityId_t& entity_id,
            bool is_builtin) = 0;

    /**
     * @brief Method called when a reader is discovered, updated or removed.
     *
     * @param reason  The reason for the notification.
     * @param info    Information about the discovered reader.
     */
    virtual void notify_reader_discovery(
            ReaderDiscoveryStatus reason,
            const SubscriptionBuiltinTopicData& info) = 0;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_CONGESTION_CONTROL__ICONGESTIONCONTROL_HPP_
