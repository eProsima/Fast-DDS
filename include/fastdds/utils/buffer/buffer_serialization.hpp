// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03

#ifndef FASTDDS_UTILS_BUFFER__BUFFER_SERIALIZATION_HPP_
#define FASTDDS_UTILS_BUFFER__BUFFER_SERIALIZATION_HPP_

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrEncoding.hpp>
#include <fastcdr/CdrSizeCalculator.hpp>
#include <fastcdr/exceptions/BadParamException.h>

#include <fastdds/utils/buffer/buffer.hpp>

namespace eprosima {
namespace fastcdr {

template<>
inline size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::Buffer<uint8_t>& data,
        size_t& current_alignment)
{
    if (data.get_backend_type() == "cpu")
    {
        // For CPU buffers, we can directly calculate size from the underlying vector
        const std::vector<uint8_t>& vec = static_cast<const std::vector<uint8_t>&>(data);
        return calculator.calculate_serialized_size(vec, current_alignment);
    }
    else
    {
        if (CdrVersion::XCDRv1 == calculator.get_cdr_version())
        {
            // For non-CPU buffers and XCDRv1, we can calculate size directly from the buffer size
            size_t initial_alignment{ current_alignment };

            // Align for the length field
            current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));

            // Add size for the length field
            current_alignment += sizeof(uint32_t);

            // Add size for the data
            current_alignment += data.size();

            return current_alignment - initial_alignment;
        }
        else
        {
            // For non-CPU buffers and non-XCDRv1, we need to copy to CPU memory first
            std::vector<uint8_t> vec = data.to_vector();
            return calculator.calculate_serialized_size(vec, current_alignment);
        }
    }
}

template<>
inline void serialize(
        eprosima::fastcdr::Cdr& cdr,
        const eprosima::fastdds::Buffer<uint8_t>& data)
{
    if (data.get_backend_type() == "cpu")
    {
        // For CPU buffers, we can directly serialize from the underlying vector
        const std::vector<uint8_t>& vec = static_cast<const std::vector<uint8_t>&>(data);
        cdr << vec;
    }
    else
    {
        // For non-CPU buffers, we need to copy to CPU memory first
        std::vector<uint8_t> vec = data.to_vector();
        cdr << vec;
    }
}

template<>
inline void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        eprosima::fastdds::Buffer<uint8_t>& data)
{
    if (data.get_backend_type() == "cpu")
    {
        // For CPU buffers, we can directly deserialize into the underlying vector
        std::vector<uint8_t>& vec = static_cast<std::vector<uint8_t>&>(data);
        cdr >> vec;
    }
    else
    {
        // For non-CPU buffers, we need to deserialize into a temporary vector and then move to the buffer
        std::vector<uint8_t> vec;
        cdr >> vec;
        data = std::move(vec);  // Move the deserialized vector into the buffer
    }
}

}  // namespace fastcdr
}  // namespace eprosima

#endif  // FASTDDS_UTILS_BUFFER__BUFFER_SERIALIZATION_HPP_
