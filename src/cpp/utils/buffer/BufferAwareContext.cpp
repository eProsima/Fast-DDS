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
    const std::string backend_type = data.get_backend_type();

    // For CPU buffers, we can directly calculate size from the underlying vector
    if (backend_type == "cpu")
    {
        const std::vector<uint8_t>& vec = static_cast<const std::vector<uint8_t>&>(data);
        return calculator.calculate_serialized_size(vec, current_alignment);
    }

    // For non-CPU buffers, we check if a backend is registered for the buffer's type
    std::shared_ptr<BufferBackend> backend;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = backends_.find(backend_type);
        if (it != backends_.end())
        {
            backend = it->second;
        }
    }

    // If we have a registered backend, use it to calculate the serialized size
    if (backend)
    {
        size_t initial_alignment{ current_alignment };

        // Align for the marker fields
        current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));

        // Add size for the two markers
        current_alignment += sizeof(uint32_t) * 2;

        // Add size for the backend type string
        current_alignment += calculator.calculate_serialized_size(backend_type, current_alignment);

        // Add size for the buffer data using the backend's method
        const BufferImplBase<uint8_t>* impl = data.get_impl();
        current_alignment += backend->calculate_serialized_size(calculator, *impl, current_alignment);

        // Return the total size added
        return current_alignment - initial_alignment;
    }

    // If not, fall back to the default behavior.
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

    // For CPU buffers, we can directly serialize from the underlying vector
    if (backend_type == "cpu")
    {
        const std::vector<uint8_t>& vec = static_cast<const std::vector<uint8_t>&>(data);
        cdr << vec;
        return;
    }

    // For non-CPU buffers, we check if a backend is registered for the buffer's type
    std::shared_ptr<BufferBackend> backend;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = backends_.find(backend_type);
        if (it != backends_.end())
        {
            backend = it->second;
        }
    }

    // If we have a registered backend, use it to serialize the buffer
    if (backend)
    {
        // Write the markers to indicate that this is a custom serialized buffer
        cdr << kBufferDescriptorMarker1;
        cdr << kBufferDescriptorMarker2;
        // Write the backend type as a string
        cdr << backend_type;
        // Serialize the buffer using the backend's serialization method
        const BufferImplBase<uint8_t>* impl = data.get_impl();
        backend->serialize(cdr, *impl);
        return;
    }

    // If not, convert to CPU memory and serialize that.
    std::vector<uint8_t> vec = data.to_vector();
    cdr << vec;
}

void BufferAwareContext::deserialize(
        eprosima::fastcdr::Cdr& cdr,
        eprosima::fastdds::Buffer<uint8_t>& data) const
{
    auto state = cdr.get_state();

    // Check for the custom serialization markers
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

    // This is a custom serialized buffer, so we read the backend type and use the appropriate backend to deserialize
    std::string backend_type;
    cdr >> backend_type;

    // Look up the backend for the given type
    std::shared_ptr<BufferBackend> backend;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = backends_.find(backend_type);
        if (it == backends_.end())
        {
            // If no backend is registered for this type, throw an exception
            throw eprosima::fastcdr::exception::BadParamException(
                      "Could not find backend for received type");
        }
        backend = it->second;
    }

    // Create a new buffer implementation using the backend's deserialization method
    std::unique_ptr<BufferImplBase<uint8_t>> impl = backend->deserialize(cdr);
    // Set the buffer's implementation to the deserialized backend-specific implementation
    data = eprosima::fastdds::Buffer<uint8_t>(std::move(impl));
}

}  // namespace fastdds
}  // namespace eprosima
