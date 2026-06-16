// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03

#ifndef FASTDDS_UTILS_BUFFER__BUFFERBACKEND_HPP_
#define FASTDDS_UTILS_BUFFER__BUFFERBACKEND_HPP_

#include <cstdint>
#include <memory>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/fastdds_dll.hpp>

#include <fastdds/utils/buffer/buffer_impl_base.hpp>

namespace eprosima {
namespace fastdds {

/**
 * @brief An interface for backends that can handle serialization and deserialization of BufferImplBase<uint8_t> instances.
 */
struct FASTDDS_EXPORTED_API BufferBackend
{
    virtual ~BufferBackend() = default;

    /**
     * @brief Calculates the serialized size of a BufferImplBase<uint8_t> instance.
     *
     * @param calculator         The CdrSizeCalculator to use for size calculation.
     * @param data               The BufferImplBase<uint8_t> instance whose serialized size is to be calculated.
     * @param current_alignment  The current alignment for serialization.
     *
     * @return The serialized size of the Buffer<uint8_t> instance.
     */
    virtual size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            const BufferImplBase<uint8_t>& data,
            size_t& current_alignment) const = 0;

    /**
     * @brief Serializes a BufferImplBase<uint8_t> instance into a Cdr stream.
     *
     * @param cdr   The Cdr stream to serialize into.
     * @param data  The BufferImplBase<uint8_t> instance to serialize.
     */
    virtual void serialize(
            eprosima::fastcdr::Cdr& cdr,
            const BufferImplBase<uint8_t>& data) const = 0;

    /**
     * @brief Deserializes a BufferImplBase<uint8_t> instance from a Cdr stream.
     *
     * @param cdr   The Cdr stream to deserialize from.
     *
     * @return A unique pointer to a new BufferImplBase<uint8_t> instance deserialized from the Cdr stream.
     */
    virtual std::unique_ptr<BufferImplBase<uint8_t>> deserialize(
            eprosima::fastcdr::Cdr& cdr) const = 0;

};

}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_UTILS_BUFFER__BUFFERBACKEND_HPP_
