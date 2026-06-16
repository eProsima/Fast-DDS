// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03


#include <fastdds/utils/buffer/BufferAwareContext.hpp>

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrEncoding.hpp>
#include <fastcdr/CdrSizeCalculator.hpp>
#include <fastcdr/exceptions/BadParamException.h>

#include <fastdds/utils/buffer/buffer.hpp>
#include <fastdds/utils/buffer/BufferBackend.hpp>
#include <fastdds/utils/buffer/buffer_impl_base.hpp>

namespace eprosima {
namespace fastdds {

constexpr uint32_t kBufferDescriptorMarker1 = 0xFFFFFFFFu;
constexpr uint32_t kBufferDescriptorMarker2 = 0x524F5332u;  // "ROS2" in ASCII


void BufferAwareContext::register_backend(
        const std::string& backend_type,
        const std::shared_ptr<BufferBackend>& backend)
{
    std::lock_guard<std::mutex> lock(mutex_);
    backends_[backend_type] = backend;
}

std::shared_ptr<BufferBackend> BufferAwareContext::get_backend(
        const std::string& backend_type) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = backends_.find(backend_type);
    if (it != backends_.end())
    {
        return it->second;
    }
    return nullptr;
}

size_t BufferAwareContext::calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::Buffer<uint8_t>& data,
        size_t& current_alignment) const
{
    if (data.get_backend_type() == "cpu")
    {
        // For CPU buffers, we can directly calculate size from the underlying vector
        const std::vector<uint8_t>& vec = static_cast<const std::vector<uint8_t>&>(data);
        return calculator.calculate_serialized_size(vec, current_alignment);
    }

    // TODO: Handle non-CPU backends. For now, we will convert to CPU memory and calculate size that way.
    if (fastcdr::CdrVersion::XCDRv1 == calculator.get_cdr_version())
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

    // For non-CPU buffers and non-XCDRv1, we need to copy to CPU memory first
    std::vector<uint8_t> vec = data.to_vector();
    return calculator.calculate_serialized_size(vec, current_alignment);
}

void BufferAwareContext::serialize(
        eprosima::fastcdr::Cdr& cdr,
        const eprosima::fastdds::Buffer<uint8_t>& data) const
{
    const std::string backend_type = data.get_backend_type();
    if (backend_type == "cpu")
    {
        // For CPU buffers, we can directly serialize from the underlying vector
        const std::vector<uint8_t>& vec = static_cast<const std::vector<uint8_t>&>(data);
        cdr << vec;
        return;
    }

    // TODO: Handle non-CPU backends. For now, we will convert to CPU memory and serialize that.
    std::vector<uint8_t> vec = data.to_vector();
    cdr << vec;
}

void BufferAwareContext::deserialize(
        eprosima::fastcdr::Cdr& cdr,
        eprosima::fastdds::Buffer<uint8_t>& data) const
{
    auto state = cdr.get_state();

    uint32_t length = 0;
    bool is_custom_serialized = false;
    cdr >> length;
    if (kBufferDescriptorMarker1 == length)
    {
        uint32_t tag = 0;
        cdr >> tag;
        is_custom_serialized = (kBufferDescriptorMarker2 == tag);
    }

    if (!is_custom_serialized)
    {
        // This is a CPU buffer, so we push the state back and deserialize the standard way directly into the underlying vector
        cdr.set_state(state);
        std::vector<uint8_t>& vec = static_cast<std::vector<uint8_t>&>(data);
        cdr >> vec;
        return;
    }

    std::string backend_type;
    cdr >> backend_type;

    // TODO: Handle non-CPU backends. For now, we will throw an exception.
    throw eprosima::fastcdr::exception::BadParamException(
              "Deserialization of non-CPU buffers is not implemented yet.");
}

}  // namespace fastdds
}  // namespace eprosima
