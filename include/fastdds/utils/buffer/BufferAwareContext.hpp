// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03

#ifndef FASTDDS_UTILS_BUFFER__BUFFERAWARECONTEXT_HPP_
#define FASTDDS_UTILS_BUFFER__BUFFERAWARECONTEXT_HPP_

#include <cstdint>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrContext.hpp>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/fastdds_dll.hpp>

#include <fastdds/utils/buffer/buffer.hpp>

namespace eprosima {
namespace fastdds {

/**
 * @brief A CdrContext that is aware of eprosima::fastdds::Buffer and can serialize/deserialize it.
 */
struct FASTDDS_EXPORTED_API BufferAwareContext : public eprosima::fastcdr::CdrContext
{
    ~BufferAwareContext() override = default;

    /**
     * @brief Calculates the serialized size of a Buffer<uint8_t> instance.
     *
     * @param calculator The CdrSizeCalculator to use for size calculation.
     * @param data The Buffer<uint8_t> instance whose serialized size is to be calculated.
     * @param current_alignment The current alignment for serialization.
     *
     * @return The serialized size of the Buffer<uint8_t> instance.
     */
    size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            const eprosima::fastdds::Buffer<uint8_t>& data,
            size_t& current_alignment) const;

    /**
     * @brief Serializes a Buffer<uint8_t> instance into a Cdr stream.
     *
     * @param cdr The Cdr stream to serialize into.
     * @param data The Buffer<uint8_t> instance to serialize.
     */
    void serialize(
            eprosima::fastcdr::Cdr& cdr,
            const eprosima::fastdds::Buffer<uint8_t>& data) const;

    /**
     * @brief Deserializes a Buffer<uint8_t> instance from a Cdr stream.
     *
     * @param cdr The Cdr stream to deserialize from.
     * @param data The Buffer<uint8_t> instance to deserialize into.
     */
    void deserialize(
            eprosima::fastcdr::Cdr& cdr,
            eprosima::fastdds::Buffer<uint8_t>& data) const;

};

}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_UTILS_BUFFER__BUFFERAWARECONTEXT_HPP_
