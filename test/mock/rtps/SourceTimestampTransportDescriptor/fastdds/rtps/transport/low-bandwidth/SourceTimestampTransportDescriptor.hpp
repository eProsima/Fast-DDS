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

#include <fastdds/rtps/transport/ChainingTransportDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct SourceTimestampTransportDescriptor : public ChainingTransportDescriptor
{
    FASTDDS_EXPORTED_API SourceTimestampTransportDescriptor(
            std::shared_ptr<TransportDescriptorInterface> low_level)
        : ChainingTransportDescriptor(low_level)
        , callback(nullptr)
        , callback_parameter(nullptr)
    {
    }

    FASTDDS_EXPORTED_API SourceTimestampTransportDescriptor(
            const SourceTimestampTransportDescriptor& t)
        : ChainingTransportDescriptor(t)
        , callback(t.callback)
        , callback_parameter(t.callback_parameter)
    {
    }

    uint32_t max_message_size() const override
    {
        return low_level_descriptor->max_message_size() + 5;
    }

    TransportInterface* create_transport() const override
    {
        return nullptr;
    }

    ~SourceTimestampTransportDescriptor() override = default;

    void (* callback)(
            void* parameter,
            int32_t source_sec,
            int32_t recep_sec,
            uint32_t recv_bytes);

    void* callback_parameter;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_LOWBANDWIDTH_SOURCETIMESTAMPTRANSPORTDESCRIPTOR_HPP
