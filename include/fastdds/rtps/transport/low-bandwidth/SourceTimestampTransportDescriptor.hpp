/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_RTPS_TRANSPORT_LOWBANDWIDTH_SOURCETIMESTAMPTRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT_LOWBANDWIDTH_SOURCETIMESTAMPTRANSPORTDESCRIPTOR_HPP

#include <memory>
#include <vector>

#include <fastdds/rtps/transport/ChainingTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/config.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Transport configuration
 * - low_level_descriptor: Descriptor for lower level transport.
 * - callback: Function called each time a packet is received.
 * - callback_parameter: Parameter to pass as first argument to callback.
 * @ingroup TRANSPORT_MODULE
 */
struct SourceTimestampTransportDescriptor : public ChainingTransportDescriptor
{
    FASTDDS_EXPORTED_API SourceTimestampTransportDescriptor(
            std::shared_ptr<TransportDescriptorInterface> low_level);

    FASTDDS_EXPORTED_API SourceTimestampTransportDescriptor(
            const SourceTimestampTransportDescriptor& t);

    //! Returns the maximum size expected for received messages.
    uint32_t max_message_size() const override
    {
        return low_level_descriptor->max_message_size() + 5;
    }

    /**
     * Factory method pattern.
     * @return A new TransportInterface corresponding to this descriptor.
     */
    TransportInterface* create_transport() const override;

    ~SourceTimestampTransportDescriptor() override = default;

    /**
     * Function called upon reception of data
     * @param parameter User callback parameter
     * @param source_sec Source timestamp seconds on the remote endpoint when sending the packet
     * @param recep_sec Source timestamp seconds on the local endpoint when receiving the packet
     * @param recv_bytes Number of bytes of the received packet (including the 5 bytes of overhead)
     */
    void (* callback)(
            void* parameter,
            int32_t source_sec,
            int32_t recep_sec,
            uint32_t recv_bytes);

    //! Parameter used as first argument for callback
    void* callback_parameter;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_LOWBANDWIDTH_SOURCETIMESTAMPTRANSPORTDESCRIPTOR_HPP
