/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_RTPS_TRANSPORT_LOWBANDWIDTH_PAYLOADCOMPRESSIONTRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT_LOWBANDWIDTH_PAYLOADCOMPRESSIONTRANSPORTDESCRIPTOR_HPP

#include <memory>
#include <vector>

#include <fastdds/rtps/transport/ChainingTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/config.hpp>

#if HAVE_ZLIB || HAVE_BZIP2

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Transport configuration
 * - low_level_descriptor: Descriptor for lower level transport.
 * @ingroup TRANSPORT_MODULE
 */
struct PayloadCompressionTransportDescriptor : public ChainingTransportDescriptor
{
    FASTDDS_EXPORTED_API PayloadCompressionTransportDescriptor(
            std::shared_ptr<TransportDescriptorInterface> low_level);

    FASTDDS_EXPORTED_API PayloadCompressionTransportDescriptor(
            const PayloadCompressionTransportDescriptor& t);

    /**
     * Factory method pattern.
     * @return A new TransportInterface corresponding to this descriptor.
     */
    TransportInterface* create_transport() const override;

    ~PayloadCompressionTransportDescriptor() override = default;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // if HAVE_ZLIB || HAVE_BZIP2

#endif // FASTDDS_RTPS_TRANSPORT_LOWBANDWIDTH_PAYLOADCOMPRESSIONTRANSPORTDESCRIPTOR_HPP
