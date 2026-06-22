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

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/utils/buffer/buffer.hpp>

namespace eprosima {
namespace fastcdr {

template<>
size_t FASTDDS_EXPORTED_API calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::Buffer<uint8_t>& data,
        size_t& current_alignment);

template<>
void FASTDDS_EXPORTED_API serialize(
        eprosima::fastcdr::Cdr& cdr,
        const eprosima::fastdds::Buffer<uint8_t>& data);

template<>
void FASTDDS_EXPORTED_API deserialize(
        eprosima::fastcdr::Cdr& cdr,
        eprosima::fastdds::Buffer<uint8_t>& data);

}  // namespace fastcdr
}  // namespace eprosima

#endif  // FASTDDS_UTILS_BUFFER__BUFFER_SERIALIZATION_HPP_
